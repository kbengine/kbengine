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
import org.hyperic.sigar.ProcMem;

public class TestProcMem extends SigarTestCase {

    public TestProcMem(String name) {
        super(name);
    }

    private void traceMem(Sigar sigar, long pid) throws Exception {
        ProcMem procMem;

        try {
            procMem = sigar.getProcMem(pid);
        } catch (SigarException e) {
            traceln("pid " + pid + ": " + e.getMessage());
            return;
        }

        traceln("Pid=" + pid);
        traceln("Size=" + Sigar.formatSize(procMem.getSize()));
        traceln("Resident=" + Sigar.formatSize(procMem.getResident()));
        traceln("Share=" + Sigar.formatSize(procMem.getShare()));
        traceln("MinorFaults=" + procMem.getMinorFaults());
        traceln("MajorFaults=" + procMem.getMajorFaults());
        traceln("PageFaults=" + procMem.getPageFaults());
        //assertTrue(procMem.getSize() > 0);
        // XXX vsize, resident, share, rss
    }

    public void testCreate() throws Exception {
        Sigar sigar = getSigar();

        try {
            sigar.getProcMem(getInvalidPid());
        } catch (SigarException e) {
        }

        long[] pids = sigar.getProcList();
        for (int i=0; i<pids.length; i++) {
            traceMem(sigar, pids[i]);
        }
    }
}
