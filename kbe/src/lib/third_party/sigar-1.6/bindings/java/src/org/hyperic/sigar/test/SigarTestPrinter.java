/*
 * Copyright (c) 2006, 2008 Hyperic, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.hyperic.sigar.test;

import java.io.PrintStream;
import java.util.HashMap;
import java.util.Properties;

import junit.framework.AssertionFailedError;
import junit.framework.Test;
import junit.framework.TestCase;
import junit.framework.TestFailure;
import junit.framework.TestSuite;

import junit.textui.ResultPrinter;
import junit.textui.TestRunner;

import org.hyperic.sigar.cmd.Version;

import org.apache.log4j.PropertyConfigurator;

public class SigarTestPrinter extends ResultPrinter {

    private HashMap failures = new HashMap();
    private int maxNameLen = 0;
    //just print once if we're run w/ "time 10 test"
    private static boolean printedVersion;
    private static final String PREFIX =
        "org.hyperic.sigar.test.";

    private static final String[][] LOG_PROPS = {
        {
            "log4j.rootLogger",
            "DEBUG, R"
        },
        {
            "log4j.appender.R",
            "org.apache.log4j.ConsoleAppender"
        },
        {
            "log4j.appender.R.layout",
            "org.apache.log4j.PatternLayout"
        },
        {
            "log4j.appender.R.layout.ConversionPattern",
            "%d [%t] %-5p %c - %m%n"
        },
    };

    public SigarTestPrinter(PrintStream writer) {
        super(writer);
    }

    //output similar to perl TestHarness
    //more interesting useful than the default '.', 'F', 'E'
    //for each test success, failure or error.
    public void startTest(Test test) {
        PrintStream writer = getWriter();
        String cls = test.getClass().getName();
        cls = cls.substring(PREFIX.length());
        String method = ((TestCase)test).getName();
        String name = cls + "." + method;
        writer.print(name);
        int n = ((maxNameLen+3) - name.length());
        for (int i=0; i<n; i++) {
            writer.print('.');
        }
    }

    public void addFailure(Test test, AssertionFailedError t) {
        this.failures.put(test, Boolean.TRUE);
        getWriter().println("FAILED");
    }

    public void addError(Test test, Throwable t) {
        this.failures.put(test, Boolean.TRUE);
        getWriter().println("ERROR");
    }

    public void endTest(Test test) {
        if (this.failures.get(test) != Boolean.TRUE) {
            getWriter().println("ok");
        }
    }

    protected void printDefectHeader(TestFailure failure, int count) {
        getWriter().println(count + ") " +
                            failure.failedTest().getClass().getName() + ":");
    }

    public void printVersionInfo() {
        if (printedVersion) {
            return;
        }
        else {
            printedVersion = true;
        }

        PrintStream writer = getWriter();

        Version.printInfo(writer);

        writer.println("");
    }

    public static void addTest(SigarTestPrinter printer,
                               TestSuite suite,
                               Class test) {

        int len = test.getName().length();

        if (len > printer.maxNameLen) {
            printer.maxNameLen = len;
        }

        suite.addTestSuite(test);
    }

    private static Class findTest(Class[] tests, String name) {
        String tname = "Test" + name;

        for (int i=0; i<tests.length; i++) {
            if (tests[i].getName().endsWith(tname)) {
                return tests[i];
            }
        }
        
        return null;
    }
    
    public static void runTests(Class[] tests, String[] args) {
        TestSuite suite = new TestSuite("Sigar tests");

        SigarTestPrinter printer = new SigarTestPrinter(System.out);

        printer.printVersionInfo();

        if (args.length > 0) {
            Properties props = new Properties();
            for (int i=0; i<LOG_PROPS.length; i++) {
                props.setProperty(LOG_PROPS[i][0],
                                  LOG_PROPS[i][1]);
            }
            PropertyConfigurator.configure(props);

            SigarTestCase.setVerbose(true);
            SigarTestCase.setWriter(printer.getWriter());

            for (int i=0; i<args.length; i++) {
                Class test = findTest(tests, args[i]);
                if (test == null) {
                    String msg = "Invalid test: " + args[i];
                    throw new IllegalArgumentException(msg);
                }

                addTest(printer, suite, test);
            }
        }
        else {
            for (int i=0; i<tests.length; i++) {
                addTest(printer, suite, tests[i]);
            }
        }

        TestRunner runner = new TestRunner(printer);

        runner.doRun(suite);
    }
}
