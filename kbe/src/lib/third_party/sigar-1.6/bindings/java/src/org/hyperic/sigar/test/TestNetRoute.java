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

import org.hyperic.sigar.NetRoute;
import org.hyperic.sigar.SigarException;
import org.hyperic.sigar.SigarNotImplementedException;

public class TestNetRoute extends SigarTestCase {

    public TestNetRoute(String name) {
        super(name);
    }

    public void testNetRoute() throws SigarException {
        NetRoute[] routes;
        try {
            routes = getSigar().getNetRouteList();
        } catch (SigarNotImplementedException e) {
            return;
        }
        assertTrue(routes.length > 0);
        for (int i=0; i<routes.length; i++) {
            NetRoute route = routes[i];
        }
    }
}
