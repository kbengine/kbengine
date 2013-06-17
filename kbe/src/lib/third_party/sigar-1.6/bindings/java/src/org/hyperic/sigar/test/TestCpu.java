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

import org.hyperic.sigar.Sigar;
import org.hyperic.sigar.SigarException;
import org.hyperic.sigar.SigarNotImplementedException;
import org.hyperic.sigar.Cpu;
import org.hyperic.sigar.CpuPerc;

public class TestCpu extends SigarTestCase {

    public TestCpu(String name) {
        super(name);
    }

    private void checkCpu(Cpu cpu) {
        traceln("User..." + cpu.getUser());
        assertTrue(cpu.getUser() >= 0);

        traceln("Sys...." + cpu.getSys());
        assertTrue(cpu.getSys() >= 0);

        traceln("Idle..." + cpu.getIdle());
        assertTrue(cpu.getIdle() >= 0);

        traceln("Wait..." + cpu.getWait());
        assertTrue(cpu.getWait() >= 0);

        traceln("Irq..." + cpu.getIrq());
        assertTrue(cpu.getIrq() >= 0);

        traceln("SIrq.." + cpu.getSoftIrq());
        assertTrue(cpu.getSoftIrq() >= 0);

        traceln("Stl..." + cpu.getStolen());
        assertTrue(cpu.getStolen() >= 0);

        traceln("Total.." + cpu.getTotal());
        assertTrue(cpu.getTotal() > 0);

        try {
            long current =
                getSigar().getProcState("$$").getProcessor();
            traceln("last run cpu=" + current);
        } catch (SigarException e) {
            e.printStackTrace();
        }
    }

    public void testCreate() throws Exception {
        Sigar sigar = getSigar();
        Cpu cpu = sigar.getCpu();

        traceln("getCpu:");
        checkCpu(cpu);

        try {
            Cpu[] cpuList = sigar.getCpuList();

            for (int i=0; i<cpuList.length; i++) {
                traceln("Cpu " + i + ":");
                checkCpu(cpuList[i]);
            }
        } catch (SigarNotImplementedException e) {
            //ok
        }
    }

    private static void printCpu(String prefix, CpuPerc cpu) {
        System.out.println(prefix +
                           CpuPerc.format(cpu.getUser()) + "\t" +
                           CpuPerc.format(cpu.getSys()) + "\t" +
                           CpuPerc.format(cpu.getWait()) + "\t" +
                           CpuPerc.format(cpu.getNice()) + "\t" +
                           CpuPerc.format(cpu.getIdle()) + "\t" +
                           CpuPerc.format(cpu.getCombined()));
    }

    public static void main(String[] args) throws Exception {
        final String HEADER =
            "   User\tSys\tWait\tNice\tIdle\tTotal";
        int interval = 1;
        if (args.length > 0) {
            interval = Integer.parseInt(args[0]);
        }
        int sleep = 1000 * 60 * interval;

        Sigar sigar = new Sigar();

        while (true) {
            System.out.println(HEADER);

            printCpu("   ", sigar.getCpuPerc());

            CpuPerc[] cpuList = sigar.getCpuPercList();

            for (int i=0; i<cpuList.length; i++) {
                printCpu(i+": ", cpuList[i]);
            }
            Thread.sleep(sleep);
        }
    }
}
