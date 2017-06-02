# Contributing to mongo-c-driver

Thanks for considering contributing to the mongo-c-driver!

This document intends to be a short guide to helping you contribute to the codebase.
It expects a familiarity with the C programming language and writing portable software.
Whenever in doubt, feel free to ask others that have contributed or look at the existing body of code.


## Guidelines

The mongo-c-driver has a few guidelines that help direct the process.


### Portability

mongo-c-driver is portable software. It needs to run on a multitude of
operating systems and architectures.

 * Linux (RHEL 5 and newer)
 * FreeBSD (10 and newer)
 * Windows (Vista a newer)
 * Solaris x86_64/SPARC (11 and newer)
 * SmartOS (Solaris based)
 * Possibly more if users show an interest.
 * ARM/SPARC/x86/x86_64


### Licensing

Some of the mongo-c-driver users embed the library statically in their
products.  Therefore, the driver needs to be liberally licensed (as opposed to
the authors usual preference of LGPL-2+). Therefore, all contributions must
also be under this license. As a policy, we have chosen Apache 2.0 as the
license for the project.


### Coding Style

We try not to be pedantic with taking contributions that are not properly
formatted, but we will likely perform a followup commit that cleans things up.
The basics are, in vim:

 : set ts=3 sw=3 et

3 space tabs, insert spaces instead of tabs.

Place a space between the function name and the parameter as such:

```c
static void
my_func (Param *p)

my_func (p);
```

Not all of the code does this today, but it should be cleaned up at some point.

Just look at the code around for more pedantic styling choices.


### Enum, Struct, Variable Naming

The naming conventions for mongo-c-driver should feel very object oriented.
In fact, mongo-c-driver is OOP. Those that have used the GLib library will
feel right at home, as the author has spent many years contributing to that
project as well.

Structs are suffixed in `_t`, and underscores.

```c
typedef struct _my_struct_t my_struct_t;

struct _my_struct_t
{
   int foo;
};
```

Function names should be prefixed by the type name, without the `_t`.

```c
int my_struct_get_foo (my_struct_t *my);
```

Enums are also named with the `_t` suffix.


```c
typedef enum
{
   MY_FLAGS_A = 1,
   MY_FLAGS_B = 1 << 1,
   MY_FLAGS_C = 1 << 2,
} my_flags_t;
```

### Adding a new symbol

This should be done rarely but there are several things that you need to do
when adding a new symbol.

 - Add the symbol to `src/libmongoc.symbols`
 - Add the symbol to `build/autotools/versions.ldscript`
 - Add the symbol to `build/cmake/libmongoc.def`
 - Add the symbol to `build/cmake/libmongoc-ssl.def`
 - Add documentation for the new symbol in `doc/mongoc_your_new_symbol_name.page`

### Documentation

We strive to document all symbols. See doc/ for documentation examples. If you
add a new function, add a new .txt file describing the function so that we can
generate man pages and HTML for it.


### Testing

To run the entire test suite, including authentication tests,
start `mongod` with auth enabled:

```
$ mongod --auth
```

In another terminal, use the `mongo` shell to create a user:

```
$ mongo --eval "db.createUser({user: 'admin', pwd: 'pass', roles: ['root']})" admin
```

To authenticate against MongoDB 3.0+ requires SCRAM-SHA-1, which in turn
requires a driver built with OpenSSL:

```
$ ./configure --enable-ssl`
```

Set the user and password environment variables, then build and run the tests:

```
$ export MONGOC_TEST_USER=admin
$ export MONGOC_TEST_PASSWORD=pass
$ make test
```

Additional environment variables:

* `MONGOC_TEST_HOST`: default `localhost`, the host running MongoDB.
* `MONGOC_TEST_PORT`: default 27017, MongoDB's listening port.
* `MONGOC_TEST_SERVER_VERBOSE`: set to `on` for wire protocol logging from 
  tests that use `mock_server_t`. 

If you start `mongod` with SSL, set these variables to configure how
`make test` connects to it:

* `MONGOC_TEST_SSL`: set to `on` to connect to the server with SSL.
* `MONGOC_TEST_SSL_PEM_FILE`: path to a client PEM file.
* `MONGOC_TEST_SSL_PEM_PWD`: the PEM file's password.
* `MONGOC_TEST_SSL_CA_FILE`: path to a certificate authority file.
* `MONGOC_TEST_SSL_CA_DIR`: path to a certificate authority directory.
* `MONGOC_TEST_SSL_CRL_FILE`: path to a certificate revocation list.
* `MONGOC_TEST_SSL_WEAK_CERT_VALIDATION`: set to `on` to relax the client's
  validation of the server's certificate.

The SASL / GSSAPI / Kerberos tests are skipped by default. To run them, set up a
separate `mongod` with Kerberos and set its host and Kerberos principal name
as environment variables:

* `MONGOC_TEST_GSSAPI_HOST` 
* `MONGOC_TEST_GSSAPI_USER` 

URI-escape the username, for example write "user@realm" as "user%40realm".
The user must be authorized to query `test.collection`.

The SASL / GSSAPI / Kerberos tests are skipped by default. To run them, set up a
separate `mongod` with Kerberos and set its host and Kerberos principal name
as environment variables:

* `MONGOC_TEST_GSSAPI_HOST` 
* `MONGOC_TEST_GSSAPI_USER` 

URI-escape the username, for example write "user@realm" as "user%40realm".
The user must be authorized to query `test.collection`.

MongoDB 3.2 adds support for readConcern, but does not enable support for
read concern majority by default. mongod must be launched using
`--enableMajorityReadConcern`.
The test framework does not (and can't) automatically discover if this option was
provided to MongoDB, so an additional variable must be set to enable these tests:

* `MONGOC_ENABLE_MAJORITY_READ_CONCERN`

Set this environment variable to `on` if MongoDB has enabled majority read concern.

Some tests require Internet access, e.g. to check the error message when failing
to open a MongoDB connection to example.com. Skip them with:

* `MONGOC_TEST_OFFLINE`

All tests should pass before submitting a patch.

## Configuring the test runner

The test runner can be configured by declaring the `TEST_ARGS` environment
variable. The following options can be provided:

```
    -h, --help   Show this help menu.
    -f           Do not fork() before running tests.
    -l NAME      Run test by name, e.g. "/Client/command" or "/Client/*".
    -p           Do not run tests in parallel.
    -v           Be verbose with logs.
```

`TEST_ARGS` is set to "-f -p" by default.

To run just a specific portion of the test suite use the -l option like so:

```
$ make test TEST_ARGS="-l /server_selection/*"
```

The full list of tests is shown in the help.

## Debugging failed tests

The easiest way to debug a failed tests is to use the `debug` make target:

```
$ make debug TEST_ARGS="-l /WriteConcern/bson_omits_defaults"
```

This will build all dependencies and leave you in a debugger ready to run the test.
