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

public class ShellCommand_sleep extends ShellCommandBase {

    public ShellCommand_sleep() {}

    public void processCommand(String[] args)
        throws ShellCommandUsageException, ShellCommandExecException 
    {
        if (args.length != 1) {
            throw new ShellCommandUsageException(getSyntax());
        }

        try {
            Thread.sleep(Integer.parseInt(args[0]) * 1000);
        } catch(NumberFormatException exc) {
            throw new ShellCommandExecException("Invalid time '" + args[0] + 
                                                "' -- must be an integer");
        } catch(InterruptedException exc) {
            throw new ShellCommandExecException("Sleep interrupted");
        }
    }

    public String getSyntaxArgs() {
        return "<numSeconds>";
    }

    public String getUsageShort() {
        return "Delay execution for the a number of seconds ";
    }

    public String getUsageHelp(String[] args) {
        return "    " + getUsageShort() + ".";
    }
}
