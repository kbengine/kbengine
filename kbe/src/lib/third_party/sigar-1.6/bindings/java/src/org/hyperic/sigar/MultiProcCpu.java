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

package org.hyperic.sigar;

import java.util.HashMap;
import java.util.Map;

import org.hyperic.sigar.ptql.ProcessFinder;

/**
 * Provide multi process cpu metrics.
 */
public class MultiProcCpu extends ProcCpu {

    private long pid;
    private int nproc = 0;
    private static Map ptable = new HashMap();

    static synchronized MultiProcCpu get(Sigar sigar, String query)
        throws SigarException {

        MultiProcCpu cpu;

        cpu = (MultiProcCpu)ptable.get(query);

        if (cpu == null) {
            cpu = new MultiProcCpu();
            cpu.pid = query.hashCode(); //for equals()
            ptable.put(query, cpu);
        }

        long timeNow = System.currentTimeMillis();
        double diff = timeNow - cpu.lastTime;
        if (diff == 0) {
            return cpu; //we were just called within < 1 second ago.
        }

        cpu.lastTime = timeNow;

        long otime = cpu.total;

        cpu.total = 0;
        cpu.user  = 0;
        cpu.sys   = 0;
        cpu.nproc = 0;

        long[] pids = ProcessFinder.find(sigar, query);
        cpu.nproc = pids.length;

        for (int i=0; i<pids.length; i++) {
            ProcTime time;
            try {
                time = sigar.getProcTime(pids[i]);
            } catch (SigarException e) {
                //process may have gone away or EPERM
                continue;
            }
            cpu.total += time.total;
            cpu.user  += time.user;
            cpu.sys   += time.sys;
        }

        if (otime == 0) {
            //XXX could/should pause first time called.
            return cpu;
        }

        cpu.percent = ((cpu.total - otime) / diff);
        if (cpu.percent < 0.0) {
            //counter wrapped
            cpu.percent = (0.0 - cpu.percent);
        }
        if (cpu.percent >= 1.0) {
            cpu.percent = 0.99;
        }

        return cpu;
    }

    /**
     * @return Processes CPU usage percentage.
     */
    public double getPercent() {
        return this.percent;
    }

    /**
     * @return Number of processes matched by query.
     */
    public int getProcesses() {
        return this.nproc;
    }

    /**
     * @return Pid of the process.
     */
    public int hashCode() {
        return (int)this.pid;
    }

    public boolean equals(Object cpu) {
        if (!(cpu instanceof MultiProcCpu)) {
            return false;
        }

        return ((MultiProcCpu)cpu).pid == this.pid;
    }
}
