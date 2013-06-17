/*
 * Copyright (c) 2006-2007 Hyperic, Inc.
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

package org.hyperic.sigar;

/**
 * @deprecated
 * @see org.hyperic.sigar.Sigar#getNetServicesName
 */
public class NetServices {

    private static NetServices instance;
    private Sigar sigar;

    private NetServices() {
        this.sigar = new Sigar();
    }

    protected void finalize() {
        this.sigar.close();
    }

    private static NetServices getInstance() {
        if (instance == null) {
            instance = new NetServices();
        }
        return instance;
    }

    private static String getServiceName(int protocol, long port) {
        return getInstance().sigar.getNetServicesName(protocol, port);
    }

    public static String getName(String protocol, long port) {
        if (protocol.equals("tcp")) {
            return getTcpName(port);
        }
        else if (protocol.equals("udp")) {
            return getUdpName(port);
        }
        else {
            return String.valueOf(port);
        }
    }

    public static String getTcpName(long port) {
        return getServiceName(NetFlags.CONN_TCP, port);
    }

    public static String getUdpName(long port) {
        return getServiceName(NetFlags.CONN_UDP, port);
    }
}
