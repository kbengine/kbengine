/*
 * Copyright (c) 2006-2007 Hyperic, Inc.
 * Copyright (c) 2010 VMware, Inc.
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

import java.util.Date;

import org.hyperic.sigar.Sigar;
import org.hyperic.sigar.SigarException;
import org.hyperic.sigar.ProcCpu;

public class TestProcTime extends SigarTestCase {

    public TestProcTime(String name) {
        super(name);
    }

    public void testCreate() throws Exception {
        Sigar sigar = getSigar();

        try {
            sigar.getProcTime(getInvalidPid());
        } catch (SigarException e) {
        }

        ProcCpu procTime = sigar.getProcCpu(sigar.getPid());

        assertGtEqZeroTrace("StartTime", procTime.getStartTime());
        traceln("StartDate=" + new Date(procTime.getStartTime()));
        //XXX
        //assertTrue(procTime.getStartTime() < System.currentTimeMillis());

        assertGtEqZeroTrace("User", procTime.getUser());

        assertGtEqZeroTrace("Sys", procTime.getSys());

        assertGtEqZeroTrace("Total", procTime.getTotal());

        double value = procTime.getPercent() * 100.0;
        traceln("Percent=" + value);
        assertTrue(value >= 0.0);
        int ncpu = sigar.getCpuList().length;
        assertTrue(value <= (100.0 * ncpu)); //SIGAR-145 Irix mode
    }
}
