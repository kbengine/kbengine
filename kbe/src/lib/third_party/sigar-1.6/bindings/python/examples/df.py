#
# Copyright (c) 2007 Hyperic, Inc.
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

import os, sigar;

sg = sigar.open()
fslist = sg.file_system_list()

def format_size(size):
    return sigar.format_size(size * 1024)

print 'Filesystem\tSize\tUsed\tAvail\tUse%\tMounted on\tType\n'

for fs in fslist:
    dir_name = fs.dir_name()
    usage = sg.file_system_usage(dir_name)

    total = usage.total()
    used = total - usage.free()
    avail = usage.avail()
    pct = usage.use_percent() * 100
    if pct == 0.0:
        pct = '-'

    print fs.dev_name(), format_size(total), format_size(used), format_size(avail),\
        pct, dir_name, fs.sys_type_name(), '/', fs.type_name()
