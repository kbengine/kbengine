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

import org.hyperic.sigar.SigarException;
import org.hyperic.sigar.SigarPermissionDeniedException;

/**
 * Display all process information.
 */
public class ProcInfo extends SigarCommandBase {

    private boolean isSingleProcess;

    public ProcInfo(Shell shell) {
        super(shell);
    }

    public ProcInfo() {
        super();
    }

    protected boolean validateArgs(String[] args) {
        return true;
    }

    public String getUsageShort() {
        return "Display all process info";
    }

    public boolean isPidCompleter() {
        return true;
    }

    public void output(String[] args) throws SigarException {
        this.isSingleProcess = false;

        if ((args.length != 0) && args[0].startsWith("-s")) {
            this.isSingleProcess = true;
        }

        if (this.isSingleProcess) {
            for (int i=1; i<args.length; i++) {
                try {
                    output(args[i]);
                } catch (SigarException e) {
                    println("(" + e.getMessage() + ")");
                }
                println("\n------------------------\n");
            }
        }
        else {
            long[] pids = this.shell.findPids(args);

            for (int i=0; i<pids.length; i++) {
                try {
                    output(String.valueOf(pids[i]));
                } catch (SigarPermissionDeniedException e) {
                    println(this.shell.getUserDeniedMessage(pids[i]));
                } catch (SigarException e) {
                    println("(" + e.getMessage() + ")");
                }
                println("\n------------------------\n");
            }
        }
    }

    public void output(String pid) throws SigarException {
        println("pid=" + pid);
        try {
            println("state=" + sigar.getProcState(pid));
        } catch (SigarException e) {
            if (this.isSingleProcess) {
                println(e.getMessage());
            }
        }
        try {
            println("mem=" + sigar.getProcMem(pid));
        } catch (SigarException e) {}
        try {
            println("cpu=" + sigar.getProcCpu(pid));
        } catch (SigarException e) {}
        try {
            println("cred=" + sigar.getProcCred(pid));
        } catch (SigarException e) {}
        try {
            println("credname=" + sigar.getProcCredName(pid));
        } catch (SigarException e) {}
    }

    public static void main(String[] args) throws Exception {
        new ProcInfo().processCommand(args);
    }
}
