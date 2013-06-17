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

require 'mkmf'

extension_name = 'rbsigar'

installdir = '../java/sigar-bin'
#XXX hardwired for the moment
libname = 'sigar-universal-macosx'

$CPPFLAGS += ' -I' + installdir + '/include'
$LOCAL_LIBS += '-L' + installdir + '/lib -l' + libname

dir_config(extension_name)

system('perl -Mlib=.. -MSigarWrapper -e generate Ruby .')

create_makefile(extension_name)
