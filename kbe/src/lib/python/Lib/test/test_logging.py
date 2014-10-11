# Copyright 2001-2013 by Vinay Sajip. All Rights Reserved.
#
# Permission to use, copy, modify, and distribute this software and its
# documentation for any purpose and without fee is hereby granted,
# provided that the above copyright notice appear in all copies and that
# both that copyright notice and this permission notice appear in
# supporting documentation, and that the name of Vinay Sajip
# not be used in advertising or publicity pertaining to distribution
# of the software without specific, written prior permission.
# VINAY SAJIP DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
# ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL
# VINAY SAJIP BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
# ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
# IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
# OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

"""Test harness for the logging module. Run all tests.

Copyright (C) 2001-2013 Vinay Sajip. All Rights Reserved.
"""

import logging
import logging.handlers
import logging.config

import codecs
import configparser
import datetime
import pickle
import io
import gc
import json
import os
import queue
import random
import re
import select
import socket
import struct
import sys
import tempfile
from test.script_helper import assert_python_ok
from test.support import (captured_stdout, run_with_locale, run_unittest,
                          patch, requires_zlib, TestHandler, Matcher)
import textwrap
import time
import unittest
import warnings
import weakref
try:
    import threading
    # The following imports are needed only for tests which
    # require threading
    import asynchat
    import asyncore
    import errno
    from http.server import HTTPServer, BaseHTTPRequestHandler
    import smtpd
    from urllib.parse import urlparse, parse_qs
    from socketserver import (ThreadingUDPServer, DatagramRequestHandler,
                              ThreadingTCPServer, StreamRequestHandler,
                              ThreadingUnixStreamServer,
                              ThreadingUnixDatagramServer)
except ImportError:
    threading = None
try:
    import win32evtlog
except ImportError:
    win32evtlog = None
try:
    import win32evtlogutil
except ImportError:
    win32evtlogutil = None
    win32evtlog = None
try:
    import zlib
except ImportError:
    pass

class BaseTest(unittest.TestCase):

    """Base class for logging tests."""

    log_format = "%(name)s -> %(levelname)s: %(message)s"
    expected_log_pat = r"^([\w.]+) -> (\w+): (\d+)$"
    message_num = 0

    def setUp(self):
        """Setup the default logging stream to an internal StringIO instance,
        so that we can examine log output as we want."""
        logger_dict = logging.getLogger().manager.loggerDict
        logging._acquireLock()
        try:
            self.saved_handlers = logging._handlers.copy()
            self.saved_handler_list = logging._handlerList[:]
            self.saved_loggers = saved_loggers = logger_dict.copy()
            self.saved_name_to_level = logging._nameToLevel.copy()
            self.saved_level_to_name = logging._levelToName.copy()
            self.logger_states = logger_states = {}
            for name in saved_loggers:
                logger_states[name] = getattr(saved_loggers[name],
                                              'disabled', None)
        finally:
            logging._releaseLock()

        # Set two unused loggers
        self.logger1 = logging.getLogger("\xab\xd7\xbb")
        self.logger2 = logging.getLogger("\u013f\u00d6\u0047")

        self.root_logger = logging.getLogger("")
        self.original_logging_level = self.root_logger.getEffectiveLevel()

        self.stream = io.StringIO()
        self.root_logger.setLevel(logging.DEBUG)
        self.root_hdlr = logging.StreamHandler(self.stream)
        self.root_formatter = logging.Formatter(self.log_format)
        self.root_hdlr.setFormatter(self.root_formatter)
        if self.logger1.hasHandlers():
            hlist = self.logger1.handlers + self.root_logger.handlers
            raise AssertionError('Unexpected handlers: %s' % hlist)
        if self.logger2.hasHandlers():
            hlist = self.logger2.handlers + self.root_logger.handlers
            raise AssertionError('Unexpected handlers: %s' % hlist)
        self.root_logger.addHandler(self.root_hdlr)
        self.assertTrue(self.logger1.hasHandlers())
        self.assertTrue(self.logger2.hasHandlers())

    def tearDown(self):
        """Remove our logging stream, and restore the original logging
        level."""
        self.stream.close()
        self.root_logger.removeHandler(self.root_hdlr)
        while self.root_logger.handlers:
            h = self.root_logger.handlers[0]
            self.root_logger.removeHandler(h)
            h.close()
        self.root_logger.setLevel(self.original_logging_level)
        logging._acquireLock()
        try:
            logging._levelToName.clear()
            logging._levelToName.update(self.saved_level_to_name)
            logging._nameToLevel.clear()
            logging._nameToLevel.update(self.saved_name_to_level)
            logging._handlers.clear()
            logging._handlers.update(self.saved_handlers)
            logging._handlerList[:] = self.saved_handler_list
            loggerDict = logging.getLogger().manager.loggerDict
            loggerDict.clear()
            loggerDict.update(self.saved_loggers)
            logger_states = self.logger_states
            for name in self.logger_states:
                if logger_states[name] is not None:
                    self.saved_loggers[name].disabled = logger_states[name]
        finally:
            logging._releaseLock()

    def assert_log_lines(self, expected_values, stream=None, pat=None):
        """Match the collected log lines against the regular expression
        self.expected_log_pat, and compare the extracted group values to
        the expected_values list of tuples."""
        stream = stream or self.stream
        pat = re.compile(pat or self.expected_log_pat)
        actual_lines = stream.getvalue().splitlines()
        self.assertEqual(len(actual_lines), len(expected_values))
        for actual, expected in zip(actual_lines, expected_values):
            match = pat.search(actual)
            if not match:
                self.fail("Log line does not match expected pattern:\n" +
                            actual)
            self.assertEqual(tuple(match.groups()), expected)
        s = stream.read()
        if s:
            self.fail("Remaining output at end of log stream:\n" + s)

    def next_message(self):
        """Generate a message consisting solely of an auto-incrementing
        integer."""
        self.message_num += 1
        return "%d" % self.message_num


class BuiltinLevelsTest(BaseTest):
    """Test builtin levels and their inheritance."""

    def test_flat(self):
        #Logging levels in a flat logger namespace.
        m = self.next_message

        ERR = logging.getLogger("ERR")
        ERR.setLevel(logging.ERROR)
        INF = logging.LoggerAdapter(logging.getLogger("INF"), {})
        INF.setLevel(logging.INFO)
        DEB = logging.getLogger("DEB")
        DEB.setLevel(logging.DEBUG)

        # These should log.
        ERR.log(logging.CRITICAL, m())
        ERR.error(m())

        INF.log(logging.CRITICAL, m())
        INF.error(m())
        INF.warning(m())
        INF.info(m())

        DEB.log(logging.CRITICAL, m())
        DEB.error(m())
        DEB.warning(m())
        DEB.info(m())
        DEB.debug(m())

        # These should not log.
        ERR.warning(m())
        ERR.info(m())
        ERR.debug(m())

        INF.debug(m())

        self.assert_log_lines([
            ('ERR', 'CRITICAL', '1'),
            ('ERR', 'ERROR', '2'),
            ('INF', 'CRITICAL', '3'),
            ('INF', 'ERROR', '4'),
            ('INF', 'WARNING', '5'),
            ('INF', 'INFO', '6'),
            ('DEB', 'CRITICAL', '7'),
            ('DEB', 'ERROR', '8'),
            ('DEB', 'WARNING', '9'),
            ('DEB', 'INFO', '10'),
            ('DEB', 'DEBUG', '11'),
        ])

    def test_nested_explicit(self):
        # Logging levels in a nested namespace, all explicitly set.
        m = self.next_message

        INF = logging.getLogger("INF")
        INF.setLevel(logging.INFO)
        INF_ERR  = logging.getLogger("INF.ERR")
        INF_ERR.setLevel(logging.ERROR)

        # These should log.
        INF_ERR.log(logging.CRITICAL, m())
        INF_ERR.error(m())

        # These should not log.
        INF_ERR.warning(m())
        INF_ERR.info(m())
        INF_ERR.debug(m())

        self.assert_log_lines([
            ('INF.ERR', 'CRITICAL', '1'),
            ('INF.ERR', 'ERROR', '2'),
        ])

    def test_nested_inherited(self):
        #Logging levels in a nested namespace, inherited from parent loggers.
        m = self.next_message

        INF = logging.getLogger("INF")
        INF.setLevel(logging.INFO)
        INF_ERR  = logging.getLogger("INF.ERR")
        INF_ERR.setLevel(logging.ERROR)
        INF_UNDEF = logging.getLogger("INF.UNDEF")
        INF_ERR_UNDEF = logging.getLogger("INF.ERR.UNDEF")
        UNDEF = logging.getLogger("UNDEF")

        # These should log.
        INF_UNDEF.log(logging.CRITICAL, m())
        INF_UNDEF.error(m())
        INF_UNDEF.warning(m())
        INF_UNDEF.info(m())
        INF_ERR_UNDEF.log(logging.CRITICAL, m())
        INF_ERR_UNDEF.error(m())

        # These should not log.
        INF_UNDEF.debug(m())
        INF_ERR_UNDEF.warning(m())
        INF_ERR_UNDEF.info(m())
        INF_ERR_UNDEF.debug(m())

        self.assert_log_lines([
            ('INF.UNDEF', 'CRITICAL', '1'),
            ('INF.UNDEF', 'ERROR', '2'),
            ('INF.UNDEF', 'WARNING', '3'),
            ('INF.UNDEF', 'INFO', '4'),
            ('INF.ERR.UNDEF', 'CRITICAL', '5'),
            ('INF.ERR.UNDEF', 'ERROR', '6'),
        ])

    def test_nested_with_virtual_parent(self):
        # Logging levels when some parent does not exist yet.
        m = self.next_message

        INF = logging.getLogger("INF")
        GRANDCHILD = logging.getLogger("INF.BADPARENT.UNDEF")
        CHILD = logging.getLogger("INF.BADPARENT")
        INF.setLevel(logging.INFO)

        # These should log.
        GRANDCHILD.log(logging.FATAL, m())
        GRANDCHILD.info(m())
        CHILD.log(logging.FATAL, m())
        CHILD.info(m())

        # These should not log.
        GRANDCHILD.debug(m())
        CHILD.debug(m())

        self.assert_log_lines([
            ('INF.BADPARENT.UNDEF', 'CRITICAL', '1'),
            ('INF.BADPARENT.UNDEF', 'INFO', '2'),
            ('INF.BADPARENT', 'CRITICAL', '3'),
            ('INF.BADPARENT', 'INFO', '4'),
        ])


class BasicFilterTest(BaseTest):

    """Test the bundled Filter class."""

    def test_filter(self):
        # Only messages satisfying the specified criteria pass through the
        #  filter.
        filter_ = logging.Filter("spam.eggs")
        handler = self.root_logger.handlers[0]
        try:
            handler.addFilter(filter_)
            spam = logging.getLogger("spam")
            spam_eggs = logging.getLogger("spam.eggs")
            spam_eggs_fish = logging.getLogger("spam.eggs.fish")
            spam_bakedbeans = logging.getLogger("spam.bakedbeans")

            spam.info(self.next_message())
            spam_eggs.info(self.next_message())  # Good.
            spam_eggs_fish.info(self.next_message())  # Good.
            spam_bakedbeans.info(self.next_message())

            self.assert_log_lines([
                ('spam.eggs', 'INFO', '2'),
                ('spam.eggs.fish', 'INFO', '3'),
            ])
        finally:
            handler.removeFilter(filter_)

    def test_callable_filter(self):
        # Only messages satisfying the specified criteria pass through the
        #  filter.

        def filterfunc(record):
            parts = record.name.split('.')
            prefix = '.'.join(parts[:2])
            return prefix == 'spam.eggs'

        handler = self.root_logger.handlers[0]
        try:
            handler.addFilter(filterfunc)
            spam = logging.getLogger("spam")
            spam_eggs = logging.getLogger("spam.eggs")
            spam_eggs_fish = logging.getLogger("spam.eggs.fish")
            spam_bakedbeans = logging.getLogger("spam.bakedbeans")

            spam.info(self.next_message())
            spam_eggs.info(self.next_message())  # Good.
            spam_eggs_fish.info(self.next_message())  # Good.
            spam_bakedbeans.info(self.next_message())

            self.assert_log_lines([
                ('spam.eggs', 'INFO', '2'),
                ('spam.eggs.fish', 'INFO', '3'),
            ])
        finally:
            handler.removeFilter(filterfunc)

    def test_empty_filter(self):
        f = logging.Filter()
        r = logging.makeLogRecord({'name': 'spam.eggs'})
        self.assertTrue(f.filter(r))

#
#   First, we define our levels. There can be as many as you want - the only
#     limitations are that they should be integers, the lowest should be > 0 and
#   larger values mean less information being logged. If you need specific
#   level values which do not fit into these limitations, you can use a
#   mapping dictionary to convert between your application levels and the
#   logging system.
#
SILENT      = 120
TACITURN    = 119
TERSE       = 118
EFFUSIVE    = 117
SOCIABLE    = 116
VERBOSE     = 115
TALKATIVE   = 114
GARRULOUS   = 113
CHATTERBOX  = 112
BORING      = 111

LEVEL_RANGE = range(BORING, SILENT + 1)

#
#   Next, we define names for our levels. You don't need to do this - in which
#   case the system will use "Level n" to denote the text for the level.
#
my_logging_levels = {
    SILENT      : 'Silent',
    TACITURN    : 'Taciturn',
    TERSE       : 'Terse',
    EFFUSIVE    : 'Effusive',
    SOCIABLE    : 'Sociable',
    VERBOSE     : 'Verbose',
    TALKATIVE   : 'Talkative',
    GARRULOUS   : 'Garrulous',
    CHATTERBOX  : 'Chatterbox',
    BORING      : 'Boring',
}

class GarrulousFilter(logging.Filter):

    """A filter which blocks garrulous messages."""

    def filter(self, record):
        return record.levelno != GARRULOUS

class VerySpecificFilter(logging.Filter):

    """A filter which blocks sociable and taciturn messages."""

    def filter(self, record):
        return record.levelno not in [SOCIABLE, TACITURN]


class CustomLevelsAndFiltersTest(BaseTest):

    """Test various filtering possibilities with custom logging levels."""

    # Skip the logger name group.
    expected_log_pat = r"^[\w.]+ -> (\w+): (\d+)$"

    def setUp(self):
        BaseTest.setUp(self)
        for k, v in my_logging_levels.items():
            logging.addLevelName(k, v)

    def log_at_all_levels(self, logger):
        for lvl in LEVEL_RANGE:
            logger.log(lvl, self.next_message())

    def test_logger_filter(self):
        # Filter at logger level.
        self.root_logger.setLevel(VERBOSE)
        # Levels >= 'Verbose' are good.
        self.log_at_all_levels(self.root_logger)
        self.assert_log_lines([
            ('Verbose', '5'),
            ('Sociable', '6'),
            ('Effusive', '7'),
            ('Terse', '8'),
            ('Taciturn', '9'),
            ('Silent', '10'),
        ])

    def test_handler_filter(self):
        # Filter at handler level.
        self.root_logger.handlers[0].setLevel(SOCIABLE)
        try:
            # Levels >= 'Sociable' are good.
            self.log_at_all_levels(self.root_logger)
            self.assert_log_lines([
                ('Sociable', '6'),
                ('Effusive', '7'),
                ('Terse', '8'),
                ('Taciturn', '9'),
                ('Silent', '10'),
            ])
        finally:
            self.root_logger.handlers[0].setLevel(logging.NOTSET)

    def test_specific_filters(self):
        # Set a specific filter object on the handler, and then add another
        #  filter object on the logger itself.
        handler = self.root_logger.handlers[0]
        specific_filter = None
        garr = GarrulousFilter()
        handler.addFilter(garr)
        try:
            self.log_at_all_levels(self.root_logger)
            first_lines = [
                # Notice how 'Garrulous' is missing
                ('Boring', '1'),
                ('Chatterbox', '2'),
                ('Talkative', '4'),
                ('Verbose', '5'),
                ('Sociable', '6'),
                ('Effusive', '7'),
                ('Terse', '8'),
                ('Taciturn', '9'),
                ('Silent', '10'),
            ]
            self.assert_log_lines(first_lines)

            specific_filter = VerySpecificFilter()
            self.root_logger.addFilter(specific_filter)
            self.log_at_all_levels(self.root_logger)
            self.assert_log_lines(first_lines + [
                # Not only 'Garrulous' is still missing, but also 'Sociable'
                # and 'Taciturn'
                ('Boring', '11'),
                ('Chatterbox', '12'),
                ('Talkative', '14'),
                ('Verbose', '15'),
                ('Effusive', '17'),
                ('Terse', '18'),
                ('Silent', '20'),
        ])
        finally:
            if specific_filter:
                self.root_logger.removeFilter(specific_filter)
            handler.removeFilter(garr)


class HandlerTest(BaseTest):
    def test_name(self):
        h = logging.Handler()
        h.name = 'generic'
        self.assertEqual(h.name, 'generic')
        h.name = 'anothergeneric'
        self.assertEqual(h.name, 'anothergeneric')
        self.assertRaises(NotImplementedError, h.emit, None)

    def test_builtin_handlers(self):
        # We can't actually *use* too many handlers in the tests,
        # but we can try instantiating them with various options
        if sys.platform in ('linux', 'darwin'):
            for existing in (True, False):
                fd, fn = tempfile.mkstemp()
                os.close(fd)
                if not existing:
                    os.unlink(fn)
                h = logging.handlers.WatchedFileHandler(fn, delay=True)
                if existing:
                    dev, ino = h.dev, h.ino
                    self.assertEqual(dev, -1)
                    self.assertEqual(ino, -1)
                    r = logging.makeLogRecord({'msg': 'Test'})
                    h.handle(r)
                    # Now remove the file.
                    os.unlink(fn)
                    self.assertFalse(os.path.exists(fn))
                    # The next call should recreate the file.
                    h.handle(r)
                    self.assertTrue(os.path.exists(fn))
                else:
                    self.assertEqual(h.dev, -1)
                    self.assertEqual(h.ino, -1)
                h.close()
                if existing:
                    os.unlink(fn)
            if sys.platform == 'darwin':
                sockname = '/var/run/syslog'
            else:
                sockname = '/dev/log'
            try:
                h = logging.handlers.SysLogHandler(sockname)
                self.assertEqual(h.facility, h.LOG_USER)
                self.assertTrue(h.unixsocket)
                h.close()
            except OSError: # syslogd might not be available
                pass
        for method in ('GET', 'POST', 'PUT'):
            if method == 'PUT':
                self.assertRaises(ValueError, logging.handlers.HTTPHandler,
                                  'localhost', '/log', method)
            else:
                h = logging.handlers.HTTPHandler('localhost', '/log', method)
                h.close()
        h = logging.handlers.BufferingHandler(0)
        r = logging.makeLogRecord({})
        self.assertTrue(h.shouldFlush(r))
        h.close()
        h = logging.handlers.BufferingHandler(1)
        self.assertFalse(h.shouldFlush(r))
        h.close()

    @unittest.skipIf(os.name == 'nt', 'WatchedFileHandler not appropriate for Windows.')
    @unittest.skipUnless(threading, 'Threading required for this test.')
    def test_race(self):
        # Issue #14632 refers.
        def remove_loop(fname, tries):
            for _ in range(tries):
                try:
                    os.unlink(fname)
                    self.deletion_time = time.time()
                except OSError:
                    pass
                time.sleep(0.004 * random.randint(0, 4))

        del_count = 500
        log_count = 500

        self.handle_time = None
        self.deletion_time = None

        for delay in (False, True):
            fd, fn = tempfile.mkstemp('.log', 'test_logging-3-')
            os.close(fd)
            remover = threading.Thread(target=remove_loop, args=(fn, del_count))
            remover.daemon = True
            remover.start()
            h = logging.handlers.WatchedFileHandler(fn, delay=delay)
            f = logging.Formatter('%(asctime)s: %(levelname)s: %(message)s')
            h.setFormatter(f)
            try:
                for _ in range(log_count):
                    time.sleep(0.005)
                    r = logging.makeLogRecord({'msg': 'testing' })
                    try:
                        self.handle_time = time.time()
                        h.handle(r)
                    except Exception:
                        print('Deleted at %s, '
                              'opened at %s' % (self.deletion_time,
                                                self.handle_time))
                        raise
            finally:
                remover.join()
                h.close()
                if os.path.exists(fn):
                    os.unlink(fn)


class BadStream(object):
    def write(self, data):
        raise RuntimeError('deliberate mistake')

class TestStreamHandler(logging.StreamHandler):
    def handleError(self, record):
        self.error_record = record

class StreamHandlerTest(BaseTest):
    def test_error_handling(self):
        h = TestStreamHandler(BadStream())
        r = logging.makeLogRecord({})
        old_raise = logging.raiseExceptions
        old_stderr = sys.stderr
        try:
            h.handle(r)
            self.assertIs(h.error_record, r)
            h = logging.StreamHandler(BadStream())
            sys.stderr = sio = io.StringIO()
            h.handle(r)
            self.assertIn('\nRuntimeError: deliberate mistake\n',
                          sio.getvalue())
            logging.raiseExceptions = False
            sys.stderr = sio = io.StringIO()
            h.handle(r)
            self.assertEqual('', sio.getvalue())
        finally:
            logging.raiseExceptions = old_raise
            sys.stderr = old_stderr

# -- The following section could be moved into a server_helper.py module
# -- if it proves to be of wider utility than just test_logging

if threading:
    class TestSMTPServer(smtpd.SMTPServer):
        """
        This class implements a test SMTP server.

        :param addr: A (host, port) tuple which the server listens on.
                     You can specify a port value of zero: the server's
                     *port* attribute will hold the actual port number
                     used, which can be used in client connections.
        :param handler: A callable which will be called to process
                        incoming messages. The handler will be passed
                        the client address tuple, who the message is from,
                        a list of recipients and the message data.
        :param poll_interval: The interval, in seconds, used in the underlying
                              :func:`select` or :func:`poll` call by
                              :func:`asyncore.loop`.
        :param sockmap: A dictionary which will be used to hold
                        :class:`asyncore.dispatcher` instances used by
                        :func:`asyncore.loop`. This avoids changing the
                        :mod:`asyncore` module's global state.
        """

        def __init__(self, addr, handler, poll_interval, sockmap):
            smtpd.SMTPServer.__init__(self, addr, None, map=sockmap)
            self.port = self.socket.getsockname()[1]
            self._handler = handler
            self._thread = None
            self.poll_interval = poll_interval

        def process_message(self, peer, mailfrom, rcpttos, data):
            """
            Delegates to the handler passed in to the server's constructor.

            Typically, this will be a test case method.
            :param peer: The client (host, port) tuple.
            :param mailfrom: The address of the sender.
            :param rcpttos: The addresses of the recipients.
            :param data: The message.
            """
            self._handler(peer, mailfrom, rcpttos, data)

        def start(self):
            """
            Start the server running on a separate daemon thread.
            """
            self._thread = t = threading.Thread(target=self.serve_forever,
                                                args=(self.poll_interval,))
            t.setDaemon(True)
            t.start()

        def serve_forever(self, poll_interval):
            """
            Run the :mod:`asyncore` loop until normal termination
            conditions arise.
            :param poll_interval: The interval, in seconds, used in the underlying
                                  :func:`select` or :func:`poll` call by
                                  :func:`asyncore.loop`.
            """
            try:
                asyncore.loop(poll_interval, map=self._map)
            except OSError:
                # On FreeBSD 8, closing the server repeatably
                # raises this error. We swallow it if the
                # server has been closed.
                if self.connected or self.accepting:
                    raise

        def stop(self, timeout=None):
            """
            Stop the thread by closing the server instance.
            Wait for the server thread to terminate.

            :param timeout: How long to wait for the server thread
                            to terminate.
            """
            self.close()
            self._thread.join(timeout)
            self._thread = None

    class ControlMixin(object):
        """
        This mixin is used to start a server on a separate thread, and
        shut it down programmatically. Request handling is simplified - instead
        of needing to derive a suitable RequestHandler subclass, you just
        provide a callable which will be passed each received request to be
        processed.

        :param handler: A handler callable which will be called with a
                        single parameter - the request - in order to
                        process the request. This handler is called on the
                        server thread, effectively meaning that requests are
                        processed serially. While not quite Web scale ;-),
                        this should be fine for testing applications.
        :param poll_interval: The polling interval in seconds.
        """
        def __init__(self, handler, poll_interval):
            self._thread = None
            self.poll_interval = poll_interval
            self._handler = handler
            self.ready = threading.Event()

        def start(self):
            """
            Create a daemon thread to run the server, and start it.
            """
            self._thread = t = threading.Thread(target=self.serve_forever,
                                                args=(self.poll_interval,))
            t.setDaemon(True)
            t.start()

        def serve_forever(self, poll_interval):
            """
            Run the server. Set the ready flag before entering the
            service loop.
            """
            self.ready.set()
            super(ControlMixin, self).serve_forever(poll_interval)

        def stop(self, timeout=None):
            """
            Tell the server thread to stop, and wait for it to do so.

            :param timeout: How long to wait for the server thread
                            to terminate.
            """
            self.shutdown()
            if self._thread is not None:
                self._thread.join(timeout)
                self._thread = None
            self.server_close()
            self.ready.clear()

    class TestHTTPServer(ControlMixin, HTTPServer):
        """
        An HTTP server which is controllable using :class:`ControlMixin`.

        :param addr: A tuple with the IP address and port to listen on.
        :param handler: A handler callable which will be called with a
                        single parameter - the request - in order to
                        process the request.
        :param poll_interval: The polling interval in seconds.
        :param log: Pass ``True`` to enable log messages.
        """
        def __init__(self, addr, handler, poll_interval=0.5,
                     log=False, sslctx=None):
            class DelegatingHTTPRequestHandler(BaseHTTPRequestHandler):
                def __getattr__(self, name, default=None):
                    if name.startswith('do_'):
                        return self.process_request
                    raise AttributeError(name)

                def process_request(self):
                    self.server._handler(self)

                def log_message(self, format, *args):
                    if log:
                        super(DelegatingHTTPRequestHandler,
                              self).log_message(format, *args)
            HTTPServer.__init__(self, addr, DelegatingHTTPRequestHandler)
            ControlMixin.__init__(self, handler, poll_interval)
            self.sslctx = sslctx

        def get_request(self):
            try:
                sock, addr = self.socket.accept()
                if self.sslctx:
                    sock = self.sslctx.wrap_socket(sock, server_side=True)
            except OSError as e:
                # socket errors are silenced by the caller, print them here
                sys.stderr.write("Got an error:\n%s\n" % e)
                raise
            return sock, addr

    class TestTCPServer(ControlMixin, ThreadingTCPServer):
        """
        A TCP server which is controllable using :class:`ControlMixin`.

        :param addr: A tuple with the IP address and port to listen on.
        :param handler: A handler callable which will be called with a single
                        parameter - the request - in order to process the request.
        :param poll_interval: The polling interval in seconds.
        :bind_and_activate: If True (the default), binds the server and starts it
                            listening. If False, you need to call
                            :meth:`server_bind` and :meth:`server_activate` at
                            some later time before calling :meth:`start`, so that
                            the server will set up the socket and listen on it.
        """

        allow_reuse_address = True

        def __init__(self, addr, handler, poll_interval=0.5,
                     bind_and_activate=True):
            class DelegatingTCPRequestHandler(StreamRequestHandler):

                def handle(self):
                    self.server._handler(self)
            ThreadingTCPServer.__init__(self, addr, DelegatingTCPRequestHandler,
                                        bind_and_activate)
            ControlMixin.__init__(self, handler, poll_interval)

        def server_bind(self):
            super(TestTCPServer, self).server_bind()
            self.port = self.socket.getsockname()[1]

    class TestUnixStreamServer(TestTCPServer):
        address_family = socket.AF_UNIX

    class TestUDPServer(ControlMixin, ThreadingUDPServer):
        """
        A UDP server which is controllable using :class:`ControlMixin`.

        :param addr: A tuple with the IP address and port to listen on.
        :param handler: A handler callable which will be called with a
                        single parameter - the request - in order to
                        process the request.
        :param poll_interval: The polling interval for shutdown requests,
                              in seconds.
        :bind_and_activate: If True (the default), binds the server and
                            starts it listening. If False, you need to
                            call :meth:`server_bind` and
                            :meth:`server_activate` at some later time
                            before calling :meth:`start`, so that the server will
                            set up the socket and listen on it.
        """
        def __init__(self, addr, handler, poll_interval=0.5,
                     bind_and_activate=True):
            class DelegatingUDPRequestHandler(DatagramRequestHandler):

                def handle(self):
                    self.server._handler(self)

                def finish(self):
                    data = self.wfile.getvalue()
                    if data:
                        try:
                            super(DelegatingUDPRequestHandler, self).finish()
                        except OSError:
                            if not self.server._closed:
                                raise

            ThreadingUDPServer.__init__(self, addr,
                                        DelegatingUDPRequestHandler,
                                        bind_and_activate)
            ControlMixin.__init__(self, handler, poll_interval)
            self._closed = False

        def server_bind(self):
            super(TestUDPServer, self).server_bind()
            self.port = self.socket.getsockname()[1]

        def server_close(self):
            super(TestUDPServer, self).server_close()
            self._closed = True

    class TestUnixDatagramServer(TestUDPServer):
        address_family = socket.AF_UNIX

# - end of server_helper section

@unittest.skipUnless(threading, 'Threading required for this test.')
class SMTPHandlerTest(BaseTest):
    TIMEOUT = 8.0
    def test_basic(self):
        sockmap = {}
        server = TestSMTPServer(('localhost', 0), self.process_message, 0.001,
                                sockmap)
        server.start()
        addr = ('localhost', server.port)
        h = logging.handlers.SMTPHandler(addr, 'me', 'you', 'Log',
                                         timeout=self.TIMEOUT)
        self.assertEqual(h.toaddrs, ['you'])
        self.messages = []
        r = logging.makeLogRecord({'msg': 'Hello'})
        self.handled = threading.Event()
        h.handle(r)
        self.handled.wait(self.TIMEOUT)  # 14314: don't wait forever
        server.stop()
        self.assertTrue(self.handled.is_set())
        self.assertEqual(len(self.messages), 1)
        peer, mailfrom, rcpttos, data = self.messages[0]
        self.assertEqual(mailfrom, 'me')
        self.assertEqual(rcpttos, ['you'])
        self.assertIn('\nSubject: Log\n', data)
        self.assertTrue(data.endswith('\n\nHello'))
        h.close()

    def process_message(self, *args):
        self.messages.append(args)
        self.handled.set()

class MemoryHandlerTest(BaseTest):

    """Tests for the MemoryHandler."""

    # Do not bother with a logger name group.
    expected_log_pat = r"^[\w.]+ -> (\w+): (\d+)$"

    def setUp(self):
        BaseTest.setUp(self)
        self.mem_hdlr = logging.handlers.MemoryHandler(10, logging.WARNING,
                                                        self.root_hdlr)
        self.mem_logger = logging.getLogger('mem')
        self.mem_logger.propagate = 0
        self.mem_logger.addHandler(self.mem_hdlr)

    def tearDown(self):
        self.mem_hdlr.close()
        BaseTest.tearDown(self)

    def test_flush(self):
        # The memory handler flushes to its target handler based on specific
        #  criteria (message count and message level).
        self.mem_logger.debug(self.next_message())
        self.assert_log_lines([])
        self.mem_logger.info(self.next_message())
        self.assert_log_lines([])
        # This will flush because the level is >= logging.WARNING
        self.mem_logger.warning(self.next_message())
        lines = [
            ('DEBUG', '1'),
            ('INFO', '2'),
            ('WARNING', '3'),
        ]
        self.assert_log_lines(lines)
        for n in (4, 14):
            for i in range(9):
                self.mem_logger.debug(self.next_message())
            self.assert_log_lines(lines)
            # This will flush because it's the 10th message since the last
            #  flush.
            self.mem_logger.debug(self.next_message())
            lines = lines + [('DEBUG', str(i)) for i in range(n, n + 10)]
            self.assert_log_lines(lines)

        self.mem_logger.debug(self.next_message())
        self.assert_log_lines(lines)


class ExceptionFormatter(logging.Formatter):
    """A special exception formatter."""
    def formatException(self, ei):
        return "Got a [%s]" % ei[0].__name__


class ConfigFileTest(BaseTest):

    """Reading logging config from a .ini-style config file."""

    expected_log_pat = r"^(\w+) \+\+ (\w+)$"

    # config0 is a standard configuration.
    config0 = """
    [loggers]
    keys=root

    [handlers]
    keys=hand1

    [formatters]
    keys=form1

    [logger_root]
    level=WARNING
    handlers=hand1

    [handler_hand1]
    class=StreamHandler
    level=NOTSET
    formatter=form1
    args=(sys.stdout,)

    [formatter_form1]
    format=%(levelname)s ++ %(message)s
    datefmt=
    """

    # config1 adds a little to the standard configuration.
    config1 = """
    [loggers]
    keys=root,parser

    [handlers]
    keys=hand1

    [formatters]
    keys=form1

    [logger_root]
    level=WARNING
    handlers=

    [logger_parser]
    level=DEBUG
    handlers=hand1
    propagate=1
    qualname=compiler.parser

    [handler_hand1]
    class=StreamHandler
    level=NOTSET
    formatter=form1
    args=(sys.stdout,)

    [formatter_form1]
    format=%(levelname)s ++ %(message)s
    datefmt=
    """

    # config1a moves the handler to the root.
    config1a = """
    [loggers]
    keys=root,parser

    [handlers]
    keys=hand1

    [formatters]
    keys=form1

    [logger_root]
    level=WARNING
    handlers=hand1

    [logger_parser]
    level=DEBUG
    handlers=
    propagate=1
    qualname=compiler.parser

    [handler_hand1]
    class=StreamHandler
    level=NOTSET
    formatter=form1
    args=(sys.stdout,)

    [formatter_form1]
    format=%(levelname)s ++ %(message)s
    datefmt=
    """

    # config2 has a subtle configuration error that should be reported
    config2 = config1.replace("sys.stdout", "sys.stbout")

    # config3 has a less subtle configuration error
    config3 = config1.replace("formatter=form1", "formatter=misspelled_name")

    # config4 specifies a custom formatter class to be loaded
    config4 = """
    [loggers]
    keys=root

    [handlers]
    keys=hand1

    [formatters]
    keys=form1

    [logger_root]
    level=NOTSET
    handlers=hand1

    [handler_hand1]
    class=StreamHandler
    level=NOTSET
    formatter=form1
    args=(sys.stdout,)

    [formatter_form1]
    class=""" + __name__ + """.ExceptionFormatter
    format=%(levelname)s:%(name)s:%(message)s
    datefmt=
    """

    # config5 specifies a custom handler class to be loaded
    config5 = config1.replace('class=StreamHandler', 'class=logging.StreamHandler')

    # config6 uses ', ' delimiters in the handlers and formatters sections
    config6 = """
    [loggers]
    keys=root,parser

    [handlers]
    keys=hand1, hand2

    [formatters]
    keys=form1, form2

    [logger_root]
    level=WARNING
    handlers=

    [logger_parser]
    level=DEBUG
    handlers=hand1
    propagate=1
    qualname=compiler.parser

    [handler_hand1]
    class=StreamHandler
    level=NOTSET
    formatter=form1
    args=(sys.stdout,)

    [handler_hand2]
    class=StreamHandler
    level=NOTSET
    formatter=form1
    args=(sys.stderr,)

    [formatter_form1]
    format=%(levelname)s ++ %(message)s
    datefmt=

    [formatter_form2]
    format=%(message)s
    datefmt=
    """

    # config7 adds a compiler logger.
    config7 = """
    [loggers]
    keys=root,parser,compiler

    [handlers]
    keys=hand1

    [formatters]
    keys=form1

    [logger_root]
    level=WARNING
    handlers=hand1

    [logger_compiler]
    level=DEBUG
    handlers=
    propagate=1
    qualname=compiler

    [logger_parser]
    level=DEBUG
    handlers=
    propagate=1
    qualname=compiler.parser

    [handler_hand1]
    class=StreamHandler
    level=NOTSET
    formatter=form1
    args=(sys.stdout,)

    [formatter_form1]
    format=%(levelname)s ++ %(message)s
    datefmt=
    """

    disable_test = """
    [loggers]
    keys=root

    [handlers]
    keys=screen

    [formatters]
    keys=

    [logger_root]
    level=DEBUG
    handlers=screen

    [handler_screen]
    level=DEBUG
    class=StreamHandler
    args=(sys.stdout,)
    formatter=
    """

    def apply_config(self, conf, **kwargs):
        file = io.StringIO(textwrap.dedent(conf))
        logging.config.fileConfig(file, **kwargs)

    def test_config0_ok(self):
        # A simple config file which overrides the default settings.
        with captured_stdout() as output:
            self.apply_config(self.config0)
            logger = logging.getLogger()
            # Won't output anything
            logger.info(self.next_message())
            # Outputs a message
            logger.error(self.next_message())
            self.assert_log_lines([
                ('ERROR', '2'),
            ], stream=output)
            # Original logger output is empty.
            self.assert_log_lines([])

    def test_config0_using_cp_ok(self):
        # A simple config file which overrides the default settings.
        with captured_stdout() as output:
            file = io.StringIO(textwrap.dedent(self.config0))
            cp = configparser.ConfigParser()
            cp.read_file(file)
            logging.config.fileConfig(cp)
            logger = logging.getLogger()
            # Won't output anything
            logger.info(self.next_message())
            # Outputs a message
            logger.error(self.next_message())
            self.assert_log_lines([
                ('ERROR', '2'),
            ], stream=output)
            # Original logger output is empty.
            self.assert_log_lines([])

    def test_config1_ok(self, config=config1):
        # A config file defining a sub-parser as well.
        with captured_stdout() as output:
            self.apply_config(config)
            logger = logging.getLogger("compiler.parser")
            # Both will output a message
            logger.info(self.next_message())
            logger.error(self.next_message())
            self.assert_log_lines([
                ('INFO', '1'),
                ('ERROR', '2'),
            ], stream=output)
            # Original logger output is empty.
            self.assert_log_lines([])

    def test_config2_failure(self):
        # A simple config file which overrides the default settings.
        self.assertRaises(Exception, self.apply_config, self.config2)

    def test_config3_failure(self):
        # A simple config file which overrides the default settings.
        self.assertRaises(Exception, self.apply_config, self.config3)

    def test_config4_ok(self):
        # A config file specifying a custom formatter class.
        with captured_stdout() as output:
            self.apply_config(self.config4)
            logger = logging.getLogger()
            try:
                raise RuntimeError()
            except RuntimeError:
                logging.exception("just testing")
            sys.stdout.seek(0)
            self.assertEqual(output.getvalue(),
                "ERROR:root:just testing\nGot a [RuntimeError]\n")
            # Original logger output is empty
            self.assert_log_lines([])

    def test_config5_ok(self):
        self.test_config1_ok(config=self.config5)

    def test_config6_ok(self):
        self.test_config1_ok(config=self.config6)

    def test_config7_ok(self):
        with captured_stdout() as output:
            self.apply_config(self.config1a)
            logger = logging.getLogger("compiler.parser")
            # See issue #11424. compiler-hyphenated sorts
            # between compiler and compiler.xyz and this
            # was preventing compiler.xyz from being included
            # in the child loggers of compiler because of an
            # overzealous loop termination condition.
            hyphenated = logging.getLogger('compiler-hyphenated')
            # All will output a message
            logger.info(self.next_message())
            logger.error(self.next_message())
            hyphenated.critical(self.next_message())
            self.assert_log_lines([
                ('INFO', '1'),
                ('ERROR', '2'),
                ('CRITICAL', '3'),
            ], stream=output)
            # Original logger output is empty.
            self.assert_log_lines([])
        with captured_stdout() as output:
            self.apply_config(self.config7)
            logger = logging.getLogger("compiler.parser")
            self.assertFalse(logger.disabled)
            # Both will output a message
            logger.info(self.next_message())
            logger.error(self.next_message())
            logger = logging.getLogger("compiler.lexer")
            # Both will output a message
            logger.info(self.next_message())
            logger.error(self.next_message())
            # Will not appear
            hyphenated.critical(self.next_message())
            self.assert_log_lines([
                ('INFO', '4'),
                ('ERROR', '5'),
                ('INFO', '6'),
                ('ERROR', '7'),
            ], stream=output)
            # Original logger output is empty.
            self.assert_log_lines([])

    def test_logger_disabling(self):
        self.apply_config(self.disable_test)
        logger = logging.getLogger('some_pristine_logger')
        self.assertFalse(logger.disabled)
        self.apply_config(self.disable_test)
        self.assertTrue(logger.disabled)
        self.apply_config(self.disable_test, disable_existing_loggers=False)
        self.assertFalse(logger.disabled)


@unittest.skipUnless(threading, 'Threading required for this test.')
class SocketHandlerTest(BaseTest):

    """Test for SocketHandler objects."""

    if threading:
        server_class = TestTCPServer
        address = ('localhost', 0)

    def setUp(self):
        """Set up a TCP server to receive log messages, and a SocketHandler
        pointing to that server's address and port."""
        BaseTest.setUp(self)
        self.server = server = self.server_class(self.address,
                                                 self.handle_socket, 0.01)
        server.start()
        server.ready.wait()
        hcls = logging.handlers.SocketHandler
        if isinstance(server.server_address, tuple):
            self.sock_hdlr = hcls('localhost', server.port)
        else:
            self.sock_hdlr = hcls(server.server_address, None)
        self.log_output = ''
        self.root_logger.removeHandler(self.root_logger.handlers[0])
        self.root_logger.addHandler(self.sock_hdlr)
        self.handled = threading.Semaphore(0)

    def tearDown(self):
        """Shutdown the TCP server."""
        try:
            self.server.stop(2.0)
            self.root_logger.removeHandler(self.sock_hdlr)
            self.sock_hdlr.close()
        finally:
            BaseTest.tearDown(self)

    def handle_socket(self, request):
        conn = request.connection
        while True:
            chunk = conn.recv(4)
            if len(chunk) < 4:
                break
            slen = struct.unpack(">L", chunk)[0]
            chunk = conn.recv(slen)
            while len(chunk) < slen:
                chunk = chunk + conn.recv(slen - len(chunk))
            obj = pickle.loads(chunk)
            record = logging.makeLogRecord(obj)
            self.log_output += record.msg + '\n'
            self.handled.release()

    def test_output(self):
        # The log message sent to the SocketHandler is properly received.
        logger = logging.getLogger("tcp")
        logger.error("spam")
        self.handled.acquire()
        logger.debug("eggs")
        self.handled.acquire()
        self.assertEqual(self.log_output, "spam\neggs\n")

    def test_noserver(self):
        # Avoid timing-related failures due to SocketHandler's own hard-wired
        # one-second timeout on socket.create_connection() (issue #16264).
        self.sock_hdlr.retryStart = 2.5
        # Kill the server
        self.server.stop(2.0)
        # The logging call should try to connect, which should fail
        try:
            raise RuntimeError('Deliberate mistake')
        except RuntimeError:
            self.root_logger.exception('Never sent')
        self.root_logger.error('Never sent, either')
        now = time.time()
        self.assertGreater(self.sock_hdlr.retryTime, now)
        time.sleep(self.sock_hdlr.retryTime - now + 0.001)
        self.root_logger.error('Nor this')

def _get_temp_domain_socket():
    fd, fn = tempfile.mkstemp(prefix='test_logging_', suffix='.sock')
    os.close(fd)
    # just need a name - file can't be present, or we'll get an
    # 'address already in use' error.
    os.remove(fn)
    return fn

@unittest.skipUnless(threading, 'Threading required for this test.')
class UnixSocketHandlerTest(SocketHandlerTest):

    """Test for SocketHandler with unix sockets."""

    if threading:
        server_class = TestUnixStreamServer

    def setUp(self):
        # override the definition in the base class
        self.address = _get_temp_domain_socket()
        SocketHandlerTest.setUp(self)

    def tearDown(self):
        SocketHandlerTest.tearDown(self)
        os.remove(self.address)

@unittest.skipUnless(threading, 'Threading required for this test.')
class DatagramHandlerTest(BaseTest):

    """Test for DatagramHandler."""

    if threading:
        server_class = TestUDPServer
        address = ('localhost', 0)

    def setUp(self):
        """Set up a UDP server to receive log messages, and a DatagramHandler
        pointing to that server's address and port."""
        BaseTest.setUp(self)
        self.server = server = self.server_class(self.address,
                                                 self.handle_datagram, 0.01)
        server.start()
        server.ready.wait()
        hcls = logging.handlers.DatagramHandler
        if isinstance(server.server_address, tuple):
            self.sock_hdlr = hcls('localhost', server.port)
        else:
            self.sock_hdlr = hcls(server.server_address, None)
        self.log_output = ''
        self.root_logger.removeHandler(self.root_logger.handlers[0])
        self.root_logger.addHandler(self.sock_hdlr)
        self.handled = threading.Event()

    def tearDown(self):
        """Shutdown the UDP server."""
        try:
            self.server.stop(2.0)
            self.root_logger.removeHandler(self.sock_hdlr)
            self.sock_hdlr.close()
        finally:
            BaseTest.tearDown(self)

    def handle_datagram(self, request):
        slen = struct.pack('>L', 0) # length of prefix
        packet = request.packet[len(slen):]
        obj = pickle.loads(packet)
        record = logging.makeLogRecord(obj)
        self.log_output += record.msg + '\n'
        self.handled.set()

    def test_output(self):
        # The log message sent to the DatagramHandler is properly received.
        logger = logging.getLogger("udp")
        logger.error("spam")
        self.handled.wait()
        self.handled.clear()
        logger.error("eggs")
        self.handled.wait()
        self.assertEqual(self.log_output, "spam\neggs\n")


@unittest.skipUnless(threading, 'Threading required for this test.')
class UnixDatagramHandlerTest(DatagramHandlerTest):

    """Test for DatagramHandler using Unix sockets."""

    if threading:
        server_class = TestUnixDatagramServer

    def setUp(self):
        # override the definition in the base class
        self.address = _get_temp_domain_socket()
        DatagramHandlerTest.setUp(self)

    def tearDown(self):
        DatagramHandlerTest.tearDown(self)
        os.remove(self.address)

@unittest.skipUnless(threading, 'Threading required for this test.')
class SysLogHandlerTest(BaseTest):

    """Test for SysLogHandler using UDP."""

    if threading:
        server_class = TestUDPServer
        address = ('localhost', 0)

    def setUp(self):
        """Set up a UDP server to receive log messages, and a SysLogHandler
        pointing to that server's address and port."""
        BaseTest.setUp(self)
        self.server = server = self.server_class(self.address,
                                                 self.handle_datagram, 0.01)
        server.start()
        server.ready.wait()
        hcls = logging.handlers.SysLogHandler
        if isinstance(server.server_address, tuple):
            self.sl_hdlr = hcls(('localhost', server.port))
        else:
            self.sl_hdlr = hcls(server.server_address)
        self.log_output = ''
        self.root_logger.removeHandler(self.root_logger.handlers[0])
        self.root_logger.addHandler(self.sl_hdlr)
        self.handled = threading.Event()

    def tearDown(self):
        """Shutdown the UDP server."""
        try:
            self.server.stop(2.0)
            self.root_logger.removeHandler(self.sl_hdlr)
            self.sl_hdlr.close()
        finally:
            BaseTest.tearDown(self)

    def handle_datagram(self, request):
        self.log_output = request.packet
        self.handled.set()

    def test_output(self):
        # The log message sent to the SysLogHandler is properly received.
        logger = logging.getLogger("slh")
        logger.error("sp\xe4m")
        self.handled.wait()
        self.assertEqual(self.log_output, b'<11>sp\xc3\xa4m\x00')
        self.handled.clear()
        self.sl_hdlr.append_nul = False
        logger.error("sp\xe4m")
        self.handled.wait()
        self.assertEqual(self.log_output, b'<11>sp\xc3\xa4m')
        self.handled.clear()
        self.sl_hdlr.ident = "h\xe4m-"
        logger.error("sp\xe4m")
        self.handled.wait()
        self.assertEqual(self.log_output, b'<11>h\xc3\xa4m-sp\xc3\xa4m')


@unittest.skipUnless(threading, 'Threading required for this test.')
class UnixSysLogHandlerTest(SysLogHandlerTest):

    """Test for SysLogHandler with Unix sockets."""

    if threading:
        server_class = TestUnixDatagramServer

    def setUp(self):
        # override the definition in the base class
        self.address = _get_temp_domain_socket()
        SysLogHandlerTest.setUp(self)

    def tearDown(self):
        SysLogHandlerTest.tearDown(self)
        os.remove(self.address)

@unittest.skipUnless(threading, 'Threading required for this test.')
class HTTPHandlerTest(BaseTest):
    """Test for HTTPHandler."""

    PEMFILE = """-----BEGIN RSA PRIVATE KEY-----
MIICXQIBAAKBgQDGT4xS5r91rbLJQK2nUDenBhBG6qFk+bVOjuAGC/LSHlAoBnvG
zQG3agOG+e7c5z2XT8m2ktORLqG3E4mYmbxgyhDrzP6ei2Anc+pszmnxPoK3Puh5
aXV+XKt0bU0C1m2+ACmGGJ0t3P408art82nOxBw8ZHgIg9Dtp6xIUCyOqwIDAQAB
AoGBAJFTnFboaKh5eUrIzjmNrKsG44jEyy+vWvHN/FgSC4l103HxhmWiuL5Lv3f7
0tMp1tX7D6xvHwIG9VWvyKb/Cq9rJsDibmDVIOslnOWeQhG+XwJyitR0pq/KlJIB
5LjORcBw795oKWOAi6RcOb1ON59tysEFYhAGQO9k6VL621gRAkEA/Gb+YXULLpbs
piXN3q4zcHzeaVANo69tUZ6TjaQqMeTxE4tOYM0G0ZoSeHEdaP59AOZGKXXNGSQy
2z/MddcYGQJBAMkjLSYIpOLJY11ja8OwwswFG2hEzHe0cS9bzo++R/jc1bHA5R0Y
i6vA5iPi+wopPFvpytdBol7UuEBe5xZrxWMCQQCWxELRHiP2yWpEeLJ3gGDzoXMN
PydWjhRju7Bx3AzkTtf+D6lawz1+eGTuEss5i0JKBkMEwvwnN2s1ce+EuF4JAkBb
E96h1lAzkVW5OAfYOPY8RCPA90ZO/hoyg7PpSxR0ECuDrgERR8gXIeYUYfejBkEa
rab4CfRoVJKKM28Yq/xZAkBvuq670JRCwOgfUTdww7WpdOQBYPkzQccsKNCslQW8
/DyW6y06oQusSENUvynT6dr3LJxt/NgZPhZX2+k1eYDV
-----END RSA PRIVATE KEY-----
-----BEGIN CERTIFICATE-----
MIICGzCCAYSgAwIBAgIJAIq84a2Q/OvlMA0GCSqGSIb3DQEBBQUAMBQxEjAQBgNV
BAMTCWxvY2FsaG9zdDAeFw0xMTA1MjExMDIzMzNaFw03NTAzMjEwMzU1MTdaMBQx
EjAQBgNVBAMTCWxvY2FsaG9zdDCBnzANBgkqhkiG9w0BAQEFAAOBjQAwgYkCgYEA
xk+MUua/da2yyUCtp1A3pwYQRuqhZPm1To7gBgvy0h5QKAZ7xs0Bt2oDhvnu3Oc9
l0/JtpLTkS6htxOJmJm8YMoQ68z+notgJ3PqbM5p8T6Ctz7oeWl1flyrdG1NAtZt
vgAphhidLdz+NPGq7fNpzsQcPGR4CIPQ7aesSFAsjqsCAwEAAaN1MHMwHQYDVR0O
BBYEFLWaUPO6N7efGiuoS9i3DVYcUwn0MEQGA1UdIwQ9MDuAFLWaUPO6N7efGiuo
S9i3DVYcUwn0oRikFjAUMRIwEAYDVQQDEwlsb2NhbGhvc3SCCQCKvOGtkPzr5TAM
BgNVHRMEBTADAQH/MA0GCSqGSIb3DQEBBQUAA4GBAMK5whPjLNQK1Ivvk88oqJqq
4f889OwikGP0eUhOBhbFlsZs+jq5YZC2UzHz+evzKBlgAP1u4lP/cB85CnjvWqM+
1c/lywFHQ6HOdDeQ1L72tSYMrNOG4XNmLn0h7rx6GoTU7dcFRfseahBCq8mv0IDt
IRbTpvlHWPjsSvHz0ZOH
-----END CERTIFICATE-----"""

    def setUp(self):
        """Set up an HTTP server to receive log messages, and a HTTPHandler
        pointing to that server's address and port."""
        BaseTest.setUp(self)
        self.handled = threading.Event()

    def handle_request(self, request):
        self.command = request.command
        self.log_data = urlparse(request.path)
        if self.command == 'POST':
            try:
                rlen = int(request.headers['Content-Length'])
                self.post_data = request.rfile.read(rlen)
            except:
                self.post_data = None
        request.send_response(200)
        request.end_headers()
        self.handled.set()

    def test_output(self):
        # The log message sent to the HTTPHandler is properly received.
        logger = logging.getLogger("http")
        root_logger = self.root_logger
        root_logger.removeHandler(self.root_logger.handlers[0])
        for secure in (False, True):
            addr = ('localhost', 0)
            if secure:
                try:
                    import ssl
                    fd, fn = tempfile.mkstemp()
                    os.close(fd)
                    with open(fn, 'w') as f:
                        f.write(self.PEMFILE)
                    sslctx = ssl.SSLContext(ssl.PROTOCOL_SSLv23)
                    sslctx.load_cert_chain(fn)
                    os.unlink(fn)
                except ImportError:
                    sslctx = None
            else:
                sslctx = None
            self.server = server = TestHTTPServer(addr, self.handle_request,
                                                    0.01, sslctx=sslctx)
            server.start()
            server.ready.wait()
            host = 'localhost:%d' % server.server_port
            secure_client = secure and sslctx
            self.h_hdlr = logging.handlers.HTTPHandler(host, '/frob',
                                                       secure=secure_client)
            self.log_data = None
            root_logger.addHandler(self.h_hdlr)

            for method in ('GET', 'POST'):
                self.h_hdlr.method = method
                self.handled.clear()
                msg = "sp\xe4m"
                logger.error(msg)
                self.handled.wait()
                self.assertEqual(self.log_data.path, '/frob')
                self.assertEqual(self.command, method)
                if method == 'GET':
                    d = parse_qs(self.log_data.query)
                else:
                    d = parse_qs(self.post_data.decode('utf-8'))
                self.assertEqual(d['name'], ['http'])
                self.assertEqual(d['funcName'], ['test_output'])
                self.assertEqual(d['msg'], [msg])

            self.server.stop(2.0)
            self.root_logger.removeHandler(self.h_hdlr)
            self.h_hdlr.close()

class MemoryTest(BaseTest):

    """Test memory persistence of logger objects."""

    def setUp(self):
        """Create a dict to remember potentially destroyed objects."""
        BaseTest.setUp(self)
        self._survivors = {}

    def _watch_for_survival(self, *args):
        """Watch the given objects for survival, by creating weakrefs to
        them."""
        for obj in args:
            key = id(obj), repr(obj)
            self._survivors[key] = weakref.ref(obj)

    def _assertTruesurvival(self):
        """Assert that all objects watched for survival have survived."""
        # Trigger cycle breaking.
        gc.collect()
        dead = []
        for (id_, repr_), ref in self._survivors.items():
            if ref() is None:
                dead.append(repr_)
        if dead:
            self.fail("%d objects should have survived "
                "but have been destroyed: %s" % (len(dead), ", ".join(dead)))

    def test_persistent_loggers(self):
        # Logger objects are persistent and retain their configuration, even
        #  if visible references are destroyed.
        self.root_logger.setLevel(logging.INFO)
        foo = logging.getLogger("foo")
        self._watch_for_survival(foo)
        foo.setLevel(logging.DEBUG)
        self.root_logger.debug(self.next_message())
        foo.debug(self.next_message())
        self.assert_log_lines([
            ('foo', 'DEBUG', '2'),
        ])
        del foo
        # foo has survived.
        self._assertTruesurvival()
        # foo has retained its settings.
        bar = logging.getLogger("foo")
        bar.debug(self.next_message())
        self.assert_log_lines([
            ('foo', 'DEBUG', '2'),
            ('foo', 'DEBUG', '3'),
        ])


class EncodingTest(BaseTest):
    def test_encoding_plain_file(self):
        # In Python 2.x, a plain file object is treated as having no encoding.
        log = logging.getLogger("test")
        fd, fn = tempfile.mkstemp(".log", "test_logging-1-")
        os.close(fd)
        # the non-ascii data we write to the log.
        data = "foo\x80"
        try:
            handler = logging.FileHandler(fn, encoding="utf-8")
            log.addHandler(handler)
            try:
                # write non-ascii data to the log.
                log.warning(data)
            finally:
                log.removeHandler(handler)
                handler.close()
            # check we wrote exactly those bytes, ignoring trailing \n etc
            f = open(fn, encoding="utf-8")
            try:
                self.assertEqual(f.read().rstrip(), data)
            finally:
                f.close()
        finally:
            if os.path.isfile(fn):
                os.remove(fn)

    def test_encoding_cyrillic_unicode(self):
        log = logging.getLogger("test")
        #Get a message in Unicode: Do svidanya in Cyrillic (meaning goodbye)
        message = '\u0434\u043e \u0441\u0432\u0438\u0434\u0430\u043d\u0438\u044f'
        #Ensure it's written in a Cyrillic encoding
        writer_class = codecs.getwriter('cp1251')
        writer_class.encoding = 'cp1251'
        stream = io.BytesIO()
        writer = writer_class(stream, 'strict')
        handler = logging.StreamHandler(writer)
        log.addHandler(handler)
        try:
            log.warning(message)
        finally:
            log.removeHandler(handler)
            handler.close()
        # check we wrote exactly those bytes, ignoring trailing \n etc
        s = stream.getvalue()
        #Compare against what the data should be when encoded in CP-1251
        self.assertEqual(s, b'\xe4\xee \xf1\xe2\xe8\xe4\xe0\xed\xe8\xff\n')


class WarningsTest(BaseTest):

    def test_warnings(self):
        with warnings.catch_warnings():
            logging.captureWarnings(True)
            self.addCleanup(logging.captureWarnings, False)
            warnings.filterwarnings("always", category=UserWarning)
            stream = io.StringIO()
            h = logging.StreamHandler(stream)
            logger = logging.getLogger("py.warnings")
            logger.addHandler(h)
            warnings.warn("I'm warning you...")
            logger.removeHandler(h)
            s = stream.getvalue()
            h.close()
            self.assertGreater(s.find("UserWarning: I'm warning you...\n"), 0)

            #See if an explicit file uses the original implementation
            a_file = io.StringIO()
            warnings.showwarning("Explicit", UserWarning, "dummy.py", 42,
                                 a_file, "Dummy line")
            s = a_file.getvalue()
            a_file.close()
            self.assertEqual(s,
                "dummy.py:42: UserWarning: Explicit\n  Dummy line\n")

    def test_warnings_no_handlers(self):
        with warnings.catch_warnings():
            logging.captureWarnings(True)
            self.addCleanup(logging.captureWarnings, False)

            # confirm our assumption: no loggers are set
            logger = logging.getLogger("py.warnings")
            self.assertEqual(logger.handlers, [])

            warnings.showwarning("Explicit", UserWarning, "dummy.py", 42)
            self.assertEqual(len(logger.handlers), 1)
            self.assertIsInstance(logger.handlers[0], logging.NullHandler)


def formatFunc(format, datefmt=None):
    return logging.Formatter(format, datefmt)

def handlerFunc():
    return logging.StreamHandler()

class CustomHandler(logging.StreamHandler):
    pass

class ConfigDictTest(BaseTest):

    """Reading logging config from a dictionary."""

    expected_log_pat = r"^(\w+) \+\+ (\w+)$"

    # config0 is a standard configuration.
    config0 = {
        'version': 1,
        'formatters': {
            'form1' : {
                'format' : '%(levelname)s ++ %(message)s',
            },
        },
        'handlers' : {
            'hand1' : {
                'class' : 'logging.StreamHandler',
                'formatter' : 'form1',
                'level' : 'NOTSET',
                'stream'  : 'ext://sys.stdout',
            },
        },
        'root' : {
            'level' : 'WARNING',
            'handlers' : ['hand1'],
        },
    }

    # config1 adds a little to the standard configuration.
    config1 = {
        'version': 1,
        'formatters': {
            'form1' : {
                'format' : '%(levelname)s ++ %(message)s',
            },
        },
        'handlers' : {
            'hand1' : {
                'class' : 'logging.StreamHandler',
                'formatter' : 'form1',
                'level' : 'NOTSET',
                'stream'  : 'ext://sys.stdout',
            },
        },
        'loggers' : {
            'compiler.parser' : {
                'level' : 'DEBUG',
                'handlers' : ['hand1'],
            },
        },
        'root' : {
            'level' : 'WARNING',
        },
    }

    # config1a moves the handler to the root. Used with config8a
    config1a = {
        'version': 1,
        'formatters': {
            'form1' : {
                'format' : '%(levelname)s ++ %(message)s',
            },
        },
        'handlers' : {
            'hand1' : {
                'class' : 'logging.StreamHandler',
                'formatter' : 'form1',
                'level' : 'NOTSET',
                'stream'  : 'ext://sys.stdout',
            },
        },
        'loggers' : {
            'compiler.parser' : {
                'level' : 'DEBUG',
            },
        },
        'root' : {
            'level' : 'WARNING',
            'handlers' : ['hand1'],
        },
    }

    # config2 has a subtle configuration error that should be reported
    config2 = {
        'version': 1,
        'formatters': {
            'form1' : {
                'format' : '%(levelname)s ++ %(message)s',
            },
        },
        'handlers' : {
            'hand1' : {
                'class' : 'logging.StreamHandler',
                'formatter' : 'form1',
                'level' : 'NOTSET',
                'stream'  : 'ext://sys.stdbout',
            },
        },
        'loggers' : {
            'compiler.parser' : {
                'level' : 'DEBUG',
                'handlers' : ['hand1'],
            },
        },
        'root' : {
            'level' : 'WARNING',
        },
    }

    #As config1 but with a misspelt level on a handler
    config2a = {
        'version': 1,
        'formatters': {
            'form1' : {
                'format' : '%(levelname)s ++ %(message)s',
            },
        },
        'handlers' : {
            'hand1' : {
                'class' : 'logging.StreamHandler',
                'formatter' : 'form1',
                'level' : 'NTOSET',
                'stream'  : 'ext://sys.stdout',
            },
        },
        'loggers' : {
            'compiler.parser' : {
                'level' : 'DEBUG',
                'handlers' : ['hand1'],
            },
        },
        'root' : {
            'level' : 'WARNING',
        },
    }


    #As config1 but with a misspelt level on a logger
    config2b = {
        'version': 1,
        'formatters': {
            'form1' : {
                'format' : '%(levelname)s ++ %(message)s',
            },
        },
        'handlers' : {
            'hand1' : {
                'class' : 'logging.StreamHandler',
                'formatter' : 'form1',
                'level' : 'NOTSET',
                'stream'  : 'ext://sys.stdout',
            },
        },
        'loggers' : {
            'compiler.parser' : {
                'level' : 'DEBUG',
                'handlers' : ['hand1'],
            },
        },
        'root' : {
            'level' : 'WRANING',
        },
    }

    # config3 has a less subtle configuration error
    config3 = {
        'version': 1,
        'formatters': {
            'form1' : {
                'format' : '%(levelname)s ++ %(message)s',
            },
        },
        'handlers' : {
            'hand1' : {
                'class' : 'logging.StreamHandler',
                'formatter' : 'misspelled_name',
                'level' : 'NOTSET',
                'stream'  : 'ext://sys.stdout',
            },
        },
        'loggers' : {
            'compiler.parser' : {
                'level' : 'DEBUG',
                'handlers' : ['hand1'],
            },
        },
        'root' : {
            'level' : 'WARNING',
        },
    }

    # config4 specifies a custom formatter class to be loaded
    config4 = {
        'version': 1,
        'formatters': {
            'form1' : {
                '()' : __name__ + '.ExceptionFormatter',
                'format' : '%(levelname)s:%(name)s:%(message)s',
            },
        },
        'handlers' : {
            'hand1' : {
                'class' : 'logging.StreamHandler',
                'formatter' : 'form1',
                'level' : 'NOTSET',
                'stream'  : 'ext://sys.stdout',
            },
        },
        'root' : {
            'level' : 'NOTSET',
                'handlers' : ['hand1'],
        },
    }

    # As config4 but using an actual callable rather than a string
    config4a = {
        'version': 1,
        'formatters': {
            'form1' : {
                '()' : ExceptionFormatter,
                'format' : '%(levelname)s:%(name)s:%(message)s',
            },
            'form2' : {
                '()' : __name__ + '.formatFunc',
                'format' : '%(levelname)s:%(name)s:%(message)s',
            },
            'form3' : {
                '()' : formatFunc,
                'format' : '%(levelname)s:%(name)s:%(message)s',
            },
        },
        'handlers' : {
            'hand1' : {
                'class' : 'logging.StreamHandler',
                'formatter' : 'form1',
                'level' : 'NOTSET',
                'stream'  : 'ext://sys.stdout',
            },
            'hand2' : {
                '()' : handlerFunc,
            },
        },
        'root' : {
            'level' : 'NOTSET',
                'handlers' : ['hand1'],
        },
    }

    # config5 specifies a custom handler class to be loaded
    config5 = {
        'version': 1,
        'formatters': {
            'form1' : {
                'format' : '%(levelname)s ++ %(message)s',
            },
        },
        'handlers' : {
            'hand1' : {
                'class' : __name__ + '.CustomHandler',
                'formatter' : 'form1',
                'level' : 'NOTSET',
                'stream'  : 'ext://sys.stdout',
            },
        },
        'loggers' : {
            'compiler.parser' : {
                'level' : 'DEBUG',
                'handlers' : ['hand1'],
            },
        },
        'root' : {
            'level' : 'WARNING',
        },
    }

    # config6 specifies a custom handler class to be loaded
    # but has bad arguments
    config6 = {
        'version': 1,
        'formatters': {
            'form1' : {
                'format' : '%(levelname)s ++ %(message)s',
            },
        },
        'handlers' : {
            'hand1' : {
                'class' : __name__ + '.CustomHandler',
                'formatter' : 'form1',
                'level' : 'NOTSET',
                'stream'  : 'ext://sys.stdout',
                '9' : 'invalid parameter name',
            },
        },
        'loggers' : {
            'compiler.parser' : {
                'level' : 'DEBUG',
                'handlers' : ['hand1'],
            },
        },
        'root' : {
            'level' : 'WARNING',
        },
    }

    #config 7 does not define compiler.parser but defines compiler.lexer
    #so compiler.parser should be disabled after applying it
    config7 = {
        'version': 1,
        'formatters': {
            'form1' : {
                'format' : '%(levelname)s ++ %(message)s',
            },
        },
        'handlers' : {
            'hand1' : {
                'class' : 'logging.StreamHandler',
                'formatter' : 'form1',
                'level' : 'NOTSET',
                'stream'  : 'ext://sys.stdout',
            },
        },
        'loggers' : {
            'compiler.lexer' : {
                'level' : 'DEBUG',
                'handlers' : ['hand1'],
            },
        },
        'root' : {
            'level' : 'WARNING',
        },
    }

    # config8 defines both compiler and compiler.lexer
    # so compiler.parser should not be disabled (since
    # compiler is defined)
    config8 = {
        'version': 1,
        'disable_existing_loggers' : False,
        'formatters': {
            'form1' : {
                'format' : '%(levelname)s ++ %(message)s',
            },
        },
        'handlers' : {
            'hand1' : {
                'class' : 'logging.StreamHandler',
                'formatter' : 'form1',
                'level' : 'NOTSET',
                'stream'  : 'ext://sys.stdout',
            },
        },
        'loggers' : {
            'compiler' : {
                'level' : 'DEBUG',
                'handlers' : ['hand1'],
            },
            'compiler.lexer' : {
            },
        },
        'root' : {
            'level' : 'WARNING',
        },
    }

    # config8a disables existing loggers
    config8a = {
        'version': 1,
        'disable_existing_loggers' : True,
        'formatters': {
            'form1' : {
                'format' : '%(levelname)s ++ %(message)s',
            },
        },
        'handlers' : {
            'hand1' : {
                'class' : 'logging.StreamHandler',
                'formatter' : 'form1',
                'level' : 'NOTSET',
                'stream'  : 'ext://sys.stdout',
            },
        },
        'loggers' : {
            'compiler' : {
                'level' : 'DEBUG',
                'handlers' : ['hand1'],
            },
            'compiler.lexer' : {
            },
        },
        'root' : {
            'level' : 'WARNING',
        },
    }

    config9 = {
        'version': 1,
        'formatters': {
            'form1' : {
                'format' : '%(levelname)s ++ %(message)s',
            },
        },
        'handlers' : {
            'hand1' : {
                'class' : 'logging.StreamHandler',
                'formatter' : 'form1',
                'level' : 'WARNING',
                'stream'  : 'ext://sys.stdout',
            },
        },
        'loggers' : {
            'compiler.parser' : {
                'level' : 'WARNING',
                'handlers' : ['hand1'],
            },
        },
        'root' : {
            'level' : 'NOTSET',
        },
    }

    config9a = {
        'version': 1,
        'incremental' : True,
        'handlers' : {
            'hand1' : {
                'level' : 'WARNING',
            },
        },
        'loggers' : {
            'compiler.parser' : {
                'level' : 'INFO',
            },
        },
    }

    config9b = {
        'version': 1,
        'incremental' : True,
        'handlers' : {
            'hand1' : {
                'level' : 'INFO',
            },
        },
        'loggers' : {
            'compiler.parser' : {
                'level' : 'INFO',
            },
        },
    }

    #As config1 but with a filter added
    config10 = {
        'version': 1,
        'formatters': {
            'form1' : {
                'format' : '%(levelname)s ++ %(message)s',
            },
        },
        'filters' : {
            'filt1' : {
                'name' : 'compiler.parser',
            },
        },
        'handlers' : {
            'hand1' : {
                'class' : 'logging.StreamHandler',
                'formatter' : 'form1',
                'level' : 'NOTSET',
                'stream'  : 'ext://sys.stdout',
                'filters' : ['filt1'],
            },
        },
        'loggers' : {
            'compiler.parser' : {
                'level' : 'DEBUG',
                'filters' : ['filt1'],
            },
        },
        'root' : {
            'level' : 'WARNING',
            'handlers' : ['hand1'],
        },
    }

    #As config1 but using cfg:// references
    config11 = {
        'version': 1,
        'true_formatters': {
            'form1' : {
                'format' : '%(levelname)s ++ %(message)s',
            },
        },
        'handler_configs': {
            'hand1' : {
                'class' : 'logging.StreamHandler',
                'formatter' : 'form1',
                'level' : 'NOTSET',
                'stream'  : 'ext://sys.stdout',
            },
        },
        'formatters' : 'cfg://true_formatters',
        'handlers' : {
            'hand1' : 'cfg://handler_configs[hand1]',
        },
        'loggers' : {
            'compiler.parser' : {
                'level' : 'DEBUG',
                'handlers' : ['hand1'],
            },
        },
        'root' : {
            'level' : 'WARNING',
        },
    }

    #As config11 but missing the version key
    config12 = {
        'true_formatters': {
            'form1' : {
                'format' : '%(levelname)s ++ %(message)s',
            },
        },
        'handler_configs': {
            'hand1' : {
                'class' : 'logging.StreamHandler',
                'formatter' : 'form1',
                'level' : 'NOTSET',
                'stream'  : 'ext://sys.stdout',
            },
        },
        'formatters' : 'cfg://true_formatters',
        'handlers' : {
            'hand1' : 'cfg://handler_configs[hand1]',
        },
        'loggers' : {
            'compiler.parser' : {
                'level' : 'DEBUG',
                'handlers' : ['hand1'],
            },
        },
        'root' : {
            'level' : 'WARNING',
        },
    }

    #As config11 but using an unsupported version
    config13 = {
        'version': 2,
        'true_formatters': {
            'form1' : {
                'format' : '%(levelname)s ++ %(message)s',
            },
        },
        'handler_configs': {
            'hand1' : {
                'class' : 'logging.StreamHandler',
                'formatter' : 'form1',
                'level' : 'NOTSET',
                'stream'  : 'ext://sys.stdout',
            },
        },
        'formatters' : 'cfg://true_formatters',
        'handlers' : {
            'hand1' : 'cfg://handler_configs[hand1]',
        },
        'loggers' : {
            'compiler.parser' : {
                'level' : 'DEBUG',
                'handlers' : ['hand1'],
            },
        },
        'root' : {
            'level' : 'WARNING',
        },
    }

    # As config0, but with properties
    config14 = {
        'version': 1,
        'formatters': {
            'form1' : {
                'format' : '%(levelname)s ++ %(message)s',
            },
        },
        'handlers' : {
            'hand1' : {
                'class' : 'logging.StreamHandler',
                'formatter' : 'form1',
                'level' : 'NOTSET',
                'stream'  : 'ext://sys.stdout',
                '.': {
                    'foo': 'bar',
                    'terminator': '!\n',
                }
            },
        },
        'root' : {
            'level' : 'WARNING',
            'handlers' : ['hand1'],
        },
    }

    out_of_order = {
        "version": 1,
        "formatters": {
            "mySimpleFormatter": {
                "format": "%(asctime)s (%(name)s) %(levelname)s: %(message)s",
                "style": "$"
            }
        },
        "handlers": {
            "fileGlobal": {
                "class": "logging.StreamHandler",
                "level": "DEBUG",
                "formatter": "mySimpleFormatter"
            },
            "bufferGlobal": {
                "class": "logging.handlers.MemoryHandler",
                "capacity": 5,
                "formatter": "mySimpleFormatter",
                "target": "fileGlobal",
                "level": "DEBUG"
                }
        },
        "loggers": {
            "mymodule": {
                "level": "DEBUG",
                "handlers": ["bufferGlobal"],
                "propagate": "true"
            }
        }
    }

    def apply_config(self, conf):
        logging.config.dictConfig(conf)

    def test_config0_ok(self):
        # A simple config which overrides the default settings.
        with captured_stdout() as output:
            self.apply_config(self.config0)
            logger = logging.getLogger()
            # Won't output anything
            logger.info(self.next_message())
            # Outputs a message
            logger.error(self.next_message())
            self.assert_log_lines([
                ('ERROR', '2'),
            ], stream=output)
            # Original logger output is empty.
            self.assert_log_lines([])

    def test_config1_ok(self, config=config1):
        # A config defining a sub-parser as well.
        with captured_stdout() as output:
            self.apply_config(config)
            logger = logging.getLogger("compiler.parser")
            # Both will output a message
            logger.info(self.next_message())
            logger.error(self.next_message())
            self.assert_log_lines([
                ('INFO', '1'),
                ('ERROR', '2'),
            ], stream=output)
            # Original logger output is empty.
            self.assert_log_lines([])

    def test_config2_failure(self):
        # A simple config which overrides the default settings.
        self.assertRaises(Exception, self.apply_config, self.config2)

    def test_config2a_failure(self):
        # A simple config which overrides the default settings.
        self.assertRaises(Exception, self.apply_config, self.config2a)

    def test_config2b_failure(self):
        # A simple config which overrides the default settings.
        self.assertRaises(Exception, self.apply_config, self.config2b)

    def test_config3_failure(self):
        # A simple config which overrides the default settings.
        self.assertRaises(Exception, self.apply_config, self.config3)

    def test_config4_ok(self):
        # A config specifying a custom formatter class.
        with captured_stdout() as output:
            self.apply_config(self.config4)
            #logger = logging.getLogger()
            try:
                raise RuntimeError()
            except RuntimeError:
                logging.exception("just testing")
            sys.stdout.seek(0)
            self.assertEqual(output.getvalue(),
                "ERROR:root:just testing\nGot a [RuntimeError]\n")
            # Original logger output is empty
            self.assert_log_lines([])

    def test_config4a_ok(self):
        # A config specifying a custom formatter class.
        with captured_stdout() as output:
            self.apply_config(self.config4a)
            #logger = logging.getLogger()
            try:
                raise RuntimeError()
            except RuntimeError:
                logging.exception("just testing")
            sys.stdout.seek(0)
            self.assertEqual(output.getvalue(),
                "ERROR:root:just testing\nGot a [RuntimeError]\n")
            # Original logger output is empty
            self.assert_log_lines([])

    def test_config5_ok(self):
        self.test_config1_ok(config=self.config5)

    def test_config6_failure(self):
        self.assertRaises(Exception, self.apply_config, self.config6)

    def test_config7_ok(self):
        with captured_stdout() as output:
            self.apply_config(self.config1)
            logger = logging.getLogger("compiler.parser")
            # Both will output a message
            logger.info(self.next_message())
            logger.error(self.next_message())
            self.assert_log_lines([
                ('INFO', '1'),
                ('ERROR', '2'),
            ], stream=output)
            # Original logger output is empty.
            self.assert_log_lines([])
        with captured_stdout() as output:
            self.apply_config(self.config7)
            logger = logging.getLogger("compiler.parser")
            self.assertTrue(logger.disabled)
            logger = logging.getLogger("compiler.lexer")
            # Both will output a message
            logger.info(self.next_message())
            logger.error(self.next_message())
            self.assert_log_lines([
                ('INFO', '3'),
                ('ERROR', '4'),
            ], stream=output)
            # Original logger output is empty.
            self.assert_log_lines([])

    #Same as test_config_7_ok but don't disable old loggers.
    def test_config_8_ok(self):
        with captured_stdout() as output:
            self.apply_config(self.config1)
            logger = logging.getLogger("compiler.parser")
            # All will output a message
            logger.info(self.next_message())
            logger.error(self.next_message())
            self.assert_log_lines([
                ('INFO', '1'),
                ('ERROR', '2'),
            ], stream=output)
            # Original logger output is empty.
            self.assert_log_lines([])
        with captured_stdout() as output:
            self.apply_config(self.config8)
            logger = logging.getLogger("compiler.parser")
            self.assertFalse(logger.disabled)
            # Both will output a message
            logger.info(self.next_message())
            logger.error(self.next_message())
            logger = logging.getLogger("compiler.lexer")
            # Both will output a message
            logger.info(self.next_message())
            logger.error(self.next_message())
            self.assert_log_lines([
                ('INFO', '3'),
                ('ERROR', '4'),
                ('INFO', '5'),
                ('ERROR', '6'),
            ], stream=output)
            # Original logger output is empty.
            self.assert_log_lines([])

    def test_config_8a_ok(self):
        with captured_stdout() as output:
            self.apply_config(self.config1a)
            logger = logging.getLogger("compiler.parser")
            # See issue #11424. compiler-hyphenated sorts
            # between compiler and compiler.xyz and this
            # was preventing compiler.xyz from being included
            # in the child loggers of compiler because of an
            # overzealous loop termination condition.
            hyphenated = logging.getLogger('compiler-hyphenated')
            # All will output a message
            logger.info(self.next_message())
            logger.error(self.next_message())
            hyphenated.critical(self.next_message())
            self.assert_log_lines([
                ('INFO', '1'),
                ('ERROR', '2'),
                ('CRITICAL', '3'),
            ], stream=output)
            # Original logger output is empty.
            self.assert_log_lines([])
        with captured_stdout() as output:
            self.apply_config(self.config8a)
            logger = logging.getLogger("compiler.parser")
            self.assertFalse(logger.disabled)
            # Both will output a message
            logger.info(self.next_message())
            logger.error(self.next_message())
            logger = logging.getLogger("compiler.lexer")
            # Both will output a message
            logger.info(self.next_message())
            logger.error(self.next_message())
            # Will not appear
            hyphenated.critical(self.next_message())
            self.assert_log_lines([
                ('INFO', '4'),
                ('ERROR', '5'),
                ('INFO', '6'),
                ('ERROR', '7'),
            ], stream=output)
            # Original logger output is empty.
            self.assert_log_lines([])

    def test_config_9_ok(self):
        with captured_stdout() as output:
            self.apply_config(self.config9)
            logger = logging.getLogger("compiler.parser")
            #Nothing will be output since both handler and logger are set to WARNING
            logger.info(self.next_message())
            self.assert_log_lines([], stream=output)
            self.apply_config(self.config9a)
            #Nothing will be output since both handler is still set to WARNING
            logger.info(self.next_message())
            self.assert_log_lines([], stream=output)
            self.apply_config(self.config9b)
            #Message should now be output
            logger.info(self.next_message())
            self.assert_log_lines([
                ('INFO', '3'),
            ], stream=output)

    def test_config_10_ok(self):
        with captured_stdout() as output:
            self.apply_config(self.config10)
            logger = logging.getLogger("compiler.parser")
            logger.warning(self.next_message())
            logger = logging.getLogger('compiler')
            #Not output, because filtered
            logger.warning(self.next_message())
            logger = logging.getLogger('compiler.lexer')
            #Not output, because filtered
            logger.warning(self.next_message())
            logger = logging.getLogger("compiler.parser.codegen")
            #Output, as not filtered
            logger.error(self.next_message())
            self.assert_log_lines([
                ('WARNING', '1'),
                ('ERROR', '4'),
            ], stream=output)

    def test_config11_ok(self):
        self.test_config1_ok(self.config11)

    def test_config12_failure(self):
        self.assertRaises(Exception, self.apply_config, self.config12)

    def test_config13_failure(self):
        self.assertRaises(Exception, self.apply_config, self.config13)

    def test_config14_ok(self):
        with captured_stdout() as output:
            self.apply_config(self.config14)
            h = logging._handlers['hand1']
            self.assertEqual(h.foo, 'bar')
            self.assertEqual(h.terminator, '!\n')
            logging.warning('Exclamation')
            self.assertTrue(output.getvalue().endswith('Exclamation!\n'))

    @unittest.skipUnless(threading, 'listen() needs threading to work')
    def setup_via_listener(self, text, verify=None):
        text = text.encode("utf-8")
        # Ask for a randomly assigned port (by using port 0)
        t = logging.config.listen(0, verify)
        t.start()
        t.ready.wait()
        # Now get the port allocated
        port = t.port
        t.ready.clear()
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(2.0)
            sock.connect(('localhost', port))

            slen = struct.pack('>L', len(text))
            s = slen + text
            sentsofar = 0
            left = len(s)
            while left > 0:
                sent = sock.send(s[sentsofar:])
                sentsofar += sent
                left -= sent
            sock.close()
        finally:
            t.ready.wait(2.0)
            logging.config.stopListening()
            t.join(2.0)

    @unittest.skipUnless(threading, 'Threading required for this test.')
    def test_listen_config_10_ok(self):
        with captured_stdout() as output:
            self.setup_via_listener(json.dumps(self.config10))
            logger = logging.getLogger("compiler.parser")
            logger.warning(self.next_message())
            logger = logging.getLogger('compiler')
            #Not output, because filtered
            logger.warning(self.next_message())
            logger = logging.getLogger('compiler.lexer')
            #Not output, because filtered
            logger.warning(self.next_message())
            logger = logging.getLogger("compiler.parser.codegen")
            #Output, as not filtered
            logger.error(self.next_message())
            self.assert_log_lines([
                ('WARNING', '1'),
                ('ERROR', '4'),
            ], stream=output)

    @unittest.skipUnless(threading, 'Threading required for this test.')
    def test_listen_config_1_ok(self):
        with captured_stdout() as output:
            self.setup_via_listener(textwrap.dedent(ConfigFileTest.config1))
            logger = logging.getLogger("compiler.parser")
            # Both will output a message
            logger.info(self.next_message())
            logger.error(self.next_message())
            self.assert_log_lines([
                ('INFO', '1'),
                ('ERROR', '2'),
            ], stream=output)
            # Original logger output is empty.
            self.assert_log_lines([])

    @unittest.skipUnless(threading, 'Threading required for this test.')
    def test_listen_verify(self):

        def verify_fail(stuff):
            return None

        def verify_reverse(stuff):
            return stuff[::-1]

        logger = logging.getLogger("compiler.parser")
        to_send = textwrap.dedent(ConfigFileTest.config1)
        # First, specify a verification function that will fail.
        # We expect to see no output, since our configuration
        # never took effect.
        with captured_stdout() as output:
            self.setup_via_listener(to_send, verify_fail)
            # Both will output a message
            logger.info(self.next_message())
            logger.error(self.next_message())
        self.assert_log_lines([], stream=output)
        # Original logger output has the stuff we logged.
        self.assert_log_lines([
            ('INFO', '1'),
            ('ERROR', '2'),
        ], pat=r"^[\w.]+ -> (\w+): (\d+)$")

        # Now, perform no verification. Our configuration
        # should take effect.

        with captured_stdout() as output:
            self.setup_via_listener(to_send)    # no verify callable specified
            logger = logging.getLogger("compiler.parser")
            # Both will output a message
            logger.info(self.next_message())
            logger.error(self.next_message())
        self.assert_log_lines([
            ('INFO', '3'),
            ('ERROR', '4'),
        ], stream=output)
        # Original logger output still has the stuff we logged before.
        self.assert_log_lines([
            ('INFO', '1'),
            ('ERROR', '2'),
        ], pat=r"^[\w.]+ -> (\w+): (\d+)$")

        # Now, perform verification which transforms the bytes.

        with captured_stdout() as output:
            self.setup_via_listener(to_send[::-1], verify_reverse)
            logger = logging.getLogger("compiler.parser")
            # Both will output a message
            logger.info(self.next_message())
            logger.error(self.next_message())
        self.assert_log_lines([
            ('INFO', '5'),
            ('ERROR', '6'),
        ], stream=output)
        # Original logger output still has the stuff we logged before.
        self.assert_log_lines([
            ('INFO', '1'),
            ('ERROR', '2'),
        ], pat=r"^[\w.]+ -> (\w+): (\d+)$")

    def test_out_of_order(self):
        self.apply_config(self.out_of_order)
        handler = logging.getLogger('mymodule').handlers[0]
        self.assertIsInstance(handler.target, logging.Handler)
        self.assertIsInstance(handler.formatter._style,
                              logging.StringTemplateStyle)

    def test_baseconfig(self):
        d = {
            'atuple': (1, 2, 3),
            'alist': ['a', 'b', 'c'],
            'adict': {'d': 'e', 'f': 3 },
            'nest1': ('g', ('h', 'i'), 'j'),
            'nest2': ['k', ['l', 'm'], 'n'],
            'nest3': ['o', 'cfg://alist', 'p'],
        }
        bc = logging.config.BaseConfigurator(d)
        self.assertEqual(bc.convert('cfg://atuple[1]'), 2)
        self.assertEqual(bc.convert('cfg://alist[1]'), 'b')
        self.assertEqual(bc.convert('cfg://nest1[1][0]'), 'h')
        self.assertEqual(bc.convert('cfg://nest2[1][1]'), 'm')
        self.assertEqual(bc.convert('cfg://adict.d'), 'e')
        self.assertEqual(bc.convert('cfg://adict[f]'), 3)
        v = bc.convert('cfg://nest3')
        self.assertEqual(v.pop(1), ['a', 'b', 'c'])
        self.assertRaises(KeyError, bc.convert, 'cfg://nosuch')
        self.assertRaises(ValueError, bc.convert, 'cfg://!')
        self.assertRaises(KeyError, bc.convert, 'cfg://adict[2]')

class ManagerTest(BaseTest):
    def test_manager_loggerclass(self):
        logged = []

        class MyLogger(logging.Logger):
            def _log(self, level, msg, args, exc_info=None, extra=None):
                logged.append(msg)

        man = logging.Manager(None)
        self.assertRaises(TypeError, man.setLoggerClass, int)
        man.setLoggerClass(MyLogger)
        logger = man.getLogger('test')
        logger.warning('should appear in logged')
        logging.warning('should not appear in logged')

        self.assertEqual(logged, ['should appear in logged'])

    def test_set_log_record_factory(self):
        man = logging.Manager(None)
        expected = object()
        man.setLogRecordFactory(expected)
        self.assertEqual(man.logRecordFactory, expected)

class ChildLoggerTest(BaseTest):
    def test_child_loggers(self):
        r = logging.getLogger()
        l1 = logging.getLogger('abc')
        l2 = logging.getLogger('def.ghi')
        c1 = r.getChild('xyz')
        c2 = r.getChild('uvw.xyz')
        self.assertIs(c1, logging.getLogger('xyz'))
        self.assertIs(c2, logging.getLogger('uvw.xyz'))
        c1 = l1.getChild('def')
        c2 = c1.getChild('ghi')
        c3 = l1.getChild('def.ghi')
        self.assertIs(c1, logging.getLogger('abc.def'))
        self.assertIs(c2, logging.getLogger('abc.def.ghi'))
        self.assertIs(c2, c3)


class DerivedLogRecord(logging.LogRecord):
    pass

class LogRecordFactoryTest(BaseTest):

    def setUp(self):
        class CheckingFilter(logging.Filter):
            def __init__(self, cls):
                self.cls = cls

            def filter(self, record):
                t = type(record)
                if t is not self.cls:
                    msg = 'Unexpected LogRecord type %s, expected %s' % (t,
                            self.cls)
                    raise TypeError(msg)
                return True

        BaseTest.setUp(self)
        self.filter = CheckingFilter(DerivedLogRecord)
        self.root_logger.addFilter(self.filter)
        self.orig_factory = logging.getLogRecordFactory()

    def tearDown(self):
        self.root_logger.removeFilter(self.filter)
        BaseTest.tearDown(self)
        logging.setLogRecordFactory(self.orig_factory)

    def test_logrecord_class(self):
        self.assertRaises(TypeError, self.root_logger.warning,
                          self.next_message())
        logging.setLogRecordFactory(DerivedLogRecord)
        self.root_logger.error(self.next_message())
        self.assert_log_lines([
           ('root', 'ERROR', '2'),
        ])


class QueueHandlerTest(BaseTest):
    # Do not bother with a logger name group.
    expected_log_pat = r"^[\w.]+ -> (\w+): (\d+)$"

    def setUp(self):
        BaseTest.setUp(self)
        self.queue = queue.Queue(-1)
        self.que_hdlr = logging.handlers.QueueHandler(self.queue)
        self.que_logger = logging.getLogger('que')
        self.que_logger.propagate = False
        self.que_logger.setLevel(logging.WARNING)
        self.que_logger.addHandler(self.que_hdlr)

    def tearDown(self):
        self.que_hdlr.close()
        BaseTest.tearDown(self)

    def test_queue_handler(self):
        self.que_logger.debug(self.next_message())
        self.assertRaises(queue.Empty, self.queue.get_nowait)
        self.que_logger.info(self.next_message())
        self.assertRaises(queue.Empty, self.queue.get_nowait)
        msg = self.next_message()
        self.que_logger.warning(msg)
        data = self.queue.get_nowait()
        self.assertTrue(isinstance(data, logging.LogRecord))
        self.assertEqual(data.name, self.que_logger.name)
        self.assertEqual((data.msg, data.args), (msg, None))

    @unittest.skipUnless(hasattr(logging.handlers, 'QueueListener'),
                         'logging.handlers.QueueListener required for this test')
    def test_queue_listener(self):
        handler = TestHandler(Matcher())
        listener = logging.handlers.QueueListener(self.queue, handler)
        listener.start()
        try:
            self.que_logger.warning(self.next_message())
            self.que_logger.error(self.next_message())
            self.que_logger.critical(self.next_message())
        finally:
            listener.stop()
        self.assertTrue(handler.matches(levelno=logging.WARNING, message='1'))
        self.assertTrue(handler.matches(levelno=logging.ERROR, message='2'))
        self.assertTrue(handler.matches(levelno=logging.CRITICAL, message='3'))

ZERO = datetime.timedelta(0)

class UTC(datetime.tzinfo):
    def utcoffset(self, dt):
        return ZERO

    dst = utcoffset

    def tzname(self, dt):
        return 'UTC'

utc = UTC()

class FormatterTest(unittest.TestCase):
    def setUp(self):
        self.common = {
            'name': 'formatter.test',
            'level': logging.DEBUG,
            'pathname': os.path.join('path', 'to', 'dummy.ext'),
            'lineno': 42,
            'exc_info': None,
            'func': None,
            'msg': 'Message with %d %s',
            'args': (2, 'placeholders'),
        }
        self.variants = {
        }

    def get_record(self, name=None):
        result = dict(self.common)
        if name is not None:
            result.update(self.variants[name])
        return logging.makeLogRecord(result)

    def test_percent(self):
        # Test %-formatting
        r = self.get_record()
        f = logging.Formatter('${%(message)s}')
        self.assertEqual(f.format(r), '${Message with 2 placeholders}')
        f = logging.Formatter('%(random)s')
        self.assertRaises(KeyError, f.format, r)
        self.assertFalse(f.usesTime())
        f = logging.Formatter('%(asctime)s')
        self.assertTrue(f.usesTime())
        f = logging.Formatter('%(asctime)-15s')
        self.assertTrue(f.usesTime())
        f = logging.Formatter('asctime')
        self.assertFalse(f.usesTime())

    def test_braces(self):
        # Test {}-formatting
        r = self.get_record()
        f = logging.Formatter('$%{message}%$', style='{')
        self.assertEqual(f.format(r), '$%Message with 2 placeholders%$')
        f = logging.Formatter('{random}', style='{')
        self.assertRaises(KeyError, f.format, r)
        self.assertFalse(f.usesTime())
        f = logging.Formatter('{asctime}', style='{')
        self.assertTrue(f.usesTime())
        f = logging.Formatter('{asctime!s:15}', style='{')
        self.assertTrue(f.usesTime())
        f = logging.Formatter('{asctime:15}', style='{')
        self.assertTrue(f.usesTime())
        f = logging.Formatter('asctime', style='{')
        self.assertFalse(f.usesTime())

    def test_dollars(self):
        # Test $-formatting
        r = self.get_record()
        f = logging.Formatter('$message', style='$')
        self.assertEqual(f.format(r), 'Message with 2 placeholders')
        f = logging.Formatter('$$%${message}%$$', style='$')
        self.assertEqual(f.format(r), '$%Message with 2 placeholders%$')
        f = logging.Formatter('${random}', style='$')
        self.assertRaises(KeyError, f.format, r)
        self.assertFalse(f.usesTime())
        f = logging.Formatter('${asctime}', style='$')
        self.assertTrue(f.usesTime())
        f = logging.Formatter('${asctime', style='$')
        self.assertFalse(f.usesTime())
        f = logging.Formatter('$asctime', style='$')
        self.assertTrue(f.usesTime())
        f = logging.Formatter('asctime', style='$')
        self.assertFalse(f.usesTime())

    def test_invalid_style(self):
        self.assertRaises(ValueError, logging.Formatter, None, None, 'x')

    def test_time(self):
        r = self.get_record()
        dt = datetime.datetime(1993, 4, 21, 8, 3, 0, 0, utc)
        # We use None to indicate we want the local timezone
        # We're essentially converting a UTC time to local time
        r.created = time.mktime(dt.astimezone(None).timetuple())
        r.msecs = 123
        f = logging.Formatter('%(asctime)s %(message)s')
        f.converter = time.gmtime
        self.assertEqual(f.formatTime(r), '1993-04-21 08:03:00,123')
        self.assertEqual(f.formatTime(r, '%Y:%d'), '1993:21')
        f.format(r)
        self.assertEqual(r.asctime, '1993-04-21 08:03:00,123')

class TestBufferingFormatter(logging.BufferingFormatter):
    def formatHeader(self, records):
        return '[(%d)' % len(records)

    def formatFooter(self, records):
        return '(%d)]' % len(records)

class BufferingFormatterTest(unittest.TestCase):
    def setUp(self):
        self.records = [
            logging.makeLogRecord({'msg': 'one'}),
            logging.makeLogRecord({'msg': 'two'}),
        ]

    def test_default(self):
        f = logging.BufferingFormatter()
        self.assertEqual('', f.format([]))
        self.assertEqual('onetwo', f.format(self.records))

    def test_custom(self):
        f = TestBufferingFormatter()
        self.assertEqual('[(2)onetwo(2)]', f.format(self.records))
        lf = logging.Formatter('<%(message)s>')
        f = TestBufferingFormatter(lf)
        self.assertEqual('[(2)<one><two>(2)]', f.format(self.records))

class ExceptionTest(BaseTest):
    def test_formatting(self):
        r = self.root_logger
        h = RecordingHandler()
        r.addHandler(h)
        try:
            raise RuntimeError('deliberate mistake')
        except:
            logging.exception('failed', stack_info=True)
        r.removeHandler(h)
        h.close()
        r = h.records[0]
        self.assertTrue(r.exc_text.startswith('Traceback (most recent '
                                              'call last):\n'))
        self.assertTrue(r.exc_text.endswith('\nRuntimeError: '
                                            'deliberate mistake'))
        self.assertTrue(r.stack_info.startswith('Stack (most recent '
                                              'call last):\n'))
        self.assertTrue(r.stack_info.endswith('logging.exception(\'failed\', '
                                            'stack_info=True)'))


class LastResortTest(BaseTest):
    def test_last_resort(self):
        # Test the last resort handler
        root = self.root_logger
        root.removeHandler(self.root_hdlr)
        old_stderr = sys.stderr
        old_lastresort = logging.lastResort
        old_raise_exceptions = logging.raiseExceptions
        try:
            sys.stderr = sio = io.StringIO()
            root.debug('This should not appear')
            self.assertEqual(sio.getvalue(), '')
            root.warning('This is your final chance!')
            self.assertEqual(sio.getvalue(), 'This is your final chance!\n')
            #No handlers and no last resort, so 'No handlers' message
            logging.lastResort = None
            sys.stderr = sio = io.StringIO()
            root.warning('This is your final chance!')
            self.assertEqual(sio.getvalue(), 'No handlers could be found for logger "root"\n')
            # 'No handlers' message only printed once
            sys.stderr = sio = io.StringIO()
            root.warning('This is your final chance!')
            self.assertEqual(sio.getvalue(), '')
            root.manager.emittedNoHandlerWarning = False
            #If raiseExceptions is False, no message is printed
            logging.raiseExceptions = False
            sys.stderr = sio = io.StringIO()
            root.warning('This is your final chance!')
            self.assertEqual(sio.getvalue(), '')
        finally:
            sys.stderr = old_stderr
            root.addHandler(self.root_hdlr)
            logging.lastResort = old_lastresort
            logging.raiseExceptions = old_raise_exceptions


class FakeHandler:

    def __init__(self, identifier, called):
        for method in ('acquire', 'flush', 'close', 'release'):
            setattr(self, method, self.record_call(identifier, method, called))

    def record_call(self, identifier, method_name, called):
        def inner():
            called.append('{} - {}'.format(identifier, method_name))
        return inner


class RecordingHandler(logging.NullHandler):

    def __init__(self, *args, **kwargs):
        super(RecordingHandler, self).__init__(*args, **kwargs)
        self.records = []

    def handle(self, record):
        """Keep track of all the emitted records."""
        self.records.append(record)


class ShutdownTest(BaseTest):

    """Test suite for the shutdown method."""

    def setUp(self):
        super(ShutdownTest, self).setUp()
        self.called = []

        raise_exceptions = logging.raiseExceptions
        self.addCleanup(setattr, logging, 'raiseExceptions', raise_exceptions)

    def raise_error(self, error):
        def inner():
            raise error()
        return inner

    def test_no_failure(self):
        # create some fake handlers
        handler0 = FakeHandler(0, self.called)
        handler1 = FakeHandler(1, self.called)
        handler2 = FakeHandler(2, self.called)

        # create live weakref to those handlers
        handlers = map(logging.weakref.ref, [handler0, handler1, handler2])

        logging.shutdown(handlerList=list(handlers))

        expected = ['2 - acquire', '2 - flush', '2 - close', '2 - release',
                    '1 - acquire', '1 - flush', '1 - close', '1 - release',
                    '0 - acquire', '0 - flush', '0 - close', '0 - release']
        self.assertEqual(expected, self.called)

    def _test_with_failure_in_method(self, method, error):
        handler = FakeHandler(0, self.called)
        setattr(handler, method, self.raise_error(error))
        handlers = [logging.weakref.ref(handler)]

        logging.shutdown(handlerList=list(handlers))

        self.assertEqual('0 - release', self.called[-1])

    def test_with_ioerror_in_acquire(self):
        self._test_with_failure_in_method('acquire', OSError)

    def test_with_ioerror_in_flush(self):
        self._test_with_failure_in_method('flush', OSError)

    def test_with_ioerror_in_close(self):
        self._test_with_failure_in_method('close', OSError)

    def test_with_valueerror_in_acquire(self):
        self._test_with_failure_in_method('acquire', ValueError)

    def test_with_valueerror_in_flush(self):
        self._test_with_failure_in_method('flush', ValueError)

    def test_with_valueerror_in_close(self):
        self._test_with_failure_in_method('close', ValueError)

    def test_with_other_error_in_acquire_without_raise(self):
        logging.raiseExceptions = False
        self._test_with_failure_in_method('acquire', IndexError)

    def test_with_other_error_in_flush_without_raise(self):
        logging.raiseExceptions = False
        self._test_with_failure_in_method('flush', IndexError)

    def test_with_other_error_in_close_without_raise(self):
        logging.raiseExceptions = False
        self._test_with_failure_in_method('close', IndexError)

    def test_with_other_error_in_acquire_with_raise(self):
        logging.raiseExceptions = True
        self.assertRaises(IndexError, self._test_with_failure_in_method,
                          'acquire', IndexError)

    def test_with_other_error_in_flush_with_raise(self):
        logging.raiseExceptions = True
        self.assertRaises(IndexError, self._test_with_failure_in_method,
                          'flush', IndexError)

    def test_with_other_error_in_close_with_raise(self):
        logging.raiseExceptions = True
        self.assertRaises(IndexError, self._test_with_failure_in_method,
                          'close', IndexError)


class ModuleLevelMiscTest(BaseTest):

    """Test suite for some module level methods."""

    def test_disable(self):
        old_disable = logging.root.manager.disable
        # confirm our assumptions are correct
        self.assertEqual(old_disable, 0)
        self.addCleanup(logging.disable, old_disable)

        logging.disable(83)
        self.assertEqual(logging.root.manager.disable, 83)

    def _test_log(self, method, level=None):
        called = []
        patch(self, logging, 'basicConfig',
              lambda *a, **kw: called.append((a, kw)))

        recording = RecordingHandler()
        logging.root.addHandler(recording)

        log_method = getattr(logging, method)
        if level is not None:
            log_method(level, "test me: %r", recording)
        else:
            log_method("test me: %r", recording)

        self.assertEqual(len(recording.records), 1)
        record = recording.records[0]
        self.assertEqual(record.getMessage(), "test me: %r" % recording)

        expected_level = level if level is not None else getattr(logging, method.upper())
        self.assertEqual(record.levelno, expected_level)

        # basicConfig was not called!
        self.assertEqual(called, [])

    def test_log(self):
        self._test_log('log', logging.ERROR)

    def test_debug(self):
        self._test_log('debug')

    def test_info(self):
        self._test_log('info')

    def test_warning(self):
        self._test_log('warning')

    def test_error(self):
        self._test_log('error')

    def test_critical(self):
        self._test_log('critical')

    def test_set_logger_class(self):
        self.assertRaises(TypeError, logging.setLoggerClass, object)

        class MyLogger(logging.Logger):
            pass

        logging.setLoggerClass(MyLogger)
        self.assertEqual(logging.getLoggerClass(), MyLogger)

        logging.setLoggerClass(logging.Logger)
        self.assertEqual(logging.getLoggerClass(), logging.Logger)

    def test_logging_at_shutdown(self):
        # Issue #20037
        code = """if 1:
            import logging

            class A:
                def __del__(self):
                    try:
                        raise ValueError("some error")
                    except Exception:
                        logging.exception("exception in __del__")

            a = A()"""
        rc, out, err = assert_python_ok("-c", code)
        err = err.decode()
        self.assertIn("exception in __del__", err)
        self.assertIn("ValueError: some error", err)


class LogRecordTest(BaseTest):
    def test_str_rep(self):
        r = logging.makeLogRecord({})
        s = str(r)
        self.assertTrue(s.startswith('<LogRecord: '))
        self.assertTrue(s.endswith('>'))

    def test_dict_arg(self):
        h = RecordingHandler()
        r = logging.getLogger()
        r.addHandler(h)
        d = {'less' : 'more' }
        logging.warning('less is %(less)s', d)
        self.assertIs(h.records[0].args, d)
        self.assertEqual(h.records[0].message, 'less is more')
        r.removeHandler(h)
        h.close()

    def test_multiprocessing(self):
        r = logging.makeLogRecord({})
        self.assertEqual(r.processName, 'MainProcess')
        try:
            import multiprocessing as mp
            r = logging.makeLogRecord({})
            self.assertEqual(r.processName, mp.current_process().name)
        except ImportError:
            pass

    def test_optional(self):
        r = logging.makeLogRecord({})
        NOT_NONE = self.assertIsNotNone
        if threading:
            NOT_NONE(r.thread)
            NOT_NONE(r.threadName)
        NOT_NONE(r.process)
        NOT_NONE(r.processName)
        log_threads = logging.logThreads
        log_processes = logging.logProcesses
        log_multiprocessing = logging.logMultiprocessing
        try:
            logging.logThreads = False
            logging.logProcesses = False
            logging.logMultiprocessing = False
            r = logging.makeLogRecord({})
            NONE = self.assertIsNone
            NONE(r.thread)
            NONE(r.threadName)
            NONE(r.process)
            NONE(r.processName)
        finally:
            logging.logThreads = log_threads
            logging.logProcesses = log_processes
            logging.logMultiprocessing = log_multiprocessing

class BasicConfigTest(unittest.TestCase):

    """Test suite for logging.basicConfig."""

    def setUp(self):
        super(BasicConfigTest, self).setUp()
        self.handlers = logging.root.handlers
        self.saved_handlers = logging._handlers.copy()
        self.saved_handler_list = logging._handlerList[:]
        self.original_logging_level = logging.root.level
        self.addCleanup(self.cleanup)
        logging.root.handlers = []

    def tearDown(self):
        for h in logging.root.handlers[:]:
            logging.root.removeHandler(h)
            h.close()
        super(BasicConfigTest, self).tearDown()

    def cleanup(self):
        setattr(logging.root, 'handlers', self.handlers)
        logging._handlers.clear()
        logging._handlers.update(self.saved_handlers)
        logging._handlerList[:] = self.saved_handler_list
        logging.root.level = self.original_logging_level

    def test_no_kwargs(self):
        logging.basicConfig()

        # handler defaults to a StreamHandler to sys.stderr
        self.assertEqual(len(logging.root.handlers), 1)
        handler = logging.root.handlers[0]
        self.assertIsInstance(handler, logging.StreamHandler)
        self.assertEqual(handler.stream, sys.stderr)

        formatter = handler.formatter
        # format defaults to logging.BASIC_FORMAT
        self.assertEqual(formatter._style._fmt, logging.BASIC_FORMAT)
        # datefmt defaults to None
        self.assertIsNone(formatter.datefmt)
        # style defaults to %
        self.assertIsInstance(formatter._style, logging.PercentStyle)

        # level is not explicitly set
        self.assertEqual(logging.root.level, self.original_logging_level)

    def test_strformatstyle(self):
        with captured_stdout() as output:
            logging.basicConfig(stream=sys.stdout, style="{")
            logging.error("Log an error")
            sys.stdout.seek(0)
            self.assertEqual(output.getvalue().strip(),
                "ERROR:root:Log an error")

    def test_stringtemplatestyle(self):
        with captured_stdout() as output:
            logging.basicConfig(stream=sys.stdout, style="$")
            logging.error("Log an error")
            sys.stdout.seek(0)
            self.assertEqual(output.getvalue().strip(),
                "ERROR:root:Log an error")

    def test_filename(self):

        def cleanup(h1, h2, fn):
            h1.close()
            h2.close()
            os.remove(fn)

        logging.basicConfig(filename='test.log')

        self.assertEqual(len(logging.root.handlers), 1)
        handler = logging.root.handlers[0]
        self.assertIsInstance(handler, logging.FileHandler)

        expected = logging.FileHandler('test.log', 'a')
        self.assertEqual(handler.stream.mode, expected.stream.mode)
        self.assertEqual(handler.stream.name, expected.stream.name)
        self.addCleanup(cleanup, handler, expected, 'test.log')

    def test_filemode(self):

        def cleanup(h1, h2, fn):
            h1.close()
            h2.close()
            os.remove(fn)

        logging.basicConfig(filename='test.log', filemode='wb')

        handler = logging.root.handlers[0]
        expected = logging.FileHandler('test.log', 'wb')
        self.assertEqual(handler.stream.mode, expected.stream.mode)
        self.addCleanup(cleanup, handler, expected, 'test.log')

    def test_stream(self):
        stream = io.StringIO()
        self.addCleanup(stream.close)
        logging.basicConfig(stream=stream)

        self.assertEqual(len(logging.root.handlers), 1)
        handler = logging.root.handlers[0]
        self.assertIsInstance(handler, logging.StreamHandler)
        self.assertEqual(handler.stream, stream)

    def test_format(self):
        logging.basicConfig(format='foo')

        formatter = logging.root.handlers[0].formatter
        self.assertEqual(formatter._style._fmt, 'foo')

    def test_datefmt(self):
        logging.basicConfig(datefmt='bar')

        formatter = logging.root.handlers[0].formatter
        self.assertEqual(formatter.datefmt, 'bar')

    def test_style(self):
        logging.basicConfig(style='$')

        formatter = logging.root.handlers[0].formatter
        self.assertIsInstance(formatter._style, logging.StringTemplateStyle)

    def test_level(self):
        old_level = logging.root.level
        self.addCleanup(logging.root.setLevel, old_level)

        logging.basicConfig(level=57)
        self.assertEqual(logging.root.level, 57)
        # Test that second call has no effect
        logging.basicConfig(level=58)
        self.assertEqual(logging.root.level, 57)

    def test_incompatible(self):
        assertRaises = self.assertRaises
        handlers = [logging.StreamHandler()]
        stream = sys.stderr
        assertRaises(ValueError, logging.basicConfig, filename='test.log',
                                                     stream=stream)
        assertRaises(ValueError, logging.basicConfig, filename='test.log',
                                                     handlers=handlers)
        assertRaises(ValueError, logging.basicConfig, stream=stream,
                                                     handlers=handlers)

    def test_handlers(self):
        handlers = [
            logging.StreamHandler(),
            logging.StreamHandler(sys.stdout),
            logging.StreamHandler(),
        ]
        f = logging.Formatter()
        handlers[2].setFormatter(f)
        logging.basicConfig(handlers=handlers)
        self.assertIs(handlers[0], logging.root.handlers[0])
        self.assertIs(handlers[1], logging.root.handlers[1])
        self.assertIs(handlers[2], logging.root.handlers[2])
        self.assertIsNotNone(handlers[0].formatter)
        self.assertIsNotNone(handlers[1].formatter)
        self.assertIs(handlers[2].formatter, f)
        self.assertIs(handlers[0].formatter, handlers[1].formatter)

    def _test_log(self, method, level=None):
        # logging.root has no handlers so basicConfig should be called
        called = []

        old_basic_config = logging.basicConfig
        def my_basic_config(*a, **kw):
            old_basic_config()
            old_level = logging.root.level
            logging.root.setLevel(100)  # avoid having messages in stderr
            self.addCleanup(logging.root.setLevel, old_level)
            called.append((a, kw))

        patch(self, logging, 'basicConfig', my_basic_config)

        log_method = getattr(logging, method)
        if level is not None:
            log_method(level, "test me")
        else:
            log_method("test me")

        # basicConfig was called with no arguments
        self.assertEqual(called, [((), {})])

    def test_log(self):
        self._test_log('log', logging.WARNING)

    def test_debug(self):
        self._test_log('debug')

    def test_info(self):
        self._test_log('info')

    def test_warning(self):
        self._test_log('warning')

    def test_error(self):
        self._test_log('error')

    def test_critical(self):
        self._test_log('critical')


class LoggerAdapterTest(unittest.TestCase):

    def setUp(self):
        super(LoggerAdapterTest, self).setUp()
        old_handler_list = logging._handlerList[:]

        self.recording = RecordingHandler()
        self.logger = logging.root
        self.logger.addHandler(self.recording)
        self.addCleanup(self.logger.removeHandler, self.recording)
        self.addCleanup(self.recording.close)

        def cleanup():
            logging._handlerList[:] = old_handler_list

        self.addCleanup(cleanup)
        self.addCleanup(logging.shutdown)
        self.adapter = logging.LoggerAdapter(logger=self.logger, extra=None)

    def test_exception(self):
        msg = 'testing exception: %r'
        exc = None
        try:
            1 / 0
        except ZeroDivisionError as e:
            exc = e
            self.adapter.exception(msg, self.recording)

        self.assertEqual(len(self.recording.records), 1)
        record = self.recording.records[0]
        self.assertEqual(record.levelno, logging.ERROR)
        self.assertEqual(record.msg, msg)
        self.assertEqual(record.args, (self.recording,))
        self.assertEqual(record.exc_info,
                         (exc.__class__, exc, exc.__traceback__))

    def test_critical(self):
        msg = 'critical test! %r'
        self.adapter.critical(msg, self.recording)

        self.assertEqual(len(self.recording.records), 1)
        record = self.recording.records[0]
        self.assertEqual(record.levelno, logging.CRITICAL)
        self.assertEqual(record.msg, msg)
        self.assertEqual(record.args, (self.recording,))

    def test_is_enabled_for(self):
        old_disable = self.adapter.logger.manager.disable
        self.adapter.logger.manager.disable = 33
        self.addCleanup(setattr, self.adapter.logger.manager, 'disable',
                        old_disable)
        self.assertFalse(self.adapter.isEnabledFor(32))

    def test_has_handlers(self):
        self.assertTrue(self.adapter.hasHandlers())

        for handler in self.logger.handlers:
            self.logger.removeHandler(handler)

        self.assertFalse(self.logger.hasHandlers())
        self.assertFalse(self.adapter.hasHandlers())


class LoggerTest(BaseTest):

    def setUp(self):
        super(LoggerTest, self).setUp()
        self.recording = RecordingHandler()
        self.logger = logging.Logger(name='blah')
        self.logger.addHandler(self.recording)
        self.addCleanup(self.logger.removeHandler, self.recording)
        self.addCleanup(self.recording.close)
        self.addCleanup(logging.shutdown)

    def test_set_invalid_level(self):
        self.assertRaises(TypeError, self.logger.setLevel, object())

    def test_exception(self):
        msg = 'testing exception: %r'
        exc = None
        try:
            1 / 0
        except ZeroDivisionError as e:
            exc = e
            self.logger.exception(msg, self.recording)

        self.assertEqual(len(self.recording.records), 1)
        record = self.recording.records[0]
        self.assertEqual(record.levelno, logging.ERROR)
        self.assertEqual(record.msg, msg)
        self.assertEqual(record.args, (self.recording,))
        self.assertEqual(record.exc_info,
                         (exc.__class__, exc, exc.__traceback__))

    def test_log_invalid_level_with_raise(self):
        old_raise = logging.raiseExceptions
        self.addCleanup(setattr, logging, 'raiseExecptions', old_raise)

        logging.raiseExceptions = True
        self.assertRaises(TypeError, self.logger.log, '10', 'test message')

    def test_log_invalid_level_no_raise(self):
        old_raise = logging.raiseExceptions
        self.addCleanup(setattr, logging, 'raiseExecptions', old_raise)

        logging.raiseExceptions = False
        self.logger.log('10', 'test message')  # no exception happens

    def test_find_caller_with_stack_info(self):
        called = []
        patch(self, logging.traceback, 'print_stack',
              lambda f, file: called.append(file.getvalue()))

        self.logger.findCaller(stack_info=True)

        self.assertEqual(len(called), 1)
        self.assertEqual('Stack (most recent call last):\n', called[0])

    def test_make_record_with_extra_overwrite(self):
        name = 'my record'
        level = 13
        fn = lno = msg = args = exc_info = func = sinfo = None
        rv = logging._logRecordFactory(name, level, fn, lno, msg, args,
                                       exc_info, func, sinfo)

        for key in ('message', 'asctime') + tuple(rv.__dict__.keys()):
            extra = {key: 'some value'}
            self.assertRaises(KeyError, self.logger.makeRecord, name, level,
                              fn, lno, msg, args, exc_info,
                              extra=extra, sinfo=sinfo)

    def test_make_record_with_extra_no_overwrite(self):
        name = 'my record'
        level = 13
        fn = lno = msg = args = exc_info = func = sinfo = None
        extra = {'valid_key': 'some value'}
        result = self.logger.makeRecord(name, level, fn, lno, msg, args,
                                        exc_info, extra=extra, sinfo=sinfo)
        self.assertIn('valid_key', result.__dict__)

    def test_has_handlers(self):
        self.assertTrue(self.logger.hasHandlers())

        for handler in self.logger.handlers:
            self.logger.removeHandler(handler)
        self.assertFalse(self.logger.hasHandlers())

    def test_has_handlers_no_propagate(self):
        child_logger = logging.getLogger('blah.child')
        child_logger.propagate = False
        self.assertFalse(child_logger.hasHandlers())

    def test_is_enabled_for(self):
        old_disable = self.logger.manager.disable
        self.logger.manager.disable = 23
        self.addCleanup(setattr, self.logger.manager, 'disable', old_disable)
        self.assertFalse(self.logger.isEnabledFor(22))

    def test_root_logger_aliases(self):
        root = logging.getLogger()
        self.assertIs(root, logging.root)
        self.assertIs(root, logging.getLogger(None))
        self.assertIs(root, logging.getLogger(''))
        self.assertIs(root, logging.getLogger('foo').root)
        self.assertIs(root, logging.getLogger('foo.bar').root)
        self.assertIs(root, logging.getLogger('foo').parent)

        self.assertIsNot(root, logging.getLogger('\0'))
        self.assertIsNot(root, logging.getLogger('foo.bar').parent)

    def test_invalid_names(self):
        self.assertRaises(TypeError, logging.getLogger, any)
        self.assertRaises(TypeError, logging.getLogger, b'foo')


class BaseFileTest(BaseTest):
    "Base class for handler tests that write log files"

    def setUp(self):
        BaseTest.setUp(self)
        fd, self.fn = tempfile.mkstemp(".log", "test_logging-2-")
        os.close(fd)
        self.rmfiles = []

    def tearDown(self):
        for fn in self.rmfiles:
            os.unlink(fn)
        if os.path.exists(self.fn):
            os.unlink(self.fn)
        BaseTest.tearDown(self)

    def assertLogFile(self, filename):
        "Assert a log file is there and register it for deletion"
        self.assertTrue(os.path.exists(filename),
                        msg="Log file %r does not exist" % filename)
        self.rmfiles.append(filename)


class FileHandlerTest(BaseFileTest):
    def test_delay(self):
        os.unlink(self.fn)
        fh = logging.FileHandler(self.fn, delay=True)
        self.assertIsNone(fh.stream)
        self.assertFalse(os.path.exists(self.fn))
        fh.handle(logging.makeLogRecord({}))
        self.assertIsNotNone(fh.stream)
        self.assertTrue(os.path.exists(self.fn))
        fh.close()

class RotatingFileHandlerTest(BaseFileTest):
    def next_rec(self):
        return logging.LogRecord('n', logging.DEBUG, 'p', 1,
                                 self.next_message(), None, None, None)

    def test_should_not_rollover(self):
        # If maxbytes is zero rollover never occurs
        rh = logging.handlers.RotatingFileHandler(self.fn, maxBytes=0)
        self.assertFalse(rh.shouldRollover(None))
        rh.close()

    def test_should_rollover(self):
        rh = logging.handlers.RotatingFileHandler(self.fn, maxBytes=1)
        self.assertTrue(rh.shouldRollover(self.next_rec()))
        rh.close()

    def test_file_created(self):
        # checks that the file is created and assumes it was created
        # by us
        rh = logging.handlers.RotatingFileHandler(self.fn)
        rh.emit(self.next_rec())
        self.assertLogFile(self.fn)
        rh.close()

    def test_rollover_filenames(self):
        def namer(name):
            return name + ".test"
        rh = logging.handlers.RotatingFileHandler(
            self.fn, backupCount=2, maxBytes=1)
        rh.namer = namer
        rh.emit(self.next_rec())
        self.assertLogFile(self.fn)
        rh.emit(self.next_rec())
        self.assertLogFile(namer(self.fn + ".1"))
        rh.emit(self.next_rec())
        self.assertLogFile(namer(self.fn + ".2"))
        self.assertFalse(os.path.exists(namer(self.fn + ".3")))
        rh.close()

    @requires_zlib
    def test_rotator(self):
        def namer(name):
            return name + ".gz"

        def rotator(source, dest):
            with open(source, "rb") as sf:
                data = sf.read()
                compressed = zlib.compress(data, 9)
                with open(dest, "wb") as df:
                    df.write(compressed)
            os.remove(source)

        rh = logging.handlers.RotatingFileHandler(
            self.fn, backupCount=2, maxBytes=1)
        rh.rotator = rotator
        rh.namer = namer
        m1 = self.next_rec()
        rh.emit(m1)
        self.assertLogFile(self.fn)
        m2 = self.next_rec()
        rh.emit(m2)
        fn = namer(self.fn + ".1")
        self.assertLogFile(fn)
        newline = os.linesep
        with open(fn, "rb") as f:
            compressed = f.read()
            data = zlib.decompress(compressed)
            self.assertEqual(data.decode("ascii"), m1.msg + newline)
        rh.emit(self.next_rec())
        fn = namer(self.fn + ".2")
        self.assertLogFile(fn)
        with open(fn, "rb") as f:
            compressed = f.read()
            data = zlib.decompress(compressed)
            self.assertEqual(data.decode("ascii"), m1.msg + newline)
        rh.emit(self.next_rec())
        fn = namer(self.fn + ".2")
        with open(fn, "rb") as f:
            compressed = f.read()
            data = zlib.decompress(compressed)
            self.assertEqual(data.decode("ascii"), m2.msg + newline)
        self.assertFalse(os.path.exists(namer(self.fn + ".3")))
        rh.close()

class TimedRotatingFileHandlerTest(BaseFileTest):
    # other test methods added below
    def test_rollover(self):
        fh = logging.handlers.TimedRotatingFileHandler(self.fn, 'S',
                                                       backupCount=1)
        fmt = logging.Formatter('%(asctime)s %(message)s')
        fh.setFormatter(fmt)
        r1 = logging.makeLogRecord({'msg': 'testing - initial'})
        fh.emit(r1)
        self.assertLogFile(self.fn)
        time.sleep(1.1)    # a little over a second ...
        r2 = logging.makeLogRecord({'msg': 'testing - after delay'})
        fh.emit(r2)
        fh.close()
        # At this point, we should have a recent rotated file which we
        # can test for the existence of. However, in practice, on some
        # machines which run really slowly, we don't know how far back
        # in time to go to look for the log file. So, we go back a fair
        # bit, and stop as soon as we see a rotated file. In theory this
        # could of course still fail, but the chances are lower.
        found = False
        now = datetime.datetime.now()
        GO_BACK = 5 * 60 # seconds
        for secs in range(GO_BACK):
            prev = now - datetime.timedelta(seconds=secs)
            fn = self.fn + prev.strftime(".%Y-%m-%d_%H-%M-%S")
            found = os.path.exists(fn)
            if found:
                self.rmfiles.append(fn)
                break
        msg = 'No rotated files found, went back %d seconds' % GO_BACK
        if not found:
            #print additional diagnostics
            dn, fn = os.path.split(self.fn)
            files = [f for f in os.listdir(dn) if f.startswith(fn)]
            print('Test time: %s' % now.strftime("%Y-%m-%d %H-%M-%S"), file=sys.stderr)
            print('The only matching files are: %s' % files, file=sys.stderr)
            for f in files:
                print('Contents of %s:' % f)
                path = os.path.join(dn, f)
                with open(path, 'r') as tf:
                    print(tf.read())
        self.assertTrue(found, msg=msg)

    def test_invalid(self):
        assertRaises = self.assertRaises
        assertRaises(ValueError, logging.handlers.TimedRotatingFileHandler,
                     self.fn, 'X', delay=True)
        assertRaises(ValueError, logging.handlers.TimedRotatingFileHandler,
                     self.fn, 'W', delay=True)
        assertRaises(ValueError, logging.handlers.TimedRotatingFileHandler,
                     self.fn, 'W7', delay=True)

    def test_compute_rollover_daily_attime(self):
        currentTime = 0
        atTime = datetime.time(12, 0, 0)
        rh = logging.handlers.TimedRotatingFileHandler(
            self.fn, when='MIDNIGHT', interval=1, backupCount=0, utc=True,
            atTime=atTime)
        try:
            actual = rh.computeRollover(currentTime)
            self.assertEqual(actual, currentTime + 12 * 60 * 60)

            actual = rh.computeRollover(currentTime + 13 * 60 * 60)
            self.assertEqual(actual, currentTime + 36 * 60 * 60)
        finally:
            rh.close()

    #@unittest.skipIf(True, 'Temporarily skipped while failures investigated.')
    def test_compute_rollover_weekly_attime(self):
        currentTime = int(time.time())
        today = currentTime - currentTime % 86400

        atTime = datetime.time(12, 0, 0)

        wday = time.gmtime(today).tm_wday
        for day in range(7):
            rh = logging.handlers.TimedRotatingFileHandler(
                self.fn, when='W%d' % day, interval=1, backupCount=0, utc=True,
                atTime=atTime)
            try:
                if wday > day:
                    # The rollover day has already passed this week, so we
                    # go over into next week
                    expected = (7 - wday + day)
                else:
                    expected = (day - wday)
                # At this point expected is in days from now, convert to seconds
                expected *= 24 * 60 * 60
                # Add in the rollover time
                expected += 12 * 60 * 60
                # Add in adjustment for today
                expected += today
                actual = rh.computeRollover(today)
                if actual != expected:
                    print('failed in timezone: %d' % time.timezone)
                    print('local vars: %s' % locals())
                self.assertEqual(actual, expected)
                if day == wday:
                    # goes into following week
                    expected += 7 * 24 * 60 * 60
                actual = rh.computeRollover(today + 13 * 60 * 60)
                if actual != expected:
                    print('failed in timezone: %d' % time.timezone)
                    print('local vars: %s' % locals())
                self.assertEqual(actual, expected)
            finally:
                rh.close()


def secs(**kw):
    return datetime.timedelta(**kw) // datetime.timedelta(seconds=1)

for when, exp in (('S', 1),
                  ('M', 60),
                  ('H', 60 * 60),
                  ('D', 60 * 60 * 24),
                  ('MIDNIGHT', 60 * 60 * 24),
                  # current time (epoch start) is a Thursday, W0 means Monday
                  ('W0', secs(days=4, hours=24)),
                 ):
    def test_compute_rollover(self, when=when, exp=exp):
        rh = logging.handlers.TimedRotatingFileHandler(
            self.fn, when=when, interval=1, backupCount=0, utc=True)
        currentTime = 0.0
        actual = rh.computeRollover(currentTime)
        if exp != actual:
            # Failures occur on some systems for MIDNIGHT and W0.
            # Print detailed calculation for MIDNIGHT so we can try to see
            # what's going on
            if when == 'MIDNIGHT':
                try:
                    if rh.utc:
                        t = time.gmtime(currentTime)
                    else:
                        t = time.localtime(currentTime)
                    currentHour = t[3]
                    currentMinute = t[4]
                    currentSecond = t[5]
                    # r is the number of seconds left between now and midnight
                    r = logging.handlers._MIDNIGHT - ((currentHour * 60 +
                                                       currentMinute) * 60 +
                            currentSecond)
                    result = currentTime + r
                    print('t: %s (%s)' % (t, rh.utc), file=sys.stderr)
                    print('currentHour: %s' % currentHour, file=sys.stderr)
                    print('currentMinute: %s' % currentMinute, file=sys.stderr)
                    print('currentSecond: %s' % currentSecond, file=sys.stderr)
                    print('r: %s' % r, file=sys.stderr)
                    print('result: %s' % result, file=sys.stderr)
                except Exception:
                    print('exception in diagnostic code: %s' % sys.exc_info()[1], file=sys.stderr)
        self.assertEqual(exp, actual)
        rh.close()
    setattr(TimedRotatingFileHandlerTest, "test_compute_rollover_%s" % when, test_compute_rollover)


@unittest.skipUnless(win32evtlog, 'win32evtlog/win32evtlogutil required for this test.')
class NTEventLogHandlerTest(BaseTest):
    def test_basic(self):
        logtype = 'Application'
        elh = win32evtlog.OpenEventLog(None, logtype)
        num_recs = win32evtlog.GetNumberOfEventLogRecords(elh)
        h = logging.handlers.NTEventLogHandler('test_logging')
        r = logging.makeLogRecord({'msg': 'Test Log Message'})
        h.handle(r)
        h.close()
        # Now see if the event is recorded
        self.assertLess(num_recs, win32evtlog.GetNumberOfEventLogRecords(elh))
        flags = win32evtlog.EVENTLOG_BACKWARDS_READ | \
                win32evtlog.EVENTLOG_SEQUENTIAL_READ
        found = False
        GO_BACK = 100
        events = win32evtlog.ReadEventLog(elh, flags, GO_BACK)
        for e in events:
            if e.SourceName != 'test_logging':
                continue
            msg = win32evtlogutil.SafeFormatMessage(e, logtype)
            if msg != 'Test Log Message\r\n':
                continue
            found = True
            break
        msg = 'Record not found in event log, went back %d records' % GO_BACK
        self.assertTrue(found, msg=msg)

# Set the locale to the platform-dependent default.  I have no idea
# why the test does this, but in any case we save the current locale
# first and restore it at the end.
@run_with_locale('LC_ALL', '')
def test_main():
    run_unittest(BuiltinLevelsTest, BasicFilterTest,
                 CustomLevelsAndFiltersTest, HandlerTest, MemoryHandlerTest,
                 ConfigFileTest, SocketHandlerTest, DatagramHandlerTest,
                 MemoryTest, EncodingTest, WarningsTest, ConfigDictTest,
                 ManagerTest, FormatterTest, BufferingFormatterTest,
                 StreamHandlerTest, LogRecordFactoryTest, ChildLoggerTest,
                 QueueHandlerTest, ShutdownTest, ModuleLevelMiscTest,
                 BasicConfigTest, LoggerAdapterTest, LoggerTest,
                 SMTPHandlerTest, FileHandlerTest, RotatingFileHandlerTest,
                 LastResortTest, LogRecordTest, ExceptionTest,
                 SysLogHandlerTest, HTTPHandlerTest, NTEventLogHandlerTest,
                 TimedRotatingFileHandlerTest, UnixSocketHandlerTest,
                 UnixDatagramHandlerTest, UnixSysLogHandlerTest
                )

if __name__ == "__main__":
    test_main()
