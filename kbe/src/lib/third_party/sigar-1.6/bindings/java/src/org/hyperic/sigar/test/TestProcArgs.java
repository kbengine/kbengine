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

public class TestProcArgs extends SigarTestCase {

    public TestProcArgs(String name) {
        super(name);
    }

    private boolean findArg(String[] args, String what) {
        boolean found = false;

        traceln("find=" + what);

        for (int i=0; i<args.length; i++) {
            traceln("   " + i + "=" + args[i]);

            if (args[i].equals(what)) {
                found = true;
            }
        }

        return found;
    }

    public void testCreate() throws Exception {
        Sigar sigar = getSigar();

        try {
            sigar.getProcArgs(getInvalidPid());
        } catch (SigarException e) {
        }

        try {
            String[] args = sigar.getProcArgs(sigar.getPid());

            if (getVerbose()) {
                findArg(args, TestProcArgs.class.getName());
            }

            if (args.length > 0) {
                assertTrue(args[0].indexOf("java") != -1);
            }

            //hpux has a limit less than what these args will be
            if (!System.getProperty("os.name").equals("HP-UX")) {
                //and this test only works when run under ant
                //assertTrue(findArg(args, TestProcArgs.class.getName()));
            }
        } catch (SigarNotImplementedException e) {
            //ok; might not happen on win32
        }

	long[] pids = sigar.getProcList();

	for (int i=0; i<pids.length; i++) {
            String pidTrace = "pid=" + pids[i];
            try {
                String[] args = sigar.getProcArgs(pids[i]);
                traceln(pidTrace);
                for (int j=0; j<args.length; j++) {
                    traceln("   " + j + "=>" + args[j] + "<==");
                }
            } catch (SigarException e) {
                traceln(pidTrace + ": " + e.getMessage());
            }
	}
    }
}
