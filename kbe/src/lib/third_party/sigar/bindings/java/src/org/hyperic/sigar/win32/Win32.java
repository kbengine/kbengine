/*
 * Copyright (c) 2006-2008 Hyperic, Inc.
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

package org.hyperic.sigar.win32;

import java.io.File;
import org.hyperic.sigar.Sigar;
import org.hyperic.sigar.SigarException;

public abstract class Win32 {

    public static final String EXE_EXT = ".exe";

    static {
        try {
            Sigar.load();
        } catch (SigarException e) {
            
        }
    }

    public static native String findExecutable(String name)
        throws SigarException;

    public static String findScriptExecutable(String name) {
        int ix = name.lastIndexOf(".");
        if (ix == -1) {
            return null;
        }

        String ext = name.substring(ix+1);
        if (ext.equals("exe") ||
            ext.equals("bat") ||
            ext.equals("com"))
        {
            return null;
        }

        String exe;
        try {
            exe = findExecutable(new File(name).getAbsolutePath());
        } catch (SigarException e) {
            return null;
        }
        if (exe == null) {
            return null; //no association
        }

        exe = exe.toLowerCase();
        name = name.toLowerCase();
        if (exe.equals(name) || exe.endsWith(name)) {
            return null; //same thing
        }

        File file = new File(exe);
        //rewrite to use cscript for command line stuff
        if (file.getName().equals("wscript.exe")) {
            exe =
                file.getParent() +
                File.separator +
                "cscript.exe";
        }

        return exe;
    }

    public static FileVersion getFileVersion(String name) {
        FileVersion version = new FileVersion();
        if (version.gather(name)) {
            return version;
        }
        else {
            return null;
        }
    }

    public static void main(String[] args) throws Exception {
        for (int i=0; i<args.length; i++) {
            String file =
                new File(args[i]).getAbsoluteFile().toString();
            String exe =
                findScriptExecutable(file);
            if (exe == null) {
                continue;
            }
            System.out.println(args[i] + "=" + exe);
        }
    }
}
