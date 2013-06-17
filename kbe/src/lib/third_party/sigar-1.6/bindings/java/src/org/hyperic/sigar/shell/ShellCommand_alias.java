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

public class ShellCommand_alias extends ShellCommandBase {

    private static HashMap aliases = new HashMap();

    public static String[] getAlias(String alias) {
        return (String[])aliases.get(alias);
    }

    public static Iterator getAliases() {
        return aliases.keySet().iterator();
    }

    public void processCommand(String[] args)
        throws ShellCommandUsageException, ShellCommandExecException 
    {
        if (args.length < 2) {
            throw new ShellCommandUsageException(this.getSyntax());
        }

        int aliasArgsLen = args.length - 1;
        String[] aliasArgs = new String[ aliasArgsLen ];
        System.arraycopy(args, 1, aliasArgs, 0, aliasArgsLen);

        aliases.put(args[0], aliasArgs);
    }

    public String getSyntaxArgs() {
        return "<alias> <command>";
    }

    public String getUsageShort() {
        return "Create alias command";
    }

    public String getUsageHelp(String[] args) {
        if (aliases.size() == 0) {
            return "No aliases defined";
        }

        StringBuffer sb = new StringBuffer();
        sb.append("Defined aliases:\n");

        for (Iterator it=aliases.keySet().iterator();
             it.hasNext(); )
        {
            String key = (String)it.next();
            String[] cmd = getAlias(key);
            sb.append(key).append(" => ");
            for (int i=0; i<cmd.length; i++) {
                sb.append(cmd[i]).append(" ");
            }
            sb.append("\n");
        }

        return sb.toString();
    }
}
