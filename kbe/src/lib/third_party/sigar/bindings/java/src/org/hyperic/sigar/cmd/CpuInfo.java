/*
 * Copyright (c) 2006-2008 Hyperic, Inc.
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

package org.hyperic.sigar.cmd;

import org.hyperic.sigar.CpuPerc;
import org.hyperic.sigar.Sigar;
import org.hyperic.sigar.SigarLoader;
import org.hyperic.sigar.SigarException;

/**
 * Display cpu information for each cpu found on the system.
 */
public class CpuInfo extends SigarCommandBase {

    public boolean displayTimes = true;
    
    public CpuInfo(Shell shell) {
        super(shell);
    }

    public CpuInfo() {
        super();
    }

    public String getUsageShort() {
        return "Display cpu information";
    }

    private void output(CpuPerc cpu) {
        println("User Time....." + CpuPerc.format(cpu.getUser()));
        println("Sys Time......" + CpuPerc.format(cpu.getSys()));
        println("Idle Time....." + CpuPerc.format(cpu.getIdle()));
        println("Wait Time....." + CpuPerc.format(cpu.getWait()));
        println("Nice Time....." + CpuPerc.format(cpu.getNice()));
        println("Combined......" + CpuPerc.format(cpu.getCombined()));
        println("Irq Time......" + CpuPerc.format(cpu.getIrq()));
        if (SigarLoader.IS_LINUX) {
            println("SoftIrq Time.." + CpuPerc.format(cpu.getSoftIrq()));
            println("Stolen Time...." + CpuPerc.format(cpu.getStolen()));
        }
        println("");
    }

    public void output(String[] args) throws SigarException {
        org.hyperic.sigar.CpuInfo[] infos =
            this.sigar.getCpuInfoList();

        CpuPerc[] cpus =
            this.sigar.getCpuPercList();

        org.hyperic.sigar.CpuInfo info = infos[0];
        long cacheSize = info.getCacheSize();
        println("Vendor........." + info.getVendor());
        println("Model.........." + info.getModel());
        println("Mhz............" + info.getMhz());
        println("Total CPUs....." + info.getTotalCores());
        if ((info.getTotalCores() != info.getTotalSockets()) ||
            (info.getCoresPerSocket() > info.getTotalCores()))
        {
            println("Physical CPUs.." + info.getTotalSockets());
            println("Cores per CPU.." + info.getCoresPerSocket());
        }

        if (cacheSize != Sigar.FIELD_NOTIMPL) {
            println("Cache size...." + cacheSize);
        }
        println("");

        if (!this.displayTimes) {
            return;
        }

        for (int i=0; i<cpus.length; i++) {
            println("CPU " + i + ".........");
            output(cpus[i]);
        }

        println("Totals........");
        output(this.sigar.getCpuPerc());
    }

    public static void main(String[] args) throws Exception {
        new CpuInfo().processCommand(args);
    }
}
