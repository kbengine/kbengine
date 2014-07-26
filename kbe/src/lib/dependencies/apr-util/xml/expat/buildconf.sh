#!/bin/sh
# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.
# The ASF licenses this file to You under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with
# the License.  You may obtain a copy of the License at
# 
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
#

# buildconf: Build the support scripts needed to compile from a
#            checked-out version of the source code.

if [ "$1" = "--verbose" -o "$1" = "-v" ]; then
    verbose="--verbose"
    shift
fi

libtoolize=`conftools/PrintPath glibtoolize1 glibtoolize libtoolize15 libtoolize14 libtoolize`
if [ "x$libtoolize" = "x" ]; then
    echo "libtoolize not found in path"
    exit 1
fi

# Create the libtool helper files
#
# Note: we copy (rather than link) them to simplify distribution.
# Note: APR supplies its own config.guess and config.sub -- we do not
#       rely on libtool's versions
#
echo "Copying libtool helper files using $libtoolize"

# Remove any libtool files so one can switch between libtool versions
# by simply rerunning the buildconf script.
rm -rf autom4te*.cache aclocal.m4
m4files='argz.m4 libtool.m4 ltoptions.m4 ltsugar.m4 ltversion.m4 lt~obsolete.m4'
(cd conftools ; rm -f ltconfig ltmain.sh aclocal.m4 $m4files)

# Determine libtool version, because --copy behaves differently
# w.r.t. copying libtool.m4
lt_pversion=`$libtoolize --version 2>/dev/null|sed -e 's/([^)]*)//g;s/^[^0-9]*//;s/[- ].*//g;q'`
lt_version=`echo $lt_pversion|sed -e 's/\([a-z]*\)$/.\1/'`
IFS=.; set $lt_version; IFS=' '

# libtool 1
if test "$1" = "1"; then
  $libtoolize --copy --automake
  # Unlikely, maybe for old versions the file exists
  if [ -f libtool.m4 ]; then 
    ltfile=`pwd`/libtool.m4
  else

    # Extract all lines setting variables from libtoolize up until
    # libtool_m4 gets set
    ltfindcmd="`sed -n \"/=[^\\\`]/p;/libtool_m4=/{s/.*=/echo /p;q;}\" \
                   < $libtoolize`"

    # Get path to libtool.m4 either from LIBTOOL_M4 env var or our libtoolize based script
    ltfile=${LIBTOOL_M4-`eval "$ltfindcmd"`}

    # Expecting the code above to be very portable, but just in case...
    if [ -z "$ltfile" -o ! -f "$ltfile" ]; then
      ltpath=`dirname $libtoolize`
      ltfile=`cd $ltpath/../share/aclocal ; pwd`/libtool.m4
    fi
  fi
  if [ ! -f $ltfile ]; then
    echo "$ltfile not found"
    exit 1
  fi
  # Do we need this anymore?
  echo "Using libtool.m4 at ${ltfile}."
  rm -f conftools/libtool.m4
  cp -p $ltfile conftools/libtool.m4

# libtool 2
elif test "$1" = "2"; then
  $libtoolize --copy --quiet $verbose
fi

cross_compile_warning="warning: AC_TRY_RUN called without default to allow cross compiling"

#
# Generate the autoconf header template (config.h.in) and ./configure
#
echo "Creating config.h.in ..."
${AUTOHEADER:-autoheader} $verbose 2>&1 | grep -v "$cross_compile_warning"

echo "Creating configure ..."
### do some work to toss config.cache?
${AUTOCONF:-autoconf} $verbose 2>&1 | grep -v "$cross_compile_warning"

# Clean up any leftovers and autoconf caches
rm -rf autom4te*.cache aclocal.m4 libtool.m4

exit 0
