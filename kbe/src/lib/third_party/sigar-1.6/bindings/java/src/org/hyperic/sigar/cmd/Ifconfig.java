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

package org.hyperic.sigar.cmd;

import java.util.Arrays;
import java.util.Collection;

import org.hyperic.sigar.Sigar;
import org.hyperic.sigar.SigarException;
import org.hyperic.sigar.NetInterfaceConfig;
import org.hyperic.sigar.NetInterfaceStat;
import org.hyperic.sigar.NetFlags;

/**
 * Display network interface configuration and metrics.
 */
public class Ifconfig extends SigarCommandBase {

    public Ifconfig(Shell shell) {
        super(shell);
    }

    public Ifconfig() {
        super();
    }

    protected boolean validateArgs(String[] args) {
        return args.length <= 1;
    }

    public String getSyntaxArgs() {
        return "[interface]";
    }

    public String getUsageShort() {
        return "Network interface information";
    }

    public Collection getCompletions() {
        String[] ifNames;

        try {
            ifNames = this.proxy.getNetInterfaceList();
        } catch (SigarException e) {
            return super.getCompletions();
        }

        return Arrays.asList(ifNames);
    }

    public void output(String[] args) throws SigarException {
        String[] ifNames;

        if (args.length == 1) {
            ifNames = args;
        }
        else {
            ifNames = this.proxy.getNetInterfaceList();
        }

        for (int i=0; i<ifNames.length; i++) {
            try {
                output(ifNames[i]);
            } catch (SigarException e) {
                println(ifNames[i] + "\t" + e.getMessage());
            }
        }
    }

    public void output(String name) throws SigarException {
        NetInterfaceConfig ifconfig =
            this.sigar.getNetInterfaceConfig(name);
        long flags = ifconfig.getFlags();

        String hwaddr = "";
        if (!NetFlags.NULL_HWADDR.equals(ifconfig.getHwaddr())) {
            hwaddr = " HWaddr " + ifconfig.getHwaddr();
        }

        if (!ifconfig.getName().equals(ifconfig.getDescription())) {
            println(ifconfig.getDescription());
        }

        println(ifconfig.getName() + "\t" +
                "Link encap:" + ifconfig.getType() +
                hwaddr);

        String ptp = "";
        if ((flags & NetFlags.IFF_POINTOPOINT) > 0) {
            ptp = "  P-t-P:" + ifconfig.getDestination();
        }

        String bcast = "";
        if ((flags & NetFlags.IFF_BROADCAST) > 0) {
            bcast = "  Bcast:" + ifconfig.getBroadcast();
        }

        println("\t" +
                "inet addr:" + ifconfig.getAddress() + 
                ptp + //unlikely
                bcast +
                "  Mask:" + ifconfig.getNetmask());

        println("\t" +
                NetFlags.getIfFlagsString(flags) +
                " MTU:" + ifconfig.getMtu() +
                "  Metric:" + ifconfig.getMetric());
        try {
            NetInterfaceStat ifstat =
                this.sigar.getNetInterfaceStat(name);

            println("\t" +
                    "RX packets:" + ifstat.getRxPackets() +
                    " errors:" + ifstat.getRxErrors() +
                    " dropped:" + ifstat.getRxDropped() +
                    " overruns:" + ifstat.getRxOverruns() +
                    " frame:" + ifstat.getRxFrame());

            println("\t" +
                    "TX packets:" + ifstat.getTxPackets() +
                    " errors:" + ifstat.getTxErrors() +
                    " dropped:" + ifstat.getTxDropped() +
                    " overruns:" + ifstat.getTxOverruns() +
                    " carrier:" + ifstat.getTxCarrier());
            println("\t" + "collisions:" +
                    ifstat.getTxCollisions());

            long rxBytes = ifstat.getRxBytes();
            long txBytes = ifstat.getTxBytes();

            println("\t" +
                    "RX bytes:" + rxBytes +
                    " (" + Sigar.formatSize(rxBytes) + ")" +
                    "  " +
                    "TX bytes:" + txBytes + 
                    " (" + Sigar.formatSize(txBytes) + ")");
        } catch (SigarException e) {
        }

        println("");
    }

    public static void main(String[] args) throws Exception {
        new Ifconfig().processCommand(args);
    }
}
