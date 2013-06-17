/*
 * Copyright (c) 2007, 2009 Hyperic, Inc.
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
import org.hyperic.sigar.SigarNotImplementedException;

public class TestSignal extends SigarTestCase {

    public TestSignal(String name) {
        super(name);
    }

    public void testCreate() throws Exception {
        String[] signals = {
            "HUP", "INT", "KILL", "QUIT",
            "TERM", "USR1", "USR2"
        };

        for (int i=0; i<signals.length; i++) {
            String sig = signals[i];
            traceln(sig + "=" + Sigar.getSigNum(sig));
        }
    }
}
