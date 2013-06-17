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

import java.io.File;

import org.hyperic.sigar.Sigar;
import org.hyperic.sigar.SigarException;
import org.hyperic.sigar.ProcExe;
import org.hyperic.sigar.SigarNotImplementedException;

public class TestProcExe extends SigarTestCase {

    public TestProcExe(String name) {
        super(name);
    }

    private void printExe(Sigar sigar, long pid) throws SigarException {
        traceln("\npid=" + pid);        

        try {
            ProcExe exe = sigar.getProcExe(pid);

            String cwd = exe.getCwd();
            traceln("cwd='" + cwd + "'");

            //assertTrue(new File(cwd).isDirectory());

            traceln("exe='" + exe.getName() + "'");

            //assertTrue(new File(exeFile).exists());
        } catch (SigarNotImplementedException e) {
            //ok
        }
    }

    public void testCreate() throws Exception {
        Sigar sigar = getSigar();

        try {
            sigar.getProcExe(getInvalidPid());
        } catch (SigarException e) {
        }

        try {
            ProcExe exe = sigar.getProcExe(sigar.getPid());

            File exeFile = new File(exe.getName());
            String cwd = exe.getCwd();
            traceln("cwd='" + cwd + "'");

            if (cwd.length() > 0) {
                assertTrue(new File(cwd).isDirectory());
            }

            traceln("exe='" + exe.getName() + "'");

            if (exe.getName().length() > 0) {
                assertTrue(exeFile.exists());

                //win32 has .exe
                assertTrue(exeFile.getName().startsWith("java"));
            }
        } catch (SigarNotImplementedException e) {
            //ok
        }

	long[] pids = sigar.getProcList();

        //go through all just to make sure no crashes
	for (int i=0; i<pids.length; i++) {
            try {
                printExe(sigar, pids[i]);
            } catch (SigarException e) {
            } catch (junit.framework.AssertionFailedError e) {
            }
	}
    }
}
