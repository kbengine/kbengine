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
 * Flag constants for network related ops.
 */
public class NetFlags {

    private NetFlags () { }

    /**
     * value of unknown or non-existent hardware address
     */
    public final static String NULL_HWADDR = "00:00:00:00:00:00";

    public static final String ANY_ADDR = "0.0.0.0";

    public static final String ANY_ADDR_V6 = "::";

    public static final String LOOPBACK_HOSTNAME = "localhost";

    public static final String LOOPBACK_ADDRESS  = "127.0.0.1";

    public static final String LOOPBACK_ADDRESS_V6 = "::1";

    /**
     * interface is up
     */
    public final static int IFF_UP = 0x1;

    /**
     * broadcast address valid
     */
    public final static int IFF_BROADCAST = 0x2;

    /**
     * debugging is on
     */
    public final static int IFF_DEBUG = 0x4;

    /**
     * is a loopback net
     */
    public final static int IFF_LOOPBACK = 0x8;

    /**
     * interface has a point-to-point link
     */
    public final static int IFF_POINTOPOINT = 0x10;

    /**
     * avoid use of trailers
     */
    public final static int IFF_NOTRAILERS = 0x20;

    /**
     * interface is running
     */
    public final static int IFF_RUNNING = 0x40;

    /**
     * no ARP protocol
     */
    public final static int IFF_NOARP = 0x80;

    /**
     * receive all packets
     */
    public final static int IFF_PROMISC = 0x100;

    /**
     * receive all multicast packets
     */
    public final static int IFF_ALLMULTI = 0x200;

    /**
     * supports multicast
     */
    public final static int IFF_MULTICAST = 0x800;

    public final static int IFF_SLAVE = 0x1000;

    public static final int RTF_UP = 0x1;

    public static final int RTF_GATEWAY = 0x2;

    public static final int RTF_HOST = 0x4;

    public final static int CONN_CLIENT = 0x01;
    public final static int CONN_SERVER = 0x02;

    public final static int CONN_TCP  = 0x10;
    public final static int CONN_UDP  = 0x20;
    public final static int CONN_RAW  = 0x40;
    public final static int CONN_UNIX = 0x80;

    public final static int CONN_PROTOCOLS = 
        NetFlags.CONN_TCP | NetFlags.CONN_UDP | 
        NetFlags.CONN_RAW | NetFlags.CONN_UNIX;

    public static final int TCP_ESTABLISHED = 1;
    public static final int TCP_SYN_SENT    = 2;
    public static final int TCP_SYN_RECV    = 3;
    public static final int TCP_FIN_WAIT1   = 4;
    public static final int TCP_FIN_WAIT2   = 5;
    public static final int TCP_TIME_WAIT   = 6;
    public static final int TCP_CLOSE       = 7;
    public static final int TCP_CLOSE_WAIT  = 8;
    public static final int TCP_LAST_ACK    = 9;
    public static final int TCP_LISTEN      = 10;
    public static final int TCP_CLOSING     = 11;
    public static final int TCP_IDLE        = 12;
    public static final int TCP_BOUND       = 13;
    public static final int TCP_UNKNOWN     = 14;

    public static int getConnectionProtocol(String protocol) 
        throws SigarException {

        if (protocol.equals("tcp")) {
            return NetFlags.CONN_TCP;
        }
        else if (protocol.equals("udp")) {
            return NetFlags.CONN_UDP;
        }
        else if (protocol.equals("raw")) {
            return NetFlags.CONN_RAW;
        }
        else if (protocol.equals("unix")) {
            return NetFlags.CONN_UNIX;
        }

        String msg = "Protocol '" + protocol + "' not supported";
        throw new SigarException(msg);
    }

    /**
     * @param flags network interface flags.
     * @return String representation of network interface flags.
     * @see org.hyperic.sigar.NetInterfaceConfig#getFlags
     */
    public static native String getIfFlagsString(long flags);

    public static boolean isAnyAddress(String address) {
        return
            (address == null) ||
            address.equals(ANY_ADDR) ||
            address.equals(ANY_ADDR_V6);
    }

    public static boolean isLoopback(String address) {
        return
            address.equals(LOOPBACK_HOSTNAME) ||
            address.equals(LOOPBACK_ADDRESS) ||
            address.equals(LOOPBACK_ADDRESS_V6);
    }
}
