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
EXTRA_DIST = sbr-test* \
             tbr-test*

# if we are building in a separate build tree, then copy all necessary files
all-local:
	@if test "$(top_srcdir)" != "$(top_builddir)"; then \
	  echo "Copying test suite data files ..." ; \
	  list='$(EXTRA_DIST)'; for file in $$list; do \
	    cp -p $(srcdir)/$$file . ; \
	  done \
	fi
