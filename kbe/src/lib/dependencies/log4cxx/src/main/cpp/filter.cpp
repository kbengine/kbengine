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
#include <log4cxx/spi/filter.h>

using namespace log4cxx;
using namespace log4cxx::spi;
using namespace log4cxx::helpers;

Filter::Filter() : next() {
}

void Filter::addRef() const {
    ObjectImpl::addRef();
}

void Filter::releaseRef() const {
    ObjectImpl::releaseRef();
}

FilterPtr Filter::getNext() const {
   return next;
}

void Filter::setNext(const FilterPtr& newNext) {
    next = newNext;
}

void Filter::activateOptions(Pool&) {
}

void Filter::setOption(const LogString&, const LogString&) {
}

