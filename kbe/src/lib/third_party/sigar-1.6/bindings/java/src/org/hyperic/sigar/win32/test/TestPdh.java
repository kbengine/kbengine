/*
 * Copyright (c) 2006-2007 Hyperic, Inc.
 * Copyright (c) 2009-2010 VMware, Inc.
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

package org.hyperic.sigar.win32.test;

import java.util.Iterator;
import java.util.Map;

import org.hyperic.sigar.test.SigarTestCase;
import org.hyperic.sigar.win32.Pdh;

public class TestPdh extends SigarTestCase {

    public TestPdh(String name) {
        super(name);
    }

    private static boolean isCounter(long type) {
        return (type & Pdh.PERF_TYPE_COUNTER) == Pdh.PERF_TYPE_COUNTER;
    }

    private void getValue(String key) throws Exception {
        Pdh pdh = new Pdh();

        traceln(key + ": " + pdh.getDescription(key));
        traceln("counter=" + isCounter(pdh.getCounterType(key)));
        assertGtEqZeroTrace("raw",
                            (long)pdh.getRawValue(key));
        assertGtEqZeroTrace("fmt",
                            (long)pdh.getFormattedValue(key));
    }

    public void testGetValue() throws Exception {
        Pdh.enableTranslation();
        final String DL = "\\";
        String[][] keys = {
            { "System", "System Up Time" },
            { "Memory", "Available Bytes" },
            { "Memory", "Pages/sec" },
            { "Processor(_Total)", "% User Time" },
        };

        for (int i=0; i<keys.length; i++) {
            String path =
                DL + keys[i][0] + DL + keys[i][1];

            String trans = Pdh.translate(path);
            if (!trans.equals(path)) {
                traceln(path + "-->" + trans);
            }
            traceln(path + " validate: " + Pdh.validate(path));
            getValue(path);
        }
    }

    public void testCounterMap() throws Exception {
        Map counters = Pdh.getEnglishPerflibCounterMap();

        assertGtZeroTrace("counters", counters.size());
        int dups = 0;
        for (Iterator it=counters.entrySet().iterator(); it.hasNext();) {
            Map.Entry entry = (Map.Entry)it.next();
            String name = (String)entry.getKey();
            int[] ix = (int[])entry.getValue();
            if (ix.length > 1) {
                dups++;
                //traceln(name + " has dups: " + ix.length);
            }
        }
        traceln(dups + " names have dups");

        String[] keys = {
            "System", "System Up Time"
        };
        int last = -1;
        for (int i=0; i<keys.length; i++) {
            String name = keys[i];
            int[] ix =
                (int[])counters.get(name.toLowerCase());
            assertFalse(ix[0] == last);
            traceln(name + "=" + ix[0]);
            last = ix[0];
            String lookupName =
                Pdh.getCounterName(ix[0]);
            traceln(name + "=" + lookupName);
        }
    }

    public void testValidate() {
        Object[][] tests = {
            { "\\Does Not\\Exist", new Integer(Pdh.NO_OBJECT), new Integer(Pdh.BAD_COUNTERNAME) },
            { "Does Not Exist", new Integer(Pdh.BAD_COUNTERNAME) },
            { "\\System\\DoesNotExist", new Integer(Pdh.NO_COUNTER) },
            { "\\Processor(666)\\% User Time", new Integer(Pdh.NO_INSTANCE) },
            { "\\System\\Threads", new Integer(Pdh.VALID_DATA), new Integer(Pdh.BAD_COUNTERNAME) },
            //slow
            //{ "\\\\-\\System\\Threads", new Integer(Pdh.NO_MACHINE) },
        };

        for (int i=0; i<tests.length; i++) {
            String path = (String)tests[i][0];
            int expect = ((Integer)tests[i][1]).intValue();
            int status = Pdh.validate(path);
            boolean expectedResult = (status == expect);

            if (!expectedResult) {
                if (tests[i].length == 3) {
                    expect = ((Integer)tests[i][2]).intValue();
                    expectedResult = (status == expect);
                }
            }

            if (!expectedResult) {
                traceln("[validate] " + path + "-->" +
                        Integer.toHexString(status).toUpperCase() +
                        " != " +
                        Integer.toHexString(expect).toUpperCase());
            }

            assertTrue(expectedResult);
        }
    }

    public void testPdh() throws Exception {

        String[] iface = Pdh.getKeys("Thread");
        
        assertTrue(iface.length > 0);
    }
}
