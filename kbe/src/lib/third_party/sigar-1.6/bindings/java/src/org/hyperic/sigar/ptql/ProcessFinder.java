/*
 * Copyright (c) 2006-2007 Hyperic, Inc.
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

package org.hyperic.sigar.ptql;

import org.hyperic.sigar.Sigar;
import org.hyperic.sigar.SigarException;
import org.hyperic.sigar.SigarProxy;
import org.hyperic.sigar.SigarProxyCache;

public class ProcessFinder {

    private Sigar sigar;
    private ProcessQueryFactory qf;

    /**
     * @deprecated
     */
    public ProcessFinder(SigarProxy proxy) {
        this(SigarProxyCache.getSigar(proxy));
    }

    public ProcessFinder(Sigar sigar) {
        this.sigar = sigar;
        this.qf = ProcessQueryFactory.getInstance();
    }

    public long findSingleProcess(String query)
        throws SigarException {

        return findSingleProcess(this.qf.getQuery(query));
    }

    public long findSingleProcess(ProcessQuery query)
        throws SigarException {

        return query.findProcess(this.sigar);
    }

    public static long[] find(Sigar sigar, String query)
        throws SigarException {

        return new ProcessFinder(sigar).find(query);
    }

    public static long[] find(SigarProxy sigar, String query)
        throws SigarException {

        return new ProcessFinder(sigar).find(query);
    }

    public long[] find(String query)
        throws SigarException {

        return find(this.qf.getQuery(query));
    }

    public long[] find(ProcessQuery query)
        throws SigarException {

        return query.find(this.sigar);
    }
}
    
