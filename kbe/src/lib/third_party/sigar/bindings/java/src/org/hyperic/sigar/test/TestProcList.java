/*
 * Copyright (c) 2006 Hyperic, Inc.
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

import java.util.ArrayList;

import org.hyperic.sigar.Sigar;

public class TestProcList extends SigarTestCase {

    public TestProcList(String name) {
        super(name);
    }

    public void testCreate() throws Exception {
        Sigar sigar = getSigar();

        ArrayList traceList = new ArrayList();

        long[] pids = sigar.getProcList();

        assertTrue(pids.length > 1);

        long pid = sigar.getPid();
        boolean foundPid = false;

        //find our pid in the process list
        for (int i=0; i<pids.length; i++) {
            traceList.add(new Long(pids[i]));
            if (pid == pids[i]) {
                foundPid = true;
            }
        }

        traceln("pids=" + traceList);

        assertTrue(foundPid);
    }
}
