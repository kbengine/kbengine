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

import java.io.File;
import java.util.Map;

import org.hyperic.sigar.Sigar;
import org.hyperic.sigar.SigarException;
import org.hyperic.sigar.SigarNotImplementedException;
import org.hyperic.sigar.SigarPermissionDeniedException;

public class TestProcEnv extends SigarTestCase {

    public TestProcEnv(String name) {
        super(name);
    }

    public void testCreate() throws Exception {
        Sigar sigar = getSigar();

        try {
            sigar.getProcEnv(getInvalidPid());
        } catch (SigarException e) {
        }

        long pid = sigar.getPid();

        try {
            Map env = sigar.getProcEnv(pid);
            traceln(env.toString());

            String key = "JAVA_HOME";
            String val = (String)env.get(key);

            String single = sigar.getProcEnv(pid, key);

            if (val != null) {
                assertTrue(new File(val, "bin").exists());
                assertTrue(val.equals(single));
                traceln(key + "==>" + single);
            }

            key = "dOeSnOtExIsT";
            val = (String)env.get(key);
            assertTrue(val == null);

            val = sigar.getProcEnv(pid, key);
            assertTrue(val == null);
        } catch (SigarNotImplementedException e) {
            //ok
        } catch (SigarPermissionDeniedException e) {
            //ok
        }

	long[] pids = sigar.getProcList();

	for (int i=0; i<pids.length; i++) {
            //traceln("pid=" + pids[i]);
            try {
                sigar.getProcEnv(pids[i]);
            } catch (SigarException e) {
            }
        }
    }
}
