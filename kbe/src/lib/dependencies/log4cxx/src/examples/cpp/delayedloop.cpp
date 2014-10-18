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

#include <log4cxx/logger.h>
#include <log4cxx/xml/domconfigurator.h>
#include <log4cxx/propertyconfigurator.h>
#include <apr_general.h>
#include <apr_time.h>
#include <iostream>
#include <log4cxx/stream.h>
#include <exception>
#include <stdlib.h>

using namespace log4cxx;
using namespace log4cxx::helpers;


/**
This test program sits in a loop and logs things. Its logging is
configured by a configuration file. Changes to this configuration
file are monitored and when a change occurs, the config file is re-read.
*/
class DelayedLoop
{
        static LoggerPtr logger;

public:
        static void main(int argc, const char * const argv[])
        {
                if(argc == 2)
                {
                        init(argv[1]);
                }
                else
                {
                        usage(argv[0], "Wrong number of arguments.");
                }

                test();
        }

        static void usage(const char * programName, const char * msg)
        {
                std::cout << msg << std::endl;
                std::cout << "Usage: " << programName <<
                                " configFile" << std::endl;
                exit(1);
        }


        static void init(const std::string& configFile)
        {
                if(configFile.length() > 4 &&
                     configFile.substr(configFile.length() - 4) == ".xml")
                {
#if APR_HAS_THREADS
               xml::DOMConfigurator::configureAndWatch(configFile, 3000);
#else
               xml::DOMConfigurator::configure(configFile);
#endif
                }
                else
                {
#if APR_HAS_THREADS
                        PropertyConfigurator::configureAndWatch(configFile, 3000);
#else
                        PropertyConfigurator::configure(configFile);
#endif
                }
        }

        static void test()
        {
                int i = 0;
                while(true)
                {
                   LOG4CXX_DEBUG(logger, "MSG " << i++);
                        try
                        {
                                apr_sleep(1000000);
                        }
                        catch(std::exception& e)
                        {
                        }
                }
        }
};

LoggerPtr DelayedLoop::logger = Logger::getLogger("DelayedLoop");

int main(int argc, const char * const argv[])
{
    apr_app_initialize(&argc, &argv, NULL);
    int result = EXIT_SUCCESS;
    try
    {
        DelayedLoop::main(argc, argv);
    }
    catch(std::exception&)
    {
        result = EXIT_FAILURE;
    }

    apr_terminate();
    return result;
}
