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

#ifndef _LOG4CXX_HELPERS_LOCALE_CHANGER_H
#define _LOG4CXX_HELPERS_LOCALE_CHANGER_H

#include <locale>

namespace log4cxx {
  namespace helpers {
    /**
    *   Utility class to change the locale for the duration of a test.
    *
    * 
    * 
    *
    */
    class LocaleChanger {
    public:
    /**
    *   Construction attemtps to change default locale.
    * @param locale locale.
    */
       LocaleChanger(const char* locale);

    /**
    * Restores previous locale.
    */
      ~LocaleChanger();

      /**
      * Determines whether locale change was effective.
      * @return true if effective.
      */
      inline bool isEffective() { return effective; }

    private:
      LocaleChanger(LocaleChanger&);
      LocaleChanger& operator=(LocaleChanger&);
      std::locale initial;
      bool effective;
    };
  }
}
#endif
