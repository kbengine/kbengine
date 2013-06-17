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

package org.hyperic.sigar.cmd;

import org.hyperic.sigar.SigarException;
import org.hyperic.sigar.SigarNotImplementedException;
import org.hyperic.sigar.SigarPermissionDeniedException;
import org.hyperic.sigar.ProcFd;
import org.hyperic.sigar.ProcExe;

/**
 * Display process file information.
 */
public class ProcFileInfo extends SigarCommandBase {

    public ProcFileInfo(Shell shell) {
        super(shell);
    }

    public ProcFileInfo() {
        super();
    }

    protected boolean validateArgs(String[] args) {
        return true;
    }

    public String getUsageShort() {
        return "Display process file info";
    }

    public boolean isPidCompleter() {
        return true;
    }

    public void output(String[] args) throws SigarException {
        long[] pids = this.shell.findPids(args);

        for (int i=0; i<pids.length; i++) {
            try {
                output(pids[i]);
            } catch (SigarPermissionDeniedException e) {
                println(this.shell.getUserDeniedMessage(pids[i]));
            } catch (SigarException e) {
                println("(" + e.getMessage() + ")");
            }
            println("\n------------------------\n");
        }
    }

    public void output(long pid) throws SigarException {
        println("pid=" + pid);

        try {
            ProcFd fd = sigar.getProcFd(pid);
            println("open file descriptors=" + fd.getTotal());
        } catch (SigarNotImplementedException e) {}

        ProcExe exe = sigar.getProcExe(pid);
        String name = exe.getName();
        if (name.length() == 0) {
            name = "unknown";
        }
        println("name=" + name);

        println("cwd=" + exe.getCwd());
    }

    public static void main(String[] args) throws Exception {
        new ProcFileInfo().processCommand(args);
    }
}
