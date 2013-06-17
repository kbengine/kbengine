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

require 'rbsigar'

def format_size(size)
  return Sigar.format_size(size * 1024)
end

sigar = Sigar.new
fslist = sigar.file_system_list

puts "Filesystem\tSize\tUsed\tAvail\tUse%\tMounted on\tType\n"

fslist.each do |fs|
  dir_name = fs.dir_name
  usage = sigar.file_system_usage(dir_name)

  total = usage.total
  used = total - usage.free
  avail = usage.avail
  pct = usage.use_percent * 100

  puts fs.dev_name + "\t" +
    format_size(total) + "\t" +
    format_size(used) + "\t" +
    format_size(avail) + "\t" +
    (pct == 0.0 ? '-' : pct.to_s) + "\t" +
    dir_name + "\t" + 
    fs.sys_type_name + "/" + fs.type_name
end
