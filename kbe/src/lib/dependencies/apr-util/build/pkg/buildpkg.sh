#!/bin/sh
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
#
#

# buildpkg.sh: This script builds a Solaris PKG from the source tree
#              provided.

PREFIX=/usr/local
TEMPDIR=/var/tmp/$USER/apr-util-root
rm -rf $TEMPDIR

apr_util_src_dir=.
apr_src_dir=../apr
expat_dir=/usr

while test $# -gt 0 
do
  # Normalize
  case "$1" in
  -*=*) optarg=`echo "$1" | sed 's/[-_a-zA-Z0-9]*=//'` ;;
  *) optarg= ;;
  esac

  case "$1" in
  --with-apr=*)
  apr_src_dir=$optarg
  ;;
  esac

  case "$1" in
  --with-apr-util=*)
  apr_util_src_dir=$optarg
  ;;
  esac

  case "$1" in
  --with-expat=*)
  expat_dir=$optarg
  ;;
  esac

  shift
done

if [ -f "$apr_util_src_dir/configure.in" ]; then
  cd $apr_util_src_dir
else
  echo "The apr-util source could not be found within $apr_util_src_dir"
  echo "Usage: buildpkg [--with-apr=dir] [--with-apr-util=dir] [--with-expat=dir]"
  exit 1
fi

if [ ! -f "$apr_src_dir/configure.in" ]; then
  echo "The apr source could not be found within $apr_src_dir"
  echo "Usage: buildpkg [--with-apr=dir] [--with-apr-util=dir] [--with-expat=dir]"
  exit 1
fi

if [ ! -d "$expat_dir" ]; then
  echo "The expat directory could not be found within $expat_dir"
  echo "Usage: buildpkg [--with-apr=dir] [--with-apr-util=dir] [--with-expat=dir]"
  exit 1
fi

./configure --prefix=$PREFIX --with-apr=$apr_src_dir \
            --with-ldap --with-expat=$expat_dir
make
make install DESTDIR=$TEMPDIR
rm $TEMPDIR$PREFIX/lib/aprutil.exp
. build/pkg/pkginfo
cp build/pkg/pkginfo $TEMPDIR$PREFIX

current=`pwd`
cd $TEMPDIR$PREFIX
echo "i pkginfo=./pkginfo" > prototype
find . -print | grep -v ./prototype | grep -v ./pkginfo | pkgproto | awk '{print $1" "$2" "$3" "$4" root bin"}' >> prototype
mkdir $TEMPDIR/pkg
pkgmk -r $TEMPDIR$PREFIX -d $TEMPDIR/pkg

cd $current
pkgtrans -s $TEMPDIR/pkg $current/$NAME-$VERSION-$ARCH-local
gzip $current/$NAME-$VERSION-$ARCH-local

rm -rf $TEMPDIR

