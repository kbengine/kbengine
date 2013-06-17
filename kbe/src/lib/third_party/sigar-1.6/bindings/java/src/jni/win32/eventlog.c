/*
 * Copyright (c) 2004-2007, 2009 Hyperic, Inc.
 * Copyright (c) 2009-2010 VMware, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef WIN32
#define UNICODE
#define _UNICODE

#include "javasigar.h"
#include "win32bindings.h"

#define MAX_MSG_LENGTH   8192
#define MAX_ERROR_LENGTH 1024

#define REG_MSGFILE_ROOT L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\"
#define FILESEP L";"
#define STRING_SIG "Ljava/lang/String;"

#define UNICODE_SetStringField(field, str) \
    id = JENV->GetFieldID(env, cls, field, STRING_SIG); \
    value = JENV->NewString(env, (const jchar *)str, wcslen(str)); \
    JENV->SetObjectField(env, obj, id, value)

#define ARRLEN(arr) (sizeof(arr) / sizeof(arr[0]))

static void win32_set_pointer(JNIEnv *env, jobject obj, const void *ptr)
{
    jfieldID pointer_field;
    int pointer_int;
    jclass cls;

    cls = JENV->GetObjectClass(env, obj);
    
    pointer_field = JENV->GetFieldID(env, cls, "eventLogHandle", "I");
    pointer_int = (int)ptr;

    JENV->SetIntField(env, obj, pointer_field, pointer_int);
}

static HANDLE win32_get_pointer(JNIEnv *env, jobject obj)
{
    jfieldID pointer_field;
    HANDLE h;
    jclass cls;

    cls = JENV->GetObjectClass(env, obj);

    pointer_field = JENV->GetFieldID(env, cls, "eventLogHandle", "I");
    h = (HANDLE)JENV->GetIntField(env, obj, pointer_field);

    if (!h) {
        win32_throw_exception(env, "Event log not opened");
    }

    return h;
}

static int get_messagefile_dll(LPWSTR app, LPWSTR source, LPWSTR entry, LPWSTR dllfile)
{
    HKEY hk;
    WCHAR buf[MAX_MSG_LENGTH];
    DWORD type, data = sizeof(buf);
    LONG rc;

    wcscpy(buf, REG_MSGFILE_ROOT);
    wcscat(buf, app);
    wcscat(buf, L"\\");
    wcscat(buf, source);

    rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, buf,
                      0, KEY_READ, &hk); 
    if (rc) {
        return rc;
    }

    rc = RegQueryValueEx(hk, entry, NULL, &type,
                         (LPBYTE)buf, &data);
    if (rc) {
        RegCloseKey(hk);
        return rc;
    }

    wcsncpy(dllfile, buf, MAX_MSG_LENGTH);
    dllfile[MAX_MSG_LENGTH-1] = '\0';

    RegCloseKey(hk);

    return ERROR_SUCCESS;
}

static int get_formatted_message(EVENTLOGRECORD *pevlr,
                                 DWORD id,
                                 LPWSTR dllfile,
                                 LPWSTR msg)
{
    LPVOID msgbuf = NULL;
    WCHAR msgdll[MAX_MSG_LENGTH];
    LPWSTR insert_strs[56], ptr;
    int i, max = ARRLEN(insert_strs);
    const DWORD flags =
        FORMAT_MESSAGE_FROM_HMODULE |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_ARGUMENT_ARRAY |
        FORMAT_MESSAGE_MAX_WIDTH_MASK;

    if (!ExpandEnvironmentStrings(dllfile, msgdll, ARRLEN(msgdll))) {
        return GetLastError();
    }

    memset(insert_strs, '\0', sizeof(insert_strs));
    if (pevlr) {
        ptr = (LPWSTR)((LPBYTE)pevlr + pevlr->StringOffset);
        for (i = 0; i < pevlr->NumStrings && i < max; i++) {
            insert_strs[i] = ptr;
            ptr += wcslen(ptr) + 1;
        }
    }

    ptr = wcstok(msgdll, FILESEP);
    while (ptr) {
        HINSTANCE hlib;

        hlib = LoadLibraryEx(ptr, NULL,
                             LOAD_LIBRARY_AS_DATAFILE);
        if (hlib) {
            FormatMessage(flags,
                          hlib,
                          id,
                          MAKELANGID(LANG_NEUTRAL, SUBLANG_ENGLISH_US),
                          (LPWSTR) &msgbuf,
                          sizeof(msgbuf), //min bytes w/ FORMAT_MESSAGE_ALLOCATE_BUFFER
                          (va_list *)insert_strs);
            FreeLibrary(hlib);

            if (msgbuf) {
                break;
            }
        }
        ptr = wcstok(NULL, FILESEP);
    }

    if (msgbuf) {
        wcsncpy(msg, msgbuf, MAX_MSG_LENGTH);
        msg[MAX_MSG_LENGTH-1] = '\0';
        LocalFree(msgbuf);
        return ERROR_SUCCESS;
    }
    else {
        return !ERROR_SUCCESS;
    }
}

static int get_formatted_event_message(EVENTLOGRECORD *pevlr, LPWSTR name, LPWSTR source, LPWSTR msg)
{
    WCHAR dllfile[MAX_MSG_LENGTH];

    if (get_messagefile_dll(name, source, L"EventMessageFile", dllfile) != ERROR_SUCCESS) {
        return !ERROR_SUCCESS;
    }

    return get_formatted_message(pevlr, pevlr->EventID, dllfile, msg);
}

static int get_formatted_event_category(EVENTLOGRECORD *pevlr, LPWSTR name, LPWSTR source, LPWSTR msg)
{
    WCHAR dllfile[MAX_MSG_LENGTH];

    if (get_messagefile_dll(name, source, L"CategoryMessageFile", dllfile) != ERROR_SUCCESS) {
        return !ERROR_SUCCESS;
    }

    return get_formatted_message(NULL, pevlr->EventCategory, dllfile, msg);
}


JNIEXPORT void SIGAR_JNI(win32_EventLog_openlog)
(JNIEnv *env, jobject obj, jstring lpSourceName)
{
    HANDLE h;
    LPWSTR name;

    name = (LPWSTR)JENV->GetStringChars(env, lpSourceName, NULL);

    h = OpenEventLog(NULL, name);
    if (h == NULL) {
        char buf[MAX_ERROR_LENGTH];
        DWORD lastError = GetLastError();

        sprintf(buf, "Unable to open event log: %d", lastError);
        JENV->ReleaseStringChars(env, lpSourceName, name);
        win32_throw_exception(env, buf);
        return;
    }

    JENV->ReleaseStringChars(env, lpSourceName, name);

    /* Save the handle for later use */
    win32_set_pointer(env, obj, h);
}

JNIEXPORT void SIGAR_JNI(win32_EventLog_close)
(JNIEnv *env, jobject obj)
{
    HANDLE h = win32_get_pointer(env, obj);

    CloseEventLog(h);

    win32_set_pointer(env, obj, NULL);
}

JNIEXPORT jint SIGAR_JNI(win32_EventLog_getNumberOfRecords)
(JNIEnv *env, jobject obj)
{
    DWORD records;
    HANDLE h = win32_get_pointer(env, obj);

    if (!GetNumberOfEventLogRecords(h, &records)) {
        win32_throw_last_error(env);
        return 0;
    }

    return records;
}

JNIEXPORT jint SIGAR_JNI(win32_EventLog_getOldestRecord)
(JNIEnv *env, jobject obj)
{
    DWORD oldest;
    HANDLE h = win32_get_pointer(env, obj);

    if (!GetOldestEventLogRecord(h, &oldest)) {
        win32_throw_last_error(env);
        return 0;
    }

    return oldest;
}

JNIEXPORT jobject SIGAR_JNI(win32_EventLog_readlog)
(JNIEnv *env, jobject obj, jstring jname, jint recordOffset)
{
    EVENTLOGRECORD *pevlr;
    BYTE buffer[8192];
    WCHAR msg[MAX_MSG_LENGTH];
    DWORD dwRead, dwNeeded;
    LPWSTR source, machineName;
    HANDLE h;
    BOOL rv;
    jclass cls = WIN32_FIND_CLASS("EventLogRecord");
    jfieldID id;
    jstring value;
    LPWSTR name;
    BOOL has_category = FALSE; /* 1.6.x compat */

    h = win32_get_pointer(env, obj);

    pevlr = (EVENTLOGRECORD *)&buffer;
    rv = ReadEventLog(h,
                      EVENTLOG_SEEK_READ | EVENTLOG_FORWARDS_READ,
                      recordOffset,
                      pevlr,
                      sizeof(buffer),
                      &dwRead,
                      &dwNeeded);
    if (!rv) {
        char buf[MAX_ERROR_LENGTH];
        DWORD lastError = GetLastError();
        
        if (lastError == ERROR_INSUFFICIENT_BUFFER) {
            /* XXX need to handle this */
            sprintf(buf, "Buffer size (%d) too small (%d needed)",
                    sizeof(buffer), dwNeeded);
        }
        else {
            sprintf(buf, "Error reading from the event log: %d", lastError);
        }
        win32_throw_exception(env, buf);
        return NULL;
    }

    obj = JENV->AllocObject(env, cls);
    SIGAR_CHEX;

    id = JENV->GetFieldID(env, cls, "recordNumber", "J");
    JENV->SetLongField(env, obj, id, pevlr->RecordNumber);

    id = JENV->GetFieldID(env, cls, "timeGenerated", "J");
    JENV->SetLongField(env, obj, id, pevlr->TimeGenerated);

    id = JENV->GetFieldID(env, cls, "timeWritten", "J");
    JENV->SetLongField(env, obj, id, pevlr->TimeWritten);

    id = JENV->GetFieldID(env, cls, "eventId", "J");
    JENV->SetLongField(env, obj, id, pevlr->EventID);

    id = JENV->GetFieldID(env, cls, "eventType", "S");
    JENV->SetShortField(env, obj, id, pevlr->EventType);

    if (!JENV->ExceptionCheck(env)) { /* careful not to clear any existing exception */
        id = JENV->GetFieldID(env, cls, "category", "S");
        if (JENV->ExceptionCheck(env)) {
            /* older version of sigar.jar being used with sigar.dll */
            JENV->ExceptionClear(env);
        }
        else {
            has_category = TRUE;
            JENV->SetShortField(env, obj, id, pevlr->EventCategory);
        }
    }

    /* Extract string data from the end of the structure.  Lame. */

    source = (LPWSTR)((LPBYTE)pevlr + sizeof(EVENTLOGRECORD));
    UNICODE_SetStringField("source", source);

    name = (LPWSTR)JENV->GetStringChars(env, jname, NULL);

    /* Get the formatted message */
    if ((pevlr->NumStrings > 0) &&
        (get_formatted_event_message(pevlr, name, source, msg) == ERROR_SUCCESS))
    {
        UNICODE_SetStringField("message", msg);
    }
    else if (pevlr->NumStrings > 0) {
        LPWSTR tmp = (LPWSTR)((LPBYTE)pevlr + pevlr->StringOffset);            
        UNICODE_SetStringField("message", tmp);
    }

    /* Get the formatted category */
    if (has_category &&
        (get_formatted_event_category(pevlr, name, source, msg) == ERROR_SUCCESS))
    {
        UNICODE_SetStringField("categoryString", msg);
    }

    JENV->ReleaseStringChars(env, jname, name);

    /* Increment up to the machine name. */
    machineName = (LPWSTR)((LPBYTE)pevlr + sizeof(EVENTLOGRECORD) +
                           (wcslen(source) + 1) * sizeof(WCHAR));
    UNICODE_SetStringField("computerName", machineName);

    /* Get user id info */
    if (pevlr->UserSidLength > 0) {
        WCHAR name[256];
        WCHAR domain[256];
        DWORD namelen = ARRLEN(name);
        DWORD domainlen = ARRLEN(domain);
        DWORD len;
        SID_NAME_USE snu;
        PSID sid;
        
        sid = (PSID)((LPBYTE)pevlr + pevlr->UserSidOffset);
        if (LookupAccountSid(NULL, sid, name, &namelen, domain,
                             &domainlen, &snu)) {
            UNICODE_SetStringField("user", name);
        }
    }
    
    return obj;
}

JNIEXPORT void SIGAR_JNI(win32_EventLog_waitForChange)
(JNIEnv *env, jobject obj, jint timeout)
{
    HANDLE h, hEvent;
    DWORD millis;

    h = win32_get_pointer(env, obj);

    hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (hEvent == NULL) {
        win32_throw_exception(env, "Unable to create event");
        return;
    }

    if (timeout == -1)
        millis = INFINITE;
    else
        millis = timeout;

    if(!(NotifyChangeEventLog(h, hEvent))) {
        char buf[MAX_ERROR_LENGTH];
        sprintf(buf, "Error registering for event log to change: %d",
                GetLastError());
        win32_throw_exception(env, buf);
        return;
    }

    if (WaitForSingleObject(hEvent, millis) == WAIT_FAILED)
    {
        char buf[MAX_ERROR_LENGTH];
        sprintf(buf, "Error waiting for event log change: %d",
                GetLastError());
        win32_throw_exception(env, buf);
    }

    return;
}
#endif /* WIN32 */
