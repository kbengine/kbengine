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

import java.util.Arrays;
import java.util.List;
import org.hyperic.sigar.win32.Service;
import org.hyperic.sigar.win32.Win32Exception;

/*

Example to show the status of a Windows services.

Compile the example:
% javac -classpath sigar-bin/lib/sigar.jar ServiceStatus.java

Status of all services:
% java -classpath sigar-bin/lib/sigar.jar:. ServiceStatus
Alerter: Stopped
ALG: Running
Apache Tomcat 4.1: Stopped
Apache2: Running
...

Status of a specific service:
% java -classpath sigar-bin/lib/sigar.jar:. ServiceStatus Eventlog
Eventlog: Running

See also: examples/Win32Service.java

*/
public class ServiceStatus {

    private static void printStatus(String name)
        throws Win32Exception {

        Service service = new Service(name);
        System.out.println(name + ": " +
                           service.getStatusString());
        service.close();
    }

    public static void main(String[] args)
        throws Exception {

        List services;
        String name;

        if (args.length == 0) {
            services = Service.getServiceNames();
        }
        else {
            services = Arrays.asList(args);
        }

        for (int i=0; i<services.size(); i++) {
            printStatus((String)services.get(i));
        }
    }
}
