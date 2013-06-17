#!/usr/bin/perl
#
# Copyright (c) 2004-2005 Hyperic, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

use strict;
use Sigar;

my $sigar = new Sigar;

my $iflist = $sigar->net_interface_list;

for my $ifname (@$iflist) {
    my $ifconfig = $sigar->net_interface_config($ifname);
    my $flags = $ifconfig->flags;

    my $encap = $ifconfig->type;

    my $hwaddr = $ifconfig->hwaddr;
    if ($hwaddr eq Sigar::NULL_HWADDR) {
        $hwaddr = "";
    }
    else {
        $hwaddr = " HWaddr " . $hwaddr;
    }

    print $ifname . "\t" . "Link encap:" . $encap . $hwaddr . "\n";

    my $ptp = "";
    if ($flags & Sigar::IFF_POINTOPOINT) {
        $ptp = "  P-t-P:" . $ifconfig->destination;
    }

    my $bcast = "";
    if ($flags & Sigar::IFF_BROADCAST) {
        $bcast = "  Bcast:" . $ifconfig->broadcast;
    }

    print "\t" . "inet addr:" . $ifconfig->address .
      $ptp . $bcast . "  Mask:" . $ifconfig->netmask . "\n";

    print "\t" .
      Sigar::net_interface_flags_string($flags) .
      " MTU:" . $ifconfig->mtu .
      "  Metric:" . $ifconfig->metric . "\n";

    my $ifstat;
    eval {
        $ifstat = $sigar->net_interface_stat($ifname);
    } or next;

    print "\t" .
      "RX packets:" . $ifstat->rx_packets .
      " errors:" . $ifstat->rx_errors .
      " dropped:" . $ifstat->rx_dropped .
      " overruns:" . $ifstat->rx_overruns .
      " frame:" . $ifstat->rx_frame . "\n";

    print "\t" .
      "TX packets:" . $ifstat->tx_packets .
      " errors:" . $ifstat->tx_errors .
      " dropped:" . $ifstat->tx_dropped .
      " overruns:" . $ifstat->tx_overruns .
      " carrier:" . $ifstat->tx_carrier . "\n";

    print "\t" . "collisions:" . $ifstat->tx_collisions . "\n";

    my $rx_bytes = $ifstat->rx_bytes;
    my $tx_bytes = $ifstat->tx_bytes;

    print "\t" .
      "RX bytes:" . $rx_bytes .
        " (" . Sigar::format_size($rx_bytes) . ")" .
        "  " .
        "TX bytes:" . $tx_bytes .
        " (" . Sigar::format_size($tx_bytes) . ")" . "\n\n";
}
