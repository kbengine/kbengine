/*
 * Copyright (c) 2006-2008 Hyperic, Inc.
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

import java.net.InetAddress;
import java.net.UnknownHostException;

public class NfsFileSystem extends FileSystem implements java.io.Serializable {

    private static final long serialVersionUID = 02242007L;

    private static final int NFS_PROGRAM = 100003;

    String hostname = null;

    public String getHostname() {
        if (this.hostname == null) {
            String dev = getDevName();
            int ix = dev.indexOf(":");
            if (ix != -1) {
                String host = dev.substring(0, ix);
                InetAddress addr;
                //try converting to ip in java land to take
                //advantage of InetAddress' lookup cache.
                try {
                    addr = InetAddress.getByName(host);
                    this.hostname = addr.getHostAddress();
                } catch (UnknownHostException e) {
                    this.hostname = host;
                }
            }
        }
        return this.hostname;
    }

    public boolean ping() {
        String hostname = getHostname();
        return
            (RPC.ping(hostname, RPC.TCP, NFS_PROGRAM, 2) == 0) ||
            (RPC.ping(hostname, RPC.UDP, NFS_PROGRAM, 2) == 0);
    }

    public String getUnreachableMessage() {
        return getDevName() + " nfs server unreachable";
    }

    public NfsUnreachableException getUnreachableException() {
        return new NfsUnreachableException(getUnreachableMessage());
    }

    public static void main(String[] args) throws Exception {
        Sigar.load();
        System.out.println(RPC.ping(args[0], "nfs"));
    }
}
