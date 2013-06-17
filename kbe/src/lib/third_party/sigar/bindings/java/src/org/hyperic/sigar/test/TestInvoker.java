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
import org.hyperic.sigar.SigarException;
import org.hyperic.sigar.SigarNotImplementedException;
import org.hyperic.sigar.SigarPermissionDeniedException;
import org.hyperic.sigar.SigarProxy;
import org.hyperic.sigar.SigarProxyCache;
import org.hyperic.sigar.jmx.SigarInvokerJMX;

public class TestInvoker extends SigarTestCase {

    private static final String[][] OK_QUERIES = {
        { "sigar:Type=Mem", "Free" },
        { "sigar:Type=Mem", "Total" },
        { "sigar:Type=Cpu", "User" },
        { "sigar:Type=Cpu", "Sys" },
        { "sigar:Type=CpuPerc", "User" },
        { "sigar:Type=CpuPerc", "Sys" },
        { "sigar:Type=Swap", "Free" },
        { "sigar:Type=Swap", "Used" },
        { "sigar:Type=Uptime", "Uptime" },
        { "sigar:Type=LoadAverage", "0" },
        { "sigar:Type=LoadAverage", "1" },
        { "sigar:Type=LoadAverage", "2" },
        { "sigar:Type=ProcMem,Arg=$$", "Size" },
        { "sigar:Type=ProcMem,Arg=$$", "Resident" },
        { "sigar:Type=ProcTime,Arg=$$", "Sys" },
        { "sigar:Type=ProcTime,Arg=$$", "User" },
        { "sigar:Type=ProcTime,Arg=$$", "Total" },
        { "sigar:Type=MultiProcCpu,Arg=CredName.User.eq%3Ddougm", "Sys" },
        { "sigar:Type=MultiProcMem,Arg=CredName.User.eq%3Ddougm", "Size" },
        //test Utime/Stime backcompat.
        { "sigar:Type=ProcTime,Arg=$$", "Stime" },
        { "sigar:Type=ProcTime,Arg=$$", "Utime" },
        { "sigar:Type=CpuPercList,Arg=0", "Idle" },
        { "sigar:Type=NetStat", "TcpOutboundTotal" },
        { "sigar:Type=NetStat", "TcpListen" },
    };

    private static final String[][] BROKEN_QUERIES = {
        { "sigar:Type=BREAK", "Free" },
        { "sigar:Type=Mem", "BREAK" },
        { "sigar:Type=ProcTime,Arg=BREAK", "Sys" },
        { "sigar:Type=CpuPercList,Arg=1000", "Idle" },
        { "sigar:Type=CpuPercList,Arg=BREAK", "Idle" },
    };

    public TestInvoker(String name) {
        super(name);
    }

    public void testCreate() throws Exception {
        Sigar sigar = new Sigar();

        SigarProxy proxy =
            SigarProxyCache.newInstance(sigar);

        testOK(proxy);

        sigar.close();
    }

    private void testOK(SigarProxy proxy) throws Exception {
        for (int i=0; i<OK_QUERIES.length; i++) {
            String[] query = OK_QUERIES[i];
            SigarInvokerJMX invoker =
                SigarInvokerJMX.getInstance(proxy, query[0]);

            try {
                Object o = invoker.invoke(query[1]);
                traceln(query[0] + ":" + query[1] + "=" + o);
                assertTrue(true);
            } catch (SigarNotImplementedException e) {
                traceln(query[0] + " NotImplemented");
            } catch (SigarPermissionDeniedException e) {
                traceln(query[0] + " PermissionDenied");
            } catch (SigarException e) {
                traceln(query[0] + ":" + query[1] + "=" + e);
                assertTrue(false);
            }
        }

        for (int i=0; i<BROKEN_QUERIES.length; i++) {
            String[] query = BROKEN_QUERIES[i];
            SigarInvokerJMX invoker =
                SigarInvokerJMX.getInstance(proxy, query[0]);
            try {
                invoker.invoke(query[1]);
                assertTrue(false);
            } catch (SigarException e) {
                traceln(query[0] + ":" + query[1] + "=" + e.getMessage());
                assertTrue(true);
            }
        }
    }
}
