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

import org.hyperic.sigar.Sigar;
import org.hyperic.sigar.SigarException;
import org.hyperic.sigar.ProcState;

public class TestProcState extends SigarTestCase {

    public TestProcState(String name) {
        super(name);
    }

    private void traceState(Sigar sigar, long pid) {
        try {
            ProcState procState =
                sigar.getProcState(pid);
            traceln("[pid=" + pid + "] " + procState);
        } catch (SigarException e) {
            traceln("pid " + pid + ": " + e.getMessage());
        }
    }

    public void testCreate() throws Exception {
        Sigar sigar = getSigar();

        try {
            sigar.getProcState(getInvalidPid());
        } catch (SigarException e) {
        }

        ProcState procState = sigar.getProcState(sigar.getPid());
        traceState(sigar, sigar.getPid());

        char state = procState.getState();
        assertTrue((state == 'R') || (state == 'S'));
        assertTrue(procState.getName().indexOf("java") != -1);

        long[] pids = sigar.getProcList();
        for (int i=0; i<pids.length; i++) {
            traceState(sigar, pids[i]);
        }
    }
}
