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

/**
 * Show process command line arguments.
 */
public class ShowArgs extends SigarCommandBase {

    public ShowArgs(Shell shell) {
        super(shell);
    }

    public ShowArgs() {
        super();
    }

    protected boolean validateArgs(String[] args) {
        return true;
    }

    public String getUsageShort() {
        return "Show process command line arguments";
    }

    public boolean isPidCompleter() {
        return true;
    }

    public void output(String[] args) throws SigarException {
        long[] pids = this.shell.findPids(args);

        for (int i=0; i<pids.length; i++) {
            try {
                println("pid=" + pids[i]);
                output(pids[i]);
            } catch (SigarException e) {
                println(e.getMessage());
            }
            println("\n------------------------\n");
        }
    }

    public void output(long pid) throws SigarException {

        String[] argv = this.proxy.getProcArgs(pid);
        
        try {
            String exe = this.proxy.getProcExe(pid).getName();
            println("exe=" + exe);
        } catch (SigarNotImplementedException e) {
        } catch (SigarException e) {
            println("exe=???");
        }

        try {
            String cwd = this.proxy.getProcExe(pid).getCwd();
            println("cwd=" + cwd);
        } catch (SigarNotImplementedException e) {
        } catch (SigarException e) {
            println("cwd=???");
        }

        for (int i=0; i<argv.length; i++) {
            println("   " + i + "=>" + argv[i] + "<=");
        }
    }

    public static void main(String[] args) throws Exception {
        new ShowArgs().processCommand(args);
    }
}
