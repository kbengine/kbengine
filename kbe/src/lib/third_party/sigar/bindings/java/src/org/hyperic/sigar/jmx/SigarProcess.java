/*
 * Copyright (c) 2006-2007 Hyperic, Inc.
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

package org.hyperic.sigar.jmx;

import org.hyperic.sigar.ProcCpu;
import org.hyperic.sigar.ProcFd;
import org.hyperic.sigar.ProcMem;
import org.hyperic.sigar.Sigar;
import org.hyperic.sigar.SigarException;
import org.hyperic.sigar.SigarProxy;
import org.hyperic.sigar.SigarProxyCache;

/**
 * Implement the SigarProcessMBean to provide current process info
 * via JMX.
 */

public class SigarProcess implements SigarProcessMBean {

    private Sigar sigarImpl;
    private SigarProxy sigar;

    public SigarProcess() {
        this(new Sigar());
    }

    public SigarProcess(Sigar sigar) {
        this.sigarImpl = sigar;
        this.sigar = SigarProxyCache.newInstance(sigarImpl);
    }

    public void close() {
        this.sigarImpl.close();
    }

    private RuntimeException unexpectedError(String type,
                                             SigarException e) {
        String msg =
            "Unexected error in Sigar.get" + type +
            ": " + e.getMessage();
        return new IllegalArgumentException(msg);
    }
                                   
    private synchronized ProcMem getMem() {
        try {
            long pid = this.sigar.getPid();
            return this.sigar.getProcMem(pid);
        } catch (SigarException e) {
            throw unexpectedError("Mem", e);
        }
    }

    private synchronized ProcCpu getCpu() {
        try {
            long pid = this.sigar.getPid();
            return this.sigar.getProcCpu(pid);
        } catch (SigarException e) {
            throw unexpectedError("Cpu", e);
        }   
    }

    private synchronized ProcFd getFd() {
        try {
            long pid = this.sigar.getPid();
            return this.sigar.getProcFd(pid);
        } catch (SigarException e) {
            throw unexpectedError("Fd", e);
        }   
    }
    
    public Long getMemSize() {
        return new Long(getMem().getSize());
    }

    /**
     * @deprecated
     * @see getMemSize
     */
    public Long getMemVsize() {
        return getMemSize();
    }

    public Long getMemResident() {
        return new Long(getMem().getResident());
    }

    public Long getMemShare() {
        return new Long(getMem().getShare());
    }

    public Long getMemPageFaults() {
        return new Long(getMem().getPageFaults());
    }

    public Long getTimeUser() {
        return new Long(getCpu().getUser());
    }

    public Long getTimeSys() {
        return new Long(getCpu().getSys());
    }

    public Double getCpuUsage() {
        return new Double(getCpu().getPercent());
    }

    public Long getOpenFd() {
        return new Long(getFd().getTotal());
    }

    public static void main(String args[]) {
        SigarProcessMBean proc = new SigarProcess();
        System.out.println("MemSize=" + proc.getMemSize());
        System.out.println("MemResident=" + proc.getMemResident());
        System.out.println("MemShared=" + proc.getMemShare());
        System.out.println("MemPageFaults=" + proc.getMemPageFaults());
        System.out.println("TimeUser=" + proc.getTimeUser());
        System.out.println("TimeSys=" + proc.getTimeSys());
        System.out.println("OpenFd=" + proc.getOpenFd());
    }
}
