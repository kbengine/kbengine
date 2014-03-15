/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "apr.h"
#include "apr_arch_file_io.h"
#include "apr_strings.h"

/* Win32 Exceptions:
 *
 * Note that trailing spaces and trailing periods are never recorded
 * in the file system, except by a very obscure bug where any file
 * that is created with a trailing space or period, followed by the 
 * ':' stream designator on an NTFS volume can never be accessed again.
 * In other words, don't ever accept them when designating a stream!
 *
 * An interesting side effect is that two or three periods are both 
 * treated as the parent directory, although the fourth and on are
 * not [strongly suggest all trailing periods are trimmed off, or
 * down to two if there are no other characters.]
 *
 * Leading spaces and periods are accepted, however.
 * The * ? < > codes all have wildcard side effects
 * The " / \ : are exclusively component separator tokens 
 * The system doesn't accept | for any (known) purpose 
 * Oddly, \x7f _is_ acceptable ;)
 */

/* apr_c_is_fnchar[] maps Win32's file name and shell escape symbols
 *
 *   element & 1 == valid file name character [excluding delimiters]
 *   element & 2 == character should be shell (caret) escaped from cmd.exe
 *
 * this must be in-sync with Apache httpd's gen_test_char.c for cgi escaping.
 */

const char apr_c_is_fnchar[256] =
{/* Reject all ctrl codes... Escape \n and \r (ascii 10 and 13)      */
    0,0,0,0,0,0,0,0,0,0,2,0,0,2,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 /*   ! " # $ % & ' ( ) * + , - . /  0 1 2 3 4 5 6 7 8 9 : ; < = > ? */
    1,1,2,1,3,3,3,3,3,3,2,1,1,1,1,0, 1,1,1,1,1,1,1,1,1,1,0,3,2,1,2,2,
 /* @ A B C D E F G H I J K L M N O  P Q R S T U V W X Y Z [ \ ] ^ _ */
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,3,2,3,3,1,
 /* ` a b c d e f g h i j k l m n o  p q r s t u v w x y z { | } ~   */
    3,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,3,2,3,3,1,
 /* High bit codes are accepted (subject to utf-8->Unicode xlation)  */
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};


apr_status_t filepath_root_test(char *path, apr_pool_t *p)
{
    apr_status_t rv;
#if APR_HAS_UNICODE_FS
    if (apr_os_level >= APR_WIN_NT)
    {
        apr_wchar_t wpath[APR_PATH_MAX];
        if ((rv = utf8_to_unicode_path(wpath, sizeof(wpath) 
                                            / sizeof(apr_wchar_t), path)))
            return rv;
        rv = GetDriveTypeW(wpath);
    }
    else
#endif
        rv = GetDriveType(path);

    if (rv == DRIVE_UNKNOWN || rv == DRIVE_NO_ROOT_DIR)
        return APR_EBADPATH;
    return APR_SUCCESS;
}


apr_status_t filepath_drive_get(char **rootpath, char drive, 
                                apr_int32_t flags, apr_pool_t *p)
{
    char path[APR_PATH_MAX];
#if APR_HAS_UNICODE_FS
    IF_WIN_OS_IS_UNICODE
    {
        apr_wchar_t *ignored;
        apr_wchar_t wdrive[8];
        apr_wchar_t wpath[APR_PATH_MAX];
        apr_status_t rv;
        /* ???: This needs review, apparently "\\?\d:." returns "\\?\d:" 
         * as if that is useful for anything.
         */
        wcscpy(wdrive, L"D:.");
        wdrive[0] = (apr_wchar_t)(unsigned char)drive;
        if (!GetFullPathNameW(wdrive, sizeof(wpath) / sizeof(apr_wchar_t), wpath, &ignored))
            return apr_get_os_error();
        if ((rv = unicode_to_utf8_path(path, sizeof(path), wpath)))
            return rv;
    }
#endif
#if APR_HAS_ANSI_FS
    ELSE_WIN_OS_IS_ANSI
    {
        char *ignored;
        char drivestr[4];
        drivestr[0] = drive;
        drivestr[1] = ':';
        drivestr[2] = '.';;
        drivestr[3] = '\0';
        if (!GetFullPathName(drivestr, sizeof(path), path, &ignored))
            return apr_get_os_error();
    }
#endif
    if (!(flags & APR_FILEPATH_NATIVE)) {
        for (*rootpath = path; **rootpath; ++*rootpath) {
            if (**rootpath == '\\')
                **rootpath = '/';
        }
    }
    *rootpath = apr_pstrdup(p, path);
    return APR_SUCCESS;
}


apr_status_t filepath_root_case(char **rootpath, char *root, apr_pool_t *p)
{
#if APR_HAS_UNICODE_FS
    IF_WIN_OS_IS_UNICODE
    {
        apr_wchar_t *ignored;
        apr_wchar_t wpath[APR_PATH_MAX];
        apr_status_t rv;
        apr_wchar_t wroot[APR_PATH_MAX];
        /* ???: This needs review, apparently "\\?\d:." returns "\\?\d:" 
         * as if that is useful for anything.
         */
        if ((rv = utf8_to_unicode_path(wroot, sizeof(wroot) 
                                            / sizeof(apr_wchar_t), root)))
            return rv;
        if (!GetFullPathNameW(wroot, sizeof(wpath) / sizeof(apr_wchar_t), wpath, &ignored))
            return apr_get_os_error();

        /* Borrow wroot as a char buffer (twice as big as necessary) 
         */
        if ((rv = unicode_to_utf8_path((char*)wroot, sizeof(wroot), wpath)))
            return rv;
        *rootpath = apr_pstrdup(p, (char*)wroot);
    }
#endif
#if APR_HAS_ANSI_FS
    ELSE_WIN_OS_IS_ANSI
    {
        char path[APR_PATH_MAX];
        char *ignored;
        if (!GetFullPathName(root, sizeof(path), path, &ignored))
            return apr_get_os_error();
        *rootpath = apr_pstrdup(p, path);
    }
#endif
    return APR_SUCCESS;
}


APR_DECLARE(apr_status_t) apr_filepath_get(char **rootpath, apr_int32_t flags,
                                           apr_pool_t *p)
{
    char path[APR_PATH_MAX];
#if APR_HAS_UNICODE_FS
    IF_WIN_OS_IS_UNICODE
    {
        apr_wchar_t wpath[APR_PATH_MAX];
        apr_status_t rv;
        if (!GetCurrentDirectoryW(sizeof(wpath) / sizeof(apr_wchar_t), wpath))
            return apr_get_os_error();
        if ((rv = unicode_to_utf8_path(path, sizeof(path), wpath)))
            return rv;
    }
#endif
#if APR_HAS_ANSI_FS
    ELSE_WIN_OS_IS_ANSI
    {
        if (!GetCurrentDirectory(sizeof(path), path))
            return apr_get_os_error();
    }
#endif
    if (!(flags & APR_FILEPATH_NATIVE)) {
        for (*rootpath = path; **rootpath; ++*rootpath) {
            if (**rootpath == '\\')
                **rootpath = '/';
        }
    }
    *rootpath = apr_pstrdup(p, path);
    return APR_SUCCESS;
}


APR_DECLARE(apr_status_t) apr_filepath_set(const char *rootpath,
                                           apr_pool_t *p)
{
#if APR_HAS_UNICODE_FS
    IF_WIN_OS_IS_UNICODE
    {
        apr_wchar_t wpath[APR_PATH_MAX];
        apr_status_t rv;
        if ((rv = utf8_to_unicode_path(wpath, sizeof(wpath) 
                                            / sizeof(apr_wchar_t), rootpath)))
            return rv;
        if (!SetCurrentDirectoryW(wpath))
            return apr_get_os_error();
    }
#endif
#if APR_HAS_ANSI_FS
    ELSE_WIN_OS_IS_ANSI
    {
        if (!SetCurrentDirectory(rootpath))
            return apr_get_os_error();
    }
#endif
    return APR_SUCCESS;
}
