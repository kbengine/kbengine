/*
 * Copyright (c) 2006, 2008 Hyperic, Inc.
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

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.Properties;

public class OperatingSystem extends SysInfo {

    public static final String NAME_LINUX   = "Linux";
    public static final String NAME_SOLARIS = "Solaris";
    public static final String NAME_HPUX    = "HPUX";
    public static final String NAME_AIX     = "AIX";
    public static final String NAME_MACOSX  = "MacOSX";
    public static final String NAME_FREEBSD = "FreeBSD";
    public static final String NAME_OPENBSD = "OpenBSD";
    public static final String NAME_NETBSD  = "NetBSD";
    public static final String NAME_WIN32   = "Win32";
    public static final String NAME_NETWARE = "NetWare";
    
    public static final String[] UNIX_NAMES = {
        OperatingSystem.NAME_LINUX,
        OperatingSystem.NAME_SOLARIS,
        OperatingSystem.NAME_HPUX,
        OperatingSystem.NAME_AIX,
        OperatingSystem.NAME_MACOSX,
        OperatingSystem.NAME_FREEBSD,
        OperatingSystem.NAME_OPENBSD,
        OperatingSystem.NAME_NETBSD,
    };
        
    public static final String[] WIN32_NAMES = {
        OperatingSystem.NAME_WIN32,
    };

    public static final String[] NAMES;
    
    public static final boolean IS_WIN32 =
        System.getProperty("os.name").indexOf("Windows") != -1;
    
    private static final Map supportedPlatforms = new HashMap();
    
    static {
        int len = UNIX_NAMES.length + WIN32_NAMES.length;
        String[] all = new String[len];
        System.arraycopy(UNIX_NAMES, 0, all, 0, UNIX_NAMES.length);
        all[len-1] = NAME_WIN32;
        NAMES = all;
        
        for (int i=0; i<NAMES.length; i++) {
            supportedPlatforms.put(NAMES[i], Boolean.TRUE);
        }
    }

    public static boolean isSupported(String name) {
        return supportedPlatforms.get(name) == Boolean.TRUE;
    }

    public static boolean isWin32(String name) {
        return OperatingSystem.NAME_WIN32.equals(name);
    }
    
    private static OperatingSystem instance = null;

    private String dataModel;
    private String cpuEndian;
    
    private OperatingSystem() {
    }

    public static synchronized OperatingSystem getInstance() {
        if (instance == null) {
            Sigar sigar = new Sigar();
            OperatingSystem os = new OperatingSystem();
            try {
                os.gather(sigar);
            } catch (SigarException e) {
                throw new IllegalStateException(e.getMessage());
            } finally {
                sigar.close();
            }
            Properties props = System.getProperties();
            os.dataModel = props.getProperty("sun.arch.data.model");
            os.cpuEndian = props.getProperty("sun.cpu.endian");

            instance = os;
        }

        return instance;
    }

    public String getDataModel() {
        return this.dataModel;
    }

    public String getCpuEndian() {
        return this.cpuEndian;
    }

    public static void main(String[] args) {
        System.out.println("all.............." + Arrays.asList(NAMES));
        OperatingSystem os = OperatingSystem.getInstance();
        System.out.println("description......" + os.getDescription());
        System.out.println("name............." + os.name);
        System.out.println("version.........." + os.version);
        System.out.println("arch............." + os.arch);
        System.out.println("patch level......" + os.patchLevel);
        System.out.println("vendor..........." + os.vendor);
        System.out.println("vendor name......" + os.vendorName);
        System.out.println("vendor version..." + os.vendorVersion);
    }
}
