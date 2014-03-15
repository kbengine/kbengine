/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#if (defined(WIN32) || defined(_WIN32)) && !defined(_WIN32_WCE)

#include <windows.h>
#undef ERROR
#include <log4cxx/nt/nteventlogappender.h>
#include <log4cxx/spi/loggingevent.h>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/level.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/helpers/transcoder.h>
#include <log4cxx/helpers/pool.h>
#include <apr_strings.h>

using namespace log4cxx;
using namespace log4cxx::spi;
using namespace log4cxx::helpers;
using namespace log4cxx::nt;

class CCtUserSIDHelper
{
public:
        static bool FreeSid(SID * pSid)
        {
                return ::HeapFree(GetProcessHeap(), 0, (LPVOID)pSid) != 0;
        }

        static bool CopySid(SID * * ppDstSid, SID * pSrcSid)
        {
                bool bSuccess = false;

                DWORD dwLength = ::GetLengthSid(pSrcSid);
                *ppDstSid = (SID *) ::HeapAlloc(GetProcessHeap(),
                 HEAP_ZERO_MEMORY, dwLength);

                if (::CopySid(dwLength, *ppDstSid, pSrcSid))
                {
                        bSuccess = true;
                }
                else
                {
                        FreeSid(*ppDstSid);
                }

                return bSuccess;
        }

        static bool GetCurrentUserSID(SID * * ppSid)
        {
                bool bSuccess = false;

                // Pseudohandle so don't need to close it
                HANDLE hProcess = ::GetCurrentProcess();
                HANDLE hToken = NULL;
                if (::OpenProcessToken(hProcess, TOKEN_QUERY, &hToken))
                {
                        // Get the required size
                        DWORD tusize = 0;
                        GetTokenInformation(hToken, TokenUser, NULL, 0, &tusize);
                        TOKEN_USER* ptu = (TOKEN_USER*)new BYTE[tusize];

                        if (GetTokenInformation(hToken, TokenUser, (LPVOID)ptu, tusize, &tusize))
                        {
                                bSuccess = CopySid(ppSid, (SID *)ptu->User.Sid);
                        }

                        CloseHandle(hToken);
                        delete [] ptu;
                }

                return bSuccess;
        }
};

IMPLEMENT_LOG4CXX_OBJECT(NTEventLogAppender)

NTEventLogAppender::NTEventLogAppender() : hEventLog(NULL), pCurrentUserSID(NULL)
{
}

NTEventLogAppender::NTEventLogAppender(const LogString& server, const LogString& log, const LogString& source, const LayoutPtr& layout)
: server(server), log(log), source(source), hEventLog(NULL), pCurrentUserSID(NULL)
{
        this->layout = layout;
        Pool pool;
        activateOptions(pool);
}

NTEventLogAppender::~NTEventLogAppender()
{
        finalize();
}


void NTEventLogAppender::close()
{
        if (hEventLog != NULL)
        {
                ::DeregisterEventSource(hEventLog);
                hEventLog = NULL;
        }

        if (pCurrentUserSID != NULL)
        {
                CCtUserSIDHelper::FreeSid((::SID*) pCurrentUserSID);
                pCurrentUserSID = NULL;
        }
}

void NTEventLogAppender::setOption(const LogString& option, const LogString& value)
{
        if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("SERVER"), LOG4CXX_STR("server")))
        {
                server = value;
        }
        else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("LOG"), LOG4CXX_STR("log")))
        {
                log = value;
        }
        else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("SOURCE"), LOG4CXX_STR("source")))
        {
                source = value;
        }
        else
        {
                AppenderSkeleton::setOption(option, value);
        }
}

void NTEventLogAppender::activateOptions(Pool&)
{
        if (source.empty())
        {
                LogLog::warn(
             ((LogString) LOG4CXX_STR("Source option not set for appender ["))
                + name + LOG4CXX_STR("]."));
                return;
        }

        if (log.empty())
        {
                log = LOG4CXX_STR("Application");
        }

        close();

        // current user security identifier
        CCtUserSIDHelper::GetCurrentUserSID((::SID**) &pCurrentUserSID);

        addRegistryInfo();

        LOG4CXX_ENCODE_WCHAR(wsource, source);
        LOG4CXX_ENCODE_WCHAR(wserver, server);
        hEventLog = ::RegisterEventSourceW(
            wserver.empty() ? NULL : wserver.c_str(),
            wsource.c_str());
        if (hEventLog == NULL) {
            LogString msg(LOG4CXX_STR("Cannot register NT EventLog -- server: '"));
            msg.append(server);
            msg.append(LOG4CXX_STR("' source: '"));
            msg.append(source);
            LogLog::error(msg);
            LogLog::error(getErrorString(LOG4CXX_STR("RegisterEventSource")));
        }
}

void NTEventLogAppender::append(const LoggingEventPtr& event, Pool& p)
{
        if (hEventLog == NULL)
        {
                LogLog::warn(LOG4CXX_STR("NT EventLog not opened."));
                return;
        }

        LogString oss;
        layout->format(oss, event, p);
        wchar_t* msgs = Transcoder::wencode(oss, p);
        BOOL bSuccess = ::ReportEventW(
                hEventLog,
                getEventType(event),
                getEventCategory(event),
                0x1000,
                pCurrentUserSID,
                1,
                0,
                (LPCWSTR*) &msgs,
                NULL);

        if (!bSuccess)
        {
                LogLog::error(getErrorString(LOG4CXX_STR("ReportEvent")));
        }
}

/*
 * Add this source with appropriate configuration keys to the registry.
 */
void NTEventLogAppender::addRegistryInfo()
{
        DWORD disposition = 0;
        ::HKEY hkey = 0;
        LogString subkey(LOG4CXX_STR("SYSTEM\\CurrentControlSet\\Services\\EventLog\\"));
        subkey.append(log);
        subkey.append(1, (logchar) 0x5C /* '\\' */);
        subkey.append(source);
        LOG4CXX_ENCODE_WCHAR(wsubkey, subkey);

        long stat = RegCreateKeyExW(HKEY_LOCAL_MACHINE, wsubkey.c_str(), 0, NULL,
                REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL,
                &hkey, &disposition);
        if (stat == ERROR_SUCCESS && disposition == REG_CREATED_NEW_KEY) {
            HMODULE hmodule = GetModuleHandleW(L"log4cxx");
            if (hmodule == NULL) {
                hmodule = GetModuleHandleW(0);
            }
            wchar_t modpath[_MAX_PATH];
            DWORD modlen = GetModuleFileNameW(hmodule, modpath, _MAX_PATH - 1);
            if (modlen > 0) {
                modpath[modlen] = 0;
                RegSetValueExW(hkey, L"EventMessageFile", 0, REG_SZ, 
                        (LPBYTE) modpath, wcslen(modpath) * sizeof(wchar_t));
                RegSetValueExW(hkey, L"CategoryMessageFile", 0, REG_SZ, 
                        (LPBYTE) modpath, wcslen(modpath) * sizeof(wchar_t));
                    DWORD typesSupported = 7;
                    DWORD categoryCount = 6;
                RegSetValueExW(hkey, L"TypesSupported", 0, REG_DWORD, 
                           (LPBYTE)&typesSupported, sizeof(DWORD));
                RegSetValueExW(hkey, L"CategoryCount", 0, REG_DWORD, 
                           (LPBYTE)&categoryCount, sizeof(DWORD));
            }
        }

        RegCloseKey(hkey);
        return;
}

WORD NTEventLogAppender::getEventType(const LoggingEventPtr& event)
{
  int priority = event->getLevel()->toInt();
  WORD type = EVENTLOG_SUCCESS;
  if (priority >= Level::INFO_INT) {
      type = EVENTLOG_INFORMATION_TYPE;
      if (priority >= Level::WARN_INT) {
          type = EVENTLOG_WARNING_TYPE;
          if (priority >= Level::ERROR_INT) {
             type = EVENTLOG_ERROR_TYPE;
          }
      }
  }
  return type;
}

WORD NTEventLogAppender::getEventCategory(const LoggingEventPtr& event)
{
  int priority = event->getLevel()->toInt();
  WORD category = 1;
  if (priority >= Level::DEBUG_INT) {
      category = 2;
      if (priority >= Level::INFO_INT) {
          category = 3;
          if (priority >= Level::WARN_INT) {
             category = 4;
             if (priority >= Level::ERROR_INT) {
                category = 5;
                if (priority >= Level::FATAL_INT) {
                    category = 6;
                }
             }
          }
      }
  }
  return category;
}

LogString NTEventLogAppender::getErrorString(const LogString& function)
{
    Pool p;
    enum { MSGSIZE = 5000 };

    wchar_t* lpMsgBuf = (wchar_t*) p.palloc(MSGSIZE * sizeof(wchar_t));
    DWORD dw = GetLastError();

    FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        lpMsgBuf,
        MSGSIZE, NULL );

    LogString msg(function);
    msg.append(LOG4CXX_STR(" failed with error "));
    StringHelper::toString((size_t) dw, p, msg);
    msg.append(LOG4CXX_STR(": "));
    Transcoder::decode(lpMsgBuf, msg);

    return msg;
}

#endif // WIN32
