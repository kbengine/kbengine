/*
 * Copyright (c) 2008-2009 Hyperic, Inc.
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

package org.hyperic.sigar.win32.test;

import java.io.File;

import org.hyperic.sigar.test.SigarTestCase;

import org.hyperic.sigar.Sigar;
import org.hyperic.sigar.SigarException;
import org.hyperic.sigar.ProcExe;
import org.hyperic.sigar.win32.Win32;
import org.hyperic.sigar.win32.FileVersion;

public class TestFileVersion extends SigarTestCase {

    public TestFileVersion(String name) {
        super(name);
    }

    private void printExe(long pid) {
        traceln("\npid=" + pid);        

        String name;
        try {
            name = getSigar().getProcExe(pid).getName();
        } catch (SigarException e) {
            return;
        }

        FileVersion info = Win32.getFileVersion(name);
        if (info != null) {
            traceln("exe='" + name + "'");
            traceln("version=" + info.getProductVersion());
        }
    }

    public void testCreate() throws Exception {
        assertTrue(Win32.getFileVersion("DoEsNoTeXist.exe") == null);

	long[] pids = getSigar().getProcList();

        //go through all just to make sure no crashes
	for (int i=0; i<pids.length; i++) {
            printExe(pids[i]);
	}
    }
}
