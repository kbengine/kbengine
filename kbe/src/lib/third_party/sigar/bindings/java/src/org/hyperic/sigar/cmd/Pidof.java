/*
 * Copyright (c) 2008 Hyperic, Inc.
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

public class Pidof extends SigarCommandBase {

    public Pidof(Shell shell) {
        super(shell);
    }

    public Pidof() {
        super();
    }

    protected boolean validateArgs(String[] args) {
        return args.length > 0;
    }

    public String getSyntaxArgs() {
        return "query";
    }

    public String getUsageShort() {
        return "Find the process ID of a running program";
    }

    public void output(String[] args) throws SigarException {
        long[] pids = this.shell.findPids(args);

        for (int i=0; i<pids.length; i++) {
            this.out.print(pids[i]);
            this.out.print(' ');
        }
        this.out.println();
    }
}
