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

package org.hyperic.sigar.cmd;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.ArrayList;

import org.hyperic.sigar.SigarException;
import org.hyperic.sigar.NetConnection;
import org.hyperic.sigar.NetFlags;
import org.hyperic.sigar.Tcp;

/**
 * Display network connections.
 */
public class Netstat extends SigarCommandBase {

    private static final int LADDR_LEN = 20;
    private static final int RADDR_LEN = 35;

    private static final String[] HEADER = new String[] {
        "Proto",
        "Local Address",
        "Foreign Address",
        "State",
        ""
    };

    private static boolean isNumeric, wantPid, isStat;

    public Netstat(Shell shell) {
        super(shell);
    }

    public Netstat() {
        super();
    }

    protected boolean validateArgs(String[] args) {
        return true;
    }

    public String getUsageShort() {
        return "Display network connections";
    }

    //poor mans getopt.
    public static int getFlags(String[] args, int flags) {
        int proto_flags = 0;
        isNumeric = false;
        wantPid = false;
        isStat = false;

        for (int i=0; i<args.length; i++) {
            String arg = args[i];
            int j = 0;

            while (j<arg.length()) {
                switch (arg.charAt(j++)) {
                  case '-':
                    continue;
                  case 'l':
                    flags &= ~NetFlags.CONN_CLIENT;
                    flags |= NetFlags.CONN_SERVER;
                    break;
                  case 'a':
                    flags |= NetFlags.CONN_SERVER | NetFlags.CONN_CLIENT;
                    break;
                  case 'n':
                    isNumeric = true;
                    break;
                  case 'p':
                    wantPid = true;
                    break;
                  case 's':
                    isStat = true;
                    break;
                  case 't':
                    proto_flags |= NetFlags.CONN_TCP;
                    break;
                  case 'u':
                    proto_flags |= NetFlags.CONN_UDP;
                    break;
                  case 'w':
                    proto_flags |= NetFlags.CONN_RAW;
                    break;
                  case 'x':
                    proto_flags |= NetFlags.CONN_UNIX;
                    break;
                  default:
                    System.err.println("unknown option");
                }
            }
        }

        if (proto_flags != 0) {
            flags &= ~NetFlags.CONN_PROTOCOLS;
            flags |= proto_flags;
        }

        return flags;
    }

    private String formatPort(int proto, long port) {
        if (port == 0) {
            return "*";
        }
        if (!isNumeric) {
            String service = this.sigar.getNetServicesName(proto, port);
            if (service != null) {
                return service;
            }
        }
        return String.valueOf(port);
    }

    private String formatAddress(int proto, String ip,
                                 long portnum, int max) {
        
        String port = formatPort(proto, portnum);
        String address;

        if (NetFlags.isAnyAddress(ip)) {
            address = "*";
        }
        else if (isNumeric) {
            address = ip;
        }
        else {
            try {
                address = InetAddress.getByName(ip).getHostName();
            } catch (UnknownHostException e) {
                address = ip;
            }
        }

        max -= port.length() + 1;
        if (address.length() > max) {
            address = address.substring(0, max);
        }

        return address + ":" + port; 
    }

    private void outputTcpStats() throws SigarException {
        Tcp stat = this.sigar.getTcp();
        final String dnt = "    ";
        println(dnt + stat.getActiveOpens() + " active connections openings");
        println(dnt + stat.getPassiveOpens() + " passive connection openings");
        println(dnt + stat.getAttemptFails() + " failed connection attempts");
        println(dnt + stat.getEstabResets() + " connection resets received");
        println(dnt + stat.getCurrEstab() + " connections established");
        println(dnt + stat.getInSegs() + " segments received");
        println(dnt + stat.getOutSegs() + " segments send out");
        println(dnt + stat.getRetransSegs() + " segments retransmited");
        println(dnt + stat.getInErrs() + " bad segments received.");
        println(dnt + stat.getOutRsts() + " resets sent");
    }

    private void outputStats(int flags) throws SigarException {
        if ((flags & NetFlags.CONN_TCP) != 0) {
            println("Tcp:");
            try {
                outputTcpStats();
            } catch (SigarException e) {
                println("    " + e);
            }
        }
    }

    //XXX currently weak sauce.  should end up like netstat command.
    public void output(String[] args) throws SigarException {
        //default
        int flags = NetFlags.CONN_CLIENT | NetFlags.CONN_PROTOCOLS;

        if (args.length > 0) {
            flags = getFlags(args, flags);
            if (isStat) {
                outputStats(flags);
                return;
            }
        }

        NetConnection[] connections = this.sigar.getNetConnectionList(flags);
        printf(HEADER);

        for (int i=0; i<connections.length; i++) {
            NetConnection conn = connections[i];
            String proto = conn.getTypeString();
            String state;

            if (conn.getType() == NetFlags.CONN_UDP) {
                state = "";
            }
            else {
                state = conn.getStateString();
            }

            ArrayList items = new ArrayList();
            items.add(proto);
            items.add(formatAddress(conn.getType(),
                                    conn.getLocalAddress(),
                                    conn.getLocalPort(),
                                    LADDR_LEN));
            items.add(formatAddress(conn.getType(),
                                    conn.getRemoteAddress(),
                                    conn.getRemotePort(),
                                    RADDR_LEN));
            items.add(state);

            String process = null;
            if (wantPid &&
                //XXX only works w/ listen ports
                (conn.getState() == NetFlags.TCP_LISTEN))
            {
                try {
                    long pid =
                        this.sigar.getProcPort(conn.getType(),
                                               conn.getLocalPort());
                    if (pid != 0) { //XXX another bug
                        String name =
                            this.sigar.getProcState(pid).getName();
                        process = pid + "/" + name;
                    }
                } catch (SigarException e) {
                }
            }

            if (process == null) {
                process = "";
            }

            items.add(process);

            printf(items);
        }
    }

    public static void main(String[] args) throws Exception {
        new Netstat().processCommand(args);
    }
}
