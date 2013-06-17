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
import java.io.FileInputStream;

import org.hyperic.sigar.Sigar;
import org.hyperic.sigar.SigarException;
import org.hyperic.sigar.ThreadCpu;

//import java.lang.management.ManagementFactory;

public class RunThreadCpu {
    static Sigar sigar = new Sigar();
    static ThreadCpu cpu = new ThreadCpu();
    static int iter = 5000;

    private static long toMillis(long nano) {
        return nano / 1000000;
    }

    private static void printTimes(long start) {
        //ThreadMXBean mx = ManagementFactory.getThreadMXBean();
        try {
            cpu.gather(sigar, 0);
        } catch (SigarException e) {
            e.printStackTrace();
            return;
        }
        System.out.println(Thread.currentThread().getName() + ":");
        System.out.println("   real....." +
                           (System.currentTimeMillis() - start) / 1000);
        System.out.println("   sys......" + toMillis(cpu.getSys()));
        System.out.println("   user....." + toMillis(cpu.getUser()));
        System.out.println("   total...." + toMillis(cpu.getTotal()));
        /*
        System.out.println("   mxtotal.." +
                           toMillis(mx.getCurrentThreadCpuTime()));
        System.out.println("   mxuser.." +
                           toMillis(mx.getCurrentThreadUserTime()));
        */
    }

    private static void pause(int sec) {
        try {
            Thread.sleep(sec * 1000);
        } catch (Exception e) {}
    }

    static class RealThread implements Runnable {
        public void run() {
            long start = System.currentTimeMillis();
            pause(2);
            printTimes(start);
        }
    }

    static class UserThread implements Runnable {
        public void run() {
            long start = System.currentTimeMillis();
            pause(2);
            String s = "";
            for (int i=0; i<iter; i++) {
                s += System.getProperty("java.home");
                for (int j=0; j<s.length(); j++) {
                    s.charAt(j);
                }
            }
            printTimes(start);
        }
    }

    private static void readRtJar() {
        String rt =
            System.getProperty("java.home") +
            "/lib/rt.jar";
        int bytes = 0;
        int max = 1500000; //reading the whole thing takes bloody forever
        FileInputStream is = null;
        try {
            is = new FileInputStream(new File(rt));
            while (is.read() != -1) {
                if (bytes++ > max) {
                    break;
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            if (is != null) {
                try { is.close(); } catch (Exception e) {}
            }
        }
    }

    private static void scanDir() {
        for (int i=0; i<iter; i++) {
            String[] files = new File(".").list();
            for (int j=0; j<files.length; j++) {
                File f = new File(files[j]);
                f.exists();
                if (f.isDirectory()) {
                    f.list();
                }
                f.lastModified();
            }
        }
    }

    static class ReadThread implements Runnable {
        public void run() {
            long start = System.currentTimeMillis();
            pause(2);

            readRtJar();

            printTimes(start);
        }
    }

    static class ScanThread implements Runnable {
        public void run() {
            long start = System.currentTimeMillis();
            pause(2);

            scanDir();

            printTimes(start);
        }
    }

    public static void main(String[] args) throws Exception {
        if (args.length == 1) {
            iter = Integer.parseInt(args[0]);
        }

        long start  = System.currentTimeMillis();
        Thread user = new Thread(new UserThread(), "user");
        Thread read = new Thread(new ReadThread(), "read");
        Thread scan = new Thread(new ScanThread(), "scan");
        Thread real = new Thread(new RealThread(), "real");

        user.start();
        read.start();
        scan.start();
        real.start();

        user.join();
        read.join();
        scan.join();
        real.join();

        pause(3);
        printTimes(start);
    }
}
