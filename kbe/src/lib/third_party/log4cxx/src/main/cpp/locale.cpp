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

#include <log4cxx/helpers/locale.h>

using namespace log4cxx;
using namespace log4cxx::helpers;


Locale::Locale(const LogString& language1)
 : language(language1)
{
}

Locale::Locale(const LogString& language1, const LogString& country1)
 : language(language1), country(country1)
{
}

Locale::Locale(const LogString& language1, const LogString& country1,
   const LogString& variant1)
: language(language1), country(country1), variant(variant1)
{
}


const LogString& Locale::getLanguage() const
{
   return language;
}

const LogString& Locale::getCountry() const
{
   return country;
}

const LogString& Locale::getVariant() const
{
   return variant;
}

