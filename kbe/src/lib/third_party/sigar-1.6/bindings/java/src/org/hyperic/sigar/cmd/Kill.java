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

/**
 * Send a signal to a process.
 */
public class Kill extends SigarCommandBase {

    public Kill(Shell shell) {
        super(shell);
    }

    public Kill() {
        super();
    }

    protected boolean validateArgs(String[] args) {
        return args.length == 1 || args.length == 2;
    }

    public String getSyntaxArgs() {
        return "[signal] <query|pid>";
    }

    public String getUsageShort() {
        return "Send signal to a process";
    }

    public boolean isPidCompleter() {
        return true;
    }

    public void output(String[] args) throws SigarException {
        String signal = "SIGTERM";
        long[] pids;
        String query;

        if (args.length == 2) {
            signal = args[0];
            query = args[1];
        }
        else {
            query = args[0];
        }

        pids = this.shell.findPids(new String[] { query });

        for (int i=0; i<pids.length; i++) {
            println("kill " + signal + " " + pids[i]);
            this.sigar.kill(pids[i], signal);
        }
    }

    public static void main(String[] args) throws Exception {
        new Kill().processCommand(args);
    }
}
