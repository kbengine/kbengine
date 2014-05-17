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

#ifndef _LOG4CXX_SPI_CONFIGURATOR_H
#define _LOG4CXX_SPI_CONFIGURATOR_H

#include <log4cxx/spi/loggerrepository.h>

namespace log4cxx
{
    class File;

   namespace spi
   {
      /**
      Implemented by classes capable of configuring log4j using a URL.
      */
      class LOG4CXX_EXPORT Configurator : virtual public helpers::Object
      {
      public:
         DECLARE_ABSTRACT_LOG4CXX_OBJECT(Configurator)
         Configurator();

         /**
         Interpret a resource pointed by a URL and set up log4j accordingly.

         The configuration is done relative to the <code>hierarchy</code>
         parameter.

         @param configFileName The file to parse
         @param repository The hierarchy to operation upon.
         */
         virtual void doConfigure(const File& configFileName,
            spi::LoggerRepositoryPtr& repository) = 0;


      private:
         Configurator(const Configurator&);
         Configurator& operator=(const Configurator&);
         bool initialized;
      };

      LOG4CXX_PTR_DEF(Configurator);
   }
}

#endif // _LOG4CXX_SPI_CONFIGURATOR_H
