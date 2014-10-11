"""Tests for events.py."""

import functools
import gc
import io
import os
import platform
import signal
import socket
try:
    import ssl
except ImportError:
    ssl = None
    HAS_SNI = False
else:
    from ssl import HAS_SNI
import subprocess
import sys
import threading
import time
import errno
import unittest
from unittest import mock
import weakref
from test import support  # find_unused_port, IPV6_ENABLED, TEST_HOME_DIR


import asyncio
from asyncio import selector_events
from asyncio import test_utils


def data_file(filename):
    if hasattr(support, 'TEST_HOME_DIR'):
        fullname = os.path.join(support.TEST_HOME_DIR, filename)
        if os.path.isfile(fullname):
            return fullname
    fullname = os.path.join(os.path.dirname(__file__), filename)
    if os.path.isfile(fullname):
        return fullname
    raise FileNotFoundError(filename)


def osx_tiger():
    """Return True if the platform is Mac OS 10.4 or older."""
    if sys.platform != 'darwin':
        return False
    version = platform.mac_ver()[0]
    version = tuple(map(int, version.split('.')))
    return version < (10, 5)


ONLYCERT = data_file('ssl_cert.pem')
ONLYKEY = data_file('ssl_key.pem')
SIGNED_CERTFILE = data_file('keycert3.pem')
SIGNING_CA = data_file('pycacert.pem')


class MyBaseProto(asyncio.Protocol):
    connected = None
    done = None

    def __init__(self, loop=None):
        self.transport = None
        self.state = 'INITIAL'
        self.nbytes = 0
        if loop is not None:
            self.connected = asyncio.Future(loop=loop)
            self.done = asyncio.Future(loop=loop)

    def connection_made(self, transport):
        self.transport = transport
        assert self.state == 'INITIAL', self.state
        self.state = 'CONNECTED'
        if self.connected:
            self.connected.set_result(None)

    def data_received(self, data):
        assert self.state == 'CONNECTED', self.state
        self.nbytes += len(data)

    def eof_received(self):
        assert self.state == 'CONNECTED', self.state
        self.state = 'EOF'

    def connection_lost(self, exc):
        assert self.state in ('CONNECTED', 'EOF'), self.state
        self.state = 'CLOSED'
        if self.done:
            self.done.set_result(None)


class MyProto(MyBaseProto):
    def connection_made(self, transport):
        super().connection_made(transport)
        transport.write(b'GET / HTTP/1.0\r\nHost: example.com\r\n\r\n')


class MyDatagramProto(asyncio.DatagramProtocol):
    done = None

    def __init__(self, loop=None):
        self.state = 'INITIAL'
        self.nbytes = 0
        if loop is not None:
            self.done = asyncio.Future(loop=loop)

    def connection_made(self, transport):
        self.transport = transport
        assert self.state == 'INITIAL', self.state
        self.state = 'INITIALIZED'

    def datagram_received(self, data, addr):
        assert self.state == 'INITIALIZED', self.state
        self.nbytes += len(data)

    def error_received(self, exc):
        assert self.state == 'INITIALIZED', self.state

    def connection_lost(self, exc):
        assert self.state == 'INITIALIZED', self.state
        self.state = 'CLOSED'
        if self.done:
            self.done.set_result(None)


class MyReadPipeProto(asyncio.Protocol):
    done = None

    def __init__(self, loop=None):
        self.state = ['INITIAL']
        self.nbytes = 0
        self.transport = None
        if loop is not None:
            self.done = asyncio.Future(loop=loop)

    def connection_made(self, transport):
        self.transport = transport
        assert self.state == ['INITIAL'], self.state
        self.state.append('CONNECTED')

    def data_received(self, data):
        assert self.state == ['INITIAL', 'CONNECTED'], self.state
        self.nbytes += len(data)

    def eof_received(self):
        assert self.state == ['INITIAL', 'CONNECTED'], self.state
        self.state.append('EOF')

    def connection_lost(self, exc):
        if 'EOF' not in self.state:
            self.state.append('EOF')  # It is okay if EOF is missed.
        assert self.state == ['INITIAL', 'CONNECTED', 'EOF'], self.state
        self.state.append('CLOSED')
        if self.done:
            self.done.set_result(None)


class MyWritePipeProto(asyncio.BaseProtocol):
    done = None

    def __init__(self, loop=None):
        self.state = 'INITIAL'
        self.transport = None
        if loop is not None:
            self.done = asyncio.Future(loop=loop)

    def connection_made(self, transport):
        self.transport = transport
        assert self.state == 'INITIAL', self.state
        self.state = 'CONNECTED'

    def connection_lost(self, exc):
        assert self.state == 'CONNECTED', self.state
        self.state = 'CLOSED'
        if self.done:
            self.done.set_result(None)


class MySubprocessProtocol(asyncio.SubprocessProtocol):

    def __init__(self, loop):
        self.state = 'INITIAL'
        self.transport = None
        self.connected = asyncio.Future(loop=loop)
        self.completed = asyncio.Future(loop=loop)
        self.disconnects = {fd: asyncio.Future(loop=loop) for fd in range(3)}
        self.data = {1: b'', 2: b''}
        self.returncode = None
        self.got_data = {1: asyncio.Event(loop=loop),
                         2: asyncio.Event(loop=loop)}

    def connection_made(self, transport):
        self.transport = transport
        assert self.state == 'INITIAL', self.state
        self.state = 'CONNECTED'
        self.connected.set_result(None)

    def connection_lost(self, exc):
        assert self.state == 'CONNECTED', self.state
        self.state = 'CLOSED'
        self.completed.set_result(None)

    def pipe_data_received(self, fd, data):
        assert self.state == 'CONNECTED', self.state
        self.data[fd] += data
        self.got_data[fd].set()

    def pipe_connection_lost(self, fd, exc):
        assert self.state == 'CONNECTED', self.state
        if exc:
            self.disconnects[fd].set_exception(exc)
        else:
            self.disconnects[fd].set_result(exc)

    def process_exited(self):
        assert self.state == 'CONNECTED', self.state
        self.returncode = self.transport.get_returncode()


class EventLoopTestsMixin:

    def setUp(self):
        super().setUp()
        self.loop = self.create_event_loop()
        asyncio.set_event_loop(None)

    def tearDown(self):
        # just in case if we have transport close callbacks
        test_utils.run_briefly(self.loop)

        self.loop.close()
        gc.collect()
        super().tearDown()

    def test_run_until_complete_nesting(self):
        @asyncio.coroutine
        def coro1():
            yield

        @asyncio.coroutine
        def coro2():
            self.assertTrue(self.loop.is_running())
            self.loop.run_until_complete(coro1())

        self.assertRaises(
            RuntimeError, self.loop.run_until_complete, coro2())

    # Note: because of the default Windows timing granularity of
    # 15.6 msec, we use fairly long sleep times here (~100 msec).

    def test_run_until_complete(self):
        t0 = self.loop.time()
        self.loop.run_until_complete(asyncio.sleep(0.1, loop=self.loop))
        t1 = self.loop.time()
        self.assertTrue(0.08 <= t1-t0 <= 0.8, t1-t0)

    def test_run_until_complete_stopped(self):
        @asyncio.coroutine
        def cb():
            self.loop.stop()
            yield from asyncio.sleep(0.1, loop=self.loop)
        task = cb()
        self.assertRaises(RuntimeError,
                          self.loop.run_until_complete, task)

    def test_call_later(self):
        results = []

        def callback(arg):
            results.append(arg)
            self.loop.stop()

        self.loop.call_later(0.1, callback, 'hello world')
        t0 = time.monotonic()
        self.loop.run_forever()
        t1 = time.monotonic()
        self.assertEqual(results, ['hello world'])
        self.assertTrue(0.08 <= t1-t0 <= 0.8, t1-t0)

    def test_call_soon(self):
        results = []

        def callback(arg1, arg2):
            results.append((arg1, arg2))
            self.loop.stop()

        self.loop.call_soon(callback, 'hello', 'world')
        self.loop.run_forever()
        self.assertEqual(results, [('hello', 'world')])

    def test_call_soon_threadsafe(self):
        results = []
        lock = threading.Lock()

        def callback(arg):
            results.append(arg)
            if len(results) >= 2:
                self.loop.stop()

        def run_in_thread():
            self.loop.call_soon_threadsafe(callback, 'hello')
            lock.release()

        lock.acquire()
        t = threading.Thread(target=run_in_thread)
        t.start()

        with lock:
            self.loop.call_soon(callback, 'world')
            self.loop.run_forever()
        t.join()
        self.assertEqual(results, ['hello', 'world'])

    def test_call_soon_threadsafe_same_thread(self):
        results = []

        def callback(arg):
            results.append(arg)
            if len(results) >= 2:
                self.loop.stop()

        self.loop.call_soon_threadsafe(callback, 'hello')
        self.loop.call_soon(callback, 'world')
        self.loop.run_forever()
        self.assertEqual(results, ['hello', 'world'])

    def test_run_in_executor(self):
        def run(arg):
            return (arg, threading.get_ident())
        f2 = self.loop.run_in_executor(None, run, 'yo')
        res, thread_id = self.loop.run_until_complete(f2)
        self.assertEqual(res, 'yo')
        self.assertNotEqual(thread_id, threading.get_ident())

    def test_reader_callback(self):
        r, w = test_utils.socketpair()
        r.setblocking(False)
        bytes_read = bytearray()

        def reader():
            try:
                data = r.recv(1024)
            except BlockingIOError:
                # Spurious readiness notifications are possible
                # at least on Linux -- see man select.
                return
            if data:
                bytes_read.extend(data)
            else:
                self.assertTrue(self.loop.remove_reader(r.fileno()))
                r.close()

        self.loop.add_reader(r.fileno(), reader)
        self.loop.call_soon(w.send, b'abc')
        test_utils.run_until(self.loop, lambda: len(bytes_read) >= 3)
        self.loop.call_soon(w.send, b'def')
        test_utils.run_until(self.loop, lambda: len(bytes_read) >= 6)
        self.loop.call_soon(w.close)
        self.loop.call_soon(self.loop.stop)
        self.loop.run_forever()
        self.assertEqual(bytes_read, b'abcdef')

    def test_writer_callback(self):
        r, w = test_utils.socketpair()
        w.setblocking(False)

        def writer(data):
            w.send(data)
            self.loop.stop()

        data = b'x' * 1024
        self.loop.add_writer(w.fileno(), writer, data)
        self.loop.run_forever()

        self.assertTrue(self.loop.remove_writer(w.fileno()))
        self.assertFalse(self.loop.remove_writer(w.fileno()))

        w.close()
        read = r.recv(len(data) * 2)
        r.close()
        self.assertEqual(read, data)

    def _basetest_sock_client_ops(self, httpd, sock):
        sock.setblocking(False)
        self.loop.run_until_complete(
            self.loop.sock_connect(sock, httpd.address))
        self.loop.run_until_complete(
            self.loop.sock_sendall(sock, b'GET / HTTP/1.0\r\n\r\n'))
        data = self.loop.run_until_complete(
            self.loop.sock_recv(sock, 1024))
        # consume data
        self.loop.run_until_complete(
            self.loop.sock_recv(sock, 1024))
        sock.close()
        self.assertTrue(data.startswith(b'HTTP/1.0 200 OK'))

    def test_sock_client_ops(self):
        with test_utils.run_test_server() as httpd:
            sock = socket.socket()
            self._basetest_sock_client_ops(httpd, sock)

    @unittest.skipUnless(hasattr(socket, 'AF_UNIX'), 'No UNIX Sockets')
    def test_unix_sock_client_ops(self):
        with test_utils.run_test_unix_server() as httpd:
            sock = socket.socket(socket.AF_UNIX)
            self._basetest_sock_client_ops(httpd, sock)

    def test_sock_client_fail(self):
        # Make sure that we will get an unused port
        address = None
        try:
            s = socket.socket()
            s.bind(('127.0.0.1', 0))
            address = s.getsockname()
        finally:
            s.close()

        sock = socket.socket()
        sock.setblocking(False)
        with self.assertRaises(ConnectionRefusedError):
            self.loop.run_until_complete(
                self.loop.sock_connect(sock, address))
        sock.close()

    def test_sock_accept(self):
        listener = socket.socket()
        listener.setblocking(False)
        listener.bind(('127.0.0.1', 0))
        listener.listen(1)
        client = socket.socket()
        client.connect(listener.getsockname())

        f = self.loop.sock_accept(listener)
        conn, addr = self.loop.run_until_complete(f)
        self.assertEqual(conn.gettimeout(), 0)
        self.assertEqual(addr, client.getsockname())
        self.assertEqual(client.getpeername(), listener.getsockname())
        client.close()
        conn.close()
        listener.close()

    @unittest.skipUnless(hasattr(signal, 'SIGKILL'), 'No SIGKILL')
    def test_add_signal_handler(self):
        caught = 0

        def my_handler():
            nonlocal caught
            caught += 1

        # Check error behavior first.
        self.assertRaises(
            TypeError, self.loop.add_signal_handler, 'boom', my_handler)
        self.assertRaises(
            TypeError, self.loop.remove_signal_handler, 'boom')
        self.assertRaises(
            ValueError, self.loop.add_signal_handler, signal.NSIG+1,
            my_handler)
        self.assertRaises(
            ValueError, self.loop.remove_signal_handler, signal.NSIG+1)
        self.assertRaises(
            ValueError, self.loop.add_signal_handler, 0, my_handler)
        self.assertRaises(
            ValueError, self.loop.remove_signal_handler, 0)
        self.assertRaises(
            ValueError, self.loop.add_signal_handler, -1, my_handler)
        self.assertRaises(
            ValueError, self.loop.remove_signal_handler, -1)
        self.assertRaises(
            RuntimeError, self.loop.add_signal_handler, signal.SIGKILL,
            my_handler)
        # Removing SIGKILL doesn't raise, since we don't call signal().
        self.assertFalse(self.loop.remove_signal_handler(signal.SIGKILL))
        # Now set a handler and handle it.
        self.loop.add_signal_handler(signal.SIGINT, my_handler)

        os.kill(os.getpid(), signal.SIGINT)
        test_utils.run_until(self.loop, lambda: caught)

        # Removing it should restore the default handler.
        self.assertTrue(self.loop.remove_signal_handler(signal.SIGINT))
        self.assertEqual(signal.getsignal(signal.SIGINT),
                         signal.default_int_handler)
        # Removing again returns False.
        self.assertFalse(self.loop.remove_signal_handler(signal.SIGINT))

    @unittest.skipUnless(hasattr(signal, 'SIGALRM'), 'No SIGALRM')
    def test_signal_handling_while_selecting(self):
        # Test with a signal actually arriving during a select() call.
        caught = 0

        def my_handler():
            nonlocal caught
            caught += 1
            self.loop.stop()

        self.loop.add_signal_handler(signal.SIGALRM, my_handler)

        signal.setitimer(signal.ITIMER_REAL, 0.01, 0)  # Send SIGALRM once.
        self.loop.run_forever()
        self.assertEqual(caught, 1)

    @unittest.skipUnless(hasattr(signal, 'SIGALRM'), 'No SIGALRM')
    def test_signal_handling_args(self):
        some_args = (42,)
        caught = 0

        def my_handler(*args):
            nonlocal caught
            caught += 1
            self.assertEqual(args, some_args)

        self.loop.add_signal_handler(signal.SIGALRM, my_handler, *some_args)

        signal.setitimer(signal.ITIMER_REAL, 0.1, 0)  # Send SIGALRM once.
        self.loop.call_later(0.5, self.loop.stop)
        self.loop.run_forever()
        self.assertEqual(caught, 1)

    def _basetest_create_connection(self, connection_fut, check_sockname=True):
        tr, pr = self.loop.run_until_complete(connection_fut)
        self.assertIsInstance(tr, asyncio.Transport)
        self.assertIsInstance(pr, asyncio.Protocol)
        if check_sockname:
            self.assertIsNotNone(tr.get_extra_info('sockname'))
        self.loop.run_until_complete(pr.done)
        self.assertGreater(pr.nbytes, 0)
        tr.close()

    def test_create_connection(self):
        with test_utils.run_test_server() as httpd:
            conn_fut = self.loop.create_connection(
                lambda: MyProto(loop=self.loop), *httpd.address)
            self._basetest_create_connection(conn_fut)

    @unittest.skipUnless(hasattr(socket, 'AF_UNIX'), 'No UNIX Sockets')
    def test_create_unix_connection(self):
        # Issue #20682: On Mac OS X Tiger, getsockname() returns a
        # zero-length address for UNIX socket.
        check_sockname = not osx_tiger()

        with test_utils.run_test_unix_server() as httpd:
            conn_fut = self.loop.create_unix_connection(
                lambda: MyProto(loop=self.loop), httpd.address)
            self._basetest_create_connection(conn_fut, check_sockname)

    def test_create_connection_sock(self):
        with test_utils.run_test_server() as httpd:
            sock = None
            infos = self.loop.run_until_complete(
                self.loop.getaddrinfo(
                    *httpd.address, type=socket.SOCK_STREAM))
            for family, type, proto, cname, address in infos:
                try:
                    sock = socket.socket(family=family, type=type, proto=proto)
                    sock.setblocking(False)
                    self.loop.run_until_complete(
                        self.loop.sock_connect(sock, address))
                except:
                    pass
                else:
                    break
            else:
                assert False, 'Can not create socket.'

            f = self.loop.create_connection(
                lambda: MyProto(loop=self.loop), sock=sock)
            tr, pr = self.loop.run_until_complete(f)
            self.assertIsInstance(tr, asyncio.Transport)
            self.assertIsInstance(pr, asyncio.Protocol)
            self.loop.run_until_complete(pr.done)
            self.assertGreater(pr.nbytes, 0)
            tr.close()

    def _basetest_create_ssl_connection(self, connection_fut,
                                        check_sockname=True):
        tr, pr = self.loop.run_until_complete(connection_fut)
        self.assertIsInstance(tr, asyncio.Transport)
        self.assertIsInstance(pr, asyncio.Protocol)
        self.assertTrue('ssl' in tr.__class__.__name__.lower())
        if check_sockname:
            self.assertIsNotNone(tr.get_extra_info('sockname'))
        self.loop.run_until_complete(pr.done)
        self.assertGreater(pr.nbytes, 0)
        tr.close()

    @unittest.skipIf(ssl is None, 'No ssl module')
    def test_create_ssl_connection(self):
        with test_utils.run_test_server(use_ssl=True) as httpd:
            conn_fut = self.loop.create_connection(
                lambda: MyProto(loop=self.loop),
                *httpd.address,
                ssl=test_utils.dummy_ssl_context())

            self._basetest_create_ssl_connection(conn_fut)

    @unittest.skipIf(ssl is None, 'No ssl module')
    @unittest.skipUnless(hasattr(socket, 'AF_UNIX'), 'No UNIX Sockets')
    def test_create_ssl_unix_connection(self):
        # Issue #20682: On Mac OS X Tiger, getsockname() returns a
        # zero-length address for UNIX socket.
        check_sockname = not osx_tiger()

        with test_utils.run_test_unix_server(use_ssl=True) as httpd:
            conn_fut = self.loop.create_unix_connection(
                lambda: MyProto(loop=self.loop),
                httpd.address,
                ssl=test_utils.dummy_ssl_context(),
                server_hostname='127.0.0.1')

            self._basetest_create_ssl_connection(conn_fut, check_sockname)

    def test_create_connection_local_addr(self):
        with test_utils.run_test_server() as httpd:
            port = support.find_unused_port()
            f = self.loop.create_connection(
                lambda: MyProto(loop=self.loop),
                *httpd.address, local_addr=(httpd.address[0], port))
            tr, pr = self.loop.run_until_complete(f)
            expected = pr.transport.get_extra_info('sockname')[1]
            self.assertEqual(port, expected)
            tr.close()

    def test_create_connection_local_addr_in_use(self):
        with test_utils.run_test_server() as httpd:
            f = self.loop.create_connection(
                lambda: MyProto(loop=self.loop),
                *httpd.address, local_addr=httpd.address)
            with self.assertRaises(OSError) as cm:
                self.loop.run_until_complete(f)
            self.assertEqual(cm.exception.errno, errno.EADDRINUSE)
            self.assertIn(str(httpd.address), cm.exception.strerror)

    def test_create_server(self):
        proto = MyProto(self.loop)
        f = self.loop.create_server(lambda: proto, '0.0.0.0', 0)
        server = self.loop.run_until_complete(f)
        self.assertEqual(len(server.sockets), 1)
        sock = server.sockets[0]
        host, port = sock.getsockname()
        self.assertEqual(host, '0.0.0.0')
        client = socket.socket()
        client.connect(('127.0.0.1', port))
        client.sendall(b'xxx')

        self.loop.run_until_complete(proto.connected)
        self.assertEqual('CONNECTED', proto.state)

        test_utils.run_until(self.loop, lambda: proto.nbytes > 0)
        self.assertEqual(3, proto.nbytes)

        # extra info is available
        self.assertIsNotNone(proto.transport.get_extra_info('sockname'))
        self.assertEqual('127.0.0.1',
                         proto.transport.get_extra_info('peername')[0])

        # close connection
        proto.transport.close()
        self.loop.run_until_complete(proto.done)

        self.assertEqual('CLOSED', proto.state)

        # the client socket must be closed after to avoid ECONNRESET upon
        # recv()/send() on the serving socket
        client.close()

        # close server
        server.close()

    def _make_unix_server(self, factory, **kwargs):
        path = test_utils.gen_unix_socket_path()
        self.addCleanup(lambda: os.path.exists(path) and os.unlink(path))

        f = self.loop.create_unix_server(factory, path, **kwargs)
        server = self.loop.run_until_complete(f)

        return server, path

    @unittest.skipUnless(hasattr(socket, 'AF_UNIX'), 'No UNIX Sockets')
    def test_create_unix_server(self):
        proto = MyProto(loop=self.loop)
        server, path = self._make_unix_server(lambda: proto)
        self.assertEqual(len(server.sockets), 1)

        client = socket.socket(socket.AF_UNIX)
        client.connect(path)
        client.sendall(b'xxx')

        self.loop.run_until_complete(proto.connected)
        self.assertEqual('CONNECTED', proto.state)
        test_utils.run_until(self.loop, lambda: proto.nbytes > 0)
        self.assertEqual(3, proto.nbytes)

        # close connection
        proto.transport.close()
        self.loop.run_until_complete(proto.done)

        self.assertEqual('CLOSED', proto.state)

        # the client socket must be closed after to avoid ECONNRESET upon
        # recv()/send() on the serving socket
        client.close()

        # close server
        server.close()

    @unittest.skipUnless(hasattr(socket, 'AF_UNIX'), 'No UNIX Sockets')
    def test_create_unix_server_path_socket_error(self):
        proto = MyProto(loop=self.loop)
        sock = socket.socket()
        with sock:
            f = self.loop.create_unix_server(lambda: proto, '/test', sock=sock)
            with self.assertRaisesRegex(ValueError,
                                        'path and sock can not be specified '
                                        'at the same time'):
                server = self.loop.run_until_complete(f)

    def _create_ssl_context(self, certfile, keyfile=None):
        sslcontext = ssl.SSLContext(ssl.PROTOCOL_SSLv23)
        sslcontext.options |= ssl.OP_NO_SSLv2
        sslcontext.load_cert_chain(certfile, keyfile)
        return sslcontext

    def _make_ssl_server(self, factory, certfile, keyfile=None):
        sslcontext = self._create_ssl_context(certfile, keyfile)

        f = self.loop.create_server(factory, '127.0.0.1', 0, ssl=sslcontext)
        server = self.loop.run_until_complete(f)

        sock = server.sockets[0]
        host, port = sock.getsockname()
        self.assertEqual(host, '127.0.0.1')
        return server, host, port

    def _make_ssl_unix_server(self, factory, certfile, keyfile=None):
        sslcontext = self._create_ssl_context(certfile, keyfile)
        return self._make_unix_server(factory, ssl=sslcontext)

    @unittest.skipIf(ssl is None, 'No ssl module')
    def test_create_server_ssl(self):
        proto = MyProto(loop=self.loop)
        server, host, port = self._make_ssl_server(
            lambda: proto, ONLYCERT, ONLYKEY)

        f_c = self.loop.create_connection(MyBaseProto, host, port,
                                          ssl=test_utils.dummy_ssl_context())
        client, pr = self.loop.run_until_complete(f_c)

        client.write(b'xxx')
        self.loop.run_until_complete(proto.connected)
        self.assertEqual('CONNECTED', proto.state)

        test_utils.run_until(self.loop, lambda: proto.nbytes > 0)
        self.assertEqual(3, proto.nbytes)

        # extra info is available
        self.assertIsNotNone(proto.transport.get_extra_info('sockname'))
        self.assertEqual('127.0.0.1',
                         proto.transport.get_extra_info('peername')[0])

        # close connection
        proto.transport.close()
        self.loop.run_until_complete(proto.done)
        self.assertEqual('CLOSED', proto.state)

        # the client socket must be closed after to avoid ECONNRESET upon
        # recv()/send() on the serving socket
        client.close()

        # stop serving
        server.close()

    @unittest.skipIf(ssl is None, 'No ssl module')
    @unittest.skipUnless(hasattr(socket, 'AF_UNIX'), 'No UNIX Sockets')
    def test_create_unix_server_ssl(self):
        proto = MyProto(loop=self.loop)
        server, path = self._make_ssl_unix_server(
            lambda: proto, ONLYCERT, ONLYKEY)

        f_c = self.loop.create_unix_connection(
            MyBaseProto, path, ssl=test_utils.dummy_ssl_context(),
            server_hostname='')

        client, pr = self.loop.run_until_complete(f_c)

        client.write(b'xxx')
        self.loop.run_until_complete(proto.connected)
        self.assertEqual('CONNECTED', proto.state)
        test_utils.run_until(self.loop, lambda: proto.nbytes > 0)
        self.assertEqual(3, proto.nbytes)

        # close connection
        proto.transport.close()
        self.loop.run_until_complete(proto.done)
        self.assertEqual('CLOSED', proto.state)

        # the client socket must be closed after to avoid ECONNRESET upon
        # recv()/send() on the serving socket
        client.close()

        # stop serving
        server.close()

    @unittest.skipIf(ssl is None, 'No ssl module')
    @unittest.skipUnless(HAS_SNI, 'No SNI support in ssl module')
    def test_create_server_ssl_verify_failed(self):
        proto = MyProto(loop=self.loop)
        server, host, port = self._make_ssl_server(
            lambda: proto, SIGNED_CERTFILE)

        sslcontext_client = ssl.SSLContext(ssl.PROTOCOL_SSLv23)
        sslcontext_client.options |= ssl.OP_NO_SSLv2
        sslcontext_client.verify_mode = ssl.CERT_REQUIRED
        if hasattr(sslcontext_client, 'check_hostname'):
            sslcontext_client.check_hostname = True

        # no CA loaded
        f_c = self.loop.create_connection(MyProto, host, port,
                                          ssl=sslcontext_client)
        with self.assertRaisesRegex(ssl.SSLError,
                                    'certificate verify failed '):
            self.loop.run_until_complete(f_c)

        # close connection
        self.assertIsNone(proto.transport)
        server.close()

    @unittest.skipIf(ssl is None, 'No ssl module')
    @unittest.skipUnless(HAS_SNI, 'No SNI support in ssl module')
    @unittest.skipUnless(hasattr(socket, 'AF_UNIX'), 'No UNIX Sockets')
    def test_create_unix_server_ssl_verify_failed(self):
        proto = MyProto(loop=self.loop)
        server, path = self._make_ssl_unix_server(
            lambda: proto, SIGNED_CERTFILE)

        sslcontext_client = ssl.SSLContext(ssl.PROTOCOL_SSLv23)
        sslcontext_client.options |= ssl.OP_NO_SSLv2
        sslcontext_client.verify_mode = ssl.CERT_REQUIRED
        if hasattr(sslcontext_client, 'check_hostname'):
            sslcontext_client.check_hostname = True

        # no CA loaded
        f_c = self.loop.create_unix_connection(MyProto, path,
                                               ssl=sslcontext_client,
                                               server_hostname='invalid')
        with self.assertRaisesRegex(ssl.SSLError,
                                    'certificate verify failed '):
            self.loop.run_until_complete(f_c)

        # close connection
        self.assertIsNone(proto.transport)
        server.close()

    @unittest.skipIf(ssl is None, 'No ssl module')
    @unittest.skipUnless(HAS_SNI, 'No SNI support in ssl module')
    def test_create_server_ssl_match_failed(self):
        proto = MyProto(loop=self.loop)
        server, host, port = self._make_ssl_server(
            lambda: proto, SIGNED_CERTFILE)

        sslcontext_client = ssl.SSLContext(ssl.PROTOCOL_SSLv23)
        sslcontext_client.options |= ssl.OP_NO_SSLv2
        sslcontext_client.verify_mode = ssl.CERT_REQUIRED
        sslcontext_client.load_verify_locations(
            cafile=SIGNING_CA)
        if hasattr(sslcontext_client, 'check_hostname'):
            sslcontext_client.check_hostname = True

        # incorrect server_hostname
        f_c = self.loop.create_connection(MyProto, host, port,
                                          ssl=sslcontext_client)
        with self.assertRaisesRegex(
                ssl.CertificateError,
                "hostname '127.0.0.1' doesn't match 'localhost'"):
            self.loop.run_until_complete(f_c)

        # close connection
        proto.transport.close()
        server.close()

    @unittest.skipIf(ssl is None, 'No ssl module')
    @unittest.skipUnless(HAS_SNI, 'No SNI support in ssl module')
    @unittest.skipUnless(hasattr(socket, 'AF_UNIX'), 'No UNIX Sockets')
    def test_create_unix_server_ssl_verified(self):
        proto = MyProto(loop=self.loop)
        server, path = self._make_ssl_unix_server(
            lambda: proto, SIGNED_CERTFILE)

        sslcontext_client = ssl.SSLContext(ssl.PROTOCOL_SSLv23)
        sslcontext_client.options |= ssl.OP_NO_SSLv2
        sslcontext_client.verify_mode = ssl.CERT_REQUIRED
        sslcontext_client.load_verify_locations(cafile=SIGNING_CA)
        if hasattr(sslcontext_client, 'check_hostname'):
            sslcontext_client.check_hostname = True

        # Connection succeeds with correct CA and server hostname.
        f_c = self.loop.create_unix_connection(MyProto, path,
                                               ssl=sslcontext_client,
                                               server_hostname='localhost')
        client, pr = self.loop.run_until_complete(f_c)

        # close connection
        proto.transport.close()
        client.close()
        server.close()

    @unittest.skipIf(ssl is None, 'No ssl module')
    @unittest.skipUnless(HAS_SNI, 'No SNI support in ssl module')
    def test_create_server_ssl_verified(self):
        proto = MyProto(loop=self.loop)
        server, host, port = self._make_ssl_server(
            lambda: proto, SIGNED_CERTFILE)

        sslcontext_client = ssl.SSLContext(ssl.PROTOCOL_SSLv23)
        sslcontext_client.options |= ssl.OP_NO_SSLv2
        sslcontext_client.verify_mode = ssl.CERT_REQUIRED
        sslcontext_client.load_verify_locations(cafile=SIGNING_CA)
        if hasattr(sslcontext_client, 'check_hostname'):
            sslcontext_client.check_hostname = True

        # Connection succeeds with correct CA and server hostname.
        f_c = self.loop.create_connection(MyProto, host, port,
                                          ssl=sslcontext_client,
                                          server_hostname='localhost')
        client, pr = self.loop.run_until_complete(f_c)

        # close connection
        proto.transport.close()
        client.close()
        server.close()

    def test_create_server_sock(self):
        proto = asyncio.Future(loop=self.loop)

        class TestMyProto(MyProto):
            def connection_made(self, transport):
                super().connection_made(transport)
                proto.set_result(self)

        sock_ob = socket.socket(type=socket.SOCK_STREAM)
        sock_ob.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock_ob.bind(('0.0.0.0', 0))

        f = self.loop.create_server(TestMyProto, sock=sock_ob)
        server = self.loop.run_until_complete(f)
        sock = server.sockets[0]
        self.assertIs(sock, sock_ob)

        host, port = sock.getsockname()
        self.assertEqual(host, '0.0.0.0')
        client = socket.socket()
        client.connect(('127.0.0.1', port))
        client.send(b'xxx')
        client.close()
        server.close()

    def test_create_server_addr_in_use(self):
        sock_ob = socket.socket(type=socket.SOCK_STREAM)
        sock_ob.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock_ob.bind(('0.0.0.0', 0))

        f = self.loop.create_server(MyProto, sock=sock_ob)
        server = self.loop.run_until_complete(f)
        sock = server.sockets[0]
        host, port = sock.getsockname()

        f = self.loop.create_server(MyProto, host=host, port=port)
        with self.assertRaises(OSError) as cm:
            self.loop.run_until_complete(f)
        self.assertEqual(cm.exception.errno, errno.EADDRINUSE)

        server.close()

    @unittest.skipUnless(support.IPV6_ENABLED, 'IPv6 not supported or enabled')
    def test_create_server_dual_stack(self):
        f_proto = asyncio.Future(loop=self.loop)

        class TestMyProto(MyProto):
            def connection_made(self, transport):
                super().connection_made(transport)
                f_proto.set_result(self)

        try_count = 0
        while True:
            try:
                port = support.find_unused_port()
                f = self.loop.create_server(TestMyProto, host=None, port=port)
                server = self.loop.run_until_complete(f)
            except OSError as ex:
                if ex.errno == errno.EADDRINUSE:
                    try_count += 1
                    self.assertGreaterEqual(5, try_count)
                    continue
                else:
                    raise
            else:
                break
        client = socket.socket()
        client.connect(('127.0.0.1', port))
        client.send(b'xxx')
        proto = self.loop.run_until_complete(f_proto)
        proto.transport.close()
        client.close()

        f_proto = asyncio.Future(loop=self.loop)
        client = socket.socket(socket.AF_INET6)
        client.connect(('::1', port))
        client.send(b'xxx')
        proto = self.loop.run_until_complete(f_proto)
        proto.transport.close()
        client.close()

        server.close()

    def test_server_close(self):
        f = self.loop.create_server(MyProto, '0.0.0.0', 0)
        server = self.loop.run_until_complete(f)
        sock = server.sockets[0]
        host, port = sock.getsockname()

        client = socket.socket()
        client.connect(('127.0.0.1', port))
        client.send(b'xxx')
        client.close()

        server.close()

        client = socket.socket()
        self.assertRaises(
            ConnectionRefusedError, client.connect, ('127.0.0.1', port))
        client.close()

    def test_create_datagram_endpoint(self):
        class TestMyDatagramProto(MyDatagramProto):
            def __init__(inner_self):
                super().__init__(loop=self.loop)

            def datagram_received(self, data, addr):
                super().datagram_received(data, addr)
                self.transport.sendto(b'resp:'+data, addr)

        coro = self.loop.create_datagram_endpoint(
            TestMyDatagramProto, local_addr=('127.0.0.1', 0))
        s_transport, server = self.loop.run_until_complete(coro)
        host, port = s_transport.get_extra_info('sockname')

        coro = self.loop.create_datagram_endpoint(
            lambda: MyDatagramProto(loop=self.loop),
            remote_addr=(host, port))
        transport, client = self.loop.run_until_complete(coro)

        self.assertEqual('INITIALIZED', client.state)
        transport.sendto(b'xxx')
        test_utils.run_until(self.loop, lambda: server.nbytes)
        self.assertEqual(3, server.nbytes)
        test_utils.run_until(self.loop, lambda: client.nbytes)

        # received
        self.assertEqual(8, client.nbytes)

        # extra info is available
        self.assertIsNotNone(transport.get_extra_info('sockname'))

        # close connection
        transport.close()
        self.loop.run_until_complete(client.done)
        self.assertEqual('CLOSED', client.state)
        server.transport.close()

    def test_internal_fds(self):
        loop = self.create_event_loop()
        if not isinstance(loop, selector_events.BaseSelectorEventLoop):
            self.skipTest('loop is not a BaseSelectorEventLoop')

        self.assertEqual(1, loop._internal_fds)
        loop.close()
        self.assertEqual(0, loop._internal_fds)
        self.assertIsNone(loop._csock)
        self.assertIsNone(loop._ssock)

    @unittest.skipUnless(sys.platform != 'win32',
                         "Don't support pipes for Windows")
    def test_read_pipe(self):
        proto = MyReadPipeProto(loop=self.loop)

        rpipe, wpipe = os.pipe()
        pipeobj = io.open(rpipe, 'rb', 1024)

        @asyncio.coroutine
        def connect():
            t, p = yield from self.loop.connect_read_pipe(
                lambda: proto, pipeobj)
            self.assertIs(p, proto)
            self.assertIs(t, proto.transport)
            self.assertEqual(['INITIAL', 'CONNECTED'], proto.state)
            self.assertEqual(0, proto.nbytes)

        self.loop.run_until_complete(connect())

        os.write(wpipe, b'1')
        test_utils.run_until(self.loop, lambda: proto.nbytes >= 1)
        self.assertEqual(1, proto.nbytes)

        os.write(wpipe, b'2345')
        test_utils.run_until(self.loop, lambda: proto.nbytes >= 5)
        self.assertEqual(['INITIAL', 'CONNECTED'], proto.state)
        self.assertEqual(5, proto.nbytes)

        os.close(wpipe)
        self.loop.run_until_complete(proto.done)
        self.assertEqual(
            ['INITIAL', 'CONNECTED', 'EOF', 'CLOSED'], proto.state)
        # extra info is available
        self.assertIsNotNone(proto.transport.get_extra_info('pipe'))

    @unittest.skipUnless(sys.platform != 'win32',
                         "Don't support pipes for Windows")
    # select, poll and kqueue don't support character devices (PTY) on Mac OS X
    # older than 10.6 (Snow Leopard)
    @support.requires_mac_ver(10, 6)
    # Issue #20495: The test hangs on FreeBSD 7.2 but pass on FreeBSD 9
    @support.requires_freebsd_version(8)
    def test_read_pty_output(self):
        proto = MyReadPipeProto(loop=self.loop)

        master, slave = os.openpty()
        master_read_obj = io.open(master, 'rb', 0)

        @asyncio.coroutine
        def connect():
            t, p = yield from self.loop.connect_read_pipe(lambda: proto,
                                                          master_read_obj)
            self.assertIs(p, proto)
            self.assertIs(t, proto.transport)
            self.assertEqual(['INITIAL', 'CONNECTED'], proto.state)
            self.assertEqual(0, proto.nbytes)

        self.loop.run_until_complete(connect())

        os.write(slave, b'1')
        test_utils.run_until(self.loop, lambda: proto.nbytes)
        self.assertEqual(1, proto.nbytes)

        os.write(slave, b'2345')
        test_utils.run_until(self.loop, lambda: proto.nbytes >= 5)
        self.assertEqual(['INITIAL', 'CONNECTED'], proto.state)
        self.assertEqual(5, proto.nbytes)

        os.close(slave)
        self.loop.run_until_complete(proto.done)
        self.assertEqual(
            ['INITIAL', 'CONNECTED', 'EOF', 'CLOSED'], proto.state)
        # extra info is available
        self.assertIsNotNone(proto.transport.get_extra_info('pipe'))

    @unittest.skipUnless(sys.platform != 'win32',
                         "Don't support pipes for Windows")
    def test_write_pipe(self):
        rpipe, wpipe = os.pipe()
        pipeobj = io.open(wpipe, 'wb', 1024)

        proto = MyWritePipeProto(loop=self.loop)
        connect = self.loop.connect_write_pipe(lambda: proto, pipeobj)
        transport, p = self.loop.run_until_complete(connect)
        self.assertIs(p, proto)
        self.assertIs(transport, proto.transport)
        self.assertEqual('CONNECTED', proto.state)

        transport.write(b'1')

        data = bytearray()
        def reader(data):
            chunk = os.read(rpipe, 1024)
            data += chunk
            return len(data)

        test_utils.run_until(self.loop, lambda: reader(data) >= 1)
        self.assertEqual(b'1', data)

        transport.write(b'2345')
        test_utils.run_until(self.loop, lambda: reader(data) >= 5)
        self.assertEqual(b'12345', data)
        self.assertEqual('CONNECTED', proto.state)

        os.close(rpipe)

        # extra info is available
        self.assertIsNotNone(proto.transport.get_extra_info('pipe'))

        # close connection
        proto.transport.close()
        self.loop.run_until_complete(proto.done)
        self.assertEqual('CLOSED', proto.state)

    @unittest.skipUnless(sys.platform != 'win32',
                         "Don't support pipes for Windows")
    def test_write_pipe_disconnect_on_close(self):
        rsock, wsock = test_utils.socketpair()
        pipeobj = io.open(wsock.detach(), 'wb', 1024)

        proto = MyWritePipeProto(loop=self.loop)
        connect = self.loop.connect_write_pipe(lambda: proto, pipeobj)
        transport, p = self.loop.run_until_complete(connect)
        self.assertIs(p, proto)
        self.assertIs(transport, proto.transport)
        self.assertEqual('CONNECTED', proto.state)

        transport.write(b'1')
        data = self.loop.run_until_complete(self.loop.sock_recv(rsock, 1024))
        self.assertEqual(b'1', data)

        rsock.close()

        self.loop.run_until_complete(proto.done)
        self.assertEqual('CLOSED', proto.state)

    @unittest.skipUnless(sys.platform != 'win32',
                         "Don't support pipes for Windows")
    # select, poll and kqueue don't support character devices (PTY) on Mac OS X
    # older than 10.6 (Snow Leopard)
    @support.requires_mac_ver(10, 6)
    def test_write_pty(self):
        master, slave = os.openpty()
        slave_write_obj = io.open(slave, 'wb', 0)

        proto = MyWritePipeProto(loop=self.loop)
        connect = self.loop.connect_write_pipe(lambda: proto, slave_write_obj)
        transport, p = self.loop.run_until_complete(connect)
        self.assertIs(p, proto)
        self.assertIs(transport, proto.transport)
        self.assertEqual('CONNECTED', proto.state)

        transport.write(b'1')

        data = bytearray()
        def reader(data):
            chunk = os.read(master, 1024)
            data += chunk
            return len(data)

        test_utils.run_until(self.loop, lambda: reader(data) >= 1,
                             timeout=10)
        self.assertEqual(b'1', data)

        transport.write(b'2345')
        test_utils.run_until(self.loop, lambda: reader(data) >= 5,
                             timeout=10)
        self.assertEqual(b'12345', data)
        self.assertEqual('CONNECTED', proto.state)

        os.close(master)

        # extra info is available
        self.assertIsNotNone(proto.transport.get_extra_info('pipe'))

        # close connection
        proto.transport.close()
        self.loop.run_until_complete(proto.done)
        self.assertEqual('CLOSED', proto.state)

    def test_prompt_cancellation(self):
        r, w = test_utils.socketpair()
        r.setblocking(False)
        f = self.loop.sock_recv(r, 1)
        ov = getattr(f, 'ov', None)
        if ov is not None:
            self.assertTrue(ov.pending)

        @asyncio.coroutine
        def main():
            try:
                self.loop.call_soon(f.cancel)
                yield from f
            except asyncio.CancelledError:
                res = 'cancelled'
            else:
                res = None
            finally:
                self.loop.stop()
            return res

        start = time.monotonic()
        t = asyncio.Task(main(), loop=self.loop)
        self.loop.run_forever()
        elapsed = time.monotonic() - start

        self.assertLess(elapsed, 0.1)
        self.assertEqual(t.result(), 'cancelled')
        self.assertRaises(asyncio.CancelledError, f.result)
        if ov is not None:
            self.assertFalse(ov.pending)
        self.loop._stop_serving(r)

        r.close()
        w.close()

    def test_timeout_rounding(self):
        def _run_once():
            self.loop._run_once_counter += 1
            orig_run_once()

        orig_run_once = self.loop._run_once
        self.loop._run_once_counter = 0
        self.loop._run_once = _run_once

        @asyncio.coroutine
        def wait():
            loop = self.loop
            yield from asyncio.sleep(1e-2, loop=loop)
            yield from asyncio.sleep(1e-4, loop=loop)
            yield from asyncio.sleep(1e-6, loop=loop)
            yield from asyncio.sleep(1e-8, loop=loop)
            yield from asyncio.sleep(1e-10, loop=loop)

        self.loop.run_until_complete(wait())
        # The ideal number of call is 12, but on some platforms, the selector
        # may sleep at little bit less than timeout depending on the resolution
        # of the clock used by the kernel. Tolerate a few useless calls on
        # these platforms.
        self.assertLessEqual(self.loop._run_once_counter, 20,
            {'clock_resolution': self.loop._clock_resolution,
             'selector': self.loop._selector.__class__.__name__})

    def test_sock_connect_address(self):
        addresses = [(socket.AF_INET, ('www.python.org', 80))]
        if support.IPV6_ENABLED:
            addresses.extend((
                (socket.AF_INET6, ('www.python.org', 80)),
                (socket.AF_INET6, ('www.python.org', 80, 0, 0)),
            ))

        for family, address in addresses:
            for sock_type in (socket.SOCK_STREAM, socket.SOCK_DGRAM):
                sock = socket.socket(family, sock_type)
                with sock:
                    connect = self.loop.sock_connect(sock, address)
                    with self.assertRaises(ValueError) as cm:
                        self.loop.run_until_complete(connect)
                    self.assertIn('address must be resolved',
                                  str(cm.exception))

    def test_remove_fds_after_closing(self):
        loop = self.create_event_loop()
        callback = lambda: None
        r, w = test_utils.socketpair()
        self.addCleanup(r.close)
        self.addCleanup(w.close)
        loop.add_reader(r, callback)
        loop.add_writer(w, callback)
        loop.close()
        self.assertFalse(loop.remove_reader(r))
        self.assertFalse(loop.remove_writer(w))

    def test_add_fds_after_closing(self):
        loop = self.create_event_loop()
        callback = lambda: None
        r, w = test_utils.socketpair()
        self.addCleanup(r.close)
        self.addCleanup(w.close)
        loop.close()
        with self.assertRaises(RuntimeError):
            loop.add_reader(r, callback)
        with self.assertRaises(RuntimeError):
            loop.add_writer(w, callback)


class SubprocessTestsMixin:

    def check_terminated(self, returncode):
        if sys.platform == 'win32':
            self.assertIsInstance(returncode, int)
            # expect 1 but sometimes get 0
        else:
            self.assertEqual(-signal.SIGTERM, returncode)

    def check_killed(self, returncode):
        if sys.platform == 'win32':
            self.assertIsInstance(returncode, int)
            # expect 1 but sometimes get 0
        else:
            self.assertEqual(-signal.SIGKILL, returncode)

    def test_subprocess_exec(self):
        prog = os.path.join(os.path.dirname(__file__), 'echo.py')

        connect = self.loop.subprocess_exec(
                        functools.partial(MySubprocessProtocol, self.loop),
                        sys.executable, prog)
        transp, proto = self.loop.run_until_complete(connect)
        self.assertIsInstance(proto, MySubprocessProtocol)
        self.loop.run_until_complete(proto.connected)
        self.assertEqual('CONNECTED', proto.state)

        stdin = transp.get_pipe_transport(0)
        stdin.write(b'Python The Winner')
        self.loop.run_until_complete(proto.got_data[1].wait())
        transp.close()
        self.loop.run_until_complete(proto.completed)
        self.check_terminated(proto.returncode)
        self.assertEqual(b'Python The Winner', proto.data[1])

    def test_subprocess_interactive(self):
        prog = os.path.join(os.path.dirname(__file__), 'echo.py')

        connect = self.loop.subprocess_exec(
                        functools.partial(MySubprocessProtocol, self.loop),
                        sys.executable, prog)
        transp, proto = self.loop.run_until_complete(connect)
        self.assertIsInstance(proto, MySubprocessProtocol)
        self.loop.run_until_complete(proto.connected)
        self.assertEqual('CONNECTED', proto.state)

        try:
            stdin = transp.get_pipe_transport(0)
            stdin.write(b'Python ')
            self.loop.run_until_complete(proto.got_data[1].wait())
            proto.got_data[1].clear()
            self.assertEqual(b'Python ', proto.data[1])

            stdin.write(b'The Winner')
            self.loop.run_until_complete(proto.got_data[1].wait())
            self.assertEqual(b'Python The Winner', proto.data[1])
        finally:
            transp.close()

        self.loop.run_until_complete(proto.completed)
        self.check_terminated(proto.returncode)

    def test_subprocess_shell(self):
        connect = self.loop.subprocess_shell(
                        functools.partial(MySubprocessProtocol, self.loop),
                        'echo Python')
        transp, proto = self.loop.run_until_complete(connect)
        self.assertIsInstance(proto, MySubprocessProtocol)
        self.loop.run_until_complete(proto.connected)

        transp.get_pipe_transport(0).close()
        self.loop.run_until_complete(proto.completed)
        self.assertEqual(0, proto.returncode)
        self.assertTrue(all(f.done() for f in proto.disconnects.values()))
        self.assertEqual(proto.data[1].rstrip(b'\r\n'), b'Python')
        self.assertEqual(proto.data[2], b'')

    def test_subprocess_exitcode(self):
        connect = self.loop.subprocess_shell(
                        functools.partial(MySubprocessProtocol, self.loop),
                        'exit 7', stdin=None, stdout=None, stderr=None)
        transp, proto = self.loop.run_until_complete(connect)
        self.assertIsInstance(proto, MySubprocessProtocol)
        self.loop.run_until_complete(proto.completed)
        self.assertEqual(7, proto.returncode)

    def test_subprocess_close_after_finish(self):
        connect = self.loop.subprocess_shell(
                        functools.partial(MySubprocessProtocol, self.loop),
                        'exit 7', stdin=None, stdout=None, stderr=None)
        transp, proto = self.loop.run_until_complete(connect)
        self.assertIsInstance(proto, MySubprocessProtocol)
        self.assertIsNone(transp.get_pipe_transport(0))
        self.assertIsNone(transp.get_pipe_transport(1))
        self.assertIsNone(transp.get_pipe_transport(2))
        self.loop.run_until_complete(proto.completed)
        self.assertEqual(7, proto.returncode)
        self.assertIsNone(transp.close())

    def test_subprocess_kill(self):
        prog = os.path.join(os.path.dirname(__file__), 'echo.py')

        connect = self.loop.subprocess_exec(
                        functools.partial(MySubprocessProtocol, self.loop),
                        sys.executable, prog)
        transp, proto = self.loop.run_until_complete(connect)
        self.assertIsInstance(proto, MySubprocessProtocol)
        self.loop.run_until_complete(proto.connected)

        transp.kill()
        self.loop.run_until_complete(proto.completed)
        self.check_killed(proto.returncode)

    def test_subprocess_terminate(self):
        prog = os.path.join(os.path.dirname(__file__), 'echo.py')

        connect = self.loop.subprocess_exec(
                        functools.partial(MySubprocessProtocol, self.loop),
                        sys.executable, prog)
        transp, proto = self.loop.run_until_complete(connect)
        self.assertIsInstance(proto, MySubprocessProtocol)
        self.loop.run_until_complete(proto.connected)

        transp.terminate()
        self.loop.run_until_complete(proto.completed)
        self.check_terminated(proto.returncode)

    @unittest.skipIf(sys.platform == 'win32', "Don't have SIGHUP")
    def test_subprocess_send_signal(self):
        prog = os.path.join(os.path.dirname(__file__), 'echo.py')

        connect = self.loop.subprocess_exec(
                        functools.partial(MySubprocessProtocol, self.loop),
                        sys.executable, prog)
        transp, proto = self.loop.run_until_complete(connect)
        self.assertIsInstance(proto, MySubprocessProtocol)
        self.loop.run_until_complete(proto.connected)

        transp.send_signal(signal.SIGHUP)
        self.loop.run_until_complete(proto.completed)
        self.assertEqual(-signal.SIGHUP, proto.returncode)

    def test_subprocess_stderr(self):
        prog = os.path.join(os.path.dirname(__file__), 'echo2.py')

        connect = self.loop.subprocess_exec(
                        functools.partial(MySubprocessProtocol, self.loop),
                        sys.executable, prog)
        transp, proto = self.loop.run_until_complete(connect)
        self.assertIsInstance(proto, MySubprocessProtocol)
        self.loop.run_until_complete(proto.connected)

        stdin = transp.get_pipe_transport(0)
        stdin.write(b'test')

        self.loop.run_until_complete(proto.completed)

        transp.close()
        self.assertEqual(b'OUT:test', proto.data[1])
        self.assertTrue(proto.data[2].startswith(b'ERR:test'), proto.data[2])
        self.assertEqual(0, proto.returncode)

    def test_subprocess_stderr_redirect_to_stdout(self):
        prog = os.path.join(os.path.dirname(__file__), 'echo2.py')

        connect = self.loop.subprocess_exec(
                        functools.partial(MySubprocessProtocol, self.loop),
                        sys.executable, prog, stderr=subprocess.STDOUT)
        transp, proto = self.loop.run_until_complete(connect)
        self.assertIsInstance(proto, MySubprocessProtocol)
        self.loop.run_until_complete(proto.connected)

        stdin = transp.get_pipe_transport(0)
        self.assertIsNotNone(transp.get_pipe_transport(1))
        self.assertIsNone(transp.get_pipe_transport(2))

        stdin.write(b'test')
        self.loop.run_until_complete(proto.completed)
        self.assertTrue(proto.data[1].startswith(b'OUT:testERR:test'),
                        proto.data[1])
        self.assertEqual(b'', proto.data[2])

        transp.close()
        self.assertEqual(0, proto.returncode)

    def test_subprocess_close_client_stream(self):
        prog = os.path.join(os.path.dirname(__file__), 'echo3.py')

        connect = self.loop.subprocess_exec(
                        functools.partial(MySubprocessProtocol, self.loop),
                        sys.executable, prog)
        transp, proto = self.loop.run_until_complete(connect)
        self.assertIsInstance(proto, MySubprocessProtocol)
        self.loop.run_until_complete(proto.connected)

        stdin = transp.get_pipe_transport(0)
        stdout = transp.get_pipe_transport(1)
        stdin.write(b'test')
        self.loop.run_until_complete(proto.got_data[1].wait())
        self.assertEqual(b'OUT:test', proto.data[1])

        stdout.close()
        self.loop.run_until_complete(proto.disconnects[1])
        stdin.write(b'xxx')
        self.loop.run_until_complete(proto.got_data[2].wait())
        if sys.platform != 'win32':
            self.assertEqual(b'ERR:BrokenPipeError', proto.data[2])
        else:
            # After closing the read-end of a pipe, writing to the
            # write-end using os.write() fails with errno==EINVAL and
            # GetLastError()==ERROR_INVALID_NAME on Windows!?!  (Using
            # WriteFile() we get ERROR_BROKEN_PIPE as expected.)
            self.assertEqual(b'ERR:OSError', proto.data[2])
        transp.close()
        self.loop.run_until_complete(proto.completed)
        self.check_terminated(proto.returncode)

    def test_subprocess_wait_no_same_group(self):
        # start the new process in a new session
        connect = self.loop.subprocess_shell(
                        functools.partial(MySubprocessProtocol, self.loop),
                        'exit 7', stdin=None, stdout=None, stderr=None,
                        start_new_session=True)
        _, proto = yield self.loop.run_until_complete(connect)
        self.assertIsInstance(proto, MySubprocessProtocol)
        self.loop.run_until_complete(proto.completed)
        self.assertEqual(7, proto.returncode)

    def test_subprocess_exec_invalid_args(self):
        @asyncio.coroutine
        def connect(**kwds):
            yield from self.loop.subprocess_exec(
                asyncio.SubprocessProtocol,
                'pwd', **kwds)

        with self.assertRaises(ValueError):
            self.loop.run_until_complete(connect(universal_newlines=True))
        with self.assertRaises(ValueError):
            self.loop.run_until_complete(connect(bufsize=4096))
        with self.assertRaises(ValueError):
            self.loop.run_until_complete(connect(shell=True))

    def test_subprocess_shell_invalid_args(self):
        @asyncio.coroutine
        def connect(cmd=None, **kwds):
            if not cmd:
                cmd = 'pwd'
            yield from self.loop.subprocess_shell(
                asyncio.SubprocessProtocol,
                cmd, **kwds)

        with self.assertRaises(ValueError):
            self.loop.run_until_complete(connect(['ls', '-l']))
        with self.assertRaises(ValueError):
            self.loop.run_until_complete(connect(universal_newlines=True))
        with self.assertRaises(ValueError):
            self.loop.run_until_complete(connect(bufsize=4096))
        with self.assertRaises(ValueError):
            self.loop.run_until_complete(connect(shell=False))


if sys.platform == 'win32':

    class SelectEventLoopTests(EventLoopTestsMixin, unittest.TestCase):

        def create_event_loop(self):
            return asyncio.SelectorEventLoop()

    class ProactorEventLoopTests(EventLoopTestsMixin,
                                 SubprocessTestsMixin,
                                 unittest.TestCase):

        def create_event_loop(self):
            return asyncio.ProactorEventLoop()

        def test_create_ssl_connection(self):
            raise unittest.SkipTest("IocpEventLoop incompatible with SSL")

        def test_create_server_ssl(self):
            raise unittest.SkipTest("IocpEventLoop incompatible with SSL")

        def test_create_server_ssl_verify_failed(self):
            raise unittest.SkipTest("IocpEventLoop incompatible with SSL")

        def test_create_server_ssl_match_failed(self):
            raise unittest.SkipTest("IocpEventLoop incompatible with SSL")

        def test_create_server_ssl_verified(self):
            raise unittest.SkipTest("IocpEventLoop incompatible with SSL")

        def test_reader_callback(self):
            raise unittest.SkipTest("IocpEventLoop does not have add_reader()")

        def test_reader_callback_cancel(self):
            raise unittest.SkipTest("IocpEventLoop does not have add_reader()")

        def test_writer_callback(self):
            raise unittest.SkipTest("IocpEventLoop does not have add_writer()")

        def test_writer_callback_cancel(self):
            raise unittest.SkipTest("IocpEventLoop does not have add_writer()")

        def test_create_datagram_endpoint(self):
            raise unittest.SkipTest(
                "IocpEventLoop does not have create_datagram_endpoint()")

        def test_remove_fds_after_closing(self):
            raise unittest.SkipTest("IocpEventLoop does not have add_reader()")
else:
    from asyncio import selectors

    class UnixEventLoopTestsMixin(EventLoopTestsMixin):
        def setUp(self):
            super().setUp()
            watcher = asyncio.SafeChildWatcher()
            watcher.attach_loop(self.loop)
            asyncio.set_child_watcher(watcher)

        def tearDown(self):
            asyncio.set_child_watcher(None)
            super().tearDown()

    if hasattr(selectors, 'KqueueSelector'):
        class KqueueEventLoopTests(UnixEventLoopTestsMixin,
                                   SubprocessTestsMixin,
                                   unittest.TestCase):

            def create_event_loop(self):
                return asyncio.SelectorEventLoop(
                    selectors.KqueueSelector())

            # kqueue doesn't support character devices (PTY) on Mac OS X older
            # than 10.9 (Maverick)
            @support.requires_mac_ver(10, 9)
            # Issue #20667: KqueueEventLoopTests.test_read_pty_output()
            # hangs on OpenBSD 5.5
            @unittest.skipIf(sys.platform.startswith('openbsd'),
                             'test hangs on OpenBSD')
            def test_read_pty_output(self):
                super().test_read_pty_output()

            # kqueue doesn't support character devices (PTY) on Mac OS X older
            # than 10.9 (Maverick)
            @support.requires_mac_ver(10, 9)
            def test_write_pty(self):
                super().test_write_pty()

    if hasattr(selectors, 'EpollSelector'):
        class EPollEventLoopTests(UnixEventLoopTestsMixin,
                                  SubprocessTestsMixin,
                                  unittest.TestCase):

            def create_event_loop(self):
                return asyncio.SelectorEventLoop(selectors.EpollSelector())

    if hasattr(selectors, 'PollSelector'):
        class PollEventLoopTests(UnixEventLoopTestsMixin,
                                 SubprocessTestsMixin,
                                 unittest.TestCase):

            def create_event_loop(self):
                return asyncio.SelectorEventLoop(selectors.PollSelector())

    # Should always exist.
    class SelectEventLoopTests(UnixEventLoopTestsMixin,
                               SubprocessTestsMixin,
                               unittest.TestCase):

        def create_event_loop(self):
            return asyncio.SelectorEventLoop(selectors.SelectSelector())


class HandleTests(unittest.TestCase):

    def test_handle(self):
        def callback(*args):
            return args

        args = ()
        h = asyncio.Handle(callback, args, mock.Mock())
        self.assertIs(h._callback, callback)
        self.assertIs(h._args, args)
        self.assertFalse(h._cancelled)

        r = repr(h)
        self.assertTrue(r.startswith(
            'Handle('
            '<function HandleTests.test_handle.<locals>.callback'))
        self.assertTrue(r.endswith('())'))

        h.cancel()
        self.assertTrue(h._cancelled)

        r = repr(h)
        self.assertTrue(r.startswith(
            'Handle('
            '<function HandleTests.test_handle.<locals>.callback'))
        self.assertTrue(r.endswith('())<cancelled>'), r)

    def test_handle_from_handle(self):
        def callback(*args):
            return args
        m_loop = object()
        h1 = asyncio.Handle(callback, (), loop=m_loop)
        self.assertRaises(
            AssertionError, asyncio.Handle, h1, (), m_loop)

    def test_callback_with_exception(self):
        def callback():
            raise ValueError()

        m_loop = mock.Mock()
        m_loop.call_exception_handler = mock.Mock()

        h = asyncio.Handle(callback, (), m_loop)
        h._run()

        m_loop.call_exception_handler.assert_called_with({
            'message': test_utils.MockPattern('Exception in callback.*'),
            'exception': mock.ANY,
            'handle': h
        })

    def test_handle_weakref(self):
        wd = weakref.WeakValueDictionary()
        h = asyncio.Handle(lambda: None, (), object())
        wd['h'] = h  # Would fail without __weakref__ slot.


class TimerTests(unittest.TestCase):

    def test_hash(self):
        when = time.monotonic()
        h = asyncio.TimerHandle(when, lambda: False, (),
                                mock.Mock())
        self.assertEqual(hash(h), hash(when))

    def test_timer(self):
        def callback(*args):
            return args

        args = ()
        when = time.monotonic()
        h = asyncio.TimerHandle(when, callback, args, mock.Mock())
        self.assertIs(h._callback, callback)
        self.assertIs(h._args, args)
        self.assertFalse(h._cancelled)

        r = repr(h)
        self.assertTrue(r.endswith('())'))

        h.cancel()
        self.assertTrue(h._cancelled)

        r = repr(h)
        self.assertTrue(r.endswith('())<cancelled>'), r)

        self.assertRaises(AssertionError,
                          asyncio.TimerHandle, None, callback, args,
                          mock.Mock())

    def test_timer_comparison(self):
        loop = mock.Mock()

        def callback(*args):
            return args

        when = time.monotonic()

        h1 = asyncio.TimerHandle(when, callback, (), loop)
        h2 = asyncio.TimerHandle(when, callback, (), loop)
        # TODO: Use assertLess etc.
        self.assertFalse(h1 < h2)
        self.assertFalse(h2 < h1)
        self.assertTrue(h1 <= h2)
        self.assertTrue(h2 <= h1)
        self.assertFalse(h1 > h2)
        self.assertFalse(h2 > h1)
        self.assertTrue(h1 >= h2)
        self.assertTrue(h2 >= h1)
        self.assertTrue(h1 == h2)
        self.assertFalse(h1 != h2)

        h2.cancel()
        self.assertFalse(h1 == h2)

        h1 = asyncio.TimerHandle(when, callback, (), loop)
        h2 = asyncio.TimerHandle(when + 10.0, callback, (), loop)
        self.assertTrue(h1 < h2)
        self.assertFalse(h2 < h1)
        self.assertTrue(h1 <= h2)
        self.assertFalse(h2 <= h1)
        self.assertFalse(h1 > h2)
        self.assertTrue(h2 > h1)
        self.assertFalse(h1 >= h2)
        self.assertTrue(h2 >= h1)
        self.assertFalse(h1 == h2)
        self.assertTrue(h1 != h2)

        h3 = asyncio.Handle(callback, (), loop)
        self.assertIs(NotImplemented, h1.__eq__(h3))
        self.assertIs(NotImplemented, h1.__ne__(h3))


class AbstractEventLoopTests(unittest.TestCase):

    def test_not_implemented(self):
        f = mock.Mock()
        loop = asyncio.AbstractEventLoop()
        self.assertRaises(
            NotImplementedError, loop.run_forever)
        self.assertRaises(
            NotImplementedError, loop.run_until_complete, None)
        self.assertRaises(
            NotImplementedError, loop.stop)
        self.assertRaises(
            NotImplementedError, loop.is_running)
        self.assertRaises(
            NotImplementedError, loop.close)
        self.assertRaises(
            NotImplementedError, loop.call_later, None, None)
        self.assertRaises(
            NotImplementedError, loop.call_at, f, f)
        self.assertRaises(
            NotImplementedError, loop.call_soon, None)
        self.assertRaises(
            NotImplementedError, loop.time)
        self.assertRaises(
            NotImplementedError, loop.call_soon_threadsafe, None)
        self.assertRaises(
            NotImplementedError, loop.run_in_executor, f, f)
        self.assertRaises(
            NotImplementedError, loop.set_default_executor, f)
        self.assertRaises(
            NotImplementedError, loop.getaddrinfo, 'localhost', 8080)
        self.assertRaises(
            NotImplementedError, loop.getnameinfo, ('localhost', 8080))
        self.assertRaises(
            NotImplementedError, loop.create_connection, f)
        self.assertRaises(
            NotImplementedError, loop.create_server, f)
        self.assertRaises(
            NotImplementedError, loop.create_datagram_endpoint, f)
        self.assertRaises(
            NotImplementedError, loop.add_reader, 1, f)
        self.assertRaises(
            NotImplementedError, loop.remove_reader, 1)
        self.assertRaises(
            NotImplementedError, loop.add_writer, 1, f)
        self.assertRaises(
            NotImplementedError, loop.remove_writer, 1)
        self.assertRaises(
            NotImplementedError, loop.sock_recv, f, 10)
        self.assertRaises(
            NotImplementedError, loop.sock_sendall, f, 10)
        self.assertRaises(
            NotImplementedError, loop.sock_connect, f, f)
        self.assertRaises(
            NotImplementedError, loop.sock_accept, f)
        self.assertRaises(
            NotImplementedError, loop.add_signal_handler, 1, f)
        self.assertRaises(
            NotImplementedError, loop.remove_signal_handler, 1)
        self.assertRaises(
            NotImplementedError, loop.remove_signal_handler, 1)
        self.assertRaises(
            NotImplementedError, loop.connect_read_pipe, f,
            mock.sentinel.pipe)
        self.assertRaises(
            NotImplementedError, loop.connect_write_pipe, f,
            mock.sentinel.pipe)
        self.assertRaises(
            NotImplementedError, loop.subprocess_shell, f,
            mock.sentinel)
        self.assertRaises(
            NotImplementedError, loop.subprocess_exec, f)


class ProtocolsAbsTests(unittest.TestCase):

    def test_empty(self):
        f = mock.Mock()
        p = asyncio.Protocol()
        self.assertIsNone(p.connection_made(f))
        self.assertIsNone(p.connection_lost(f))
        self.assertIsNone(p.data_received(f))
        self.assertIsNone(p.eof_received())

        dp = asyncio.DatagramProtocol()
        self.assertIsNone(dp.connection_made(f))
        self.assertIsNone(dp.connection_lost(f))
        self.assertIsNone(dp.error_received(f))
        self.assertIsNone(dp.datagram_received(f, f))

        sp = asyncio.SubprocessProtocol()
        self.assertIsNone(sp.connection_made(f))
        self.assertIsNone(sp.connection_lost(f))
        self.assertIsNone(sp.pipe_data_received(1, f))
        self.assertIsNone(sp.pipe_connection_lost(1, f))
        self.assertIsNone(sp.process_exited())


class PolicyTests(unittest.TestCase):

    def test_event_loop_policy(self):
        policy = asyncio.AbstractEventLoopPolicy()
        self.assertRaises(NotImplementedError, policy.get_event_loop)
        self.assertRaises(NotImplementedError, policy.set_event_loop, object())
        self.assertRaises(NotImplementedError, policy.new_event_loop)
        self.assertRaises(NotImplementedError, policy.get_child_watcher)
        self.assertRaises(NotImplementedError, policy.set_child_watcher,
                          object())

    def test_get_event_loop(self):
        policy = asyncio.DefaultEventLoopPolicy()
        self.assertIsNone(policy._local._loop)

        loop = policy.get_event_loop()
        self.assertIsInstance(loop, asyncio.AbstractEventLoop)

        self.assertIs(policy._local._loop, loop)
        self.assertIs(loop, policy.get_event_loop())
        loop.close()

    def test_get_event_loop_calls_set_event_loop(self):
        policy = asyncio.DefaultEventLoopPolicy()

        with mock.patch.object(
                policy, "set_event_loop",
                wraps=policy.set_event_loop) as m_set_event_loop:

            loop = policy.get_event_loop()

            # policy._local._loop must be set through .set_event_loop()
            # (the unix DefaultEventLoopPolicy needs this call to attach
            # the child watcher correctly)
            m_set_event_loop.assert_called_with(loop)

        loop.close()

    def test_get_event_loop_after_set_none(self):
        policy = asyncio.DefaultEventLoopPolicy()
        policy.set_event_loop(None)
        self.assertRaises(AssertionError, policy.get_event_loop)

    @mock.patch('asyncio.events.threading.current_thread')
    def test_get_event_loop_thread(self, m_current_thread):

        def f():
            policy = asyncio.DefaultEventLoopPolicy()
            self.assertRaises(AssertionError, policy.get_event_loop)

        th = threading.Thread(target=f)
        th.start()
        th.join()

    def test_new_event_loop(self):
        policy = asyncio.DefaultEventLoopPolicy()

        loop = policy.new_event_loop()
        self.assertIsInstance(loop, asyncio.AbstractEventLoop)
        loop.close()

    def test_set_event_loop(self):
        policy = asyncio.DefaultEventLoopPolicy()
        old_loop = policy.get_event_loop()

        self.assertRaises(AssertionError, policy.set_event_loop, object())

        loop = policy.new_event_loop()
        policy.set_event_loop(loop)
        self.assertIs(loop, policy.get_event_loop())
        self.assertIsNot(old_loop, policy.get_event_loop())
        loop.close()
        old_loop.close()

    def test_get_event_loop_policy(self):
        policy = asyncio.get_event_loop_policy()
        self.assertIsInstance(policy, asyncio.AbstractEventLoopPolicy)
        self.assertIs(policy, asyncio.get_event_loop_policy())

    def test_set_event_loop_policy(self):
        self.assertRaises(
            AssertionError, asyncio.set_event_loop_policy, object())

        old_policy = asyncio.get_event_loop_policy()

        policy = asyncio.DefaultEventLoopPolicy()
        asyncio.set_event_loop_policy(policy)
        self.assertIs(policy, asyncio.get_event_loop_policy())
        self.assertIsNot(policy, old_policy)


if __name__ == '__main__':
    unittest.main()
