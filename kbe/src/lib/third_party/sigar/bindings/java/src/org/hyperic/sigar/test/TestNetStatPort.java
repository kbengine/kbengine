/*
 * Copyright (c) 2007-2008 Hyperic, Inc.
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

import java.net.InetAddress;
import java.util.ArrayList;

import org.hyperic.sigar.NetConnection;
import org.hyperic.sigar.NetFlags;
import org.hyperic.sigar.NetInterfaceConfig;
import org.hyperic.sigar.NetStat;
import org.hyperic.sigar.Sigar;
import org.hyperic.sigar.SigarNotImplementedException;
import org.hyperic.sigar.SigarPermissionDeniedException;

public class TestNetStatPort extends SigarTestCase {

    public TestNetStatPort(String name) {
        super(name);
    }

    private void netstat(Sigar sigar, String addr, long port) throws Exception {
        InetAddress address = InetAddress.getByName(addr);

        traceln("");
        traceln("using address=" + address + ":" + port);

        NetStat netstat;
        try {
            netstat = sigar.getNetStat(address.getAddress(), port);
        } catch (SigarNotImplementedException e) {
            return;
        } catch (SigarPermissionDeniedException e) {
            return;
        }

        assertGtEqZeroTrace("AllOutbound", netstat.getAllOutboundTotal());
        assertGtEqZeroTrace("Outbound", netstat.getTcpOutboundTotal());
        assertGtEqZeroTrace("Inbound", netstat.getTcpInboundTotal());
        assertGtEqZeroTrace("AllInbound", netstat.getAllInboundTotal());
        int[] states = netstat.getTcpStates();
        for (int i=0; i<NetFlags.TCP_UNKNOWN; i++) {
            assertGtEqZeroTrace(NetConnection.getStateString(i), states[i]);
        }
    }

    public void testCreate() throws Exception {
        Sigar sigar = getSigar();

        NetInterfaceConfig ifconfig =
            sigar.getNetInterfaceConfig(null);

        ArrayList addrs = new ArrayList();
        addrs.add(ifconfig.getAddress());
        addrs.add(NetFlags.LOOPBACK_ADDRESS);
        if (JDK_14_COMPAT) {
            addrs.add(NetFlags.LOOPBACK_ADDRESS_V6);
        }

        for (int i=0; i<addrs.size(); i++) {
            String addr = (String)addrs.get(i);
            netstat(sigar, addr, 22);
        }
    }
}
