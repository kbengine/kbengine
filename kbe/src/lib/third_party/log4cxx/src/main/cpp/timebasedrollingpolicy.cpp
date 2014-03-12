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
#if defined(_MSC_VER)
#pragma warning ( disable: 4231 4251 4275 4786 )
#endif

#include <log4cxx/logstring.h>
#include <log4cxx/rolling/timebasedrollingpolicy.h>
#include <log4cxx/pattern/filedatepatternconverter.h>
#include <log4cxx/helpers/date.h>
#include <log4cxx/rolling/filerenameaction.h>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/rolling/gzcompressaction.h>
#include <log4cxx/rolling/zipcompressaction.h>

#ifndef INT64_C
#define INT64_C(x) x ## LL
#endif

#include <apr_time.h>


using namespace log4cxx;
using namespace log4cxx::rolling;
using namespace log4cxx::helpers;
using namespace log4cxx::pattern;

IMPLEMENT_LOG4CXX_OBJECT(TimeBasedRollingPolicy)

TimeBasedRollingPolicy::TimeBasedRollingPolicy() {
}

void TimeBasedRollingPolicy::addRef() const {
    TriggeringPolicy::addRef();
}

void TimeBasedRollingPolicy::releaseRef() const {
    TriggeringPolicy::releaseRef();
}

void TimeBasedRollingPolicy::activateOptions(log4cxx::helpers::Pool& pool) {
    // find out period from the filename pattern
    if (getFileNamePattern().length() > 0) {
      parseFileNamePattern();
    } else {
      LogLog::warn(
         LOG4CXX_STR("The FileNamePattern option must be set before using TimeBasedRollingPolicy. "));
      throw IllegalStateException();
    }

    PatternConverterPtr dtc(getDatePatternConverter());

    if (dtc == NULL) {
      throw IllegalStateException();
    }

    apr_time_t n = apr_time_now();
    LogString buf;
    ObjectPtr obj(new Date(n));
    formatFileName(obj, buf, pool);
    lastFileName = buf;

    suffixLength = 0;

    if (lastFileName.length() >= 3) {
      if (lastFileName.compare(lastFileName.length() - 3, 3, LOG4CXX_STR(".gz")) == 0) {
        suffixLength = 3;
      } else if (lastFileName.length() >= 4 && lastFileName.compare(lastFileName.length() - 4, 4, LOG4CXX_STR(".zip")) == 0) {
        suffixLength = 4;
      }
    }
}


#define RULES_PUT(spec, cls) \
specs.insert(PatternMap::value_type(LogString(LOG4CXX_STR(spec)), (PatternConstructor) cls ::newInstance))

log4cxx::pattern::PatternMap TimeBasedRollingPolicy::getFormatSpecifiers() const {
  PatternMap specs;
  RULES_PUT("d", FileDatePatternConverter);
  RULES_PUT("date", FileDatePatternConverter);
  return specs;
}

/**
 * {@inheritDoc}
 */
RolloverDescriptionPtr TimeBasedRollingPolicy::initialize(
  const LogString& currentActiveFile,
  const bool append,
  Pool& pool) {
  apr_time_t n = apr_time_now();
  nextCheck = ((n / APR_USEC_PER_SEC) + 1) * APR_USEC_PER_SEC;

  LogString buf;
  ObjectPtr obj(new Date(n));
  formatFileName(obj, buf, pool);
  lastFileName = buf;

  ActionPtr noAction;

  if (currentActiveFile.length() > 0) {
    return new RolloverDescription(
      currentActiveFile, append, noAction, noAction);
  } else {
    return new RolloverDescription(
      lastFileName.substr(0, lastFileName.length() - suffixLength), append,
      noAction, noAction);
  }
}



RolloverDescriptionPtr TimeBasedRollingPolicy::rollover(
   const LogString& currentActiveFile,
   Pool& pool) {
  apr_time_t n = apr_time_now();
  nextCheck = ((n / APR_USEC_PER_SEC) + 1) * APR_USEC_PER_SEC;

  LogString buf;
  ObjectPtr obj(new Date(n));
  formatFileName(obj, buf, pool);

  LogString newFileName(buf);

  //
  //  if file names haven't changed, no rollover
  //
  if (newFileName == lastFileName) {
    RolloverDescriptionPtr desc;
    return desc;
  }

  ActionPtr renameAction;
  ActionPtr compressAction;
  LogString lastBaseName(
    lastFileName.substr(0, lastFileName.length() - suffixLength));
  LogString nextActiveFile(
    newFileName.substr(0, newFileName.length() - suffixLength));

  //
  //   if currentActiveFile is not lastBaseName then
  //        active file name is not following file pattern
  //        and requires a rename plus maintaining the same name
  if (currentActiveFile != lastBaseName) {
    renameAction =
      new FileRenameAction(
        File().setPath(currentActiveFile), File().setPath(lastBaseName), true);
    nextActiveFile = currentActiveFile;
  }

  if (suffixLength == 3) {
    compressAction =
      new GZCompressAction(
        File().setPath(lastBaseName), File().setPath(lastFileName), true);
  }

  if (suffixLength == 4) {
    compressAction =
      new ZipCompressAction(
        File().setPath(lastBaseName), File().setPath(lastFileName), true);
  }

  lastFileName = newFileName;

  return new RolloverDescription(
    nextActiveFile, false, renameAction, compressAction);
}



bool TimeBasedRollingPolicy::isTriggeringEvent(
  Appender* /* appender */,
  const log4cxx::spi::LoggingEventPtr& /* event */,
  const LogString& /* filename */,
  size_t /* fileLength */)  {
    return apr_time_now() > nextCheck;
}
