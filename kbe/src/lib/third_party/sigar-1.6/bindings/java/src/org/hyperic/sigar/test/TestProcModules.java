/*
 * Copyright (c) 2006 Hyperic, Inc.
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

import java.util.List;

import org.hyperic.sigar.Sigar;
import org.hyperic.sigar.SigarException;
import org.hyperic.sigar.SigarNotImplementedException;

public class TestProcModules extends SigarTestCase {

    public TestProcModules(String name) {
        super(name);
    }

    private void printModules(Sigar sigar, long pid) throws SigarException {
        traceln("\npid=" + pid);

        try {
            List modules = sigar.getProcModules(pid);

            for (int i=0; i<modules.size(); i++) {
                traceln(i + "=" + modules.get(i));
            }
        } catch (SigarNotImplementedException e) {
            //ok
        }
    }

    public void testCreate() throws Exception {
        Sigar sigar = getSigar();

        try {
	    printModules(sigar, getInvalidPid());
        } catch (SigarException e) {
        }

        try {
	    printModules(sigar, sigar.getPid());
        } catch (SigarNotImplementedException e) {
            return;
        }

	long[] pids = sigar.getProcList();

	for (int i=0; i<pids.length; i++) {
            try {
                printModules(sigar, pids[i]);
            } catch (SigarException e) {
                traceln(pids[i] + ": " + e.getMessage());
            }
	}
    }
}
