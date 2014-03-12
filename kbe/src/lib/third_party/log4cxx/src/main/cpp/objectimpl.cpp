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
#include <log4cxx/helpers/objectimpl.h>
#include <apr_atomic.h>
#if !defined(LOG4CXX)
#define LOG4CXX 1
#endif
#include <log4cxx/helpers/aprinitializer.h>

using namespace log4cxx::helpers;

ObjectImpl::ObjectImpl() : ref( 0 )
{
  log4cxx::helpers::APRInitializer::initialize();
}

ObjectImpl::~ObjectImpl()
{
}

void ObjectImpl::addRef() const
{
  apr_atomic_inc32( & ref );
}

void ObjectImpl::releaseRef() const
{
  if ( apr_atomic_dec32( & ref ) == 0 )
  {
    delete this;
  }
}
