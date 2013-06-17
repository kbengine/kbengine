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

my $infos = $sigar->cpu_info_list;

my $num = scalar @$infos;

print "$num total CPUs..\n";

for my $info (@$infos) {
    print "Vendor........" . $info->vendor . "\n";
    print "Model........." . $info->model . "\n";
    print "Mhz..........." . $info->mhz . "\n";
    print "Cache size...." . $info->cache_size . "\n";
}
