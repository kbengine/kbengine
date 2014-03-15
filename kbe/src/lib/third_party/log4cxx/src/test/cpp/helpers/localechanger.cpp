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
#define LOG4CXX_TEST
#include <log4cxx/private/log4cxx_private.h>
#if LOG4CXX_HAS_STD_LOCALE

#include "localechanger.h"
#include <stdexcept>

using namespace log4cxx::helpers;

/**
*   Construction attemtps to change default locale.
* @param locale locale.
*/
LocaleChanger::LocaleChanger(const char* locale) {
    effective = false;
    try {
        std::locale newLocale(locale);
        initial = std::locale::global(newLocale);
        effective = true;
    } catch(std::runtime_error&) {
    } catch(std::exception&) {
    }
  }

/**
* Restores previous locale.
*/
LocaleChanger::~LocaleChanger() {
      if (effective) {
        std::locale::global(initial);
      }
  }

#endif
