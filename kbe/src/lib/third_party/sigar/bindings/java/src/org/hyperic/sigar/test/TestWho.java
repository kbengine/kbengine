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

import org.hyperic.sigar.SigarException;
import org.hyperic.sigar.Who;

public class TestWho extends SigarTestCase {

    public TestWho(String name) {
        super(name);
    }

    public void testWho() throws SigarException {
        traceln("");
        Who[] who = getSigar().getWhoList();
        for (int i=0; i<who.length; i++) {
            String host = who[i].getHost();
            if (host.length() != 0) {
                host = "(" + host + ")";
            }
            traceln(who[i].getUser() + "\t" +
                    who[i].getDevice() + "\t" +
                    new Date(who[i].getTime() * 1000) + "\t" +
                    host);
            assertLengthTrace("user", who[i].getUser());
        }
    }
}
