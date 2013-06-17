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

import java.util.HashMap;
import java.util.Iterator;

public class ShellCommand_set extends ShellCommandBase {
    private HashMap keyDescriptions = new HashMap();

    public ShellCommand_set() {
        this.keyDescriptions = new HashMap();
        this.keyDescriptions.put(ShellBase.PROP_PAGE_SIZE,
                                 "The maximum size of a shell page");
    }

    public void processCommand(String[] args)
        throws ShellCommandUsageException, ShellCommandExecException 
    {
        if (args.length < 1 || args.length > 2) {
            throw new ShellCommandUsageException(this.getSyntax());
        }

        if (args.length == 1) {
            System.getProperties().remove(args[0]);
        }
        else {
            if (args[0].equalsIgnoreCase(ShellBase.PROP_PAGE_SIZE)) {
                int newSize;

                try {
                    newSize = Integer.parseInt(args[1]);
                    if (newSize == 0 || newSize < -1) {
                        throw new NumberFormatException();
                    }
                } catch(NumberFormatException exc) {
                    throw new ShellCommandUsageException(args[0] + " must be "+
                                                         "an integer > 0 or " +
                                                         "-1");
                }
                this.getShell().setPageSize(newSize);
            } 

            System.setProperty(args[0], args[1]);
        }
    }

    public void addSetKey(String key, String description) {
        this.keyDescriptions.put(key, description);
    }

    public String getSyntaxArgs() {
        return "<key> [value]";
    }

    public String getUsageShort() {
        return "Set system properties";
    }

    public String getUsageHelp(String[] args) {
        String res =
            "    " + this.getUsageShort() +
            ".  If no value is provided, " +
            "the key will be\n    deleted.";

        if (this.keyDescriptions.size() != 0) {
            res += "\n\n    Common keys include:";
        }

        for (Iterator i=this.keyDescriptions.keySet().iterator();
             i.hasNext();)
        {
            String key = (String)i.next();
            String value = (String)this.keyDescriptions.get(key);

            res += "\n      " + key + ": " + value;
        }

        return res;
    }
}
