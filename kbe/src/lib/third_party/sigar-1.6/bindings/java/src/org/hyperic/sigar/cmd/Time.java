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
import org.hyperic.sigar.CpuTimer;

public class Time extends SigarCommandBase {

    public Time(Shell shell) {
        super(shell);
    }

    public Time() {
        super();
    }

    protected boolean validateArgs(String[] args) {
        return args.length >= 1;
    }

    public String getSyntaxArgs() {
        return "[command] [...]";
    }

    public String getUsageShort() {
        return "Time command";
    }

    public void output(String[] args) throws SigarException {
        boolean isInteractive = this.shell.isInteractive();
        //turn off paging.
        this.shell.setInteractive(false);
        CpuTimer cpu = new CpuTimer(this.sigar);

        int num;
        
        if (Character.isDigit(args[0].charAt(0))) {
            num = Integer.parseInt(args[0]);
            String[] xargs = new String[args.length-1];
            System.arraycopy(args, 1, xargs, 0, xargs.length);
            args = xargs;
        }
        else {
            num = 1;
        }

        cpu.start();

        try {
            for (int i=0; i<num; i++) {
                this.shell.handleCommand("time " + args[0], args);
            }
        } finally {
            this.shell.setInteractive(isInteractive);
        }

        cpu.stop();
        cpu.list(this.out);
    }

    public static void main(String[] args) throws Exception {
        new Time().processCommand(args);
    }
}
