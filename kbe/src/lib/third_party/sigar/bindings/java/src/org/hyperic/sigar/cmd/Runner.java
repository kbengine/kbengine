/*
 * Copyright (c) 2006-2007 Hyperic, Inc.
 * Copyright (c) 2010 VMware, Inc.
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
import java.io.FileFilter;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;

import java.net.URLClassLoader;
import java.net.URL;

import org.hyperic.sigar.Sigar;
import org.hyperic.sigar.SigarLoader;

public class Runner {

    private static HashMap wantedJars = new HashMap();
    private static final String JAR_EXT = ".jar";

    static {
        wantedJars.put("junit", Boolean.FALSE);
        wantedJars.put("log4j", Boolean.FALSE);
    }

    private static void printMissingJars() {
        for (Iterator it = wantedJars.entrySet().iterator();
             it.hasNext();)
        {
            Map.Entry entry = (Map.Entry)it.next();
            String jar = (String)entry.getKey();
            if (wantedJars.get(jar) == Boolean.FALSE) {
                System.out.println("Unable to locate: " + jar + JAR_EXT);
            }
        }
    }

    private static boolean missingJars() {
        for (Iterator it = wantedJars.entrySet().iterator();
             it.hasNext();)
        {
            Map.Entry entry = (Map.Entry)it.next();
            String jar = (String)entry.getKey();
            if (wantedJars.get(jar) == Boolean.FALSE) {
                return true;
            }
        }

        return false;
    }

    public static URL[] getLibJars(String dir) throws Exception {
        File[] jars = new File(dir).listFiles(new FileFilter() {
            public boolean accept(File file) {
                String name = file.getName();
                int jarIx = name.indexOf(JAR_EXT);
                if (jarIx == -1) {
                    return false;
                }
                int ix = name.indexOf('-');
                if (ix != -1) {
                    name = name.substring(0, ix); //versioned .jar
                }
                else {
                    name = name.substring(0, jarIx);
                }

                if (wantedJars.get(name) != null) {
                    wantedJars.put(name, Boolean.TRUE);
                    return true;
                }
                else {
                    return false;
                }
            }
        });

        if (jars == null) {
            return new URL[0];
        }

        URL[] urls = new URL[jars.length];

        for (int i=0; i<jars.length; i++) {
            URL url = 
                new URL("jar", null,
                        "file:" + jars[i].getAbsolutePath() + "!/");

            urls[i] = url;
        }

        return urls;
    }

    private static void addURLs(URL[] jars) throws Exception {
        URLClassLoader loader =
            (URLClassLoader)Thread.currentThread().getContextClassLoader();

        //bypass protected access.
        Method addURL =
            URLClassLoader.class.getDeclaredMethod("addURL",
                                                   new Class[] {
                                                       URL.class
                                                   });

        addURL.setAccessible(true); //pound sand.

        for (int i=0; i<jars.length; i++) {
            addURL.invoke(loader, new Object[] { jars[i] });
        }
    }

    private static boolean addJarDir(String dir) throws Exception {
        URL[] jars = getLibJars(dir);
        addURLs(jars);
        return !missingJars();
    }

    private static String getenv(String key) {
        try {
            return System.getenv("ANT_HOME"); //check for junit.jar
        } catch (Error e) {
            /*1.4*/
            Sigar sigar = new Sigar();
            try {
                return sigar.getProcEnv("$$", "ANT_HOME");
            } catch (Exception se) {
                return null;
            }
            finally { sigar.close(); }
        }
    }

    public static void main(String[] args) throws Exception {
        if (args.length < 1) {
            args = new String[] { "Shell" };
        }
        else {
            //e.g. convert
            //          "ifconfig", "eth0"
            //   to:
            // "Shell", "ifconfig", "eth0" 
            if (Character.isLowerCase(args[0].charAt(0))) {
                String[] nargs = new String[args.length + 1];
                System.arraycopy(args, 0, nargs, 1, args.length);
                nargs[0] = "Shell";
                args = nargs;
            }
        }

        String name = args[0];

        String[] pargs = new String[args.length - 1];
        System.arraycopy(args, 1, pargs, 0, args.length-1);

        String sigarLib = SigarLoader.getLocation();

        String[] dirs = { sigarLib, "lib", "." };
        for (int i=0; i<dirs.length; i++) {
            if (addJarDir(dirs[i])) {
                break;
            }
        }

        if (missingJars()) {
            File[] subdirs = new File(".").listFiles(new FileFilter() {
                public boolean accept(File file) {
                    return file.isDirectory();
                }
            });

            for (int i=0; i<subdirs.length; i++) {
                File lib = new File(subdirs[i], "lib");
                if (lib.exists()) {
                    if (addJarDir(lib.getAbsolutePath())) {
                        break;
                    }
                }
            }

            if (missingJars()) {
                String home = getenv("ANT_HOME"); //check for junit.jar
                if (home != null) {
                    addJarDir(home + "/lib");
                }
            }
        }

        Class cmd = null;
        String[] packages = {
            "org.hyperic.sigar.cmd.",
            "org.hyperic.sigar.test.",
            "org.hyperic.sigar.",
            "org.hyperic.sigar.win32.",
            "org.hyperic.sigar.jmx.",
        };

        for (int i=0; i<packages.length; i++) {
            try {
                cmd = Class.forName(packages[i] + name);
                break;
            } catch (ClassNotFoundException e) {}
        }

        if (cmd == null) {
            System.out.println("Unknown command: " + args[0]);
            return;
        }

        Method main = cmd.getMethod("main",
                                    new Class[] {
                                        String[].class
                                    });

        try {
            main.invoke(null, new Object[] { pargs });
        } catch (InvocationTargetException e) {
            Throwable t = e.getTargetException();
            if (t instanceof NoClassDefFoundError) {
                System.out.println("Class Not Found: " +
                                   t.getMessage());
                printMissingJars();
            }
            else {
                t.printStackTrace();
            }
        }
    }
}
