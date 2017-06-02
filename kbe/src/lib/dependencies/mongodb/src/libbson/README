# Libbson

libbson is a library providing useful routines related to building, parsing,
and iterating BSON documents.


## Building

Libbson tries to support a variety of operation systems and architectures.
The following are known to work. If your platform is not listed here, it may
still work, we simply haven't tested it. If you would like us to add your
platform here, we would be happy to hear from you.

### Supported Operating Systems

 * RHEL/CentOS 5, 6, 7beta
 * SLES 11 SP3
 * Ubuntu 12.04 LTS
 * Debian 7
 * SmartOS
 * Solaris
 * FreeBSD 10
 * Windows Vista, 7, 8
 * OS X 10.8

### Supported Architectures

 * x86
 * x86_64/amd64
 * SPARC
 * ARM
 * PowerPC

### Supported Compilers

 * GCC 4.1 and newer
 * Clang 3.3 and newer
 * Visual Studio (MSVC) 2010 and newer
 * Oracle Solaris Studio (5.7 and newer, Solaris 10)

### Dependencies

#### Fedora / RedHat Enterprise / CentOS

```sh
yum install git automake autoconf libtool gcc
```

#### Debian / Ubuntu

```sh
apt-get install git-core automake autoconf libtool gcc
```

#### FreeBSD

```sh
pkg install git automake autoconf libtool gcc pkgconf
```

#### OS X

You'll need to have [XCode](https://developer.apple.com/xcode/download/) (at least
the command-line package) and we recommend using [Homebrew](http://brew.sh/) for
other dependencies.
```sh
brew install git automake autoconf libtool pkgconfig
```

#### SmartOS

```sh
pkgin install git automake autoconf libtool gcc47 gmake pkg-config
export PATH=/opt/local/gcc47/bin:$PATH
```

#### Windows Vista and Higher

Builds on Windows Vista and Higher require cmake to build Visual Studio project files.
Alternatively, you can use cygwin or mingw with the automake based build.

```sh
git clone git://github.com/mongodb/libbson.git
cd libbson
cmake.exe -G "Visual Studio 10 Win64" "-DCMAKE_INSTALL_PREFIX=C:\install\path"
msbuild.exe ALL_BUILD.vcxproj
msbuild.exe INSTALL.vcxproj
```

For the adventurous, you can cross-compile for Windows from Fedora easily using mingw.

```sh
sudo yum install mingw64-gcc automake autoconf libtool
./configure --host=x86_64-w64-mingw32
```

### From Git

```sh
git clone git://github.com/mongodb/libbson.git
cd libbson/
git checkout x.y.z  # To build a particular release
./autogen.sh
make
sudo make install
```

You can run the unit tests with

```sh
make test
```

### From Tarball

```sh
tar xzf libbson-$ver.tar.gz
./configure
make
sudo make install
```

## Configuration Options

You may be interested in the following options for `./configure`.
These are not availble when using the alternate CMake build system.

```
--help                    Show all possible help options.
                          There are many more than those displayed here.

--enable-optimizations    Enable various compile and link optimizations.
--enable-debug            Enable debugging features.
--enable-debug-symbols    Link with debug symbols in tact.
--enable-hardening        Enable stack protector and other hardening features.
--enable-silent-rules     Force silent rules like the Linux kernel.
--enable-coverage         Compile with gcov support.
--enable-static           Build static archives (.a).
```

# Developing using libbson

In your source code:

```c
#include <bson.h>
```

To get the include path and libraries appropriate for your system.

```sh
gcc my_program.c $(pkg-config --cflags --libs libbson-1.0)
```

## Examples

See the `examples/` directory for how to use the libbson library in your
application.

## Documentation

See the `doc/` directory for documentation on individual types.

## Bug reports and Feature requests

Please use [the MongoDB C Driver JIRA project](https://jira.mongodb.org/browse/CDRIVER) to report bugs or request features.
