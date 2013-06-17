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

package org.hyperic.sigar.cmd;

import java.util.List;

import org.hyperic.sigar.Sigar;
import org.hyperic.sigar.SigarException;
import org.hyperic.sigar.SigarProxy;
import org.hyperic.sigar.CpuPerc;
import org.hyperic.sigar.SigarProxyCache;
import org.hyperic.sigar.ProcCpu;
import org.hyperic.sigar.ProcStat;

/**
 * Display system resource utilization summaries and process information.
 * <p>
 * This version of the top command requires a ptql query to select which
 * processes to display.
 *
 * Example to display java processes only:<br>
 * <code>% java -jar sigar-bin/lib/sigar.jar Top State.Name.eq=java</code>
 */

public class Top {
    private static final int SLEEP_TIME = 1000 * 5;

    private static final String HEADER =
        "PID\tUSER\tSTIME\tSIZE\tRSS\tSHARE\tSTATE\tTIME\t%CPU\tCOMMAND";

    private static String toString(ProcStat stat) {
        return 
            stat.getTotal()    + " processes: " +
            stat.getSleeping() + " sleeping, " +
            stat.getRunning()  + " running, " + 
            stat.getZombie()   + " zombie, " +
            stat.getStopped()  + " stopped... " + stat.getThreads() + " threads";
    }

    public static void main(String[] args) throws Exception {
        Sigar sigarImpl = new Sigar();

        SigarProxy sigar = 
            SigarProxyCache.newInstance(sigarImpl, SLEEP_TIME);

        while (true) {
            Shell.clearScreen();

            System.out.println(Uptime.getInfo(sigar));

            System.out.println(toString(sigar.getProcStat()));

            System.out.println(sigar.getCpuPerc());

            System.out.println(sigar.getMem());

            System.out.println(sigar.getSwap());
                               
            System.out.println();

            System.out.println(HEADER);

            long[] pids = Shell.getPids(sigar, args);

            for (int i=0; i<pids.length; i++) {
                long pid = pids[i];

                String cpuPerc = "?";

                List info;
                try {
                    info = Ps.getInfo(sigar, pid);
                } catch (SigarException e) {
                    continue; //process may have gone away
                }
                try {
                    ProcCpu cpu = sigar.getProcCpu(pid);
                    cpuPerc = CpuPerc.format(cpu.getPercent());
                } catch (SigarException e) {
                }

                info.add(info.size()-1, cpuPerc);

                System.out.println(Ps.join(info));
            }

            Thread.sleep(SLEEP_TIME);
            SigarProxyCache.clear(sigar);
        }
    }
}
