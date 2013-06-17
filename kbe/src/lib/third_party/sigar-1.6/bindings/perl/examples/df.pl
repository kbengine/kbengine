#!/usr/bin/perl
#
# Copyright (c) 2004 Hyperic, Inc.
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

my $fslist = $sigar->file_system_list;

print "Filesystem\tSize\tUsed\tAvail\tUse%\tMounted on\tType\n";

for my $fs (@$fslist) {
    my $dirname = $fs->dir_name;
    my $usage;

    #e.g. on win32 D:\ fails with "Device not ready"
    #if there is no cd in the drive.
    eval {
        $usage = $sigar->file_system_usage($dirname);
    } or next;

    my $total = $usage->total;
    my $used  = $total - $usage->free;
    my $avail = $usage->avail;
    my $pct   = $usage->use_percent * 100;
    my $usePct;

    if ($pct == 0) {
        $usePct = "-"
    }
    else {
        $usePct = $pct . '%';
    }

    print
      $fs->dev_name . "\t" .
      format_size($total) . "\t" .
      format_size($used) . "\t" .
      format_size($avail) . "\t" .
      $usePct . "\t" .
      $dirname . "\t" .
      $fs->sys_type_name . "/" . $fs->type_name . "\n";

}

sub format_size {
    my($size) = @_;
    return Sigar::format_size($size * 1024);
}
