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
SUBDIRS = input witness
EXTRA_DIST = L7D_en_US.properties L7D_fr_CH.properties L7D_fr.properties



# if we are building in a separate build tree, then copy all necessary files
all-local:
	@if test "$(top_srcdir)" != "$(top_builddir)"; then \
	  echo "Copying test suite data files ..." ; \
	  list='$(EXTRA_DIST)'; for file in $$list; do \
	    cp -p $(srcdir)/$$file . ; \
	  done \
	fi

prepare:
	@if test ! -d "output"; then \
		mkdir output; \
	fi;
	@rm -rf log4j.xml
	@rm -rf log4j.properties
	@rm -rf log4cxx.xml
	@rm -rf log4cxx.properties
	
build: check-recursive prepare

cleanOutputDir:
	@rm -rf output/*
	
check: prepare run-unittest DefaultInit

run-unittest: build cleanOutputDir
	@export TOTO=wonderful; \
	export key1=value1; \
	export key2=value2; \
	../cpp/testsuite -v
	
DefaultInit: build cleanOutputDir
	../cpp/testsuite -v testcase1
	@cp input/xml/defaultInit.xml log4j.xml
	../cpp/testsuite -v testcase2
	@rm -f log4j.xml
	@cp input/defaultInit3.properties log4j.properties
	../cpp/testsuite -v testcase3
	@rm -f log4j.properties
	@cp input/defaultInit3.properties log4j.properties
	@cp input/xml/defaultInit.xml log4j.xml
	../cpp/testsuite -v testcase4
	@rm -f log4j.xml
	@rm -f log4j.properties

SocketServer: build cleanOutputDir
	../cpp/testsuite -v socketservertestcase

