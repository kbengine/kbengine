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

import java.util.Date;
import org.hyperic.sigar.Sigar;
import org.hyperic.sigar.Uptime;

public class TestUptime extends SigarTestCase {

    public TestUptime(String name) {
        super(name);
    }

    public void testCreate() throws Exception {
        Sigar sigar = getSigar();

        Uptime uptime = sigar.getUptime();

        long now = System.currentTimeMillis();
        traceln("\nboottime=" +
                new Date(now - (long)uptime.getUptime()*1000));
        assertTrue(uptime.getUptime() > 0);
    }
}
