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

namespace log4cxx
{
        class File;
        namespace helpers {
            class Pool;
        }

        class Compare
        {
        public:
                static bool compare(const File& file1,
                  const File& file2);

        private:
                /// Prints file on the console.
                static void outputFile(const File& file,
                      const LogString& contents,
                      log4cxx::helpers::Pool& pool);

                static void emit(const LogString &line);
                static bool getline(LogString& buf, LogString& line);

        };
}
