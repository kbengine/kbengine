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
import org.hyperic.sigar.FileSystem;
import org.hyperic.sigar.FileSystemMap;
import org.hyperic.sigar.FileSystemUsage;

public class TestFileSystem extends SigarTestCase {

    public TestFileSystem(String name) {
        super(name);
    }

    public void testCreate() throws Exception {
        Sigar sigar = getSigar();

        FileSystem[] fslist = sigar.getFileSystemList();
        FileSystemMap mounts = sigar.getFileSystemMap();

        String dir = System.getProperty("user.home");
        assertTrueTrace("\nMountPoint for " + dir,
                        mounts.getMountPoint(dir).getDirName());

        for (int i=0; i<fslist.length; i++) {
            FileSystem fs = fslist[i];

            assertTrue(mounts.getFileSystem(fs.getDirName()) != null);
            assertLengthTrace("DevName", fs.getDevName());
            assertLengthTrace("DirName", fs.getDirName());
            assertLengthTrace("TypeName", fs.getTypeName());
            assertLengthTrace("SysTypeName", fs.getSysTypeName());
            traceln("Options=" + fs.getOptions());

            FileSystemUsage usage;

            try {
                usage = sigar.getFileSystemUsage(fs.getDirName());
            } catch (SigarException e) {
                if (fs.getType() == FileSystem.TYPE_LOCAL_DISK) {
                    throw e;
                }
                //else ok, e.g. floppy drive on windows
                continue;
            }

            switch (fs.getType()) {
              case FileSystem.TYPE_LOCAL_DISK:
                assertGtZeroTrace("  Total", usage.getTotal());
                //possible machines have full filesystems
                assertGtEqZeroTrace("  Free", usage.getFree());
                assertGtEqZeroTrace("  Avail", usage.getAvail());
                assertGtEqZeroTrace("   Used", usage.getUsed());
                double usePercent = usage.getUsePercent() * 100;
                traceln("  Usage=" + usePercent + "%");
                assertTrue(usePercent <= 100.0);
              default:
                traceln("  DiskReads=" + usage.getDiskReads());
                traceln("  DiskWrites=" + usage.getDiskWrites());
            }
        }

        try {
            sigar.getFileSystemUsage("T O T A L L Y B O G U S");
            assertTrue(false);
        } catch (SigarException e) {
            assertTrue(true);
        }
    }
}
