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

public class ShellCommandBase implements ShellCommandHandler {

    protected String itsCommandName = null;
    protected ShellBase itsShell = null;

    private PrintStream out = null;
    public String getCommandName() { return itsCommandName; }
    public ShellBase getShell() { return itsShell; }

    public PrintStream getOutStream() { 
        return this.getShell().getOutStream(); 
    }

    public PrintStream getErrStream() { 
        return this.getShell().getErrStream(); 
    }

    public void init(String commandName, ShellBase shell)
        throws ShellCommandInitException {
        itsCommandName = commandName;
        itsShell = shell;
    }

    public void processCommand(String[] args) 
        throws ShellCommandUsageException, ShellCommandExecException {

        out.println("ShellCommandBase: not implemented: " + itsCommandName);
        /*
        if (args != null && args.trim().length() > 0) {
            out.println("args were: " + args);
        }
        */
    }

    public String getSyntax() {
        return "Syntax: " + this.getCommandName() + " " + this.getSyntaxArgs();
    }

    public String getSyntaxArgs() {
        return "";
    }

    public String getUsageShort() {
        return "";
    }

    public String getUsageHelp(String[] args) {
        return "Help not available for command " + itsCommandName;
    }
}
