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

my $mem = $sigar->mem;

my $swap = $sigar->swap;

print "\tTotal\tUsed\tFree\n";

print "Mem:    " .
      ($mem->total / 1024) . "\t" .
      ($mem->used / 1024) . "\t" .
      ($mem->free / 1024) . "\n";

print "Swap:   " .
      ($swap->total / 1024) . "\t" .
      ($swap->used / 1024) . "\t" .
      ($swap->free / 1024) . "\n";

print "RAM:    " . $mem->ram . "MB\n";
