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

sigar = Sigar.new

infos = sigar.cpu_info_list

num = infos.length

puts num.to_s + " total CPUs.."

infos.each do |info|
    puts "Vendor........" + info.vendor
    puts "Model........." + info.model
    puts "Mhz..........." + info.mhz.to_s
    puts "Cache size...." + info.cache_size.to_s
end
