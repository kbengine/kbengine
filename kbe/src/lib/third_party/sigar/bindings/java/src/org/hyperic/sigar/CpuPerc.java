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

/**
 * CPU percentage usage
 */
public class CpuPerc implements java.io.Serializable {

    private static final long serialVersionUID = 05242007L;

    private double user;
    private double sys;
    private double nice;
    private double idle;
    private double wait;
    private double irq;
    private double softIrq;
    private double stolen;
    private double combined;

    CpuPerc() {}

    native void gather(Sigar sigar, Cpu oldCpu, Cpu curCpu);

    static CpuPerc fetch(Sigar sigar, Cpu oldCpu, Cpu curCpu) {
        CpuPerc perc = new CpuPerc();
        perc.gather(sigar, oldCpu, curCpu);
        return perc;
    }

    /**
     * @deprecated
     */
    public static CpuPerc calculate(Cpu oldCpu, Cpu curCpu) {
        Sigar sigar = new Sigar();
        try {
            return fetch(sigar, oldCpu, curCpu);
        } finally {
            sigar.close();
        }
    }

    public double getUser() {
        return this.user;
    }

    public double getSys() {
        return this.sys;
    }

    public double getNice() {
        return this.nice;
    }

    public double getIdle() {
        return this.idle;
    }

    public double getWait() {
        return this.wait;
    }

    public double getIrq() {
        return this.irq;
    }

    public double getSoftIrq() {
        return this.softIrq;
    }

    public double getStolen() {
        return this.stolen;
    }

    /**
     * @return Sum of User + Sys + Nice + Wait
     */ 
    public double getCombined() {
        return this.combined;
    }

    public static String format(double val) {
        String p = String.valueOf(val * 100.0);
        //cant wait for sprintf.
        int ix = p.indexOf(".") + 1;
        String percent =
            p.substring(0, ix) + 
            p.substring(ix, ix+1);
        return percent + "%";
    }

    public String toString() {
        return
            "CPU states: " +
            format(this.user) + " user, " +
            format(this.sys)  + " system, " +
            format(this.nice) + " nice, " +
            format(this.wait) + " wait, " +
            format(this.idle) + " idle";
    }
}
