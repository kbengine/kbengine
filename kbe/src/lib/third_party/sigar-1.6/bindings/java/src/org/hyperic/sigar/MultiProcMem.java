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

import org.hyperic.sigar.ptql.ProcessFinder;

public class MultiProcMem extends ProcMem {

    static ProcMem get(Sigar sigar, String query)
        throws SigarException {

        ProcMem mem = new ProcMem();
        mem.share = Sigar.FIELD_NOTIMPL;

        long[] pids = ProcessFinder.find(sigar, query);

        for (int i=0; i<pids.length; i++) {
            ProcMem pmem;
            try {
                pmem = sigar.getProcMem(pids[i]);
            } catch (SigarException e) {
                //process may have gone away or EPERM
                continue;
            }
            mem.size     += pmem.size;
            mem.resident += pmem.resident;
            if (pmem.share != Sigar.FIELD_NOTIMPL) {
                mem.share += pmem.share;
            }
        }
        
        return mem;
    }
}
