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

package org.hyperic.sigar.cmd;

import org.hyperic.sigar.Sigar;
import org.hyperic.sigar.SigarException;
import org.hyperic.sigar.CpuPerc;
import org.hyperic.sigar.MultiProcCpu;
import org.hyperic.sigar.ProcMem;

/**
 * Show multi process status.
 */
public class MultiPs extends SigarCommandBase {

    public MultiPs(Shell shell) {
        super(shell);
    }

    public MultiPs() {
        super();
    }

    protected boolean validateArgs(String[] args) {
        return args.length == 1;
    }

    public String getSyntaxArgs() {
        return "query";
    }

    public String getUsageShort() {
        return "Show multi process status";
    }

    public boolean isPidCompleter() {
        return true;
    }

    public void output(String[] args) throws SigarException {
        String query = args[0];
        MultiProcCpu cpu = this.proxy.getMultiProcCpu(query);
        println("Number of processes: " + cpu.getProcesses());
        println("Cpu usage: " + CpuPerc.format(cpu.getPercent()));
        println("Cpu time: "  + Ps.getCpuTime(cpu.getTotal()));

        ProcMem mem = this.proxy.getMultiProcMem(query);
        println("Size: " + Sigar.formatSize(mem.getSize()));
        println("Resident: " + Sigar.formatSize(mem.getResident()));
        println("Share: " + Sigar.formatSize(mem.getShare()));
    }

    public static void main(String[] args) throws Exception {
        new MultiPs().processCommand(args);
    }
}

            
