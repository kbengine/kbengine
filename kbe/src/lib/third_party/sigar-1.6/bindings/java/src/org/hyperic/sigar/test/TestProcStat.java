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

import org.hyperic.sigar.CurrentProcessSummary;
import org.hyperic.sigar.Sigar;
import org.hyperic.sigar.SigarProxy;
import org.hyperic.sigar.SigarProxyCache;
import org.hyperic.sigar.ProcStat;

public class TestProcStat extends SigarTestCase {

    public TestProcStat(String name) {
        super(name);
    }

    public void testCreate() throws Exception {
        Sigar sigar = getSigar();

        ProcStat stat = sigar.getProcStat();

        long[] pids = sigar.getProcList();

        assertTrue(stat.getTotal() > 1);
        traceln(stat.toString());
        SigarProxy proxy = SigarProxyCache.newInstance(getSigar());
        traceln(CurrentProcessSummary.get(proxy).toString());
    }
}
