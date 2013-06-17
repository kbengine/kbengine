/*
 * Copyright (c) 2008-2009 Hyperic, Inc.
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

import java.io.File;
import java.util.Iterator;
import java.util.Map;
import org.hyperic.sigar.SigarException;
import org.hyperic.sigar.ProcExe;
import org.hyperic.sigar.win32.Win32;
import org.hyperic.sigar.win32.FileVersion;

/**
 * Display process file information.
 */
public class FileVersionInfo extends SigarCommandBase {

    public FileVersionInfo(Shell shell) {
        super(shell);
    }

    public FileVersionInfo() {
        super();
    }

    protected boolean validateArgs(String[] args) {
        return args.length >= 1;
    }

    public String getUsageShort() {
        return "Display file version info";
    }

    public void output(String[] args) throws SigarException {
        for (int i=0; i<args.length; i++) {
            String exe = args[i];
            if (new File(exe).exists()) {
                output(exe);
            }
            else {
                long[] pids = this.shell.findPids(exe);
                for (int j=0; j<pids.length; j++) {
                    try {
                        output(sigar.getProcExe(pids[j]).getName());
                    } catch (SigarException e) {
                        println(exe + ": " + e.getMessage());
                    }
                }
            }
        }
    }

    private void output(String key, String val) {
        final int max = 20;
        int len = max - key.length();
        StringBuffer sb = new StringBuffer();
        sb.append("  ").append(key);
        while (len-- > 0) {
            sb.append('.');
        }
        sb.append(val);
        println(sb.toString());
    }

    public void output(String exe) throws SigarException {
        FileVersion info = Win32.getFileVersion(exe);
        if (info == null) {
            return;
        }
        println("Version info for file '" + exe + "':");
        output("FileVersion", info.getFileVersion());
        output("ProductVersion", info.getProductVersion());
        for (Iterator it = info.getInfo().entrySet().iterator();
             it.hasNext();)
        {
            Map.Entry entry = (Map.Entry)it.next();
            output((String)entry.getKey(), (String)entry.getValue());
        }
    }

    public static void main(String[] args) throws Exception {
        new FileVersionInfo().processCommand(args);
    }
}
