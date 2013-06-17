/*
 * Copyright (c) 2009 SpringSource, Inc.
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

import java.util.ArrayList;

import org.hyperic.sigar.CpuPerc;
import org.hyperic.sigar.Humidor;
import org.hyperic.sigar.Sigar;
import org.hyperic.sigar.SigarException;
import org.hyperic.sigar.SigarProxy;

public class TestHumidor extends SigarTestCase {

    public TestHumidor(String name) {
        super(name);
    }

    private static class HumidorThread extends Thread {
        private SigarProxy sigar;
        private HumidorThread(SigarProxy sigar) {
            this.sigar = sigar;
        }

        public void run() {
            try {
                getProcCpu(this.sigar);
            } catch (Exception e) {
                throw new IllegalArgumentException(e.getMessage());
            }
        }
    }

    private static void getProcCpu(SigarProxy sigar) throws Exception {
        long[] pids = sigar.getProcList();
        for (int j=0; j<10; j++) {
            for (int i=0; i<pids.length; i++) {
                try {
                    double cpu = sigar.getProcCpu(pids[i]).getPercent();
                    if (SigarTestCase.getVerbose()) {
                        System.out.println(Thread.currentThread().getName() + 
                                           " " + pids[i] + "=" + CpuPerc.format(cpu));
                    }
                } catch (SigarException e) {
                    //ok - process may have gone away or permission denied.
                }
            }
        }
    }

    private void runTests(SigarProxy sigar) throws Exception {
        ArrayList threads = new ArrayList();
        for (int i=0; i<3; i++) {
            Thread t = new HumidorThread(sigar);
            threads.add(t);
            t.start();
        }
        for (int i=0; i<threads.size(); i++) {
            Thread t = (Thread)threads.get(i);
            t.join();
        }
    }

    public void testGlobalInstance() throws Exception {
        runTests(Humidor.getInstance().getSigar());
    }

    public void testInstance() throws Exception {
        Sigar sigar = new Sigar();
        runTests(new Humidor(sigar).getSigar());
        sigar.close();
    }

    //uncomment to see if this test will indeed cause a segfault
    //without the protection of the Humidor
    //public void testUnwrapped() throws Exception {
    //    runTests(new Sigar());
    //}
}
