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
import java.util.Map;
import java.util.Set;

import org.hyperic.sigar.util.PrintfFormat;

public class MultiwordShellCommand extends ShellCommandBase {

    private Map itsSubHandlerMap = new HashMap();

    public ShellCommandHandler getSubHandler(String subName) {
        return (ShellCommandHandler)itsSubHandlerMap.get(subName);
    }

    public Set getHandlerNames() {
        return this.itsSubHandlerMap.keySet();
    }

    public void registerSubHandler(String subName, 
                                   ShellCommandHandler handler) 
        throws ShellCommandInitException {

        if (!itsSubHandlerMap.containsValue(handler)) {
            // Only init the handler if it has not been added yet.
            // We do this because a single handler could be
            // registered for multiple subName's (as in the case 
            // of aliasing).
            handler.init(getCommandName() + " " + subName, getShell());
        }

        itsSubHandlerMap.put(subName, handler);
    }

    public void processCommand(String[] args) 
        throws ShellCommandUsageException, ShellCommandExecException 
    {
        String cmdName = getCommandName();
        ShellCommandHandler handler;
        String[] subArgs;

        if (args.length < 1) {
            throw new ShellCommandUsageException(cmdName + " command " +
                                                 "requires an argument.");
        }

        handler = (ShellCommandHandler) 
            itsSubHandlerMap.get(args[0].toLowerCase());

        if (handler == null) {
            throw new ShellCommandUsageException("don't know how to " +
                                                 cmdName + " " + args[0]);
        }

        subArgs = new String[args.length - 1];
        System.arraycopy(args, 1, subArgs, 0, subArgs.length);
        handler.processCommand(subArgs);
    }

    public String getSyntaxArgs() {
        StringBuffer res = new StringBuffer();

        res.append("<");
        for (Iterator i=this.getHandlerNames().iterator(); i.hasNext();) {
            res.append((String)i.next());

            if (i.hasNext()) {
                res.append(" | ");
            }
        }
        res.append(">");
        return res.toString();
    }

    public String getUsageHelp(String[] args) {
        ShellCommandHandler handler;
        String[] subArgs;

        if (args.length == 0) {
            StringBuffer res = new StringBuffer();
            Object[] fArgs = new Object[2];
            PrintfFormat fmt;
            String fmtStr;
            int maxLen;

            res.append("    " + this.getUsageShort());
            res.append(".\n    For further help on each subcommand, ");
            res.append("type 'help ");
            res.append(this.getCommandName() + " <subcommand>'\n\n");

            maxLen = 0;
            for (Iterator i=this.getHandlerNames().iterator(); i.hasNext();) {
                String cmdName = (String)i.next();
                
                if (cmdName.length() > maxLen)
                    maxLen = cmdName.length();
            }

            fmtStr = "      %-" + (maxLen + 1) + "s %s";
            fmt = new PrintfFormat(fmtStr);
            for (Iterator i=this.getHandlerNames().iterator(); i.hasNext();) {
                String cmdName = (String)i.next();
                ShellCommandHandler sub = this.getSubHandler(cmdName);

                fArgs[0] = cmdName + ":";
                fArgs[1] = sub.getUsageShort();

                res.append(fmt.sprintf(fArgs));
                if (i.hasNext())
                    res.append("\n");
            }
            return res.toString();
        }

        if ((handler = getSubHandler(args[0].toLowerCase())) == null) {
            return null;
        }
        subArgs = new String[args.length - 1];
        System.arraycopy(args, 1, subArgs, 0, subArgs.length);
        return handler.getUsageHelp(subArgs);
    }
}
