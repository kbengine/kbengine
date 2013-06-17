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

package org.hyperic.sigar.vmware;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;

import java.util.ArrayList;
import java.util.List;

import org.hyperic.jni.ArchName;
import org.hyperic.sigar.SigarLoader;
import org.hyperic.sigar.win32.RegistryKey;
import org.hyperic.sigar.win32.Win32Exception;

public class VMControlLibrary {
    private static final boolean IS64 = ArchName.is64();

    public static final String REGISTRY_ROOT =
        "SOFTWARE\\VMware, Inc.";

    public static final String PROP_VMCONTROL_SHLIB =
        "vmcontrol.shlib";

    private static final String VMWARE_LIB =
        getProperty("lib.vmware", getVMwareLib().getPath());

    private static final String VMCONTROL_TAR =
        getProperty("control.tar", VMWARE_LIB + "/perl/control.tar");

    private static final String VMCONTROL =
        "vmcontrol" + (IS64 ? "64" : "");

    private static final String VMCONTROL_DLL =
        VMCONTROL + "lib.dll";

    private static final String VMCONTROL_SO =
        VMCONTROL + ".so";

    private static final String VMCONTROL_OBJ =
        getProperty("vmcontrol.o", "control-only/" + VMCONTROL + ".o");

    private static final String GCC =
        getProperty("bin.gcc", "/usr/bin/gcc");

    private static final String TAR =
        getProperty("bin.tar", "/bin/tar");

    private static final String LIBSSL =
        getProperty("libssl", "libssl.so.0.9.7");

    private static final String LIBCRYPTO =
        getProperty("libcrypto", "libcrypto.so.0.9.7");

    private static boolean isDebug = false;

    private static String getProperty(String key, String defval) {
        return System.getProperty("vmcontrol." + key, defval);
    }

    private static File getVMwareLib() {
        String[] locations = {
            "/usr/lib/vmware",
            "/usr/local/lib/vmware",
        };

        for (int i=0; i<locations.length; i++) {
            File lib = new File(locations[i]);
            if (lib.exists()) {
                //running on a VMware host
                return lib;
            }
        }

        for (int i=0; i<locations.length; i++) {
            File lib = new File(locations[i] + "-api");
            if (lib.exists()) {
                //remote w/ api installed
                return lib;
            }
        }

        return new File(locations[0]);
    }

    private static File getLib(String name) {
        File lib = new File(VMWARE_LIB, "lib/" + name);
        if (lib.isDirectory()) {
            lib = new File(lib, name);
        }
        return lib;
    }

    private static File getLibSSL() {
        return getLib(LIBSSL);
    }

    private static File getLibCrypto() {
        return getLib(LIBCRYPTO);
    }

    private static String toString(String[] args) {
        StringBuffer cmd = new StringBuffer();
        for (int i=0; i<args.length; i++) {
            if (cmd.length() != 0) {
                cmd.append(' ');
            }
            cmd.append("'").append(args[i]).append("'");
        }
        return cmd.toString();
    }

    private static void exec(String[] args)
        throws IOException {

        Process proc = Runtime.getRuntime().exec(args);
        try {
            int exitVal = proc.waitFor();
            if (exitVal != 0) {
                String msg =
                    "exec(" + toString(args) +
                    ") failed: " + exitVal;
                throw new IOException(msg);
            }
        } catch (InterruptedException e) {
        }
        if (isDebug) {
            System.out.println("exec(" + toString(args) + ") OK");
        }
    }

    public static String getSharedLibrary() {
        return System.getProperty(PROP_VMCONTROL_SHLIB);
    }

    public static void setSharedLibrary(String lib) {
        System.setProperty(PROP_VMCONTROL_SHLIB, lib);
    }

    public static void link()
        throws IOException {

        link(VMCONTROL_SO);
    }

    private static void linkWin32() {
        List dlls = new ArrayList();

        RegistryKey root = null;
        try {
            root =
                RegistryKey.LocalMachine.openSubKey(REGISTRY_ROOT);

            String[] keys = root.getSubKeyNames();
            for (int i=0; i<keys.length; i++) {
                String name = keys[i];
                if (!name.startsWith("VMware ")) {
                    continue;
                }

                RegistryKey subkey = null;
                try {
                    subkey = root.openSubKey(name);
                    String path = subkey.getStringValue("InstallPath");
                    if (path == null) {
                        continue;
                    }
                    path = path.trim();
                    if (path.length() == 0) {
                        continue;
                    }
                    File dll = new File(path + VMCONTROL_DLL);
                    if (dll.exists()) {
                        //prefer VMware Server or VMware GSX Server
                        if (name.endsWith(" Server")) {
                            dlls.add(0, dll.getPath());
                        }
                        //Scripting API will also work
                        else if (name.endsWith(" API")) {
                            dlls.add(dll.getPath());
                        }
                    }
                } catch (Win32Exception e) {

                } finally {
                    if (subkey != null) {
                        subkey.close();
                    }
                }
            }
        } catch (Win32Exception e) {
        } finally {
            if (root != null) {
                root.close();
            }
        }

        if (dlls.size() != 0) {
            setSharedLibrary((String)dlls.get(0));
        }
    }

    public static void link(String name)
        throws IOException {

        if (SigarLoader.IS_WIN32) {
            linkWin32();
            return;
        }

        File out = new File(name).getAbsoluteFile();
        if (out.isDirectory()) {
            out = new File(out, VMCONTROL_SO);
        }

        boolean exists = out.exists();

        if (exists) {
            setSharedLibrary(out.getPath());
            return; //already linked
        }

        if (!new File(VMCONTROL_TAR).exists()) {
            return; //VMware not installed
        }

        File dir = out.getParentFile();
        if (!(dir.isDirectory() && dir.canWrite())) {
            throw new IOException("Cannot write to: " + dir);
        }

        File obj = new File(dir, VMCONTROL_OBJ);
        if (!obj.exists()) {
            //extract vmcontrol.o
            String[] extract_args = {
                TAR,
                "-xf",
                VMCONTROL_TAR,
                "-C", dir.toString(),
                VMCONTROL_OBJ
            };

            exec(extract_args);
        }

        //create vmcontrol.so from vmcontrol.o
        List link_args = new ArrayList();

        link_args.add(GCC);
        link_args.add("-shared");
        link_args.add("-o");
        link_args.add(out.getPath());
        link_args.add(obj.getPath());
        
        if (IS64) {
            link_args.add("-lcrypto");
            link_args.add("-lssl");
        }
        else {
            File libssl = getLibSSL();
            File libcrypto = getLibCrypto();

            if (!libssl.exists()) {
                throw new FileNotFoundException(libssl.toString());
            }

            //Skip rpath for ESX 3.x
            if (!new File(libssl.getParent(), "libc.so.6").exists()) {
                final String rpath = "-Wl,-rpath";

                link_args.add(rpath);
                link_args.add(libssl.getParent());

                //check if libcrypto is in a different directory
                if (!libssl.getParent().equals(libcrypto.getParent())) {
                    link_args.add(rpath);
                    link_args.add(libcrypto.getParent());
                }
            }

            link_args.add(libssl.getPath());
            link_args.add(libcrypto.getPath());
        }
        
        exec((String[])link_args.toArray(new String[0]));

        setSharedLibrary(out.getPath());
    }

    public static boolean isLoaded() {
        return VMwareObject.LOADED;
    }

    public static void main(String[] args) throws Exception {
        isDebug = true;
        if (args.length == 0) {
            link();
        }
        else {
            link(args[0]);
        }

        String shlib = getSharedLibrary();
        if (shlib == null) {
            System.out.println("No library found");
        }
        else {
            System.out.println(PROP_VMCONTROL_SHLIB + "=" + shlib +
                               " (loaded=" + isLoaded() + ")");
        }
    }
}
