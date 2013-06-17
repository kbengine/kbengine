/*
 * Copyright (c) 2004-2005 Hyperic, Inc.
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

using System;
using Hyperic.Sigar;

public class Ifconfig {

    private static void println(String line) {
        System.Console.WriteLine(line);
    }

    private static void output(Sigar sigar, String name) {
        NetInterfaceConfig ifconfig =
            sigar.NetInterfaceConfig(name);
        long flags = ifconfig.Flags;

        String hwaddr = "";
        if (!Sigar.NULL_HWADDR.Equals(ifconfig.Hwaddr)) {
            hwaddr = " HWaddr " + ifconfig.Hwaddr;
        }

        println(ifconfig.Name + "\t" +
                "Link encap:" + ifconfig.Type +
                hwaddr);

        String ptp = "";
        if ((flags & Sigar.IFF_POINTOPOINT) > 0) {
            ptp = "  P-t-P:" + ifconfig.Destination;
        }

        String bcast = "";
        if ((flags & Sigar.IFF_BROADCAST) > 0) {
            bcast = "  Bcast:" + ifconfig.Broadcast;
        }

        println("\t" +
                "inet addr:" + ifconfig.Address + 
                ptp + //unlikely
                bcast +
                "  Mask:" + ifconfig.Netmask);

        println("\t" +
                ifconfig.FlagsString() +
                " MTU:" + ifconfig.Mtu +
                "  Metric:" + ifconfig.Metric);
        try {
            NetInterfaceStat ifstat =
                sigar.NetInterfaceStat(name);

            println("\t" +
                    "RX packets:" + ifstat.RxPackets +
                    " errors:" + ifstat.RxErrors +
                    " dropped:" + ifstat.RxDropped +
                    " overruns:" + ifstat.RxOverruns +
                    " frame:" + ifstat.RxFrame);

            println("\t" +
                    "TX packets:" + ifstat.TxPackets +
                    " errors:" + ifstat.TxErrors +
                    " dropped:" + ifstat.TxDropped +
                    " overruns:" + ifstat.TxOverruns +
                    " carrier:" + ifstat.TxCarrier);
            println("\t" + "collisions:" +
                    ifstat.TxCollisions);

            long rxBytes = ifstat.RxBytes;
            long txBytes = ifstat.TxBytes;

            println("\t" +
                    "RX bytes:" + rxBytes +
                    " (" + Sigar.FormatSize(rxBytes) + ")" +
                    "  " +
                    "TX bytes:" + txBytes + 
                    " (" + Sigar.FormatSize(txBytes) + ")");
        } catch (SigarException) { }

        println("");
    }

    public static void Main() {
        Sigar sigar = new Sigar();

        foreach (String name in sigar.NetInterfaceList()) {
            output(sigar, name);
        }
    }
}
