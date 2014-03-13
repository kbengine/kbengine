# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.
# The ASF licenses this file to You under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with
# the License.  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
noinst_PROGRAMS = trivial delayedloop stream console

INCLUDES = -I$(top_srcdir)/src/main/include -I$(top_builddir)/src/main/include

trivial_SOURCES = trivial.cpp
trivial_LDADD = $(top_builddir)/src/main/cpp/liblog4cxx.la

delayedloop_SOURCES = delayedloop.cpp
delayedloop_LDADD = $(top_builddir)/src/main/cpp/liblog4cxx.la

stream_SOURCES = stream.cpp
stream_LDADD = $(top_builddir)/src/main/cpp/liblog4cxx.la

console_SOURCES = console.cpp
console_LDADD = $(top_builddir)/src/main/cpp/liblog4cxx.la

