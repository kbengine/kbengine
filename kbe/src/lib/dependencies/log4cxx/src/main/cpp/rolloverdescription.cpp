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

#include <log4cxx/logstring.h>
#include <log4cxx/rolling/rolloverdescription.h>

using namespace log4cxx;
using namespace log4cxx::rolling;
using namespace log4cxx::helpers;

IMPLEMENT_LOG4CXX_OBJECT(RolloverDescription)


RolloverDescription::RolloverDescription() {
}

RolloverDescription::RolloverDescription(
    const LogString& activeFileName1,
    const bool append1,
    const ActionPtr& synchronous1,
    const ActionPtr& asynchronous1)
       : activeFileName(activeFileName1),
         append(append1),
         synchronous(synchronous1),
         asynchronous(asynchronous1) {
}

LogString RolloverDescription::getActiveFileName() const {
    return activeFileName;
}

bool RolloverDescription::getAppend() const {
    return append;
}

ActionPtr RolloverDescription::getSynchronous() const {
    return synchronous;
}

  /**
   * Action to be completed after close of current active log file
   * and before next rollover attempt, may be executed asynchronously.
   *
   * @return action, may be null.
   */
ActionPtr RolloverDescription::getAsynchronous() const {
    return asynchronous;
}
