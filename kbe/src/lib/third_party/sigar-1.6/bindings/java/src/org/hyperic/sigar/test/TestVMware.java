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
import java.io.IOException;

import org.hyperic.sigar.SigarException;
import org.hyperic.sigar.vmware.VMControlLibrary;

public class TestVMware extends SigarTestCase {

    public TestVMware(String name) {
        super(name);
    }

    public void testVMware() throws SigarException {
        File build = new File("build");
        if (!build.exists()) {
            //skip unless we are in the source tree
            return;
        }

        try {
            VMControlLibrary.link(build.getPath());
        } catch (IOException e) {
            traceln(e.getMessage());
        }

        traceln("vmware support=" + VMControlLibrary.isLoaded());
    }
}
