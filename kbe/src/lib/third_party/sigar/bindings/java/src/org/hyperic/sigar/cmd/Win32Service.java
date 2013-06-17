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

import java.util.Arrays;
import java.util.Collection;
import java.util.List;

import org.hyperic.sigar.SigarException;
import org.hyperic.sigar.win32.Service;
import org.hyperic.sigar.win32.Win32Exception;

public class Win32Service extends SigarCommandBase {

    private static final List COMMANDS = 
        Arrays.asList(new String[] {
            "state",
            "start",
            "stop",
            "pause",
            "resume",
            "restart",
        });
    
    public Win32Service() {
        super();
    }

    public Win32Service(Shell shell) {
        super(shell);
    }

    public String getSyntaxArgs() {
        return "[name] [action]";
    }

    public String getUsageShort() {
        return "Windows service commands";
    }

    protected boolean validateArgs(String[] args) {
        return (args.length == 1) || (args.length == 2);
    }

    public Collection getCompletions() {
        try {
            return Service.getServiceNames();
        } catch (Win32Exception e) {
            return null;
        }
    }

    public void output(String[] args) throws SigarException {
        Service service = null;
        String name = args[0];
        String cmd = null;

        if (args.length == 2) {
            cmd = args[1];
        }
        
        try {
            service = new Service(name);
        
            if ((cmd == null) || cmd.equals("state")) {
                service.list(this.out);
            }
            else if (cmd.equals("start")) {
                service.start();
            }
            else if (cmd.equals("stop")) {
                service.stop();
            }
            else if (cmd.equals("pause")) {
                service.pause();
            }
            else if (cmd.equals("resume")) {
                service.resume();
            }
            else if (cmd.equals("delete")) {
                service.delete();
            }
            else if (cmd.equals("restart")) {
                service.stop(0);
                service.start();
            }
            else {
                println("Unsupported service command: " + args[1]);
                println("Valid commands: " + COMMANDS);
            }
        } finally {
            if (service != null) {
                service.close();
            }
        }
    }
}
