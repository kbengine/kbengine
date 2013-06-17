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

import java.io.File;
import java.io.IOException;

import org.hyperic.sigar.util.GetlineCompleter;

public class ShellCommand_source
    extends ShellCommandBase
    implements GetlineCompleter {

    public String complete(String line) {
        return new FileCompleter(getShell()).complete(line);
    }

    public void processCommand(String[] args) 
        throws ShellCommandUsageException, ShellCommandExecException 
    {
        File rcFile;

        if(args.length != 1){
            throw new ShellCommandUsageException("Syntax: " + 
                                                 this.getCommandName() + 
                                                 " <rcfile>");
        }

        rcFile = new File(FileCompleter.expand(args[0]));
        
        if(rcFile.isFile() == false){
            throw new ShellCommandExecException("File '" + rcFile + 
                                                "' not found");
        }

        try {
            this.getShell().readRCFile(rcFile, true);
        } catch(IOException exc){
            throw new ShellCommandExecException("Error reading file '" + 
                                                rcFile + ": " + 
                                                exc.getMessage());
        }
    }

    public String getSyntaxArgs(){
        return "<rcfile>";
    }

    public String getUsageShort(){
        return "Read a file, executing the contents";
    }

    public String getUsageHelp(String[] args) {
        return "    " + this.getUsageShort() + ".  The file must contain " +
            "commands\n    which are executable by the shell.";
    }
}
