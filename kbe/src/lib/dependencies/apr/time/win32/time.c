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

#include "apr_arch_atime.h"
#include "apr_time.h"
#include "apr_general.h"
#include "apr_lib.h"
#include "apr_portable.h"
#if APR_HAVE_TIME_H
#include <time.h>
#endif
#if APR_HAVE_ERRNO_H
#include <errno.h>
#endif
#include <string.h>
#include <winbase.h>
#include "apr_arch_misc.h"

/* Leap year is any year divisible by four, but not by 100 unless also
 * divisible by 400
 */
#define IsLeapYear(y) ((!(y % 4)) ? (((y % 400) && !(y % 100)) ? 0 : 1) : 0)

static DWORD get_local_timezone(TIME_ZONE_INFORMATION **tzresult)
{
    static TIME_ZONE_INFORMATION tz;
    static DWORD result;
    static int init = 0;

    if (!init) {
        result = GetTimeZoneInformation(&tz);
        init = 1;
    }

    *tzresult = &tz;
    return result;
}

static void SystemTimeToAprExpTime(apr_time_exp_t *xt, SYSTEMTIME *tm)
{
    static const int dayoffset[12] =
    {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

    /* Note; the caller is responsible for filling in detailed tm_usec,
     * tm_gmtoff and tm_isdst data when applicable.
     */
    xt->tm_usec = tm->wMilliseconds * 1000;
    xt->tm_sec  = tm->wSecond;
    xt->tm_min  = tm->wMinute;
    xt->tm_hour = tm->wHour;
    xt->tm_mday = tm->wDay;
    xt->tm_mon  = tm->wMonth - 1;
    xt->tm_year = tm->wYear - 1900;
    xt->tm_wday = tm->wDayOfWeek;
    xt->tm_yday = dayoffset[xt->tm_mon] + (tm->wDay - 1);
    xt->tm_isdst = 0;
    xt->tm_gmtoff = 0;

    /* If this is a leap year, and we're past the 28th of Feb. (the
     * 58th day after Jan. 1), we'll increment our tm_yday by one.
     */
    if (IsLeapYear(tm->wYear) && (xt->tm_yday > 58))
        xt->tm_yday++;
}

APR_DECLARE(apr_status_t) apr_time_ansi_put(apr_time_t *result, 
                                                    time_t input)
{
    *result = (apr_time_t) input * APR_USEC_PER_SEC;
    return APR_SUCCESS;
}

/* Return micro-seconds since the Unix epoch (jan. 1, 1970) UTC */
APR_DECLARE(apr_time_t) apr_time_now(void)
{
    LONGLONG aprtime = 0;
    FILETIME time;
#ifndef _WIN32_WCE
    GetSystemTimeAsFileTime(&time);
#else
    SYSTEMTIME st;
    GetSystemTime(&st);
    SystemTimeToFileTime(&st, &time);
#endif
    FileTimeToAprTime(&aprtime, &time);
    return aprtime; 
}

APR_DECLARE(apr_status_t) apr_time_exp_gmt(apr_time_exp_t *result,
                                           apr_time_t input)
{
    FILETIME ft;
    SYSTEMTIME st;
    AprTimeToFileTime(&ft, input);
    FileTimeToSystemTime(&ft, &st);
    /* The Platform SDK documents that SYSTEMTIME/FILETIME are
     * generally UTC, so no timezone info needed
     */
    SystemTimeToAprExpTime(result, &st);
    result->tm_usec = (apr_int32_t) (input % APR_USEC_PER_SEC);
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_time_exp_tz(apr_time_exp_t *result, 
                                          apr_time_t input, 
                                          apr_int32_t offs)
{
    FILETIME ft;
    SYSTEMTIME st;
    AprTimeToFileTime(&ft, input + (offs *  APR_USEC_PER_SEC));
    FileTimeToSystemTime(&ft, &st);
    /* The Platform SDK documents that SYSTEMTIME/FILETIME are
     * generally UTC, so we will simply note the offs used.
     */
    SystemTimeToAprExpTime(result, &st);
    result->tm_usec = (apr_int32_t) (input % APR_USEC_PER_SEC);
    result->tm_gmtoff = offs;
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_time_exp_lt(apr_time_exp_t *result,
                                          apr_time_t input)
{
    SYSTEMTIME st;
    FILETIME ft, localft;

    AprTimeToFileTime(&ft, input);

#if APR_HAS_UNICODE_FS && !defined(_WIN32_WCE)
    IF_WIN_OS_IS_UNICODE
    {
        TIME_ZONE_INFORMATION *tz;
        SYSTEMTIME localst;
        apr_time_t localtime;

        get_local_timezone(&tz);

        FileTimeToSystemTime(&ft, &st);

        /* The Platform SDK documents that SYSTEMTIME/FILETIME are
         * generally UTC.  We use SystemTimeToTzSpecificLocalTime
         * because FileTimeToLocalFileFime is documented that the
         * resulting time local file time would have DST relative
         * to the *present* date, not the date converted.
         */
        SystemTimeToTzSpecificLocalTime(tz, &st, &localst);
        SystemTimeToAprExpTime(result, &localst);
        result->tm_usec = (apr_int32_t) (input % APR_USEC_PER_SEC);


        /* Recover the resulting time as an apr time and use the
         * delta for gmtoff in seconds (and ignore msec rounding) 
         */
        SystemTimeToFileTime(&localst, &localft);
        FileTimeToAprTime(&localtime, &localft);
        result->tm_gmtoff = (int)apr_time_sec(localtime) 
                          - (int)apr_time_sec(input);

        /* To compute the dst flag, we compare the expected 
         * local (standard) timezone bias to the delta.
         * [Note, in war time or double daylight time the
         * resulting tm_isdst is, desireably, 2 hours]
         */
        result->tm_isdst = (result->tm_gmtoff / 3600)
                         - (-(tz->Bias + tz->StandardBias) / 60);
    }
#endif
#if APR_HAS_ANSI_FS || defined(_WIN32_WCE)
    ELSE_WIN_OS_IS_ANSI
    {
        TIME_ZONE_INFORMATION tz;
	/* XXX: This code is simply *wrong*.  The time converted will always
         * map to the *now current* status of daylight savings time.
         */

        FileTimeToLocalFileTime(&ft, &localft);
        FileTimeToSystemTime(&localft, &st);
        SystemTimeToAprExpTime(result, &st);
        result->tm_usec = (apr_int32_t) (input % APR_USEC_PER_SEC);

        switch (GetTimeZoneInformation(&tz)) {
            case TIME_ZONE_ID_UNKNOWN:
                result->tm_isdst = 0;
                /* Bias = UTC - local time in minutes
                 * tm_gmtoff is seconds east of UTC
                 */
                result->tm_gmtoff = tz.Bias * -60;
                break;
            case TIME_ZONE_ID_STANDARD:
                result->tm_isdst = 0;
                result->tm_gmtoff = (tz.Bias + tz.StandardBias) * -60;
                break;
            case TIME_ZONE_ID_DAYLIGHT:
                result->tm_isdst = 1;
                result->tm_gmtoff = (tz.Bias + tz.DaylightBias) * -60;
                break;
            default:
                /* noop */;
        }
    }
#endif

    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_time_exp_get(apr_time_t *t,
                                           apr_time_exp_t *xt)
{
    apr_time_t year = xt->tm_year;
    apr_time_t days;
    static const int dayoffset[12] =
    {306, 337, 0, 31, 61, 92, 122, 153, 184, 214, 245, 275};

    /* shift new year to 1st March in order to make leap year calc easy */

    if (xt->tm_mon < 2)
        year--;

    /* Find number of days since 1st March 1900 (in the Gregorian calendar). */

    days = year * 365 + year / 4 - year / 100 + (year / 100 + 3) / 4;
    days += dayoffset[xt->tm_mon] + xt->tm_mday - 1;
    days -= 25508;              /* 1 jan 1970 is 25508 days since 1 mar 1900 */

    days = ((days * 24 + xt->tm_hour) * 60 + xt->tm_min) * 60 + xt->tm_sec;

    if (days < 0) {
        return APR_EBADDATE;
    }
    *t = days * APR_USEC_PER_SEC + xt->tm_usec;
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_time_exp_gmt_get(apr_time_t *t,
                                               apr_time_exp_t *xt)
{
    apr_status_t status = apr_time_exp_get(t, xt);
    if (status == APR_SUCCESS)
        *t -= (apr_time_t) xt->tm_gmtoff * APR_USEC_PER_SEC;
    return status;
}

APR_DECLARE(apr_status_t) apr_os_imp_time_get(apr_os_imp_time_t **ostime,
                                              apr_time_t *aprtime)
{
    /* TODO: Consider not passing in pointer to apr_time_t (e.g., call by value) */
    AprTimeToFileTime(*ostime, *aprtime);
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_os_exp_time_get(apr_os_exp_time_t **ostime, 
                                              apr_time_exp_t *aprexptime)
{
    (*ostime)->wYear = aprexptime->tm_year + 1900;
    (*ostime)->wMonth = aprexptime->tm_mon + 1;
    (*ostime)->wDayOfWeek = aprexptime->tm_wday;
    (*ostime)->wDay = aprexptime->tm_mday;
    (*ostime)->wHour = aprexptime->tm_hour;
    (*ostime)->wMinute = aprexptime->tm_min;
    (*ostime)->wSecond = aprexptime->tm_sec;
    (*ostime)->wMilliseconds = aprexptime->tm_usec / 1000;
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_os_imp_time_put(apr_time_t *aprtime,
                                              apr_os_imp_time_t **ostime,
                                              apr_pool_t *cont)
{
    /* XXX: sanity failure, what is file time, gmt or local ?
     */
    FileTimeToAprTime(aprtime, *ostime);
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_os_exp_time_put(apr_time_exp_t *aprtime,
                                              apr_os_exp_time_t **ostime,
                                              apr_pool_t *cont)
{
    /* The Platform SDK documents that SYSTEMTIME/FILETIME are
     * generally UTC, so no timezone info needed
     */
    SystemTimeToAprExpTime(aprtime, *ostime);
    return APR_SUCCESS;
}

APR_DECLARE(void) apr_sleep(apr_interval_time_t t)
{
    /* One of the few sane situations for a cast, Sleep
     * is in ms, not us, and passed as a DWORD value
     */
    Sleep((DWORD)(t / 1000));
}

#if defined(_WIN32_WCE)
/* A noop on WinCE, like Unix implementation */
APR_DECLARE(void) apr_time_clock_hires(apr_pool_t *p)
{
    return;
}
#else
static apr_status_t clock_restore(void *unsetres)
{
    ULONG newRes;
    SetTimerResolution((ULONG)(apr_ssize_t)unsetres, FALSE, &newRes);
    return APR_SUCCESS;
}

APR_DECLARE(void) apr_time_clock_hires(apr_pool_t *p)
{
    ULONG newRes;
    /* Timer resolution is stated in 100ns units.  Note that TRUE requests the
     * new clock resolution, FALSE above releases the request.
     */
    if (SetTimerResolution(10000, TRUE, &newRes) == 0 /* STATUS_SUCCESS */) {
        /* register the cleanup... */
        apr_pool_cleanup_register(p, (void*)10000, clock_restore,
                                  apr_pool_cleanup_null);
    }
}
#endif
