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
#define __STDC_CONSTANT_MACROS
#include <log4cxx/logstring.h>
#include <log4cxx/helpers/cacheddateformat.h>


#include <apr_time.h>
#include <log4cxx/helpers/pool.h>
#include <limits>
#include <log4cxx/helpers/exception.h>

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::pattern;




/**
*  Supported digit set.  If the wrapped DateFormat uses
*  a different unit set, the millisecond pattern
*  will not be recognized and duplicate requests
*  will use the cache.
*/
const logchar CachedDateFormat::digits[] = { 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0 };

/**
 *  Expected representation of first magic number.
 */
const logchar CachedDateFormat::magicString1[] = { 0x36, 0x35, 0x34, 0 };


/**
 *  Expected representation of second magic number.
 */
const logchar CachedDateFormat::magicString2[] = { 0x39, 0x38, 0x37, 0};


/**
 *  Expected representation of 0 milliseconds.
 */
const logchar CachedDateFormat::zeroString[] = { 0x30, 0x30, 0x30, 0 };

#undef min

/**
 *  Creates a new CachedDateFormat object.
 *  @param dateFormat Date format, may not be null.
 *  @param expiration maximum cached range in milliseconds.
 *    If the dateFormat is known to be incompatible with the
 *      caching algorithm, use a value of 0 to totally disable
 *      caching or 1 to only use cache for duplicate requests.
 */
CachedDateFormat::CachedDateFormat(const DateFormatPtr& dateFormat,
        int expiration1) :
       formatter(dateFormat),
       millisecondStart(0),
       slotBegin(std::numeric_limits<log4cxx_time_t>::min()),
       cache(50, 0x20),
       expiration(expiration1),
       previousTime(std::numeric_limits<log4cxx_time_t>::min()) {
  if (dateFormat == NULL) {
    throw IllegalArgumentException(LOG4CXX_STR("dateFormat cannot be null"));
  }
  if (expiration1 < 0) {
    throw IllegalArgumentException(LOG4CXX_STR("expiration must be non-negative"));
  }
}


/**
 * Finds start of millisecond field in formatted time.
 * @param time long time, must be integral number of seconds
 * @param formatted String corresponding formatted string
 * @param formatter DateFormat date format
 * @return int position in string of first digit of milliseconds,
 *    -1 indicates no millisecond field, -2 indicates unrecognized
 *    field (likely RelativeTimeDateFormat)
 */
int CachedDateFormat::findMillisecondStart(
  log4cxx_time_t time, const LogString& formatted,
  const DateFormatPtr& formatter,
  Pool& pool) {

  apr_time_t slotBegin = (time / 1000000) * 1000000;
  if (slotBegin > time) {
     slotBegin -= 1000000;
  }
  int millis = (int) (time - slotBegin)/1000;

  int magic = magic1;
  LogString magicString(magicString1);
  if (millis == magic1) {
      magic = magic2;
      magicString = magicString2;
  }

  LogString plusMagic;
  formatter->format(plusMagic, slotBegin + magic, pool);

  /**
   *   If the string lengths differ then
   *      we can't use the cache except for duplicate requests.
   */
  if (plusMagic.length() != formatted.length()) {
      return UNRECOGNIZED_MILLISECONDS;
  } else {
      // find first difference between values
     for (LogString::size_type i = 0; i < formatted.length(); i++) {
        if (formatted[i] != plusMagic[i]) {
           //
           //   determine the expected digits for the base time
           const logchar abc[] = { 0x41, 0x42, 0x43, 0 };
           LogString formattedMillis(abc);
           millisecondFormat(millis, formattedMillis, 0);

           LogString plusZero;
           formatter->format(plusZero, slotBegin, pool);

           //   If the next 3 characters match the magic
           //      strings and the remaining fragments are identical
           //
           //
           if (plusZero.length() == formatted.length()
              && regionMatches(magicString, 0, plusMagic, i, magicString.length())
              && regionMatches(formattedMillis, 0, formatted, i, magicString.length())
              && regionMatches(zeroString, 0, plusZero, i, 3)
              && (formatted.length() == i + 3
                 || plusZero.compare(i + 3,
                       LogString::npos, plusMagic, i+3, LogString::npos) == 0)) {
              return i;
           } else {
              return UNRECOGNIZED_MILLISECONDS;
          }
        }
     }
  }
  return  NO_MILLISECONDS;
}


/**
 * Formats a millisecond count into a date/time string.
 *
 *  @param now Number of milliseconds after midnight 1 Jan 1970 GMT.
 *  @param sbuf the string buffer to write to
 */
 void CachedDateFormat::format(LogString& buf, log4cxx_time_t now, Pool& p) const {

  //
  // If the current requested time is identical to the previously
  //     requested time, then append the cache contents.
  //
  if (now == previousTime) {
       buf.append(cache);
       return;
  }

  //
  //   If millisecond pattern was not unrecognized
  //     (that is if it was found or milliseconds did not appear)
  //
  if (millisecondStart != UNRECOGNIZED_MILLISECONDS) {

      //    Check if the cache is still valid.
      //    If the requested time is within the same integral second
      //       as the last request and a shorter expiration was not requested.
      if (now < slotBegin + expiration
          && now >= slotBegin
          && now < slotBegin + 1000000L) {

          //
          //    if there was a millisecond field then update it
          //
          if (millisecondStart >= 0 ) {
              millisecondFormat((int) ((now - slotBegin)/1000), cache, millisecondStart);
          }
          //
          //   update the previously requested time
          //      (the slot begin should be unchanged)
          previousTime = now;
          buf.append(cache);
          return;
      }
  }


  //
  //  could not use previous value.
  //    Call underlying formatter to format date.
  cache.erase(cache.begin(), cache.end());
  formatter->format(cache, now, p);
  buf.append(cache);
  previousTime = now;
  slotBegin = (previousTime / 1000000) * 1000000;
  if (slotBegin > previousTime) {
      slotBegin -= 1000000;
  }


  //
  //    if the milliseconds field was previous found
  //       then reevaluate in case it moved.
  //
  if (millisecondStart >= 0) {
      millisecondStart = findMillisecondStart(now, cache, formatter, p);
  }
}


/**
 *   Formats a count of milliseconds (0-999) into a numeric representation.
 *   @param millis Millisecond coun between 0 and 999.
 *   @buf String buffer, may not be null.
 *   @offset Starting position in buffer, the length of the
 *       buffer must be at least offset + 3.
 */
void CachedDateFormat::millisecondFormat(int millis,
     LogString& buf,
     int offset) {
     buf[offset] = digits[ millis / 100];
     buf[offset + 1] = digits[(millis / 10) % 10];
     buf[offset + 2] = digits[millis  % 10];
 }

/**
 * Set timezone.
 *
 * @remarks Setting the timezone using getCalendar().setTimeZone()
 * will likely cause caching to misbehave.
 * @param timeZone TimeZone new timezone
 */
void CachedDateFormat::setTimeZone(const TimeZonePtr& timeZone) {
  formatter->setTimeZone(timeZone);
  previousTime = std::numeric_limits<log4cxx_time_t>::min();
  slotBegin = std::numeric_limits<log4cxx_time_t>::min();
}



void CachedDateFormat::numberFormat(LogString& s, int n, Pool& p) const {
  formatter->numberFormat(s, n, p);
}


/**
 * Gets maximum cache validity for the specified SimpleDateTime
 *    conversion pattern.
 *  @param pattern conversion pattern, may not be null.
 *  @returns Duration in microseconds from an integral second
 *      that the cache will return consistent results.
 */
int CachedDateFormat::getMaximumCacheValidity(const LogString& pattern) {
   //
   //   If there are more "S" in the pattern than just one "SSS" then
   //      (for example, "HH:mm:ss,SSS SSS"), then set the expiration to
   //      one millisecond which should only perform duplicate request caching.
   //
   const logchar S = 0x53;
   const logchar SSS[] = { 0x53, 0x53, 0x53, 0 };
   size_t firstS = pattern.find(S);
   size_t len = pattern.length();
   //
   //   if there are no S's or
   //      three that start with the first S and no fourth S in the string
   //
   if (firstS == LogString::npos ||
       (len >= firstS + 3 && pattern.compare(firstS, 3, SSS) == 0
           && (len == firstS + 3 ||
                pattern.find(S, firstS + 3) == LogString::npos))) {
           return 1000000;
   }
   return 1000;
}


/**
* Tests if two string regions are equal.
* @param target target string.
* @param toffset character position in target to start comparison.
* @param other other string.
* @param ooffset character position in other to start comparison.
* @param len length of region.
* @return true if regions are equal.
*/
bool CachedDateFormat::regionMatches(
    const LogString& target,
    size_t toffset,
    const LogString& other,
    size_t ooffset,
    size_t len) {
    return target.compare(toffset, len, other, ooffset, len) == 0;
}

