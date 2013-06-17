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

package org.hyperic.sigar.win32.test;

import java.util.List;

import org.hyperic.sigar.test.SigarTestCase;

import org.hyperic.sigar.win32.Service;
import org.hyperic.sigar.win32.ServiceConfig;
import org.hyperic.sigar.win32.Win32Exception;

public class TestService extends SigarTestCase {
    private static final String TEST_NAME = "MyTestService";

    private static final String PREFIX =
        "sigar.test.service.";
    
    private static final boolean TEST_CREATE =
        "true".equals(System.getProperty(PREFIX + "create"));

    private static final boolean TEST_DELETE =
        "true".equals(System.getProperty(PREFIX + "delete"));
    
    public TestService(String name) {
        super(name);
    }

    public void testServiceOpen() throws Exception {
        Service service = new Service("Eventlog");
        service.getConfig();
        service.close();
        
        String dummyName = "DOESNOTEXIST"; 
        try {
            new Service(dummyName);
            assertTrue(false);
        } catch (Win32Exception e) {
            traceln(dummyName + ": " + e.getMessage());
            assertTrue(true);
        }
    }

    public void testServiceNames() throws Exception {
        List services = Service.getServiceNames();
        assertGtZeroTrace("getServiceNames", services.size());

        final String[] ptql = {
            "Service.Name.ct=Ev",
            "Service.Path.ew=.exe",
        };

        for (int i=0; i<ptql.length; i++) {
            services = Service.getServiceNames(getSigar(), ptql[i]);
            assertGtZeroTrace(ptql[i], services.size());
        }

        final String[] invalid = {
            "State.Name.ct=Ev",
            "Service.Invalid.ew=.exe",
            "-"
        };

        for (int i=0; i<invalid.length; i++) {
            try {
                services = Service.getServiceNames(getSigar(), invalid[i]);
                fail("'" + invalid[i] + "' did not throw Exception");
            } catch (Exception e) {
                //expected
            }
        }
    }

    public void testServiceConfig() throws Exception {
        List configs =
            Service.getServiceConfigs(getSigar(), "svchost.exe");
        assertGtZeroTrace("getServiceConfigs", configs.size());
    }

    public void testServiceCreateDelete() throws Exception {
        if (!TEST_CREATE) {
            return;
        }
        ServiceConfig config = new ServiceConfig(TEST_NAME);
        config.setStartType(ServiceConfig.START_MANUAL);
        config.setDisplayName("My Test Service");
        config.setDescription("A Description of " + config.getDisplayName());
        config.setPath("C:\\Program Files\\My Test 1.0\\mytest.exe");

        Service.create(config);
    }

    public void testDeleteService() throws Exception {
        if (!TEST_DELETE) {
            return;
        }
        Service service = new Service(TEST_NAME);
        service.delete();
    }
}
