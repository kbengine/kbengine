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

package org.hyperic.sigar;

import java.io.File;
import java.io.IOException;
import java.util.jar.Attributes;
import java.util.jar.JarFile;

public class ProcUtil {

    private static boolean isClassName(String name) {
        int len = name.length();
        if (len == 0) {
            return false;
        }

        for (int i=0; i<len; i++) {
            char c = name.charAt(i);
            if (!((c == '.') || Character.isLetter(c))) {
                return false;
            }
        }

        return true;
    }

    /**
     * Try to determina classname for java programs
     */
    public static String getJavaMainClass(SigarProxy sigar, long pid)
        throws SigarException {

        String[] args = sigar.getProcArgs(pid);
        for (int i=1; i<args.length; i++) {
            String arg = args[i];
            if (isClassName(arg.trim())) {
                //example: "java:weblogic.Server"
                return arg;
            }
            else if (arg.equals("-jar")) {
                File file = new File(args[i+1]);
                if (!file.isAbsolute()) {
                    try {
                        String cwd =
                            sigar.getProcExe(pid).getCwd();
                        file = new File(cwd + File.separator + file);
                    } catch (SigarException e) {}
                }

                if (file.exists()) {
                    JarFile jar = null;
                    try {
                        jar = new JarFile(file);
                        return
                            jar.getManifest().
                            getMainAttributes().
                            getValue(Attributes.Name.MAIN_CLASS);
                    } catch (IOException e) {
                    } finally {
                        if (jar != null) {
                            try { jar.close(); }
                            catch (IOException e){}
                        }
                    }
                }
                
                return file.toString();
            }
        }

        return null;
    }

    public static String getDescription(SigarProxy sigar, long pid)
        throws SigarException {

        String[] args;
        ProcState state = sigar.getProcState(pid);
        String name = state.getName();

        try {
            args = sigar.getProcArgs(pid);
        } catch (SigarException e) {
            args = new String[0];
        }

        if (name.equals("java") || name.equals("javaw")) {
            String className = null;
            try {
                className = getJavaMainClass(sigar, pid);
            } catch (SigarException e) {}
            if (className != null) {
                name += ":" + className;
            }
        }
        else if (args.length != 0) {
            name = args[0];
        }
        else {
            try {
                String exe =
                    sigar.getProcExe(pid).getName();
                if (exe.length() != 0) {
                    name = exe;
                }
            } catch (SigarException e) {}
        }

        return name;
    }
}
