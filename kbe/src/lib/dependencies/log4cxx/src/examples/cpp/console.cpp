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

#include <stdlib.h>
#include <log4cxx/logger.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/simplelayout.h>
#include <log4cxx/logmanager.h>
#include <iostream>
#include <locale.h>
#include <string.h>
#include <stdio.h>

using namespace log4cxx;
using namespace log4cxx::helpers;

/**
 *   Configures console appender.
 *   @param err if true, use stderr, otherwise stdout.
 */
static void configure(bool err) {
    log4cxx::ConsoleAppenderPtr appender(new log4cxx::ConsoleAppender());
    if (err) {
        appender->setTarget(LOG4CXX_STR("System.err"));
    }
    log4cxx::LayoutPtr layout(new log4cxx::SimpleLayout());
    appender->setLayout(layout);
    log4cxx::helpers::Pool pool;
    appender->activateOptions(pool);
    log4cxx::Logger::getRootLogger()->addAppender(appender);
    LogManager::getLoggerRepository()->setConfigured(true);
}

/**
 *   Program to test compatibility of C RTL, C++ STL and log4cxx output to standard
 *       output and error streams.
 *
 *    See bug LOGCXX_126.
 *
 *    
 */
int main(int argc, char** argv)
{
    setlocale(LC_ALL, "");
    if (argc <= 1) {
        puts("Console test program\nUsage: console [-err] [ puts | putws | cout | wcout | configure | log | wide | byte ]*\n");  
    }
    bool configured = false;
    bool err = false;
    for (int i = 1; i < argc; i++) {
        if (strcmp("-err", argv[i]) == 0) {
            err = true;
        } else if (strcmp("puts", argv[i]) == 0) {
            fputs("Hello, fputs\n", err ? stderr : stdout);
#if LOG4CXX_WCHAR_T_API
        } else if (strcmp("putws", argv[i]) == 0) {
            fputws(L"Hello, fputws\n", err ? stderr : stdout);
#endif
        } else if (strcmp("cout", argv[i]) == 0) {
            if (err) {
                std::cerr << "Hello, cout" << std::endl;
            } else {
                std::cout << "Hello, cout" << std::endl;
            }
        } else if (strcmp("wcout", argv[i]) == 0) {
            if (err) {
            #if LOG4CXX_HAS_STD_WCOUT
                std::wcerr << L"Hello, wcout" << std::endl;
            #else
                std::cerr << "Log4cxx has not wcout" << std::endl;
            #endif
            } else {
            #if LOG4CXX_HAS_STD_WCOUT
                std::wcout << L"Hello, wcout" << std::endl;
            #else
                std::cout << "Log4cxx has not wcout" << std::endl;
            #endif
            }
        } else if (strcmp("configure", argv[i]) == 0) {
            configure(err);
            configured = true;
        } else if (strcmp("log", argv[i]) == 0) {
            if (!configured) {
                configure(err);
                configured = true;
            }
            log4cxx::Logger::getRootLogger()->info("Hello, log4cxx");
#if LOG4CXX_WCHAR_T_API
        } else if (strcmp("wide", argv[i]) == 0) {
            fwide(err ? stderr : stdout, 1);
        } else if (strcmp("byte", argv[i]) == 0) {
            fwide(err ? stderr : stdout, -1);
#endif
        } else {
            fputs("Unrecognized option: ", stderr);
            fputs(argv[i], stderr);
            fputs("\n", stderr);
            fflush(stderr);
        }
    }
    return 0;
}
