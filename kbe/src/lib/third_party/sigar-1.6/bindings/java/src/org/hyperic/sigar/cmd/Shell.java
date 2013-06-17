/*
 * Copyright (c) 2006-2008 Hyperic, Inc.
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

import java.io.IOException;
import java.io.File;
import java.lang.reflect.Constructor;

import org.hyperic.sigar.Sigar;
import org.hyperic.sigar.SigarException;
import org.hyperic.sigar.SigarPermissionDeniedException;
import org.hyperic.sigar.SigarLoader;
import org.hyperic.sigar.SigarProxy;
import org.hyperic.sigar.SigarProxyCache;

import org.hyperic.sigar.ptql.ProcessFinder;

import org.hyperic.sigar.shell.ShellBase;
import org.hyperic.sigar.shell.ShellCommandExecException;
import org.hyperic.sigar.shell.ShellCommandHandler;
import org.hyperic.sigar.shell.ShellCommandInitException;
import org.hyperic.sigar.shell.ShellCommandUsageException;

import org.hyperic.sigar.util.Getline;

/**
 * The Sigar Shell provides a command shell for running the example
 * commands and Sigar tests.
 */
public class Shell extends ShellBase {

    public static final String RCFILE_NAME = ".sigar_shellrc";
    private static final String CLEAR_SCREEN = "\033[2J";

    private Sigar sigar = new Sigar();
    private SigarProxy proxy = SigarProxyCache.newInstance(this.sigar);
    private long[] foundPids = new long[0];
    private boolean isInteractive = false;

    public Shell() {
    }

    public static void clearScreen() {
        System.out.print(CLEAR_SCREEN);
    }

    public SigarProxy getSigarProxy() {
        return this.proxy;
    }

    public Sigar getSigar() {
        return this.sigar;
    }

    public boolean isInteractive() {
        return this.isInteractive;
    }

    public void setInteractive(boolean value) {
        this.isInteractive = value;
    }

    public void registerCommands() throws ShellCommandInitException {
        registerCommandHandler("df", new Df(this));
        registerCommandHandler("du", new Du(this));
        registerCommandHandler("ls", new Ls(this));
        registerCommandHandler("iostat", new Iostat(this));
        registerCommandHandler("free", new Free(this));
        registerCommandHandler("pargs", new ShowArgs(this));
        registerCommandHandler("penv", new ShowEnv(this));
        registerCommandHandler("pfile", new ProcFileInfo(this));
        registerCommandHandler("pmodules", new ProcModuleInfo(this));
        registerCommandHandler("pinfo", new ProcInfo(this));
        registerCommandHandler("cpuinfo", new CpuInfo(this));
        registerCommandHandler("ifconfig", new Ifconfig(this));
        registerCommandHandler("uptime", new Uptime(this));
        registerCommandHandler("ps", new Ps(this));
        registerCommandHandler("pidof", new Pidof(this));
        registerCommandHandler("kill", new Kill(this));
        registerCommandHandler("netstat", new Netstat(this));
        registerCommandHandler("netinfo", new NetInfo(this));
        registerCommandHandler("nfsstat", new Nfsstat(this));
        registerCommandHandler("route", new Route(this));
        registerCommandHandler("version", new Version(this));
        registerCommandHandler("mps", new MultiPs(this));
        registerCommandHandler("sysinfo", new SysInfo(this));
        registerCommandHandler("time", new Time(this));
        registerCommandHandler("ulimit", new Ulimit(this));
        registerCommandHandler("who", new Who(this));
        if (SigarLoader.IS_WIN32) {
            registerCommandHandler("service", new Win32Service(this));
            registerCommandHandler("fversion", new FileVersionInfo(this));
        }
        try {
            //requires junit.jar
            registerCommandHandler("test", "org.hyperic.sigar.test.SigarTestRunner");
        } catch (NoClassDefFoundError e) { }
        catch (Exception e) { }
    }

    private void registerCommandHandler(String name, String className) throws Exception {
        Class cls = Class.forName(className);
        Constructor con = cls.getConstructor(new Class[] { this.getClass() });
        registerCommandHandler(name, (ShellCommandHandler)con.newInstance(new Object[] { this }));
    }

    public void processCommand(ShellCommandHandler handler, String args[])
        throws ShellCommandUsageException, ShellCommandExecException
    {
        try {
            super.processCommand(handler, args);
            if (handler instanceof SigarCommandBase) {
                ((SigarCommandBase)handler).flush();
            }
        } finally {
            SigarProxyCache.clear(this.proxy);
        }
    }

    public static long[] getPids(SigarProxy sigar, String[] args)
        throws SigarException {

        long[] pids;

        switch (args.length) {
          case 0:
            pids = new long[] { sigar.getPid() };
            break;
          case 1:
            if (args[0].indexOf("=") > 0) {
                pids = ProcessFinder.find(sigar, args[0]);
            }
            else if (args[0].equals("$$")) {
                pids = new long[] { sigar.getPid() };
            }
            else {
                pids = new long[] {
                    Long.parseLong(args[0])
                };
            }
            break;
          default:
            pids = new long[args.length];
            for (int i=0; i<args.length; i++) {
                pids[i] = Long.parseLong(args[i]);
            }
            break;
        }

        return pids;
    }

    public long[] findPids(String[] args) throws SigarException {

        if ((args.length == 1) && args[0].equals("-")) {
            return this.foundPids;
        }

        this.foundPids = getPids(this.proxy, args);

        return this.foundPids;
    }

    public long[] findPids(String query) throws SigarException {
        return findPids(new String[] { query });
    }

    public void readCommandFile(String dir) {
        try {
            File rc = new File(dir, RCFILE_NAME);
            readRCFile(rc, false);
            if (this.isInteractive && Getline.isTTY()) {
                this.out.println("Loaded rc file: " + rc);
            }
        } catch (IOException e) { }
    }

    public String getUserDeniedMessage(long pid) {
        return
            SigarPermissionDeniedException.getUserDeniedMessage(this.proxy,
                                                                pid);
    }

    public void shutdown() {
        this.sigar.close();
        //cleanup for dmalloc
        //using reflection incase junit.jar is not present
        try {
            //SigarTestCase.closeSigar();
            Class.forName("org.hyperic.sigar.test.SigarTestCase").
                getMethod("closeSigar", new Class[0]).invoke(null, new Object[0]);
        } catch (ClassNotFoundException e) {
            //SigarTestCase.java not compiled w/o junit.jar
        } catch (Exception e) {
            e.printStackTrace();
        } catch (NoClassDefFoundError e) {
            //avoiding possible Class Not Found: junit/framework/TestCase
        }
        super.shutdown();
    }

    public static void main(String[] args) {
        Shell shell = new Shell();

        try {
            if (args.length == 0) {
                shell.isInteractive = true;
            }

            shell.init("sigar", System.out, System.err);
            shell.registerCommands();

            shell.readCommandFile(System.getProperty("user.home"));
            shell.readCommandFile(".");
            shell.readCommandFile(SigarLoader.getLocation());

            if (shell.isInteractive) {
                shell.initHistory();
                Getline.setCompleter(shell);
                shell.run();
            }
            else {
                shell.handleCommand(null, args);
            }
        } catch (Exception e) {
            System.err.println("Unexpected exception: " + e);
        } finally {
            shell.shutdown();
        }
    }
}
