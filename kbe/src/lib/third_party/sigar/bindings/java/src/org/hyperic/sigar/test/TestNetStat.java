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

package org.hyperic.sigar.test;

import org.hyperic.sigar.NetConnection;
import org.hyperic.sigar.NetFlags;
import org.hyperic.sigar.NetStat;
import org.hyperic.sigar.Sigar;
import org.hyperic.sigar.SigarNotImplementedException;
import org.hyperic.sigar.SigarPermissionDeniedException;

public class TestNetStat extends SigarTestCase {

    public TestNetStat(String name) {
        super(name);
    }

    public void testCreate() throws Exception {
        Sigar sigar = getSigar();
        NetStat netstat;
        
        try {
            netstat = sigar.getNetStat();
        } catch (SigarNotImplementedException e) {
            return;
        } catch (SigarPermissionDeniedException e) {
            return;
        }

        traceln("");
        assertGtEqZeroTrace("Outbound", netstat.getTcpOutboundTotal());
        assertGtEqZeroTrace("Inbound", netstat.getTcpInboundTotal());
        int[] states = netstat.getTcpStates();
        for (int i=0; i<NetFlags.TCP_UNKNOWN; i++) {
            assertGtEqZeroTrace(NetConnection.getStateString(i), states[i]);
        }
    }
}
