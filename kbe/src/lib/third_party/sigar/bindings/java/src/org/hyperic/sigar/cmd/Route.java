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

package org.hyperic.sigar.cmd;

import java.util.ArrayList;

import org.hyperic.sigar.NetFlags;
import org.hyperic.sigar.NetRoute;
import org.hyperic.sigar.SigarException;

public class Route extends SigarCommandBase {

    private static final String OUTPUT_FORMAT =
        "%-15s %-15s %-15s %-5s %-6s %-3s %-s";

    //like route -n
    private static final String[] HEADER = new String[] {
        "Destination",
        "Gateway",
        "Genmask",
        "Flags",
        "Metric",
        "Ref",
        "Iface"
    };

    public Route(Shell shell) {
        super(shell);
        setOutputFormat(OUTPUT_FORMAT);
    }

    public Route() {
        super();
        setOutputFormat(OUTPUT_FORMAT);
    }

    private static String flags(long flags) {
        StringBuffer sb = new StringBuffer();
        if ((flags & NetFlags.RTF_UP) != 0) {
            sb.append('U');
        }
        if ((flags & NetFlags.RTF_GATEWAY) != 0) {
            sb.append('G');
        }
        return sb.toString();
    }

    public String getUsageShort() {
        return "Kernel IP routing table";
    }

    //netstat -r
    public void output(String[] args) throws SigarException {
        NetRoute[] routes = this.sigar.getNetRouteList();

        printf(HEADER);

        for (int i=0; i<routes.length; i++) {
            NetRoute route = routes[i];

            ArrayList items = new ArrayList();
            items.add(route.getDestination());
            items.add(route.getGateway());
            items.add(route.getMask());
            items.add(flags(route.getFlags()));
            items.add(String.valueOf(route.getMetric()));
            items.add(String.valueOf(route.getRefcnt()));
            items.add(route.getIfname());

            printf(items);
        }
    }

    public static void main(String[] args) throws Exception {
        new Route().processCommand(args);
    }
}
