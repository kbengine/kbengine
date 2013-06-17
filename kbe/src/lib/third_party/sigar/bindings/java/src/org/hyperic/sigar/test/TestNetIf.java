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

import org.hyperic.sigar.Sigar;
import org.hyperic.sigar.SigarException;
import org.hyperic.sigar.SigarNotImplementedException;
import org.hyperic.sigar.NetInterfaceConfig;
import org.hyperic.sigar.NetInterfaceStat;
import org.hyperic.sigar.NetFlags;

public class TestNetIf extends SigarTestCase {

    public TestNetIf(String name) {
        super(name);
    }

    private void getNetIflist(Sigar sigar, boolean getStats) throws Exception {
        String[] ifNames = sigar.getNetInterfaceList();

        for (int i=0; i<ifNames.length; i++) {
            String name = ifNames[i];
            NetInterfaceConfig ifconfig =
                sigar.getNetInterfaceConfig(name);

            traceln("name=" + name);

            assertTrueTrace("Address", ifconfig.getAddress());
            assertTrueTrace("Netmask", ifconfig.getNetmask());

            if (!getStats) {
                continue;
            }

            if ((ifconfig.getFlags() & NetFlags.IFF_UP) <= 0) {
                traceln("!IFF_UP...skipping getNetInterfaceStat");
                continue;
            }

            try {
                NetInterfaceStat ifstat = sigar.getNetInterfaceStat(name);
                assertGtEqZeroTrace("RxPackets", ifstat.getRxPackets());
                assertGtEqZeroTrace("TxPackets", ifstat.getTxPackets());
                traceMethods(ifstat);
            } catch (SigarNotImplementedException e) {
                //ok
            } catch (SigarException e) {
                if (name.indexOf(':') == -1) {
                    fail("getNetInterfaceStat(" + name + "): " +
                         e.getMessage());
                } //else alias may not have metrics
            }
        }
    }

    private void getGarbage(Sigar sigar) {
        //test bogus arg results in exception (and not a segfault)
        try {
            traceln("testing bogus getNetInterfaceStat");
            sigar.getNetInterfaceStat("were switching to night vision");
            fail("switched to night vision");
        } catch (SigarException e) {
            //expected
        }

        //test bogus arg results in exception (and not a segfault)
        try {
            traceln("testing bogus getNetInterfaceConfig");
            sigar.getNetInterfaceConfig("happy meal");
            fail("unexpected treat in happy meal");
        } catch (SigarException e) {
            //expected
        }
    }

    public void testCreate() throws Exception {
        Sigar sigar = getSigar();

        /* call twice to make sure caching works */
        getNetIflist(sigar, false);
        getNetIflist(sigar, false);
        getNetIflist(sigar, true);
        traceln("Default IP=" +
                sigar.getNetInterfaceConfig().getAddress());

        getGarbage(sigar);
    }
}
