:mod:`socket` --- Low-level networking interface
================================================

.. module:: socket
   :synopsis: Low-level networking interface.


This module provides access to the BSD *socket* interface. It is available on
all modern Unix systems, Windows, MacOS, and probably additional platforms.

.. note::

   Some behavior may be platform dependent, since calls are made to the operating
   system socket APIs.

.. index:: object: socket

The Python interface is a straightforward transliteration of the Unix system
call and library interface for sockets to Python's object-oriented style: the
:func:`.socket` function returns a :dfn:`socket object` whose methods implement
the various socket system calls.  Parameter types are somewhat higher-level than
in the C interface: as with :meth:`read` and :meth:`write` operations on Python
files, buffer allocation on receive operations is automatic, and buffer length
is implicit on send operations.


.. seealso::

   Module :mod:`socketserver`
      Classes that simplify writing network servers.

   Module :mod:`ssl`
      A TLS/SSL wrapper for socket objects.


Socket families
---------------

Depending on the system and the build options, various socket families
are supported by this module.

The address format required by a particular socket object is automatically
selected based on the address family specified when the socket object was
created.  Socket addresses are represented as follows:

- The address of an :const:`AF_UNIX` socket bound to a file system node
  is represented as a string, using the file system encoding and the
  ``'surrogateescape'`` error handler (see :pep:`383`).  An address in
  Linux's abstract namespace is returned as a :class:`bytes` object with
  an initial null byte; note that sockets in this namespace can
  communicate with normal file system sockets, so programs intended to
  run on Linux may need to deal with both types of address.  A string or
  :class:`bytes` object can be used for either type of address when
  passing it as an argument.

   .. versionchanged:: 3.3
      Previously, :const:`AF_UNIX` socket paths were assumed to use UTF-8
      encoding.

- A pair ``(host, port)`` is used for the :const:`AF_INET` address family,
  where *host* is a string representing either a hostname in Internet domain
  notation like ``'daring.cwi.nl'`` or an IPv4 address like ``'100.50.200.5'``,
  and *port* is an integer.

- For :const:`AF_INET6` address family, a four-tuple ``(host, port, flowinfo,
  scopeid)`` is used, where *flowinfo* and *scopeid* represent the ``sin6_flowinfo``
  and ``sin6_scope_id`` members in :const:`struct sockaddr_in6` in C.  For
  :mod:`socket` module methods, *flowinfo* and *scopeid* can be omitted just for
  backward compatibility.  Note, however, omission of *scopeid* can cause problems
  in manipulating scoped IPv6 addresses.

- :const:`AF_NETLINK` sockets are represented as pairs ``(pid, groups)``.

- Linux-only support for TIPC is available using the :const:`AF_TIPC`
  address family.  TIPC is an open, non-IP based networked protocol designed
  for use in clustered computer environments.  Addresses are represented by a
  tuple, and the fields depend on the address type. The general tuple form is
  ``(addr_type, v1, v2, v3 [, scope])``, where:

  - *addr_type* is one of :const:`TIPC_ADDR_NAMESEQ`, :const:`TIPC_ADDR_NAME`,
    or :const:`TIPC_ADDR_ID`.
  - *scope* is one of :const:`TIPC_ZONE_SCOPE`, :const:`TIPC_CLUSTER_SCOPE`, and
    :const:`TIPC_NODE_SCOPE`.
  - If *addr_type* is :const:`TIPC_ADDR_NAME`, then *v1* is the server type, *v2* is
    the port identifier, and *v3* should be 0.

    If *addr_type* is :const:`TIPC_ADDR_NAMESEQ`, then *v1* is the server type, *v2*
    is the lower port number, and *v3* is the upper port number.

    If *addr_type* is :const:`TIPC_ADDR_ID`, then *v1* is the node, *v2* is the
    reference, and *v3* should be set to 0.

    If *addr_type* is :const:`TIPC_ADDR_ID`, then *v1* is the node, *v2* is the
    reference, and *v3* should be set to 0.

- A tuple ``(interface, )`` is used for the :const:`AF_CAN` address family,
  where *interface* is a string representing a network interface name like
  ``'can0'``. The network interface name ``''`` can be used to receive packets
  from all network interfaces of this family.

- A string or a tuple ``(id, unit)`` is used for the :const:`SYSPROTO_CONTROL`
  protocol of the :const:`PF_SYSTEM` family. The string is the name of a
  kernel control using a dynamically-assigned ID. The tuple can be used if ID
  and unit number of the kernel control are known or if a registered ID is
  used.

  .. versionadded:: 3.3

- Certain other address families (:const:`AF_BLUETOOTH`, :const:`AF_PACKET`,
  :const:`AF_CAN`) support specific representations.

  .. XXX document them!

For IPv4 addresses, two special forms are accepted instead of a host address:
the empty string represents :const:`INADDR_ANY`, and the string
``'<broadcast>'`` represents :const:`INADDR_BROADCAST`.  This behavior is not
compatible with IPv6, therefore, you may want to avoid these if you intend
to support IPv6 with your Python programs.

If you use a hostname in the *host* portion of IPv4/v6 socket address, the
program may show a nondeterministic behavior, as Python uses the first address
returned from the DNS resolution.  The socket address will be resolved
differently into an actual IPv4/v6 address, depending on the results from DNS
resolution and/or the host configuration.  For deterministic behavior use a
numeric address in *host* portion.

All errors raise exceptions.  The normal exceptions for invalid argument types
and out-of-memory conditions can be raised; starting from Python 3.3, errors
related to socket or address semantics raise :exc:`OSError` or one of its
subclasses (they used to raise :exc:`socket.error`).

Non-blocking mode is supported through :meth:`~socket.setblocking`.  A
generalization of this based on timeouts is supported through
:meth:`~socket.settimeout`.


Module contents
---------------

The module :mod:`socket` exports the following elements.


Exceptions
^^^^^^^^^^

.. exception:: error

   A deprecated alias of :exc:`OSError`.

   .. versionchanged:: 3.3
      Following :pep:`3151`, this class was made an alias of :exc:`OSError`.


.. exception:: herror

   A subclass of :exc:`OSError`, this exception is raised for
   address-related errors, i.e. for functions that use *h_errno* in the POSIX
   C API, including :func:`gethostbyname_ex` and :func:`gethostbyaddr`.
   The accompanying value is a pair ``(h_errno, string)`` representing an
   error returned by a library call.  *h_errno* is a numeric value, while
   *string* represents the description of *h_errno*, as returned by the
   :c:func:`hstrerror` C function.

   .. versionchanged:: 3.3
      This class was made a subclass of :exc:`OSError`.

.. exception:: gaierror

   A subclass of :exc:`OSError`, this exception is raised for
   address-related errors by :func:`getaddrinfo` and :func:`getnameinfo`.
   The accompanying value is a pair ``(error, string)`` representing an error
   returned by a library call.  *string* represents the description of
   *error*, as returned by the :c:func:`gai_strerror` C function.  The
   numeric *error* value will match one of the :const:`EAI_\*` constants
   defined in this module.

   .. versionchanged:: 3.3
      This class was made a subclass of :exc:`OSError`.

.. exception:: timeout

   A subclass of :exc:`OSError`, this exception is raised when a timeout
   occurs on a socket which has had timeouts enabled via a prior call to
   :meth:`~socket.settimeout` (or implicitly through
   :func:`~socket.setdefaulttimeout`).  The accompanying value is a string
   whose value is currently always "timed out".

   .. versionchanged:: 3.3
      This class was made a subclass of :exc:`OSError`.


Constants
^^^^^^^^^

.. data:: AF_UNIX
          AF_INET
          AF_INET6

   These constants represent the address (and protocol) families, used for the
   first argument to :func:`.socket`.  If the :const:`AF_UNIX` constant is not
   defined then this protocol is unsupported.  More constants may be available
   depending on the system.


.. data:: SOCK_STREAM
          SOCK_DGRAM
          SOCK_RAW
          SOCK_RDM
          SOCK_SEQPACKET

   These constants represent the socket types, used for the second argument to
   :func:`.socket`.  More constants may be available depending on the system.
   (Only :const:`SOCK_STREAM` and :const:`SOCK_DGRAM` appear to be generally
   useful.)

.. data:: SOCK_CLOEXEC
          SOCK_NONBLOCK

   These two constants, if defined, can be combined with the socket types and
   allow you to set some flags atomically (thus avoiding possible race
   conditions and the need for separate calls).

   .. seealso::

      `Secure File Descriptor Handling <http://udrepper.livejournal.com/20407.html>`_
      for a more thorough explanation.

   Availability: Linux >= 2.6.27.

   .. versionadded:: 3.2

.. data:: SO_*
          SOMAXCONN
          MSG_*
          SOL_*
          SCM_*
          IPPROTO_*
          IPPORT_*
          INADDR_*
          IP_*
          IPV6_*
          EAI_*
          AI_*
          NI_*
          TCP_*

   Many constants of these forms, documented in the Unix documentation on sockets
   and/or the IP protocol, are also defined in the socket module. They are
   generally used in arguments to the :meth:`setsockopt` and :meth:`getsockopt`
   methods of socket objects.  In most cases, only those symbols that are defined
   in the Unix header files are defined; for a few symbols, default values are
   provided.

.. data:: AF_CAN
          PF_CAN
          SOL_CAN_*
          CAN_*

   Many constants of these forms, documented in the Linux documentation, are
   also defined in the socket module.

   Availability: Linux >= 2.6.25.

   .. versionadded:: 3.3

.. data:: CAN_BCM
          CAN_BCM_*

   CAN_BCM, in the CAN protocol family, is the broadcast manager (BCM) protocol.
   Broadcast manager constants, documented in the Linux documentation, are also
   defined in the socket module.

   Availability: Linux >= 2.6.25.

   .. versionadded:: 3.4

.. data:: AF_RDS
          PF_RDS
          SOL_RDS
          RDS_*

   Many constants of these forms, documented in the Linux documentation, are
   also defined in the socket module.

   Availability: Linux >= 2.6.30.

   .. versionadded:: 3.3


.. data:: SIO_*
          RCVALL_*

   Constants for Windows' WSAIoctl(). The constants are used as arguments to the
   :meth:`~socket.socket.ioctl` method of socket objects.


.. data:: TIPC_*

   TIPC related constants, matching the ones exported by the C socket API. See
   the TIPC documentation for more information.

.. data:: AF_LINK

  Availability: BSD, OSX.

  .. versionadded:: 3.4

.. data:: has_ipv6

   This constant contains a boolean value which indicates if IPv6 is supported on
   this platform.


Functions
^^^^^^^^^

Creating sockets
''''''''''''''''

The following functions all create :ref:`socket objects <socket-objects>`.


.. function:: socket(family=AF_INET, type=SOCK_STREAM, proto=0, fileno=None)

   Create a new socket using the given address family, socket type and protocol
   number.  The address family should be :const:`AF_INET` (the default),
   :const:`AF_INET6`, :const:`AF_UNIX`, :const:`AF_CAN` or :const:`AF_RDS`. The
   socket type should be :const:`SOCK_STREAM` (the default),
   :const:`SOCK_DGRAM`, :const:`SOCK_RAW` or perhaps one of the other ``SOCK_``
   constants. The protocol number is usually zero and may be omitted or in the
   case where the address family is :const:`AF_CAN` the protocol should be one
   of :const:`CAN_RAW` or :const:`CAN_BCM`.

   The newly created socket is :ref:`non-inheritable <fd_inheritance>`.

   .. versionchanged:: 3.3
      The AF_CAN family was added.
      The AF_RDS family was added.

   .. versionchanged:: 3.4
       The CAN_BCM protocol was added.

   .. versionchanged:: 3.4
      The returned socket is now non-inheritable.


.. function:: socketpair([family[, type[, proto]]])

   Build a pair of connected socket objects using the given address family, socket
   type, and protocol number.  Address family, socket type, and protocol number are
   as for the :func:`.socket` function above. The default family is :const:`AF_UNIX`
   if defined on the platform; otherwise, the default is :const:`AF_INET`.
   Availability: Unix.

   The newly created sockets are :ref:`non-inheritable <fd_inheritance>`.

   .. versionchanged:: 3.2
      The returned socket objects now support the whole socket API, rather
      than a subset.

   .. versionchanged:: 3.4
      The returned sockets are now non-inheritable.


.. function:: create_connection(address[, timeout[, source_address]])

   Connect to a TCP service listening on the Internet *address* (a 2-tuple
   ``(host, port)``), and return the socket object.  This is a higher-level
   function than :meth:`socket.connect`: if *host* is a non-numeric hostname,
   it will try to resolve it for both :data:`AF_INET` and :data:`AF_INET6`,
   and then try to connect to all possible addresses in turn until a
   connection succeeds.  This makes it easy to write clients that are
   compatible to both IPv4 and IPv6.

   Passing the optional *timeout* parameter will set the timeout on the
   socket instance before attempting to connect.  If no *timeout* is
   supplied, the global default timeout setting returned by
   :func:`getdefaulttimeout` is used.

   If supplied, *source_address* must be a 2-tuple ``(host, port)`` for the
   socket to bind to as its source address before connecting.  If host or port
   are '' or 0 respectively the OS default behavior will be used.

   .. versionchanged:: 3.2
      *source_address* was added.

   .. versionchanged:: 3.2
      support for the :keyword:`with` statement was added.


.. function:: fromfd(fd, family, type, proto=0)

   Duplicate the file descriptor *fd* (an integer as returned by a file object's
   :meth:`fileno` method) and build a socket object from the result.  Address
   family, socket type and protocol number are as for the :func:`.socket` function
   above. The file descriptor should refer to a socket, but this is not checked ---
   subsequent operations on the object may fail if the file descriptor is invalid.
   This function is rarely needed, but can be used to get or set socket options on
   a socket passed to a program as standard input or output (such as a server
   started by the Unix inet daemon).  The socket is assumed to be in blocking mode.

   The newly created socket is :ref:`non-inheritable <fd_inheritance>`.

   .. versionchanged:: 3.4
      The returned socket is now non-inheritable.


.. function:: fromshare(data)

   Instantiate a socket from data obtained from the :meth:`socket.share`
   method.  The socket is assumed to be in blocking mode.

   Availability: Windows.

   .. versionadded:: 3.3


.. data:: SocketType

   This is a Python type object that represents the socket object type. It is the
   same as ``type(socket(...))``.


Other functions
'''''''''''''''

The :mod:`socket` module also offers various network-related services:


.. function:: getaddrinfo(host, port, family=0, type=0, proto=0, flags=0)

   Translate the *host*/*port* argument into a sequence of 5-tuples that contain
   all the necessary arguments for creating a socket connected to that service.
   *host* is a domain name, a string representation of an IPv4/v6 address
   or ``None``. *port* is a string service name such as ``'http'``, a numeric
   port number or ``None``.  By passing ``None`` as the value of *host*
   and *port*, you can pass ``NULL`` to the underlying C API.

   The *family*, *type* and *proto* arguments can be optionally specified
   in order to narrow the list of addresses returned.  Passing zero as a
   value for each of these arguments selects the full range of results.
   The *flags* argument can be one or several of the ``AI_*`` constants,
   and will influence how results are computed and returned.
   For example, :const:`AI_NUMERICHOST` will disable domain name resolution
   and will raise an error if *host* is a domain name.

   The function returns a list of 5-tuples with the following structure:

   ``(family, type, proto, canonname, sockaddr)``

   In these tuples, *family*, *type*, *proto* are all integers and are
   meant to be passed to the :func:`.socket` function.  *canonname* will be
   a string representing the canonical name of the *host* if
   :const:`AI_CANONNAME` is part of the *flags* argument; else *canonname*
   will be empty.  *sockaddr* is a tuple describing a socket address, whose
   format depends on the returned *family* (a ``(address, port)`` 2-tuple for
   :const:`AF_INET`, a ``(address, port, flow info, scope id)`` 4-tuple for
   :const:`AF_INET6`), and is meant to be passed to the :meth:`socket.connect`
   method.

   The following example fetches address information for a hypothetical TCP
   connection to ``www.python.org`` on port 80 (results may differ on your
   system if IPv6 isn't enabled)::

      >>> socket.getaddrinfo("www.python.org", 80, proto=socket.SOL_TCP)
      [(2, 1, 6, '', ('82.94.164.162', 80)),
       (10, 1, 6, '', ('2001:888:2000:d::a2', 80, 0, 0))]

   .. versionchanged:: 3.2
      parameters can now be passed using keyword arguments.

.. function:: getfqdn([name])

   Return a fully qualified domain name for *name*. If *name* is omitted or empty,
   it is interpreted as the local host.  To find the fully qualified name, the
   hostname returned by :func:`gethostbyaddr` is checked, followed by aliases for the
   host, if available.  The first name which includes a period is selected.  In
   case no fully qualified domain name is available, the hostname as returned by
   :func:`gethostname` is returned.


.. function:: gethostbyname(hostname)

   Translate a host name to IPv4 address format.  The IPv4 address is returned as a
   string, such as  ``'100.50.200.5'``.  If the host name is an IPv4 address itself
   it is returned unchanged.  See :func:`gethostbyname_ex` for a more complete
   interface. :func:`gethostbyname` does not support IPv6 name resolution, and
   :func:`getaddrinfo` should be used instead for IPv4/v6 dual stack support.


.. function:: gethostbyname_ex(hostname)

   Translate a host name to IPv4 address format, extended interface. Return a
   triple ``(hostname, aliaslist, ipaddrlist)`` where *hostname* is the primary
   host name responding to the given *ip_address*, *aliaslist* is a (possibly
   empty) list of alternative host names for the same address, and *ipaddrlist* is
   a list of IPv4 addresses for the same interface on the same host (often but not
   always a single address). :func:`gethostbyname_ex` does not support IPv6 name
   resolution, and :func:`getaddrinfo` should be used instead for IPv4/v6 dual
   stack support.


.. function:: gethostname()

   Return a string containing the hostname of the machine where  the Python
   interpreter is currently executing.

   If you want to know the current machine's IP address, you may want to use
   ``gethostbyname(gethostname())``. This operation assumes that there is a
   valid address-to-host mapping for the host, and the assumption does not
   always hold.

   Note: :func:`gethostname` doesn't always return the fully qualified domain
   name; use ``getfqdn()`` (see above).


.. function:: gethostbyaddr(ip_address)

   Return a triple ``(hostname, aliaslist, ipaddrlist)`` where *hostname* is the
   primary host name responding to the given *ip_address*, *aliaslist* is a
   (possibly empty) list of alternative host names for the same address, and
   *ipaddrlist* is a list of IPv4/v6 addresses for the same interface on the same
   host (most likely containing only a single address). To find the fully qualified
   domain name, use the function :func:`getfqdn`. :func:`gethostbyaddr` supports
   both IPv4 and IPv6.


.. function:: getnameinfo(sockaddr, flags)

   Translate a socket address *sockaddr* into a 2-tuple ``(host, port)``. Depending
   on the settings of *flags*, the result can contain a fully-qualified domain name
   or numeric address representation in *host*.  Similarly, *port* can contain a
   string port name or a numeric port number.


.. function:: getprotobyname(protocolname)

   Translate an Internet protocol name (for example, ``'icmp'``) to a constant
   suitable for passing as the (optional) third argument to the :func:`.socket`
   function.  This is usually only needed for sockets opened in "raw" mode
   (:const:`SOCK_RAW`); for the normal socket modes, the correct protocol is chosen
   automatically if the protocol is omitted or zero.


.. function:: getservbyname(servicename[, protocolname])

   Translate an Internet service name and protocol name to a port number for that
   service.  The optional protocol name, if given, should be ``'tcp'`` or
   ``'udp'``, otherwise any protocol will match.


.. function:: getservbyport(port[, protocolname])

   Translate an Internet port number and protocol name to a service name for that
   service.  The optional protocol name, if given, should be ``'tcp'`` or
   ``'udp'``, otherwise any protocol will match.


.. function:: ntohl(x)

   Convert 32-bit positive integers from network to host byte order.  On machines
   where the host byte order is the same as network byte order, this is a no-op;
   otherwise, it performs a 4-byte swap operation.


.. function:: ntohs(x)

   Convert 16-bit positive integers from network to host byte order.  On machines
   where the host byte order is the same as network byte order, this is a no-op;
   otherwise, it performs a 2-byte swap operation.


.. function:: htonl(x)

   Convert 32-bit positive integers from host to network byte order.  On machines
   where the host byte order is the same as network byte order, this is a no-op;
   otherwise, it performs a 4-byte swap operation.


.. function:: htons(x)

   Convert 16-bit positive integers from host to network byte order.  On machines
   where the host byte order is the same as network byte order, this is a no-op;
   otherwise, it performs a 2-byte swap operation.


.. function:: inet_aton(ip_string)

   Convert an IPv4 address from dotted-quad string format (for example,
   '123.45.67.89') to 32-bit packed binary format, as a bytes object four characters in
   length.  This is useful when conversing with a program that uses the standard C
   library and needs objects of type :c:type:`struct in_addr`, which is the C type
   for the 32-bit packed binary this function returns.

   :func:`inet_aton` also accepts strings with less than three dots; see the
   Unix manual page :manpage:`inet(3)` for details.

   If the IPv4 address string passed to this function is invalid,
   :exc:`OSError` will be raised. Note that exactly what is valid depends on
   the underlying C implementation of :c:func:`inet_aton`.

   :func:`inet_aton` does not support IPv6, and :func:`inet_pton` should be used
   instead for IPv4/v6 dual stack support.


.. function:: inet_ntoa(packed_ip)

   Convert a 32-bit packed IPv4 address (a bytes object four characters in
   length) to its standard dotted-quad string representation (for example,
   '123.45.67.89').  This is useful when conversing with a program that uses the
   standard C library and needs objects of type :c:type:`struct in_addr`, which
   is the C type for the 32-bit packed binary data this function takes as an
   argument.

   If the byte sequence passed to this function is not exactly 4 bytes in
   length, :exc:`OSError` will be raised. :func:`inet_ntoa` does not
   support IPv6, and :func:`inet_ntop` should be used instead for IPv4/v6 dual
   stack support.


.. function:: inet_pton(address_family, ip_string)

   Convert an IP address from its family-specific string format to a packed,
   binary format. :func:`inet_pton` is useful when a library or network protocol
   calls for an object of type :c:type:`struct in_addr` (similar to
   :func:`inet_aton`) or :c:type:`struct in6_addr`.

   Supported values for *address_family* are currently :const:`AF_INET` and
   :const:`AF_INET6`. If the IP address string *ip_string* is invalid,
   :exc:`OSError` will be raised. Note that exactly what is valid depends on
   both the value of *address_family* and the underlying implementation of
   :c:func:`inet_pton`.

   Availability: Unix (maybe not all platforms), Windows.

   .. versionchanged:: 3.4
      Windows support added


.. function:: inet_ntop(address_family, packed_ip)

   Convert a packed IP address (a bytes object of some number of characters) to its
   standard, family-specific string representation (for example, ``'7.10.0.5'`` or
   ``'5aef:2b::8'``). :func:`inet_ntop` is useful when a library or network protocol
   returns an object of type :c:type:`struct in_addr` (similar to :func:`inet_ntoa`)
   or :c:type:`struct in6_addr`.

   Supported values for *address_family* are currently :const:`AF_INET` and
   :const:`AF_INET6`. If the string *packed_ip* is not the correct length for the
   specified address family, :exc:`ValueError` will be raised.  A
   :exc:`OSError` is raised for errors from the call to :func:`inet_ntop`.

   Availability: Unix (maybe not all platforms), Windows.

   .. versionchanged:: 3.4
      Windows support added


..
   XXX: Are sendmsg(), recvmsg() and CMSG_*() available on any
   non-Unix platforms?  The old (obsolete?) 4.2BSD form of the
   interface, in which struct msghdr has no msg_control or
   msg_controllen members, is not currently supported.

.. function:: CMSG_LEN(length)

   Return the total length, without trailing padding, of an ancillary
   data item with associated data of the given *length*.  This value
   can often be used as the buffer size for :meth:`~socket.recvmsg` to
   receive a single item of ancillary data, but :rfc:`3542` requires
   portable applications to use :func:`CMSG_SPACE` and thus include
   space for padding, even when the item will be the last in the
   buffer.  Raises :exc:`OverflowError` if *length* is outside the
   permissible range of values.

   Availability: most Unix platforms, possibly others.

   .. versionadded:: 3.3


.. function:: CMSG_SPACE(length)

   Return the buffer size needed for :meth:`~socket.recvmsg` to
   receive an ancillary data item with associated data of the given
   *length*, along with any trailing padding.  The buffer space needed
   to receive multiple items is the sum of the :func:`CMSG_SPACE`
   values for their associated data lengths.  Raises
   :exc:`OverflowError` if *length* is outside the permissible range
   of values.

   Note that some systems might support ancillary data without
   providing this function.  Also note that setting the buffer size
   using the results of this function may not precisely limit the
   amount of ancillary data that can be received, since additional
   data may be able to fit into the padding area.

   Availability: most Unix platforms, possibly others.

   .. versionadded:: 3.3


.. function:: getdefaulttimeout()

   Return the default timeout in seconds (float) for new socket objects. A value
   of ``None`` indicates that new socket objects have no timeout. When the socket
   module is first imported, the default is ``None``.


.. function:: setdefaulttimeout(timeout)

   Set the default timeout in seconds (float) for new socket objects.  When
   the socket module is first imported, the default is ``None``.  See
   :meth:`~socket.settimeout` for possible values and their respective
   meanings.


.. function:: sethostname(name)

   Set the machine's hostname to *name*.  This will raise a
   :exc:`OSError` if you don't have enough rights.

   Availability: Unix.

   .. versionadded:: 3.3


.. function:: if_nameindex()

   Return a list of network interface information
   (index int, name string) tuples.
   :exc:`OSError` if the system call fails.

   Availability: Unix.

   .. versionadded:: 3.3


.. function:: if_nametoindex(if_name)

   Return a network interface index number corresponding to an
   interface name.
   :exc:`OSError` if no interface with the given name exists.

   Availability: Unix.

   .. versionadded:: 3.3


.. function:: if_indextoname(if_index)

   Return a network interface name corresponding to a
   interface index number.
   :exc:`OSError` if no interface with the given index exists.

   Availability: Unix.

   .. versionadded:: 3.3


.. _socket-objects:

Socket Objects
--------------

Socket objects have the following methods.  Except for
:meth:`~socket.makefile`, these correspond to Unix system calls applicable
to sockets.


.. method:: socket.accept()

   Accept a connection. The socket must be bound to an address and listening for
   connections. The return value is a pair ``(conn, address)`` where *conn* is a
   *new* socket object usable to send and receive data on the connection, and
   *address* is the address bound to the socket on the other end of the connection.

   The newly created socket is :ref:`non-inheritable <fd_inheritance>`.

   .. versionchanged:: 3.4
      The socket is now non-inheritable.


.. method:: socket.bind(address)

   Bind the socket to *address*.  The socket must not already be bound. (The format
   of *address* depends on the address family --- see above.)


.. method:: socket.close()

   Mark the socket closed.  The underlying system resource (e.g. a file
   descriptor) is also closed when all file objects from :meth:`makefile()`
   are closed.  Once that happens, all future operations on the socket
   object will fail. The remote end will receive no more data (after
   queued data is flushed).

   Sockets are automatically closed when they are garbage-collected, but
   it is recommended to :meth:`close` them explicitly, or to use a
   :keyword:`with` statement around them.

   .. note::

      :meth:`close()` releases the resource associated with a connection but
      does not necessarily close the connection immediately.  If you want
      to close the connection in a timely fashion, call :meth:`shutdown()`
      before :meth:`close()`.


.. method:: socket.connect(address)

   Connect to a remote socket at *address*. (The format of *address* depends on the
   address family --- see above.)


.. method:: socket.connect_ex(address)

   Like ``connect(address)``, but return an error indicator instead of raising an
   exception for errors returned by the C-level :c:func:`connect` call (other
   problems, such as "host not found," can still raise exceptions).  The error
   indicator is ``0`` if the operation succeeded, otherwise the value of the
   :c:data:`errno` variable.  This is useful to support, for example, asynchronous
   connects.


.. method:: socket.detach()

   Put the socket object into closed state without actually closing the
   underlying file descriptor.  The file descriptor is returned, and can
   be reused for other purposes.

   .. versionadded:: 3.2


.. method:: socket.dup()

   Duplicate the socket.

   The newly created socket is :ref:`non-inheritable <fd_inheritance>`.

   .. versionchanged:: 3.4
      The socket is now non-inheritable.


.. method:: socket.fileno()

   Return the socket's file descriptor (a small integer).  This is useful with
   :func:`select.select`.

   Under Windows the small integer returned by this method cannot be used where a
   file descriptor can be used (such as :func:`os.fdopen`).  Unix does not have
   this limitation.


.. method:: socket.get_inheritable()

   Get the :ref:`inheritable flag <fd_inheritance>` of the socket's file
   descriptor or socket's handle: ``True`` if the socket can be inherited in
   child processes, ``False`` if it cannot.

   .. versionadded:: 3.4


.. method:: socket.getpeername()

   Return the remote address to which the socket is connected.  This is useful to
   find out the port number of a remote IPv4/v6 socket, for instance. (The format
   of the address returned depends on the address family --- see above.)  On some
   systems this function is not supported.


.. method:: socket.getsockname()

   Return the socket's own address.  This is useful to find out the port number of
   an IPv4/v6 socket, for instance. (The format of the address returned depends on
   the address family --- see above.)


.. method:: socket.getsockopt(level, optname[, buflen])

   Return the value of the given socket option (see the Unix man page
   :manpage:`getsockopt(2)`).  The needed symbolic constants (:const:`SO_\*` etc.)
   are defined in this module.  If *buflen* is absent, an integer option is assumed
   and its integer value is returned by the function.  If *buflen* is present, it
   specifies the maximum length of the buffer used to receive the option in, and
   this buffer is returned as a bytes object.  It is up to the caller to decode the
   contents of the buffer (see the optional built-in module :mod:`struct` for a way
   to decode C structures encoded as byte strings).


.. method:: socket.gettimeout()

   Return the timeout in seconds (float) associated with socket operations,
   or ``None`` if no timeout is set.  This reflects the last call to
   :meth:`setblocking` or :meth:`settimeout`.


.. method:: socket.ioctl(control, option)

   :platform: Windows

   The :meth:`ioctl` method is a limited interface to the WSAIoctl system
   interface.  Please refer to the `Win32 documentation
   <http://msdn.microsoft.com/en-us/library/ms741621%28VS.85%29.aspx>`_ for more
   information.

   On other platforms, the generic :func:`fcntl.fcntl` and :func:`fcntl.ioctl`
   functions may be used; they accept a socket object as their first argument.

.. method:: socket.listen(backlog)

   Listen for connections made to the socket.  The *backlog* argument specifies the
   maximum number of queued connections and should be at least 0; the maximum value
   is system-dependent (usually 5), the minimum value is forced to 0.


.. method:: socket.makefile(mode='r', buffering=None, *, encoding=None, \
                            errors=None, newline=None)

   .. index:: single: I/O control; buffering

   Return a :term:`file object` associated with the socket.  The exact returned
   type depends on the arguments given to :meth:`makefile`.  These arguments are
   interpreted the same way as by the built-in :func:`open` function.

   The socket must be in blocking mode; it can have a timeout, but the file
   object's internal buffer may end up in a inconsistent state if a timeout
   occurs.

   Closing the file object returned by :meth:`makefile` won't close the
   original socket unless all other file objects have been closed and
   :meth:`socket.close` has been called on the socket object.

   .. note::

      On Windows, the file-like object created by :meth:`makefile` cannot be
      used where a file object with a file descriptor is expected, such as the
      stream arguments of :meth:`subprocess.Popen`.


.. method:: socket.recv(bufsize[, flags])

   Receive data from the socket.  The return value is a bytes object representing the
   data received.  The maximum amount of data to be received at once is specified
   by *bufsize*.  See the Unix manual page :manpage:`recv(2)` for the meaning of
   the optional argument *flags*; it defaults to zero.

   .. note::

      For best match with hardware and network realities, the value of  *bufsize*
      should be a relatively small power of 2, for example, 4096.


.. method:: socket.recvfrom(bufsize[, flags])

   Receive data from the socket.  The return value is a pair ``(bytes, address)``
   where *bytes* is a bytes object representing the data received and *address* is the
   address of the socket sending the data.  See the Unix manual page
   :manpage:`recv(2)` for the meaning of the optional argument *flags*; it defaults
   to zero. (The format of *address* depends on the address family --- see above.)


.. method:: socket.recvmsg(bufsize[, ancbufsize[, flags]])

   Receive normal data (up to *bufsize* bytes) and ancillary data from
   the socket.  The *ancbufsize* argument sets the size in bytes of
   the internal buffer used to receive the ancillary data; it defaults
   to 0, meaning that no ancillary data will be received.  Appropriate
   buffer sizes for ancillary data can be calculated using
   :func:`CMSG_SPACE` or :func:`CMSG_LEN`, and items which do not fit
   into the buffer might be truncated or discarded.  The *flags*
   argument defaults to 0 and has the same meaning as for
   :meth:`recv`.

   The return value is a 4-tuple: ``(data, ancdata, msg_flags,
   address)``.  The *data* item is a :class:`bytes` object holding the
   non-ancillary data received.  The *ancdata* item is a list of zero
   or more tuples ``(cmsg_level, cmsg_type, cmsg_data)`` representing
   the ancillary data (control messages) received: *cmsg_level* and
   *cmsg_type* are integers specifying the protocol level and
   protocol-specific type respectively, and *cmsg_data* is a
   :class:`bytes` object holding the associated data.  The *msg_flags*
   item is the bitwise OR of various flags indicating conditions on
   the received message; see your system documentation for details.
   If the receiving socket is unconnected, *address* is the address of
   the sending socket, if available; otherwise, its value is
   unspecified.

   On some systems, :meth:`sendmsg` and :meth:`recvmsg` can be used to
   pass file descriptors between processes over an :const:`AF_UNIX`
   socket.  When this facility is used (it is often restricted to
   :const:`SOCK_STREAM` sockets), :meth:`recvmsg` will return, in its
   ancillary data, items of the form ``(socket.SOL_SOCKET,
   socket.SCM_RIGHTS, fds)``, where *fds* is a :class:`bytes` object
   representing the new file descriptors as a binary array of the
   native C :c:type:`int` type.  If :meth:`recvmsg` raises an
   exception after the system call returns, it will first attempt to
   close any file descriptors received via this mechanism.

   Some systems do not indicate the truncated length of ancillary data
   items which have been only partially received.  If an item appears
   to extend beyond the end of the buffer, :meth:`recvmsg` will issue
   a :exc:`RuntimeWarning`, and will return the part of it which is
   inside the buffer provided it has not been truncated before the
   start of its associated data.

   On systems which support the :const:`SCM_RIGHTS` mechanism, the
   following function will receive up to *maxfds* file descriptors,
   returning the message data and a list containing the descriptors
   (while ignoring unexpected conditions such as unrelated control
   messages being received).  See also :meth:`sendmsg`. ::

      import socket, array

      def recv_fds(sock, msglen, maxfds):
          fds = array.array("i")   # Array of ints
          msg, ancdata, flags, addr = sock.recvmsg(msglen, socket.CMSG_LEN(maxfds * fds.itemsize))
          for cmsg_level, cmsg_type, cmsg_data in ancdata:
              if (cmsg_level == socket.SOL_SOCKET and cmsg_type == socket.SCM_RIGHTS):
                  # Append data, ignoring any truncated integers at the end.
                  fds.fromstring(cmsg_data[:len(cmsg_data) - (len(cmsg_data) % fds.itemsize)])
          return msg, list(fds)

   Availability: most Unix platforms, possibly others.

   .. versionadded:: 3.3


.. method:: socket.recvmsg_into(buffers[, ancbufsize[, flags]])

   Receive normal data and ancillary data from the socket, behaving as
   :meth:`recvmsg` would, but scatter the non-ancillary data into a
   series of buffers instead of returning a new bytes object.  The
   *buffers* argument must be an iterable of objects that export
   writable buffers (e.g. :class:`bytearray` objects); these will be
   filled with successive chunks of the non-ancillary data until it
   has all been written or there are no more buffers.  The operating
   system may set a limit (:func:`~os.sysconf` value ``SC_IOV_MAX``)
   on the number of buffers that can be used.  The *ancbufsize* and
   *flags* arguments have the same meaning as for :meth:`recvmsg`.

   The return value is a 4-tuple: ``(nbytes, ancdata, msg_flags,
   address)``, where *nbytes* is the total number of bytes of
   non-ancillary data written into the buffers, and *ancdata*,
   *msg_flags* and *address* are the same as for :meth:`recvmsg`.

   Example::

      >>> import socket
      >>> s1, s2 = socket.socketpair()
      >>> b1 = bytearray(b'----')
      >>> b2 = bytearray(b'0123456789')
      >>> b3 = bytearray(b'--------------')
      >>> s1.send(b'Mary had a little lamb')
      22
      >>> s2.recvmsg_into([b1, memoryview(b2)[2:9], b3])
      (22, [], 0, None)
      >>> [b1, b2, b3]
      [bytearray(b'Mary'), bytearray(b'01 had a 9'), bytearray(b'little lamb---')]

   Availability: most Unix platforms, possibly others.

   .. versionadded:: 3.3


.. method:: socket.recvfrom_into(buffer[, nbytes[, flags]])

   Receive data from the socket, writing it into *buffer* instead of creating a
   new bytestring.  The return value is a pair ``(nbytes, address)`` where *nbytes* is
   the number of bytes received and *address* is the address of the socket sending
   the data.  See the Unix manual page :manpage:`recv(2)` for the meaning of the
   optional argument *flags*; it defaults to zero.  (The format of *address*
   depends on the address family --- see above.)


.. method:: socket.recv_into(buffer[, nbytes[, flags]])

   Receive up to *nbytes* bytes from the socket, storing the data into a buffer
   rather than creating a new bytestring.  If *nbytes* is not specified (or 0),
   receive up to the size available in the given buffer.  Returns the number of
   bytes received.  See the Unix manual page :manpage:`recv(2)` for the meaning
   of the optional argument *flags*; it defaults to zero.


.. method:: socket.send(bytes[, flags])

   Send data to the socket.  The socket must be connected to a remote socket.  The
   optional *flags* argument has the same meaning as for :meth:`recv` above.
   Returns the number of bytes sent. Applications are responsible for checking that
   all data has been sent; if only some of the data was transmitted, the
   application needs to attempt delivery of the remaining data. For further
   information on this topic, consult the :ref:`socket-howto`.


.. method:: socket.sendall(bytes[, flags])

   Send data to the socket.  The socket must be connected to a remote socket.  The
   optional *flags* argument has the same meaning as for :meth:`recv` above.
   Unlike :meth:`send`, this method continues to send data from *bytes* until
   either all data has been sent or an error occurs.  ``None`` is returned on
   success.  On error, an exception is raised, and there is no way to determine how
   much data, if any, was successfully sent.


.. method:: socket.sendto(bytes, address)
            socket.sendto(bytes, flags, address)

   Send data to the socket.  The socket should not be connected to a remote socket,
   since the destination socket is specified by *address*.  The optional *flags*
   argument has the same meaning as for :meth:`recv` above.  Return the number of
   bytes sent. (The format of *address* depends on the address family --- see
   above.)


.. method:: socket.sendmsg(buffers[, ancdata[, flags[, address]]])

   Send normal and ancillary data to the socket, gathering the
   non-ancillary data from a series of buffers and concatenating it
   into a single message.  The *buffers* argument specifies the
   non-ancillary data as an iterable of buffer-compatible objects
   (e.g. :class:`bytes` objects); the operating system may set a limit
   (:func:`~os.sysconf` value ``SC_IOV_MAX``) on the number of buffers
   that can be used.  The *ancdata* argument specifies the ancillary
   data (control messages) as an iterable of zero or more tuples
   ``(cmsg_level, cmsg_type, cmsg_data)``, where *cmsg_level* and
   *cmsg_type* are integers specifying the protocol level and
   protocol-specific type respectively, and *cmsg_data* is a
   buffer-compatible object holding the associated data.  Note that
   some systems (in particular, systems without :func:`CMSG_SPACE`)
   might support sending only one control message per call.  The
   *flags* argument defaults to 0 and has the same meaning as for
   :meth:`send`.  If *address* is supplied and not ``None``, it sets a
   destination address for the message.  The return value is the
   number of bytes of non-ancillary data sent.

   The following function sends the list of file descriptors *fds*
   over an :const:`AF_UNIX` socket, on systems which support the
   :const:`SCM_RIGHTS` mechanism.  See also :meth:`recvmsg`. ::

      import socket, array

      def send_fds(sock, msg, fds):
          return sock.sendmsg([msg], [(socket.SOL_SOCKET, socket.SCM_RIGHTS, array.array("i", fds))])

   Availability: most Unix platforms, possibly others.

   .. versionadded:: 3.3


.. method:: socket.set_inheritable(inheritable)

   Set the :ref:`inheritable flag <fd_inheritance>` of the socket's file
   descriptor or socket's handle.

   .. versionadded:: 3.4


.. method:: socket.setblocking(flag)

   Set blocking or non-blocking mode of the socket: if *flag* is false, the
   socket is set to non-blocking, else to blocking mode.

   This method is a shorthand for certain :meth:`~socket.settimeout` calls:

   * ``sock.setblocking(True)`` is equivalent to ``sock.settimeout(None)``

   * ``sock.setblocking(False)`` is equivalent to ``sock.settimeout(0.0)``


.. method:: socket.settimeout(value)

   Set a timeout on blocking socket operations.  The *value* argument can be a
   nonnegative floating point number expressing seconds, or ``None``.
   If a non-zero value is given, subsequent socket operations will raise a
   :exc:`timeout` exception if the timeout period *value* has elapsed before
   the operation has completed.  If zero is given, the socket is put in
   non-blocking mode. If ``None`` is given, the socket is put in blocking mode.

   For further information, please consult the :ref:`notes on socket timeouts <socket-timeouts>`.


.. method:: socket.setsockopt(level, optname, value)

   .. index:: module: struct

   Set the value of the given socket option (see the Unix manual page
   :manpage:`setsockopt(2)`).  The needed symbolic constants are defined in the
   :mod:`socket` module (:const:`SO_\*` etc.).  The value can be an integer or a
   bytes object representing a buffer.  In the latter case it is up to the caller to
   ensure that the bytestring contains the proper bits (see the optional built-in
   module :mod:`struct` for a way to encode C structures as bytestrings).


.. method:: socket.shutdown(how)

   Shut down one or both halves of the connection.  If *how* is :const:`SHUT_RD`,
   further receives are disallowed.  If *how* is :const:`SHUT_WR`, further sends
   are disallowed.  If *how* is :const:`SHUT_RDWR`, further sends and receives are
   disallowed.


.. method:: socket.share(process_id)

   Duplicate a socket and prepare it for sharing with a target process.  The
   target process must be provided with *process_id*.  The resulting bytes object
   can then be passed to the target process using some form of interprocess
   communication and the socket can be recreated there using :func:`fromshare`.
   Once this method has been called, it is safe to close the socket since
   the operating system has already duplicated it for the target process.

   Availability: Windows.

   .. versionadded:: 3.3


Note that there are no methods :meth:`read` or :meth:`write`; use
:meth:`~socket.recv` and :meth:`~socket.send` without *flags* argument instead.

Socket objects also have these (read-only) attributes that correspond to the
values given to the :class:`socket` constructor.


.. attribute:: socket.family

   The socket family.


.. attribute:: socket.type

   The socket type.


.. attribute:: socket.proto

   The socket protocol.



.. _socket-timeouts:

Notes on socket timeouts
------------------------

A socket object can be in one of three modes: blocking, non-blocking, or
timeout.  Sockets are by default always created in blocking mode, but this
can be changed by calling :func:`setdefaulttimeout`.

* In *blocking mode*, operations block until complete or the system returns
  an error (such as connection timed out).

* In *non-blocking mode*, operations fail (with an error that is unfortunately
  system-dependent) if they cannot be completed immediately: functions from the
  :mod:`select` can be used to know when and whether a socket is available for
  reading or writing.

* In *timeout mode*, operations fail if they cannot be completed within the
  timeout specified for the socket (they raise a :exc:`timeout` exception)
  or if the system returns an error.

.. note::
   At the operating system level, sockets in *timeout mode* are internally set
   in non-blocking mode.  Also, the blocking and timeout modes are shared between
   file descriptors and socket objects that refer to the same network endpoint.
   This implementation detail can have visible consequences if e.g. you decide
   to use the :meth:`~socket.fileno()` of a socket.

Timeouts and the ``connect`` method
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The :meth:`~socket.connect` operation is also subject to the timeout
setting, and in general it is recommended to call :meth:`~socket.settimeout`
before calling :meth:`~socket.connect` or pass a timeout parameter to
:meth:`create_connection`.  However, the system network stack may also
return a connection timeout error of its own regardless of any Python socket
timeout setting.

Timeouts and the ``accept`` method
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If :func:`getdefaulttimeout` is not :const:`None`, sockets returned by
the :meth:`~socket.accept` method inherit that timeout.  Otherwise, the
behaviour depends on settings of the listening socket:

* if the listening socket is in *blocking mode* or in *timeout mode*,
  the socket returned by :meth:`~socket.accept` is in *blocking mode*;

* if the listening socket is in *non-blocking mode*, whether the socket
  returned by :meth:`~socket.accept` is in blocking or non-blocking mode
  is operating system-dependent.  If you want to ensure cross-platform
  behaviour, it is recommended you manually override this setting.


.. _socket-example:

Example
-------

Here are four minimal example programs using the TCP/IP protocol: a server that
echoes all data that it receives back (servicing only one client), and a client
using it.  Note that a server must perform the sequence :func:`.socket`,
:meth:`~socket.bind`, :meth:`~socket.listen`, :meth:`~socket.accept` (possibly
repeating the :meth:`~socket.accept` to service more than one client), while a
client only needs the sequence :func:`.socket`, :meth:`~socket.connect`.  Also
note that the server does not :meth:`~socket.sendall`/:meth:`~socket.recv` on
the socket it is listening on but on the new socket returned by
:meth:`~socket.accept`.

The first two examples support IPv4 only. ::

   # Echo server program
   import socket

   HOST = ''                 # Symbolic name meaning all available interfaces
   PORT = 50007              # Arbitrary non-privileged port
   s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
   s.bind((HOST, PORT))
   s.listen(1)
   conn, addr = s.accept()
   print('Connected by', addr)
   while True:
       data = conn.recv(1024)
       if not data: break
       conn.sendall(data)
   conn.close()

::

   # Echo client program
   import socket

   HOST = 'daring.cwi.nl'    # The remote host
   PORT = 50007              # The same port as used by the server
   s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
   s.connect((HOST, PORT))
   s.sendall(b'Hello, world')
   data = s.recv(1024)
   s.close()
   print('Received', repr(data))

The next two examples are identical to the above two, but support both IPv4 and
IPv6. The server side will listen to the first address family available (it
should listen to both instead). On most of IPv6-ready systems, IPv6 will take
precedence and the server may not accept IPv4 traffic. The client side will try
to connect to the all addresses returned as a result of the name resolution, and
sends traffic to the first one connected successfully. ::

   # Echo server program
   import socket
   import sys

   HOST = None               # Symbolic name meaning all available interfaces
   PORT = 50007              # Arbitrary non-privileged port
   s = None
   for res in socket.getaddrinfo(HOST, PORT, socket.AF_UNSPEC,
                                 socket.SOCK_STREAM, 0, socket.AI_PASSIVE):
       af, socktype, proto, canonname, sa = res
       try:
           s = socket.socket(af, socktype, proto)
       except OSError as msg:
           s = None
           continue
       try:
           s.bind(sa)
           s.listen(1)
       except OSError as msg:
           s.close()
           s = None
           continue
       break
   if s is None:
       print('could not open socket')
       sys.exit(1)
   conn, addr = s.accept()
   print('Connected by', addr)
   while True:
       data = conn.recv(1024)
       if not data: break
       conn.send(data)
   conn.close()

::

   # Echo client program
   import socket
   import sys

   HOST = 'daring.cwi.nl'    # The remote host
   PORT = 50007              # The same port as used by the server
   s = None
   for res in socket.getaddrinfo(HOST, PORT, socket.AF_UNSPEC, socket.SOCK_STREAM):
       af, socktype, proto, canonname, sa = res
       try:
           s = socket.socket(af, socktype, proto)
       except OSError as msg:
           s = None
           continue
       try:
           s.connect(sa)
       except OSError as msg:
           s.close()
           s = None
           continue
       break
   if s is None:
       print('could not open socket')
       sys.exit(1)
   s.sendall(b'Hello, world')
   data = s.recv(1024)
   s.close()
   print('Received', repr(data))


The next example shows how to write a very simple network sniffer with raw
sockets on Windows. The example requires administrator privileges to modify
the interface::

   import socket

   # the public network interface
   HOST = socket.gethostbyname(socket.gethostname())

   # create a raw socket and bind it to the public interface
   s = socket.socket(socket.AF_INET, socket.SOCK_RAW, socket.IPPROTO_IP)
   s.bind((HOST, 0))

   # Include IP headers
   s.setsockopt(socket.IPPROTO_IP, socket.IP_HDRINCL, 1)

   # receive all packages
   s.ioctl(socket.SIO_RCVALL, socket.RCVALL_ON)

   # receive a package
   print(s.recvfrom(65565))

   # disabled promiscuous mode
   s.ioctl(socket.SIO_RCVALL, socket.RCVALL_OFF)

The last example shows how to use the socket interface to communicate to a CAN
network using the raw socket protocol. To use CAN with the broadcast
manager protocol instead, open a socket with::

    socket.socket(socket.AF_CAN, socket.SOCK_DGRAM, socket.CAN_BCM)

After binding (:const:`CAN_RAW`) or connecting (:const:`CAN_BCM`) the socket, you
can use the :meth:`socket.send`, and the :meth:`socket.recv` operations (and
their counterparts) on the socket object as usual.

This example might require special priviledge::

   import socket
   import struct


   # CAN frame packing/unpacking (see 'struct can_frame' in <linux/can.h>)

   can_frame_fmt = "=IB3x8s"
   can_frame_size = struct.calcsize(can_frame_fmt)

   def build_can_frame(can_id, data):
       can_dlc = len(data)
       data = data.ljust(8, b'\x00')
       return struct.pack(can_frame_fmt, can_id, can_dlc, data)

   def dissect_can_frame(frame):
       can_id, can_dlc, data = struct.unpack(can_frame_fmt, frame)
       return (can_id, can_dlc, data[:can_dlc])


   # create a raw socket and bind it to the 'vcan0' interface
   s = socket.socket(socket.AF_CAN, socket.SOCK_RAW, socket.CAN_RAW)
   s.bind(('vcan0',))

   while True:
       cf, addr = s.recvfrom(can_frame_size)

       print('Received: can_id=%x, can_dlc=%x, data=%s' % dissect_can_frame(cf))

       try:
           s.send(cf)
       except OSError:
           print('Error sending CAN frame')

       try:
           s.send(build_can_frame(0x01, b'\x01\x02\x03'))
       except OSError:
           print('Error sending CAN frame')

Running an example several times with too small delay between executions, could
lead to this error::

   OSError: [Errno 98] Address already in use

This is because the previous execution has left the socket in a ``TIME_WAIT``
state, and can't be immediately reused.

There is a :mod:`socket` flag to set, in order to prevent this,
:data:`socket.SO_REUSEADDR`::

   s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
   s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
   s.bind((HOST, PORT))

the :data:`SO_REUSEADDR` flag tells the kernel to reuse a local socket in
``TIME_WAIT`` state, without waiting for its natural timeout to expire.


.. seealso::

   For an introduction to socket programming (in C), see the following papers:

   - *An Introductory 4.3BSD Interprocess Communication Tutorial*, by Stuart Sechrest

   - *An Advanced 4.3BSD Interprocess Communication Tutorial*, by Samuel J.  Leffler et
     al,

   both in the UNIX Programmer's Manual, Supplementary Documents 1 (sections
   PS1:7 and PS1:8).  The platform-specific reference material for the various
   socket-related system calls are also a valuable source of information on the
   details of socket semantics.  For Unix, refer to the manual pages; for Windows,
   see the WinSock (or Winsock 2) specification.  For IPv6-ready APIs, readers may
   want to refer to :rfc:`3493` titled Basic Socket Interface Extensions for IPv6.
