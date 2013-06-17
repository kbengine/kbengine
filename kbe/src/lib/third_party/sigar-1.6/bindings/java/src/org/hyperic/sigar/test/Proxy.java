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

package org.hyperic.sigar.test;

import java.io.File;
import java.io.FileFilter;
import java.io.PrintStream;

import org.hyperic.sigar.FileSystem;
import org.hyperic.sigar.Sigar;
import org.hyperic.sigar.SigarException;
import org.hyperic.sigar.SigarInvoker;
import org.hyperic.sigar.SigarProxy;
import org.hyperic.sigar.SigarProxyCache;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

public class Proxy {

    private static final String HOME = System.getProperty("user.home");
    private static boolean sameArg = true; //set false if using leaktest to also stress test
    private boolean pause = false;
    private boolean verbose = true;
    private boolean leakVerbose = false;
    private boolean fukksor = false;
    //compare SigarProxyCache to straightup reflection
    private boolean useReflection = false;
    private PrintStream out = System.out;

    private String ourPid;
    private Sigar sigar;
    private SigarProxy proxy;
    private long lastChange = 0, startSize = 0, currentSize = 0;
    private PidList pids;
    private NetifList netif;
    private FsList fs;
    private DirList dirs;
    private FileList files;

    public Proxy(Sigar sigar, SigarProxy proxy) {
        this.sigar = sigar;
        this.proxy = proxy;
        this.pids = new PidList(sigar);
        this.netif = new NetifList(sigar);
        this.fs = new FsList(sigar);
        this.dirs = new DirList(HOME);
        this.files = new FileList(HOME);
    }

    public void setOutputStream(PrintStream out) {
        this.out = out;
    }

    public void setVerbose(boolean value) {
        this.verbose = value;
    }

    public void setLeakVerbose(boolean value) {
        this.leakVerbose = value;
    }

    private void output() {
        this.out.println();
    }

    private void output(String s) {
        String name = Thread.currentThread().getName();
        this.out.println("[" + name + "] " + s);
    }

    private long getSize() throws SigarException {
        return sigar.getProcMem(ourPid).getResident();
    }

    private boolean memstat(long i) throws SigarException {
        long size = getSize();
        String changed = "";
        if (currentSize != size) {
            long diff = size - currentSize;
            long iters = i - lastChange;
            changed = " (change=" + diff + ", iters=" + iters + ")";
            output(i + ") size=" + size + changed);
            currentSize = size;
            lastChange = i;
            return true;
        }

        return false;
    }

    private void trace(String msg) {
        if (verbose) {
            output(msg);
        }
    }

    private boolean isNonStringArg(Method method) {
        Class[] paramTypes = method.getParameterTypes();
        if ((paramTypes.length >= 1) &&
            (paramTypes[0] != String.class)) {
            return true;
        }
        return false;
    }

    private String argsToString(Object[] args) {
        if ((args == null) || (args.length == 0)) {
            return "";
        }

        StringBuffer sb = new StringBuffer();

        sb.append('(').append(args[0].toString());

        for (int i=1; i<args.length; i++) {
            sb.append(',').append(args[i].toString());
        }

        sb.append(')');

        return sb.toString();
    }

    private static abstract class ArgList {
        String[] values;
        int ix = 0;

        public String get() {
            if (ix == values.length) {
                ix = 0;
            }
            return values[ix++];
        }

        public String getName(int iter) {
            if ((iter == 0) || sameArg) {
                return values[0];
            }
            return get();
        }
    }

    private static class PidList extends ArgList {

        public PidList(Sigar sigar) {
            try {
                long[] pids = sigar.getProcList();
                values = new String[pids.length + 1];
                values[0] = String.valueOf(sigar.getPid()); //ourPid
                for (int i=0; i<pids.length; i++) {
                    values[i+1] = String.valueOf(pids[i]);
                }
            } catch (SigarException e) {
                e.printStackTrace();
                return;
            }
        }
    }

    private static class NetifList extends ArgList {

        public NetifList(Sigar sigar) {
            try {
                values = sigar.getNetInterfaceList();
            } catch (SigarException e) {
                e.printStackTrace();
            }
        }
    }

    private static class DirList extends ArgList {

        public DirList(String dir) {
            File[] dirs = new File(dir).listFiles(new FileFilter() {
                public boolean accept(File f) {
                    return f.isDirectory();
                }
            });

            values = new String[dirs.length];
            for (int i=0; i<dirs.length; i++) {
                values[i] = dirs[i].getAbsolutePath();
            }
        }
    }

    private static class FileList extends ArgList {

        public FileList(String dir) {
            File[] files = new File(dir).listFiles(new FileFilter() {
                public boolean accept(File f) {
                    return !f.isDirectory();
                }
            });

            values = new String[files.length];
            for (int i=0; i<files.length; i++) {
                values[i] = files[i].getAbsolutePath();
            }
        }
    }

    private static class FsList extends ArgList {

        public FsList(Sigar sigar) {
            try {
                FileSystem[] fs = sigar.getFileSystemList();
                values = new String[fs.length];
                for (int i=0; i<fs.length; i++) {
                    values[i] = fs[i].getDirName();
                }
            } catch (SigarException e) {
                e.printStackTrace();
            }
        }
    }

    private void runall(int iter)
        throws SigarException,
               IllegalAccessException,
               InvocationTargetException {
        //dump everything sigar can give us.
        Method[] types = SigarProxy.class.getMethods();

        for (int i=0; i<types.length; i++) {
            Object arg = null;
            Method method = types[i];
            Class[] parms = method.getParameterTypes();
            String type = method.getName().substring(3);

            Class attrClass = method.getReturnType();

            if (isNonStringArg(method)) {
                continue;
            }

            Object[] objArgs = new Object[0];

            if (attrClass.isArray() ||
                !attrClass.getName().startsWith("org.hyperic.sigar")) {
                if (parms.length > 0) {
                    if (type.startsWith("Proc")) {
                        arg = this.pids.getName(iter);

                        switch (parms.length) {
                          case 1:
                            objArgs = new Object[] { arg };
                            break;
                          case 2:
                            if (type.equals("ProcEnv")) {
                                objArgs = new Object[] { arg, "SHELL" };
                            }
                            else if (type.equals("ProcPort")) {
                                objArgs = new Object[] { "tcp", "80" };
                            }
                            break;
                        }
                    }
                    else {
                        trace("SKIPPING: " + type);
                        continue;
                    }
                }

                Object obj;
                if (useReflection) {
                    obj = method.invoke((Object)sigar, objArgs);
                }
                else {
                    obj = invoke(new SigarInvoker(proxy, type), objArgs, null);
                }
                
                if (iter > 0) {
                    if (memstat(iter)) {
                        this.out.print(type);
                        if (arg != null) {
                            this.out.print(" " + arg);
                        }
                        output();
                    }
                }

                String value;
                if (obj instanceof Object[]) {
                    value = argsToString((Object[])obj);
                }
                else {
                    value = String.valueOf(obj);
                }
                trace(type + argsToString(objArgs) + "=" + value);
                continue;
            }

            Method[] attrs = attrClass.getMethods();

            for (int j=0; j<attrs.length; j++) {
                Method getter = attrs[j];
                if (getter.getDeclaringClass() != attrClass) {
                    continue;
                }

                String attrName = getter.getName();
                if (!attrName.startsWith("get")) {
                    continue;
                }

                attrName = attrName.substring(3);
                objArgs = new Object[0];
                if (parms.length > 0) {
                    if (type.startsWith("Proc")) {
                        arg = this.pids.getName(iter);
                    }
                    else if (type.startsWith("Net")) {
                        arg = this.netif.getName(iter);
                    }
                    else if (type.startsWith("MultiProc")) {
                        arg = "State.Name.eq=java";
                    }
                    else if (type.equals("FileSystemUsage") ||
                             type.equals("MountedFileSystemUsage"))
                    {
                        arg = this.fs.getName(iter);
                    }
                    else if (type.equals("FileInfo") ||
                             type.equals("LinkInfo"))
                    {
                        arg = this.files.getName(iter);
                    }
                    else if (type.equals("DirStat")) {
                        arg = this.dirs.getName(iter);
                    }
                    else {
                        trace("SKIPPING: " + type);
                        continue;
                    }

                    objArgs = new Object[] { arg };
                }

                if (isNonStringArg(method)) {
                    continue;
                }

                Object obj;
                if (useReflection) {
                    Object typeObject = method.invoke((Object)sigar, objArgs);
                    obj = getter.invoke(typeObject, new Object[0]);
                }
                else {
                    obj = invoke(new SigarInvoker(proxy, type), objArgs, attrName);
                }

                if (iter > 0) {
                    if (memstat(iter)) {
                        this.out.print(type);
                        if (arg != null) {
                            this.out.print(" " + arg);
                        }
                        output();
                    }
                }

                trace(type + argsToString(objArgs) +
                      "." + attrName + "=" + obj);

                if (pause) {
                    //test cache expire
                    pause();
                }
            }
        }
    }

    private void pause() {
        output("hit enter to continue");
        try {
            System.in.read();
        } catch (Exception e) {}
    }

    private Object invoke(SigarInvoker invoker,
                          Object[] args, String attr) {

        String type = invoker.getType();

        if (fukksor) {
            //make args bogus to test exception handling/messages
            if (args.length != 0) {
                if (args[0] instanceof String) {
                    if (type.startsWith("Proc")) {
                        args[0] = new String("666666");
                    }
                    else {
                        args[0] = new String("bogus");
                    }
                }
            }
        }

        if (args.length == 0) {
            args = null;
        }

        try {
            return invoker.invoke(args, attr);
        } catch (SigarException e) {
            String msg =
                type + " failed: " + e.getMessage();
            System.err.println(msg);
            return null;
        }
    }

    public static void main(String[] args) throws Exception {
        int expire = 30 * 1000;

        Sigar sigar = new Sigar();

        SigarProxy proxy = SigarProxyCache.newInstance(sigar, expire);

        new Proxy(sigar, proxy).run(args);
    }

    public void run(String[] args) throws SigarException {
        ourPid = String.valueOf(sigar.getPid());

        output("ourPid=" + ourPid);

        if (args.length >= 2) {
            String type = args[0], attr = args[args.length - 1];

            if (type.equals("leaktest")) {
                int num = Integer.parseInt(args[1]);
                verbose = leakVerbose;
                startSize = currentSize = getSize();
                long startTime = System.currentTimeMillis();

                for (int i=0; i<num; i++) {
                    //test many iterations of open/close
                    Sigar s = new Sigar();

                    try {
                        runall(i);
                    } catch (IllegalAccessException e) {
                        throw new SigarException(e.getMessage());
                    } catch (InvocationTargetException e) {
                        throw new SigarException(e.getMessage());
                    } finally {
                        s.close();
                    }
                }

                long totalTime = System.currentTimeMillis() - startTime;
                
                proxy = null;
                output("Running garbage collector..");
                System.gc();
                memstat(lastChange+1);

                output(num + " iterations took " +
                       totalTime + "ms");

                output("startSize=" + startSize +
                       ", endSize=" + currentSize +
                       ", diff=" + (currentSize - startSize));
            }
            else {
                Object obj = invoke(new SigarInvoker(proxy, type), null, attr);

                output(obj.toString());
            }
        }
        else {
            try {
                runall(0);
            } catch (IllegalAccessException e) {
                throw new SigarException(e.getMessage());
            } catch (InvocationTargetException e) {
                        throw new SigarException(e.getMessage());
            }
        }
    }
}
