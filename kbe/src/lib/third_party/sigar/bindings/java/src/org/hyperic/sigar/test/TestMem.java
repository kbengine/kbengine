/*
 * Copyright (c) 2006, 2008 Hyperic, Inc.
 * Copyright (c) 2010 VMware, Inc.
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
import org.hyperic.sigar.Mem;

public class TestMem extends SigarTestCase {

    public TestMem(String name) {
        super(name);
    }

    public void testCreate() throws Exception {
        Sigar sigar = getSigar();

        Mem mem = sigar.getMem();

        assertGtZeroTrace("Total", mem.getTotal());

        assertGtZeroTrace("Used", mem.getUsed());

        traceln("UsedPercent=" + mem.getUsedPercent());
        assertGtZeroTrace("(long)UsedPercent", (long)mem.getUsedPercent());

        assertTrue(mem.getUsedPercent() <= 100);

        traceln("FreePercent=" + mem.getFreePercent());
        assertGtEqZeroTrace("(long)FreePercent", (long)mem.getFreePercent());

        assertTrue(mem.getFreePercent() < 100);

        assertGtZeroTrace("Free", mem.getFree());

        assertGtZeroTrace("ActualUsed", mem.getActualUsed());

        assertGtZeroTrace("ActualFree", mem.getActualFree());

        assertGtZeroTrace("Ram", mem.getRam());

        assertTrue((mem.getRam() % 8) == 0);
    }
}
