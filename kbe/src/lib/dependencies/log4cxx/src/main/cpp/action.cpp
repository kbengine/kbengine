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
#include <log4cxx/rolling/action.h>
#include <log4cxx/helpers/synchronized.h>

using namespace log4cxx;
using namespace log4cxx::rolling;
using namespace log4cxx::helpers;

IMPLEMENT_LOG4CXX_OBJECT(Action)

Action::Action() :
   complete(false),
   interrupted(false),
   pool(),
   mutex(pool) {
}

Action::~Action() {
}

/**
 * {@inheritDoc}
 */
void Action::run(log4cxx::helpers::Pool& pool1) {
  synchronized sync(mutex);
  if (!interrupted) {
      try {
         execute(pool1);
      } catch(std::exception& ex) {
         reportException(ex);
      }
      complete = true;
      interrupted = true;
  }
}

  /**
   * {@inheritDoc}
   */
void Action::close() {
    synchronized sync(mutex);
    interrupted = true;
}

  /**
   * Tests if the action is complete.
   * @return true if action is complete.
   */
bool Action::isComplete() const {
    return complete;
}

/**
 * Capture exception.
 *
 * @param ex exception.
 */
void Action::reportException(const std::exception& /* ex */) {
}
