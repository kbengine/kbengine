/*
 * Copyright (c) 2009 Hyperic, Inc.
 * Copyright (c) 2009 SpringSource, Inc.
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

package org.hyperic.jni;

/**
 * Helper class for naming the jni library with 
 * platform/arch name for which it is binary compatible.
 */
public class ArchName {

    static boolean useDmalloc =
        System.getProperty("jni.dmalloc") != null;

    public static String getName() throws ArchNotSupportedException {
        String name = getArchName();
        if (useDmalloc) {
            name += "-dmalloc";
        }
        return name;
    }

    public static boolean is64() {
        return "64".equals(System.getProperty("sun.arch.data.model"));
    }

    private static String getArchName() throws ArchNotSupportedException {
        String name    = System.getProperty("os.name");
        String arch    = System.getProperty("os.arch");
        String version = System.getProperty("os.version");
        String majorVersion = version.substring(0, 1); //4.x, 5.x, etc.

        if (arch.endsWith("86")) {
            arch = "x86";
        }

        if (name.equals("Linux")) {
            return arch + "-linux";
        }
        else if (name.indexOf("Windows") > -1) {
            return arch + "-winnt";
        }
        else if (name.equals("SunOS")) {
            if (arch.startsWith("sparcv") && is64()) {
                arch = "sparc64";
            }
            return arch + "-solaris";
        }
        else if (name.equals("HP-UX")) {
            if (arch.startsWith("IA64")) {
                arch = "ia64";
            }
            else {
                arch = "pa";
                if (is64()) {
                    arch += "64";
                }
            }
            if (version.indexOf("11") > -1) {
                return arch + "-hpux-11";
            }
        }
        else if (name.equals("AIX")) {
            if (majorVersion.equals("6")) {
                //v5 binary is compatible with v6
                majorVersion = "5";
            }
            //arch == "ppc" on 32-bit, "ppc64" on 64-bit 
            return arch + "-aix-" + majorVersion;
        }
        else if (name.equals("Mac OS X") || name.equals("Darwin")) {
            if (is64()) {
                return "universal64-macosx";
            }
            else {
                return "universal-macosx";
            }
        }
        else if (name.equals("FreeBSD")) {
            //none of the 4,5,6 major versions are binary compatible
            return arch + "-freebsd-" + majorVersion;
        }
        else if (name.equals("OpenBSD")) {
            return arch + "-openbsd-" + majorVersion;
        }
        else if (name.equals("NetBSD")) {
            return arch + "-netbsd-" + majorVersion;
        }
        else if (name.equals("OSF1")) {
            return "alpha-osf1-" + majorVersion;
        }
        else if (name.equals("NetWare")) {
            return "x86-netware-" + majorVersion;
        }

        String desc = arch + "-" + name + "-" + version;

        throw new ArchNotSupportedException("platform (" + desc + ") not supported");
    }

    private ArchName () { }

}
