# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.
# The ASF licenses this file to You under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with
# the License.  You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

BEGIN {

  # fetch APR version numbers from input file and write them to STDOUT

  while ((getline < ARGV[1]) > 0) {
    if (match ($0, /^#define APR_MAJOR_VERSION/)) {
      ver_major = $3;
    }
    else if (match ($0, /^#define APR_MINOR_VERSION/)) {
      ver_minor = $3;
    }
    else if (match ($0, /^#define APR_PATCH_VERSION/)) {
      ver_patch = $3;
    }
    else if (match ($0, /^#define APR_IS_DEV_VERSION/)) {
      ver_devbuild = 1;
    }
  }
  ver_str = ver_major "." ver_minor "." ver_patch (ver_devbuild ? "-dev" : "");
  if (WANTED) {
    ver_num = ver_major * 1000000 + ver_minor * 1000 + ver_patch;
    if (ver_num < WANTED) {
      print "ERROR: APR version " ver_str " does NOT match!";
      exit 1;
    } else if (ver_num > (WANTED + 1000)) {
      print "WARNING: APR version " ver_str " higher than expected!";
      exit 0;
    } else {
      print "OK: APR version " ver_str "";
      exit 0;
    }
  } else {
    ver_nlm = ver_major "," ver_minor "," ver_patch;
    print "VERSION = " ver_nlm "";
    print "VERSION_STR = " ver_str "";
    print "VERSION_MAJMIN = " ver_major ver_minor "";
  }

}


