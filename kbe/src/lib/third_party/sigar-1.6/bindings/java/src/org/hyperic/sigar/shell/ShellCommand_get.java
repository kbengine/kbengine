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

package org.hyperic.sigar.shell;

import java.io.PrintStream;

public class ShellCommand_get extends ShellCommandBase {

    private void printProperty(String key, String value) {
        PrintStream out = getOutStream();

        out.print(key + "=");

        if (value.trim() != value) {
            out.println("'" + value + "'");
        }
        else {
            out.println(value);
        }
    }

    public void processCommand(String[] args)
        throws ShellCommandUsageException, ShellCommandExecException 
    {
        if (args.length < 1) {
            throw new ShellCommandUsageException(this.getSyntax());
        }

        for (int i=0; i<args.length; i++) {
            String val = System.getProperty(args[i], "UNDEFINED");

            printProperty(args[i], val);
        }
    }

    public String getSyntaxArgs() {
        return "<key1> [key2] ...";
    }

    public String getUsageShort(){
        return "Get system properties";
    }

    public String getUsageHelp(String[] args) {
        return "    " + getUsageShort() + ".";
    }
}
