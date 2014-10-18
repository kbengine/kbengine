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
#include <log4cxx/helpers/propertyresourcebundle.h>
#include <log4cxx/helpers/exception.h>
#if !defined(LOG4CXX)
#define LOG4CXX 1
#endif
#include <log4cxx/private/log4cxx_private.h>

using namespace log4cxx;
using namespace log4cxx::helpers;

IMPLEMENT_LOG4CXX_OBJECT(PropertyResourceBundle)


PropertyResourceBundle::PropertyResourceBundle(InputStreamPtr inStream)
{
   properties.load(inStream);
}

LogString PropertyResourceBundle::getString(const LogString& key) const
{
   LogString resource;
   PropertyResourceBundlePtr resourceBundle(const_cast<PropertyResourceBundle*>(this));

   do
   {
      resource = resourceBundle->properties.getProperty(key);
      if (!resource.empty())
      {
         return resource;
      }

      resourceBundle = resourceBundle->parent;
   }
   while (resourceBundle != 0);

   throw MissingResourceException(key);
#if LOG4CXX_RETURN_AFTER_THROW
   return resource;
#endif
}
