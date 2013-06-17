/*
 * Copyright (c) 2006, 2008 Hyperic, Inc.
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
import org.hyperic.sigar.CpuInfo;

public class TestCpuInfo extends SigarTestCase {

    public TestCpuInfo(String name) {
        super(name);
    }

    public void testCreate() throws Exception {
        Sigar sigar = getSigar();

        CpuInfo[] infos = sigar.getCpuInfoList();

        for (int i=0; i<infos.length; i++) {
            CpuInfo info = infos[i];

            traceln("num=" + i);
            traceln("vendor=" + info.getVendor());
            traceln("model=" + info.getModel());
            traceln("mhz=" + info.getMhz());
            traceln("cache size=" + info.getCacheSize());
            assertGtZeroTrace("totalSockets", info.getTotalSockets());
            assertGtZeroTrace("totalCores", info.getTotalCores());
            assertTrue(info.getTotalSockets() <= info.getTotalCores());
        }

        int mhz = infos[0].getMhz();
        int current = sigar.getCpuInfoList()[0].getMhz();
        assertEquals("Mhz=" + current + "/" + mhz, current, mhz);
    }
}
