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

import org.hyperic.sigar.ResourceLimit;
import org.hyperic.sigar.SigarException;
import org.hyperic.sigar.jmx.SigarInvokerJMX;

/**
 * Display system resource limits.
 */
public class Ulimit extends SigarCommandBase {

    private SigarInvokerJMX invoker;
    private String mode;
    
    public Ulimit(Shell shell) {
        super(shell);
    }

    public Ulimit() {
        super();
    }

    public String getUsageShort() {
        return "Display system resource limits";
    }

    protected boolean validateArgs(String[] args) {
        return true;
    }

    private static String format(long val) {
        if (val == ResourceLimit.INFINITY()) {
            return "unlimited";
        }
        else {
            return String.valueOf(val);
        }
    }

    private String getValue(String attr)
        throws SigarException {
        Long val = (Long)this.invoker.invoke(attr + this.mode);
        return format(val.longValue());
    }
    
    public void output(String[] args) throws SigarException {

        this.mode = "Cur";
        this.invoker =
            SigarInvokerJMX.getInstance(this.proxy, "Type=ResourceLimit");

        for (int i=0; i<args.length; i++) {
            String arg = args[i];
            if (arg.equals("-H")) {
                this.mode = "Max";
            }
            else if (arg.equals("-S")) {
                this.mode = "Cur";
            }
            else {
                throw new SigarException("Unknown argument: " + arg);
            }
        }
        
        println("core file size......." + getValue("Core"));
        println("data seg size........" + getValue("Data"));
        println("file size............" + getValue("FileSize"));
        println("pipe size............" + getValue("PipeSize"));
        println("max memory size......" + getValue("Memory"));
        println("open files..........." + getValue("OpenFiles"));
        println("stack size..........." + getValue("Stack"));
        println("cpu time............." + getValue("Cpu"));
        println("max user processes..." + getValue("Processes"));
        println("virtual memory......." + getValue("VirtualMemory"));
    }

    public static void main(String[] args) throws Exception {
        new Ulimit().processCommand(args);
    }
}
