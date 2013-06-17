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

public class NetStat implements java.io.Serializable {

    private static final long serialVersionUID = 04052007L;

    protected int[] tcpStates;
    protected int tcpInboundTotal, tcpOutboundTotal;
    protected int allInboundTotal, allOutboundTotal;

    native void stat(Sigar sigar, int flags,
                     byte[] address, long port) throws SigarException;

    public NetStat() { }

    public void stat(Sigar sigar) throws SigarException {
        int flags =
            NetFlags.CONN_SERVER | NetFlags.CONN_CLIENT |
            NetFlags.CONN_TCP;

        stat(sigar, flags, null, -1);
    }

    public void stat(Sigar sigar,
                     byte[] address, long port)
        throws SigarException {

        int flags =
            NetFlags.CONN_CLIENT | NetFlags.CONN_TCP;

        stat(sigar, flags, address, port);
    }

    public int getTcpInboundTotal() {
        return this.tcpInboundTotal;
    }

    public int getTcpOutboundTotal() {
        return this.tcpOutboundTotal;
    }

    public int getAllInboundTotal() {
        return this.allInboundTotal;
    }

    public int getAllOutboundTotal() {
        return this.allOutboundTotal;
    }

    public int[] getTcpStates() {
        return this.tcpStates;
    }

    //state counters
    public int getTcpEstablished() {
        return this.tcpStates[NetFlags.TCP_ESTABLISHED];
    }

    public int getTcpSynSent() {
        return this.tcpStates[NetFlags.TCP_SYN_SENT];
    }

    public int getTcpSynRecv() {
        return this.tcpStates[NetFlags.TCP_SYN_RECV];
    }

    public int getTcpFinWait1() {
        return this.tcpStates[NetFlags.TCP_FIN_WAIT1];
    }

    public int getTcpFinWait2() {
        return this.tcpStates[NetFlags.TCP_FIN_WAIT2];
    }

    public int getTcpTimeWait() {
        return this.tcpStates[NetFlags.TCP_TIME_WAIT];
    }

    public int getTcpClose() {
        return this.tcpStates[NetFlags.TCP_CLOSE];
    }

    public int getTcpCloseWait() {
        return this.tcpStates[NetFlags.TCP_CLOSE_WAIT];
    }

    public int getTcpLastAck() {
        return this.tcpStates[NetFlags.TCP_LAST_ACK];
    }

    public int getTcpListen() {
        return this.tcpStates[NetFlags.TCP_LISTEN];
    }

    public int getTcpClosing() {
        return this.tcpStates[NetFlags.TCP_CLOSING];
    }

    public int getTcpIdle() {
        return this.tcpStates[NetFlags.TCP_IDLE];
    }

    public int getTcpBound() {
        return this.tcpStates[NetFlags.TCP_BOUND];
    }
}
