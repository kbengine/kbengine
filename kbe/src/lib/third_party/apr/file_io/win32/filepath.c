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
#include "apr_private.h"
#include "apr_arch_file_io.h"
#include "apr_strings.h"
#include "apr_lib.h"
#include <string.h>
#include <ctype.h>

#ifdef NETWARE
#include <unistd.h>
#include <fsio.h>
#endif

 /* WinNT accepts several odd forms of a 'root' path.  Under Unicode
 * calls (ApiFunctionW) the //?/C:/foo or //?/UNC/mach/share/foo forms
 * are accepted.  Ansi and Unicode functions both accept the //./C:/foo 
 * form under WinNT/2K.  Since these forms are handled in the utf-8 to 
 * unicode translation phase, we don't want the user confused by them, so 
 * we will accept them but always return the canonical C:/ or //mach/share/
 *
 * OS2 appears immune from the nonsense :)
 */

APR_DECLARE(apr_status_t) apr_filepath_root(const char **rootpath, 
                                            const char **inpath, 
                                            apr_int32_t flags,
                                            apr_pool_t *p)
{
    const char *testpath = *inpath;
    char *newpath;
#ifdef NETWARE
    char seperator[2] = { 0, 0};
    char server[APR_PATH_MAX+1];
    char volume[APR_PATH_MAX+1];
    char file[APR_PATH_MAX+1];
    char *volsep = NULL;
    int elements;

    if (inpath && *inpath)
        volsep = strchr (*inpath, ':');
    else
        return APR_EBADPATH;

    if (strlen(*inpath) > APR_PATH_MAX) {
        return APR_EBADPATH;
    }

    seperator[0] = (flags & APR_FILEPATH_NATIVE) ? '\\' : '/';

    /* Allocate and initialize each of the segment buffers
    */
    server[0] = volume[0] = file[0] = '\0';

    /* If we don't have a volume separator then don't bother deconstructing
        the path since we won't use the deconstructed information anyway.
    */
    if (volsep) {
        /* Split the inpath into its separate parts. */
        deconstruct(testpath, server, volume, NULL, file, NULL, &elements, PATH_UNDEF);
    
        /* If we got a volume part then continue splitting out the root.
            Otherwise we either have an incomplete or relative path
        */
        if (volume && strlen(volume) > 0) {
            newpath = apr_pcalloc(p, strlen(server)+strlen(volume)+5);
            construct(newpath, server, volume, NULL, NULL, NULL, PATH_NETWARE);

            /* NetWare doesn't add the root slash so we need to add it manually.
            */
            strcat(newpath, seperator);
            *rootpath = newpath;

            /* Skip the inpath pointer down to the first non-root character
            */
            newpath = volsep;
            do {
                ++newpath;
            } while (*newpath && ((*newpath == '/') || (*newpath == '\\')));
            *inpath = newpath;

            /* Need to handle APR_FILEPATH_TRUENAME checking here. */

            return APR_SUCCESS;
        }
        else
            return APR_EBADPATH;
    }
    else if ((**inpath == '/') || (**inpath == '\\')) {
        /* if we have a root path without a volume then just split
            in same manner as unix although this path will be
            incomplete.
        */
        *rootpath = apr_pstrdup(p, seperator);
        do {
            ++(*inpath);
        } while ((**inpath == '/') || (**inpath == '\\'));
    }
    else
        return APR_ERELATIVE;

    return APR_EINCOMPLETE;

#else /* ndef(NETWARE) */

    char seperator[2];
    const char *delim1;
    const char *delim2;

    seperator[0] = (flags & APR_FILEPATH_NATIVE) ? '\\' : '/';
    seperator[1] = 0;

    if (testpath[0] == '/' || testpath[0] == '\\') {
        if (testpath[1] == '/' || testpath[1] == '\\') {

#ifdef WIN32 /* //server/share isn't the only // delimited syntax */
            if ((testpath[2] == '?' || testpath[2] == '.')
                    && (testpath[3] == '/' || testpath[3] == '\\')) {
                if (IS_FNCHAR(testpath[4]) && testpath[5] == ':') 
                {
                    apr_status_t rv;
                    testpath += 4;
                    /* given  '//?/C: or //./C: let us try this
                     * all over again from the drive designator
                     */
                    rv = apr_filepath_root(rootpath, &testpath, flags, p);
                    if (!rv || rv == APR_EINCOMPLETE)
                        *inpath = testpath;
                    return rv;
                }
                else if (strncasecmp(testpath + 4, "UNC", 3) == 0
                      && (testpath[7] == '/' || testpath[7] == '\\') 
                      && (testpath[2] == '?')) {
                    /* given  '//?/UNC/machine/share, a little magic 
                     * at the end makes this all work out by using
                     * 'C/machine' as the starting point and replacing
                     * the UNC delimiters with \'s, including the 'C'
                     */
                    testpath += 6;
                }
                else
                    /* This must not be a path to a file, but rather
                     * a volume or device.  Die for now.
                     */
                    return APR_EBADPATH;
            }
#endif /* WIN32 (non - //server/share syntax) */

            /* Evaluate path of '//[machine/[share[/]]]' */
            delim1 = testpath + 2;
            do {
                /* Protect against //X/ where X is illegal */
                if (*delim1 && !IS_FNCHAR(*(delim1++)))
                    return APR_EBADPATH;
            } while (*delim1 && *delim1 != '/' && *delim1 != '\\');

            if (*delim1) {
                apr_status_t rv;
                delim2 = delim1 + 1;
                while (*delim2 && *delim2 != '/' && *delim2 != '\\') {
                    /* Protect against //machine/X/ where X is illegal */
                    if (!IS_FNCHAR(*(delim2++)))
                        return APR_EBADPATH;
                } 

                /* Copy the '//machine/[share[/]]' path, always providing 
                 * an extra byte for the trailing slash.
                 */
                newpath = apr_pstrmemdup(p, testpath, delim2 - testpath + 1);

                if (delim2 == delim1 + 1) {
                    /* We found simply \\machine\, so give up already
                     */
                    *rootpath = newpath;
                    *inpath = delim2;
                    return APR_EINCOMPLETE;
                }

                if (flags & APR_FILEPATH_TRUENAME) {
                    /* Validate the \\Machine\Share\ designation, 
                     * Win32 will argue about slashed in UNC paths, 
                     * so use backslashes till we finish testing,
                     * and add the trailing backslash [required].
                     * apr_pstrmemdup above guarentees us the new 
                     * trailing null character.
                     */
                    newpath[0] = '\\';
                    newpath[1] = '\\';
                    newpath[delim1 - testpath] = '\\';
                    newpath[delim2 - testpath] = '\\';

                    rv = filepath_root_test(newpath, p);
                    if (rv)
                        return rv;
                    rv = filepath_root_case(&newpath, newpath, p);
                    if (rv)
                        return rv;
                    newpath[0] = seperator[0];
                    newpath[1] = seperator[0];
                    newpath[delim1 - testpath] = seperator[0];
                    newpath[delim2 - testpath] = (*delim2 ? seperator[0] : '\0');
                }
                else {                
                    /* Give back the caller's own choice of delimiters
                     */
                    newpath[0] = testpath[0];
                    newpath[1] = testpath[1];
                    newpath[delim1 - testpath] = *delim1;
                    newpath[delim2 - testpath] = *delim2;
                }

                /* If this root included the trailing / or \ designation 
                 * then lop off multiple trailing slashes and give back
                 * appropriate delimiters.
                 */
                if (*delim2) {
                    *inpath = delim2 + 1;
                    while (**inpath == '/' || **inpath == '\\')
                        ++*inpath;
                }
                else {
                    *inpath = delim2;
                }

                *rootpath = newpath;
                return APR_SUCCESS;
            }
            
            /* Have path of '\\[machine]', if the machine is given,
             * append same trailing slash as the leading slash
             */
            delim1 = strchr(testpath, '\0');
            if (delim1 > testpath + 2) {
                newpath = apr_pstrndup(p, testpath, delim1 - testpath + 1);
                if (flags & APR_FILEPATH_TRUENAME)
                    newpath[delim1 - testpath] = seperator[0];
                else
                    newpath[delim1 - testpath] = newpath[0];
                newpath[delim1 - testpath + 1] = '\0';
            }
            else {
                newpath = apr_pstrndup(p, testpath, delim1 - testpath);
            }
            if (flags & APR_FILEPATH_TRUENAME) {
                newpath[0] = seperator[0];
                newpath[1] = seperator[0];
            }
            *rootpath = newpath;
            *inpath = delim1;
            return APR_EINCOMPLETE;
        }

        /* Left with a path of '/', what drive are we asking about? 
         */
        *inpath = testpath + 1;
        newpath = apr_palloc(p, 2);
        if (flags & APR_FILEPATH_TRUENAME)
            newpath[0] = seperator[0];
        else
            newpath[0] = testpath[0];
        newpath[1] = '\0';
        *rootpath = newpath;
        return APR_EINCOMPLETE;
    }

    /* Evaluate path of 'd:[/]' */
    if (IS_FNCHAR(*testpath) && testpath[1] == ':') 
    {
        apr_status_t rv;
        /* Validate that D:\ drive exists, test must be rooted
         * Note that posix/win32 insists a drive letter is upper case,
         * so who are we to argue with a 'feature'.
         * It is a safe fold, since only A-Z is legal, and has no
         * side effects of legal mis-mapped non-us-ascii codes.
         */
        newpath = apr_palloc(p, 4);
        newpath[0] = testpath[0];
        newpath[1] = testpath[1];
        newpath[2] = seperator[0];
        newpath[3] = '\0';
        if (flags & APR_FILEPATH_TRUENAME) {
            newpath[0] = apr_toupper(newpath[0]);
            rv = filepath_root_test(newpath, p);
            if (rv)
                return rv;
        }
        /* Just give back the root the user handed to us.
         */
        if (testpath[2] != '/' && testpath[2] != '\\') {
            newpath[2] = '\0';
            *rootpath = newpath;
            *inpath = testpath + 2;
            return APR_EINCOMPLETE;
        }

        /* strip off remaining slashes that designate the root,
         * give the caller back their original choice of slash
         * unless this is TRUENAME'ed
         */
        *inpath = testpath + 3;
        while (**inpath == '/' || **inpath == '\\')
            ++*inpath;
        if (!(flags & APR_FILEPATH_TRUENAME))
            newpath[2] = testpath[2];
        *rootpath = newpath;
        return APR_SUCCESS;
    }

    /* Nothing interesting */
    return APR_ERELATIVE;

#endif /* ndef(NETWARE) */
}

#if !defined(NETWARE)
static int same_drive(const char *path1, const char *path2)
{
    char drive1 = path1[0];
    char drive2 = path2[0];

    if (!drive1 || !drive2 || path1[1] != ':' || path2[1] != ':')
        return FALSE;

    if (drive1 == drive2)
        return TRUE;

    if (drive1 >= 'a' && drive1 <= 'z')
        drive1 += 'A' - 'a';

    if (drive2 >= 'a' && drive2 <= 'z')
        drive2 += 'A' - 'a';

    return (drive1 == drive2);
}
#endif

APR_DECLARE(apr_status_t) apr_filepath_merge(char **newpath, 
                                             const char *basepath, 
                                             const char *addpath, 
                                             apr_int32_t flags,
                                             apr_pool_t *p)
{
    char path[APR_PATH_MAX]; /* isn't null term */
    const char *baseroot = NULL;
    const char *addroot;
    apr_size_t rootlen; /* the length of the root portion of path, d:/ is 3 */
    apr_size_t baselen; /* the length of basepath (excluding baseroot) */
    apr_size_t keptlen; /* the length of the retained basepath (incl root) */
    apr_size_t pathlen; /* the length of the result path */
    apr_size_t segend;  /* the end of the current segment */
    apr_size_t seglen;  /* the length of the segment (excl trailing chars) */
    apr_status_t basetype = 0; /* from parsing the basepath's baseroot */
    apr_status_t addtype;      /* from parsing the addpath's addroot */
    apr_status_t rv;
#ifndef NETWARE
    int fixunc = 0;  /* flag to complete an incomplete UNC basepath */
#endif
    
    /* Treat null as an empty path, otherwise split addroot from the addpath
     */
    if (!addpath) {
        addpath = addroot = "";
        addtype = APR_ERELATIVE;
    }
    else {
        /* This call _should_ test the path
         */
        addtype = apr_filepath_root(&addroot, &addpath, 
                                    APR_FILEPATH_TRUENAME
                                    | (flags & APR_FILEPATH_NATIVE),
                                    p);
        if (addtype == APR_SUCCESS) {
            addtype = APR_EABSOLUTE;
        }
        else if (addtype == APR_ERELATIVE) {
            addroot = "";
        }
        else if (addtype != APR_EINCOMPLETE) {
            /* apr_filepath_root was incomprehensible so fail already
             */
            return addtype;
        }
    }

    /* If addpath is (even partially) rooted, then basepath is
     * unused.  Ths violates any APR_FILEPATH_SECUREROOTTEST 
     * and APR_FILEPATH_NOTABSOLUTE flags specified.
     */
    if (addtype == APR_EABSOLUTE || addtype == APR_EINCOMPLETE)
    {
        if (flags & APR_FILEPATH_SECUREROOTTEST)
            return APR_EABOVEROOT;
        if (flags & APR_FILEPATH_NOTABSOLUTE)
            return addtype;
    }

    /* Optimized tests before we query the current working path
     */
    if (!basepath) {

        /* If APR_FILEPATH_NOTABOVEROOT wasn't specified,
         * we won't test the root again, it's ignored.
         * Waste no CPU retrieving the working path.
         */
        if (addtype == APR_EABSOLUTE && !(flags & APR_FILEPATH_NOTABOVEROOT)) {
            basepath = baseroot = "";
            basetype = APR_ERELATIVE;
        }

        /* If APR_FILEPATH_NOTABSOLUTE is specified, the caller 
         * requires an absolutely relative result, So do not retrieve 
         * the working path.
         */
        if (addtype == APR_ERELATIVE && (flags & APR_FILEPATH_NOTABSOLUTE)) {
            basepath = baseroot = "";
            basetype = APR_ERELATIVE;
        }
    }

    if (!basepath) 
    {
        /* Start with the current working path.  This is bass akwards,
         * but required since the compiler (at least vc) doesn't like
         * passing the address of a char const* for a char** arg.
         * We must grab the current path of the designated drive 
         * if addroot is given in drive-relative form (e.g. d:foo)
         */
        char *getpath;
#ifndef NETWARE
        if (addtype == APR_EINCOMPLETE && addroot[1] == ':')
            rv = filepath_drive_get(&getpath, addroot[0], flags, p);
        else
#endif
            rv = apr_filepath_get(&getpath, flags, p);
        if (rv != APR_SUCCESS)
            return rv;
        basepath = getpath;
    }

    if (!baseroot) {
        /* This call should _not_ test the path
         */
        basetype = apr_filepath_root(&baseroot, &basepath,
                                     (flags & APR_FILEPATH_NATIVE), p);
        if (basetype == APR_SUCCESS) {
            basetype = APR_EABSOLUTE;
        }
        else if (basetype == APR_ERELATIVE) {
            baseroot = "";
        }
        else if (basetype != APR_EINCOMPLETE) {
            /* apr_filepath_root was incomprehensible so fail already
             */
            return basetype;
        }
    }
    baselen = strlen(basepath);

    /* If APR_FILEPATH_NOTABSOLUTE is specified, the caller 
     * requires an absolutely relative result.  If the given 
     * basepath is not relative then fail.
     */
    if ((flags & APR_FILEPATH_NOTABSOLUTE) && basetype != APR_ERELATIVE)
        return basetype;

    /* The Win32 nightmare on unc street... start combining for
     * many possible root combinations.
     */
    if (addtype == APR_EABSOLUTE)
    {
        /* Ignore the given root path, and start with the addroot
         */
        if ((flags & APR_FILEPATH_NOTABOVEROOT) 
                && strncmp(baseroot, addroot, strlen(baseroot)))
            return APR_EABOVEROOT;
        keptlen = 0;
        rootlen = pathlen = strlen(addroot);
        memcpy(path, addroot, pathlen);
    }
    else if (addtype == APR_EINCOMPLETE)
    {
        /* There are several types of incomplete paths, 
         *     incomplete UNC paths         (//foo/ or //),
         *     drives without rooted paths  (d: as in d:foo), 
         * and simple roots                 (/ as in /foo).
         * Deal with these in significantly different manners...
         */
#ifndef NETWARE
        if ((addroot[0] == '/' || addroot[0] == '\\') &&
            (addroot[1] == '/' || addroot[1] == '\\')) 
        {
            /* Ignore the given root path if the incomplete addpath is UNC,
             * (note that the final result will be incomplete).
             */
            if (flags & APR_FILEPATH_NOTRELATIVE)
                return addtype;
            if ((flags & APR_FILEPATH_NOTABOVEROOT) 
                    && strncmp(baseroot, addroot, strlen(baseroot)))
                return APR_EABOVEROOT;
            fixunc = 1;
            keptlen = 0;
            rootlen = pathlen = strlen(addroot);
            memcpy(path, addroot, pathlen);
        }
        else
#endif            
        if ((addroot[0] == '/' || addroot[0] == '\\') && !addroot[1]) 
        {
            /* Bring together the drive or UNC root from the baseroot
             * if the addpath is a simple root and basepath is rooted,
             * otherwise disregard the basepath entirely.
             */
            if (basetype != APR_EABSOLUTE && (flags & APR_FILEPATH_NOTRELATIVE))
                return basetype;
            if (basetype != APR_ERELATIVE) {
#ifndef NETWARE
                if (basetype == APR_INCOMPLETE 
                        && (baseroot[0] == '/' || baseroot[0] == '\\')
                        && (baseroot[1] == '/' || baseroot[1] == '\\'))
                    fixunc = 1;
#endif
                keptlen = rootlen = pathlen = strlen(baseroot);
                memcpy(path, baseroot, pathlen);
            }
            else {
                if (flags & APR_FILEPATH_NOTABOVEROOT)
                    return APR_EABOVEROOT;
                keptlen = 0;
                rootlen = pathlen = strlen(addroot);
                memcpy(path, addroot, pathlen);
            }
        }
#ifdef NETWARE
        else if (filepath_has_drive(addroot, DRIVE_ONLY, p)) 
        {
            /* If the addroot is a drive (without a volume root)
             * use the basepath _if_ it matches this drive letter!
             * Otherwise we must discard the basepath.
             */
            if (!filepath_compare_drive(addroot, baseroot, p) && 
                filepath_has_drive(baseroot, 0, p)) {
#else
        else if (addroot[0] && addroot[1] == ':' && !addroot[2]) 
        {
            /* If the addroot is a drive (without a volume root)
             * use the basepath _if_ it matches this drive letter!
             * Otherwise we must discard the basepath.
             */
            if (same_drive(addroot, baseroot)) {
#endif
                /* Base the result path on the basepath
                 */
                if (basetype != APR_EABSOLUTE && (flags & APR_FILEPATH_NOTRELATIVE))
                    return basetype;
                rootlen = strlen(baseroot);
                keptlen = pathlen = rootlen + baselen;
                if (keptlen >= sizeof(path))
                    return APR_ENAMETOOLONG;
                memcpy(path, baseroot, rootlen);
                memcpy(path + rootlen, basepath, baselen);
            } 
            else {
                if (flags & APR_FILEPATH_NOTRELATIVE)
                    return addtype;
                if (flags & APR_FILEPATH_NOTABOVEROOT)
                    return APR_EABOVEROOT;
                keptlen = 0;
                rootlen = pathlen = strlen(addroot);
                memcpy(path, addroot, pathlen);
            }
        }
        else {
            /* Now this is unexpected, we aren't aware of any other
             * incomplete path forms!  Fail now.
             */
            return APR_EBADPATH;
        }
    }
    else { /* addtype == APR_ERELATIVE */
        /* If both paths are relative, fail early
         */
        if (basetype != APR_EABSOLUTE && (flags & APR_FILEPATH_NOTRELATIVE))
            return basetype;

#ifndef NETWARE
        /* An incomplete UNC path must be completed
         */
        if (basetype == APR_INCOMPLETE 
                && (baseroot[0] == '/' || baseroot[0] == '\\')
                && (baseroot[1] == '/' || baseroot[1] == '\\'))
            fixunc = 1;
#endif

        /* Base the result path on the basepath
         */
        rootlen = strlen(baseroot);
        keptlen = pathlen = rootlen + baselen;
        if (keptlen >= sizeof(path))
            return APR_ENAMETOOLONG;
        memcpy(path, baseroot, rootlen);
        memcpy(path + rootlen, basepath, baselen);
    }

    /* '/' terminate the given root path unless it's already terminated
     * or is an incomplete drive root.  Correct the trailing slash unless
     * we have an incomplete UNC path still to fix.
     */
    if (pathlen && path[pathlen - 1] != ':') {
        if (path[pathlen - 1] != '/' && path[pathlen - 1] != '\\') {
            if (pathlen + 1 >= sizeof(path))
                return APR_ENAMETOOLONG;
        
            path[pathlen++] = ((flags & APR_FILEPATH_NATIVE) ? '\\' : '/');
        }
    /*  XXX: wrong, but gotta figure out what I intended;
     *  else if (!fixunc)
     *      path[pathlen++] = ((flags & APR_FILEPATH_NATIVE) ? '\\' : '/');
     */
    }

    while (*addpath) 
    {
        /* Parse each segment, find the closing '/' 
         */
        seglen = 0;
        while (addpath[seglen] && addpath[seglen] != '/'
                               && addpath[seglen] != '\\')
            ++seglen;

        /* Truncate all trailing spaces and all but the first two dots */
        segend = seglen;
        while (seglen && (addpath[seglen - 1] == ' ' 
                       || addpath[seglen - 1] == '.')) {
            if (seglen > 2 || addpath[seglen - 1] != '.' || addpath[0] != '.')
                --seglen;
            else
                break;
        }

        if (seglen == 0 || (seglen == 1 && addpath[0] == '.')) 
        {
            /* NOTE: win32 _hates_ '/ /' and '/. /' (yes, with spaces in there)
             * so eliminate all preconceptions that it is valid.
             */
            if (seglen < segend)
                return APR_EBADPATH;

#ifndef NETWARE
            /* This isn't legal unless the unc path is completed
             */
            if (fixunc)
                return APR_EBADPATH;
#endif

            /* Otherwise, this is a noop segment (/ or ./) so ignore it 
             */
        }
        else if (seglen == 2 && addpath[0] == '.' && addpath[1] == '.') 
        {
            /* NOTE: win32 _hates_ '/.. /' (yes, with a space in there)
             * and '/..../', some functions treat it as ".", and some 
             * fail! Eliminate all preconceptions that they are valid.
             */
            if (seglen < segend && (seglen != 3 || addpath[2] != '.'))
                return APR_EBADPATH;

#ifndef NETWARE
            /* This isn't legal unless the unc path is completed
             */
            if (fixunc)
                return APR_EBADPATH;
#endif

            /* backpath (../) when an absolute path is given */
            if (rootlen && (pathlen <= rootlen)) 
            {
                /* Attempt to move above root.  Always die if the 
                 * APR_FILEPATH_SECUREROOTTEST flag is specified.
                 */
                if (flags & APR_FILEPATH_SECUREROOTTEST)
                    return APR_EABOVEROOT;
                
                /* Otherwise this is simply a noop, above root is root.
                 */
            }
            else if (pathlen == 0 
                      || (pathlen >= 3 
                           && (pathlen == 3
                                || path[pathlen - 4] == ':'
                                || path[pathlen - 4] == '/' 
                                || path[pathlen - 4] == '\\')
                           &&  path[pathlen - 3] == '.' 
                           &&  path[pathlen - 2] == '.' 
                           && (path[pathlen - 1] == '/' 
                                || path[pathlen - 1] == '\\')))
            {
                /* Verified path is empty, exactly "..[/\]", or ends
                 * in "[:/\]..[/\]" - these patterns we will not back
                 * over since they aren't 'prior segements'.
                 * 
                 * If APR_FILEPATH_SECUREROOTTEST.was given, die now.
                 */
                if (flags & APR_FILEPATH_SECUREROOTTEST)
                    return APR_EABOVEROOT;

                /* Otherwise append another backpath.
                 */
                if (pathlen + 3 >= sizeof(path))
                    return APR_ENAMETOOLONG;
                path[pathlen++] = '.';
                path[pathlen++] = '.';
                if (addpath[segend]) {
                    path[pathlen++] = ((flags & APR_FILEPATH_NATIVE) 
                                    ? '\\' : ((flags & APR_FILEPATH_TRUENAME)
                                           ? '/' : addpath[segend]));
                }
                /* The 'root' part of this path now includes the ../ path,
                 * because that backpath will not be parsed by the truename
                 * code below.
                 */
                keptlen = pathlen;
            }
            else 
            {
                /* otherwise crop the prior segment 
                 */
                do {
                    --pathlen;
                } while (pathlen && path[pathlen - 1] != '/'
                                 && path[pathlen - 1] != '\\');

                /* Now test if we are above where we started and back up
                 * the keptlen offset to reflect the added/altered path.
                 */
                if (pathlen < keptlen) 
                {
                    if (flags & APR_FILEPATH_SECUREROOTTEST)
                        return APR_EABOVEROOT;
                    keptlen = pathlen;
                }
            }
        }
        else /* not empty or dots */
        {
#ifndef NETWARE
            if (fixunc) {
                const char *testpath = path;
                const char *testroot;
                apr_status_t testtype;
                apr_size_t i = (addpath[segend] != '\0');
                
                /* This isn't legal unless the unc path is complete!
                 */
                if (seglen < segend)
                    return APR_EBADPATH;
                if (pathlen + seglen + 1 >= sizeof(path))
                    return APR_ENAMETOOLONG;
                memcpy(path + pathlen, addpath, seglen + i);
                
                /* Always add the trailing slash to a UNC segment
                 */
                path[pathlen + seglen] = ((flags & APR_FILEPATH_NATIVE) 
                                             ? '\\' : '/');
                pathlen += seglen + 1;

                /* Recanonicalize the UNC root with the new UNC segment,
                 * and if we succeed, reset this test and the rootlen,
                 * and replace our path with the canonical UNC root path
                 */
                path[pathlen] = '\0';
                /* This call _should_ test the path
                 */
                testtype = apr_filepath_root(&testroot, &testpath, 
                                             APR_FILEPATH_TRUENAME
                                             | (flags & APR_FILEPATH_NATIVE),
                                             p);
                if (testtype == APR_SUCCESS) {
                    rootlen = pathlen = (testpath - path);
                    memcpy(path, testroot, pathlen);
                    fixunc = 0;
                }
                else if (testtype != APR_EINCOMPLETE) {
                    /* apr_filepath_root was very unexpected so fail already
                     */
                    return testtype;
                }
            }
            else
#endif
            {
                /* An actual segment, append it to the destination path
                 */
                apr_size_t i = (addpath[segend] != '\0');
                if (pathlen + seglen + i >= sizeof(path))
                    return APR_ENAMETOOLONG;
                memcpy(path + pathlen, addpath, seglen + i);
                if (i)
                    path[pathlen + seglen] = ((flags & APR_FILEPATH_NATIVE) 
                                                 ? '\\' : '/');
                pathlen += seglen + i;
            }
        }

        /* Skip over trailing slash to the next segment
         */
        if (addpath[segend])
            ++segend;

        addpath += segend;
    }
    
    /* keptlen will be the baselen unless the addpath contained
     * backpath elements.  If so, and APR_FILEPATH_NOTABOVEROOT
     * is specified (APR_FILEPATH_SECUREROOTTEST was caught above),
     * compare the string beyond the root to assure the result path 
     * is still within given basepath.  Note that the root path 
     * segment is thoroughly tested prior to path parsing.
     */
    if ((flags & APR_FILEPATH_NOTABOVEROOT) && baselen) {
        if (memcmp(basepath, path + rootlen, baselen) != 0)
            return APR_EABOVEROOT;
 
         /* Ahem... if we have a basepath without a trailing slash,
          * we better be sure that /foo wasn't replaced with /foobar!
          */
        if (basepath[baselen - 1] != '/' && basepath[baselen - 1] != '\\'
              && path[rootlen + baselen] && path[rootlen + baselen] != '/' 
                                         && path[rootlen + baselen] != '\\')
            return APR_EABOVEROOT;
    }

    if (addpath && (flags & APR_FILEPATH_TRUENAME)) {
        /* We can always skip the root, it's already true-named. */
        if (rootlen > keptlen)
            keptlen = rootlen;
        if ((path[keptlen] == '/') || (path[keptlen] == '\\')) {
            /* By rights, keptlen may grown longer than pathlen.
             * we wont' use it again (in that case) so we don't care.
             */
            ++keptlen;
        }
        /* Go through all the new segments */
        while (keptlen < pathlen) {
            apr_finfo_t finfo;
            char saveslash = 0;
            seglen = 0;
            /* find any slash and set it aside for a minute. */
            for (seglen = 0; keptlen + seglen < pathlen; ++seglen) {
                if ((path[keptlen + seglen] == '/')  ||
                    (path[keptlen + seglen] == '\\')) {
                    saveslash = path[keptlen + seglen];
                    break;
                }
            }
            /* Null term for stat! */
            path[keptlen + seglen] = '\0';
            if ((rv = apr_stat(&finfo, path, 
                               APR_FINFO_LINK | APR_FINFO_TYPE | APR_FINFO_NAME, p))
                == APR_SUCCESS) {
                apr_size_t namelen = strlen(finfo.name);

#if defined(OS2) /* only has case folding, never aliases that change the length */

                if (memcmp(finfo.name, path + keptlen, seglen) != 0) {
                    memcpy(path + keptlen, finfo.name, namelen);
                }
#else /* WIN32 || NETWARE; here there be aliases that gire and gimble and change length */

                if ((namelen != seglen) || 
                    (memcmp(finfo.name, path + keptlen, seglen) != 0)) 
                {
                    if (namelen <= seglen) {
                        memcpy(path + keptlen, finfo.name, namelen);
                        if ((namelen < seglen) && saveslash) {
                            memmove(path + keptlen + namelen + 1,
                                   path + keptlen + seglen + 1,
                                   pathlen - keptlen - seglen);
                            pathlen += namelen - seglen;
                            seglen = namelen;
                        }
                    }
                    else { /* namelen > seglen */
                        if (pathlen + namelen - seglen >= sizeof(path))
                            return APR_ENAMETOOLONG;
                        if (saveslash) {
                            memmove(path + keptlen + namelen + 1,
                                   path + keptlen + seglen + 1,
                                   pathlen - keptlen - seglen);
                        }
                        memcpy(path + keptlen, finfo.name, namelen);
                        pathlen += namelen - seglen;
                        seglen = namelen;
                    }
                }
#endif /* !OS2 (Whatever that alias was we're over it) */

                /* That's it, the rest is path info. 
                 * I don't know how we aught to handle this.  Should
                 * we define a new error to indicate 'more info'?
                 * Should we split out the rest of the path?
                 */
                if ((finfo.filetype != APR_DIR) && 
                    (finfo.filetype != APR_LNK) && saveslash) 
                    rv = APR_ENOTDIR;
#ifdef XXX_FIGURE_THIS_OUT
                {
                    /* the example inserts a null between the end of 
                     * the filename and the next segment, and increments
                     * the path length so we would return both segments.
                     */
                    if (saveslash) {
                        keptlen += seglen;
                        path[keptlen] = saveslash;
                        if (pathlen + 1 >= sizeof(path))
                            return APR_ENAMETOOLONG;
                        memmove(path + keptlen + 1,
                               path + keptlen,
                               pathlen - keptlen);
                        path[keptlen] = '\0';
                        ++pathlen;
                        break;
                    }
                }
#endif
            }

            /* put back the '/' */
            if (saveslash) {
                path[keptlen + seglen] = saveslash;
                ++seglen;
            }
            keptlen += seglen;

            if (rv != APR_SUCCESS) {
                if (APR_STATUS_IS_ENOENT(rv))
                    break;
                if (APR_STATUS_IS_EPATHWILD(rv))
                    /* This path included wildcards.  The path elements
                     * that did not contain wildcards are canonicalized,
                     * so we will return the path, although later elements
                     * don't necessarily exist, and aren't canonical.
                     */
                    break;
                else if (APR_STATUS_IS_ENOTDIR(rv))
                    /* This is a little more serious, we just added a name
                     * onto a filename (think http's PATH_INFO)
                     * If the caller is foolish enough to do this, we expect
                     * the've already canonicalized the root) that they knew
                     * what they are doing :(
                     */
                    break;
                else
                    return rv;
            }
        }
    }

    *newpath = apr_pstrmemdup(p, path, pathlen);
    return APR_SUCCESS;
}


APR_DECLARE(apr_status_t) apr_filepath_list_split(apr_array_header_t **pathelts,
                                                  const char *liststr,
                                                  apr_pool_t *p)
{
    return apr_filepath_list_split_impl(pathelts, liststr, ';', p);
}

APR_DECLARE(apr_status_t) apr_filepath_list_merge(char **liststr,
                                                  apr_array_header_t *pathelts,
                                                  apr_pool_t *p)
{
    return apr_filepath_list_merge_impl(liststr, pathelts, ';', p);
}


APR_DECLARE(apr_status_t) apr_filepath_encoding(int *style, apr_pool_t *p)
{
#if APR_HAS_UNICODE_FS
    IF_WIN_OS_IS_UNICODE
    {
        *style = APR_FILEPATH_ENCODING_UTF8;
        return APR_SUCCESS;
    }
#endif

    *style = APR_FILEPATH_ENCODING_LOCALE;
    return APR_SUCCESS;
}
