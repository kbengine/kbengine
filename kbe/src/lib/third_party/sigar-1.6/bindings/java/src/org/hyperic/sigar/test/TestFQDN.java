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

public class TestFQDN extends SigarTestCase {

    public TestFQDN(String name) {
        super(name);
    }

    public void testCreate() throws Exception {

        String fqdn = getSigar().getFQDN();

        traceln("fqdn=" + fqdn);

        boolean validFQDN = fqdn.indexOf(".") > 0;
        /*
        if (!validFQDN) {
            //wont get a valid fqdn on laptop at home
            //allow to fake with ant -Dsigar.fqdn=foo.bar
            String pfake = getProperty("sigar.fqdn");
            String fake = 
                System.getProperty("sigar.fqdn", pfake);
            if ("".equals(fake)) {
                fake = pfake;
            }
            if (fake != null) {
                traceln("fake='" + fake + "'");
                validFQDN = fake.indexOf(".") > 0;
            }
        }
        */
        assertTrue(validFQDN);
    }
}
