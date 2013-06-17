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
import org.hyperic.sigar.NetInfo;
import org.hyperic.sigar.NetInterfaceConfig;
import org.hyperic.sigar.SigarException;
import org.hyperic.sigar.SigarPermissionDeniedException;

public class TestNetInfo extends SigarTestCase {

    public TestNetInfo(String name) {
        super(name);
    }

    public void testNetInfo() throws SigarException {
        NetInfo info = getSigar().getNetInfo();
        NetInterfaceConfig config = getSigar().getNetInterfaceConfig(null);
        traceln("");
        traceln(info.toString());
        traceln(config.toString());

        int flags = NetFlags.CONN_SERVER | NetFlags.CONN_TCP;

        NetConnection[] connections;

        try {
            connections =
                getSigar().getNetConnectionList(flags);
        } catch (SigarPermissionDeniedException e) {
            return;
        }

        for (int i=0; i<connections.length; i++) {
            long port = connections[i].getLocalPort();
            String listenAddress = 
                getSigar().getNetListenAddress(port);
            if (NetFlags.isAnyAddress(listenAddress)) {
                listenAddress = "*";
            }
            traceln("Listen " + listenAddress + ":" + port);
        }
    }
}
