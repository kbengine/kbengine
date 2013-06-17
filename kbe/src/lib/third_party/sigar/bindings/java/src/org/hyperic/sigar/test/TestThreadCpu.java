/*
 * Copyright (c) 2006-2007 Hyperic, Inc.
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

import org.hyperic.sigar.CpuPerc;
import org.hyperic.sigar.CpuTimer;
import org.hyperic.sigar.Sigar;
import org.hyperic.sigar.SigarNotImplementedException;
import org.hyperic.sigar.ThreadCpu;

public class TestThreadCpu extends SigarTestCase {

    public TestThreadCpu(String name) {
        super(name);
    }

    public void testCreate() throws Exception {
        Sigar sigar = getSigar();

        ThreadCpu cpu;
        try {
            cpu = sigar.getThreadCpu();
        } catch (SigarNotImplementedException e) {
            return;
        }

        assertGtEqZeroTrace("User", cpu.getUser());

        assertGtEqZeroTrace("Sys", cpu.getSys());

        assertGtEqZeroTrace("Total", cpu.getTotal());

        CpuTimer timer = new CpuTimer(sigar);
        timer.start();

        for (int i=0; i<1000000; i++) {
            System.getProperty("java.home");
        }

        String sleepTime =
            System.getProperty("sigar.testThreadCpu.sleep");
        if (sleepTime != null) {
            Thread.sleep(Integer.parseInt(sleepTime) * 1000);
        }
        timer.stop();

        traceln("\nUsage...\n");

        assertGtEqZeroTrace("User", timer.getCpuUser());

        assertGtEqZeroTrace("Sys", timer.getCpuSys());

        assertGtEqZeroTrace("Total", timer.getCpuTotal());

        assertGtEqZeroTrace("Real Time", timer.getTotalTime());

        traceln("Cpu Percent=" + CpuPerc.format(timer.getCpuUsage()));
    }
}
