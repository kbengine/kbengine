#
# Unit tests for the multiprocessing package
#

import unittest
import queue as pyqueue
import time
import io
import itertools
import sys
import os
import gc
import errno
import signal
import array
import socket
import random
import logging
import struct
import operator
import test.support
import test.script_helper


# Skip tests if _multiprocessing wasn't built.
_multiprocessing = test.support.import_module('_multiprocessing')
# Skip tests if sem_open implementation is broken.
test.support.import_module('multiprocessing.synchronize')
# import threading after _multiprocessing to raise a more revelant error
# message: "No module named _multiprocessing". _multiprocessing is not compiled
# without thread support.
import threading

import multiprocessing.dummy
import multiprocessing.connection
import multiprocessing.managers
import multiprocessing.heap
import multiprocessing.pool

from multiprocessing import util

try:
    from multiprocessing import reduction
    HAS_REDUCTION = reduction.HAVE_SEND_HANDLE
except ImportError:
    HAS_REDUCTION = False

try:
    from multiprocessing.sharedctypes import Value, copy
    HAS_SHAREDCTYPES = True
except ImportError:
    HAS_SHAREDCTYPES = False

try:
    import msvcrt
except ImportError:
    msvcrt = None

#
#
#

def latin(s):
    return s.encode('latin')

#
# Constants
#

LOG_LEVEL = util.SUBWARNING
#LOG_LEVEL = logging.DEBUG

DELTA = 0.1
CHECK_TIMINGS = False     # making true makes tests take a lot longer
                          # and can sometimes cause some non-serious
                          # failures because some calls block a bit
                          # longer than expected
if CHECK_TIMINGS:
    TIMEOUT1, TIMEOUT2, TIMEOUT3 = 0.82, 0.35, 1.4
else:
    TIMEOUT1, TIMEOUT2, TIMEOUT3 = 0.1, 0.1, 0.1

HAVE_GETVALUE = not getattr(_multiprocessing,
                            'HAVE_BROKEN_SEM_GETVALUE', False)

WIN32 = (sys.platform == "win32")

from multiprocessing.connection import wait

def wait_for_handle(handle, timeout):
    if timeout is not None and timeout < 0.0:
        timeout = None
    return wait([handle], timeout)

try:
    MAXFD = os.sysconf("SC_OPEN_MAX")
except:
    MAXFD = 256

# To speed up tests when using the forkserver, we can preload these:
PRELOAD = ['__main__', 'test.test_multiprocessing_forkserver']

#
# Some tests require ctypes
#

try:
    from ctypes import Structure, c_int, c_double
except ImportError:
    Structure = object
    c_int = c_double = None


def check_enough_semaphores():
    """Check that the system supports enough semaphores to run the test."""
    # minimum number of semaphores available according to POSIX
    nsems_min = 256
    try:
        nsems = os.sysconf("SC_SEM_NSEMS_MAX")
    except (AttributeError, ValueError):
        # sysconf not available or setting not available
        return
    if nsems == -1 or nsems >= nsems_min:
        return
    raise unittest.SkipTest("The OS doesn't support enough semaphores "
                            "to run the test (required: %d)." % nsems_min)


#
# Creates a wrapper for a function which records the time it takes to finish
#

class TimingWrapper(object):

    def __init__(self, func):
        self.func = func
        self.elapsed = None

    def __call__(self, *args, **kwds):
        t = time.time()
        try:
            return self.func(*args, **kwds)
        finally:
            self.elapsed = time.time() - t

#
# Base class for test cases
#

class BaseTestCase(object):

    ALLOWED_TYPES = ('processes', 'manager', 'threads')

    def assertTimingAlmostEqual(self, a, b):
        if CHECK_TIMINGS:
            self.assertAlmostEqual(a, b, 1)

    def assertReturnsIfImplemented(self, value, func, *args):
        try:
            res = func(*args)
        except NotImplementedError:
            pass
        else:
            return self.assertEqual(value, res)

    # For the sanity of Windows users, rather than crashing or freezing in
    # multiple ways.
    def __reduce__(self, *args):
        raise NotImplementedError("shouldn't try to pickle a test case")

    __reduce_ex__ = __reduce__

#
# Return the value of a semaphore
#

def get_value(self):
    try:
        return self.get_value()
    except AttributeError:
        try:
            return self._Semaphore__value
        except AttributeError:
            try:
                return self._value
            except AttributeError:
                raise NotImplementedError

#
# Testcases
#

class _TestProcess(BaseTestCase):

    ALLOWED_TYPES = ('processes', 'threads')

    def test_current(self):
        if self.TYPE == 'threads':
            self.skipTest('test not appropriate for {}'.format(self.TYPE))

        current = self.current_process()
        authkey = current.authkey

        self.assertTrue(current.is_alive())
        self.assertTrue(not current.daemon)
        self.assertIsInstance(authkey, bytes)
        self.assertTrue(len(authkey) > 0)
        self.assertEqual(current.ident, os.getpid())
        self.assertEqual(current.exitcode, None)

    def test_daemon_argument(self):
        if self.TYPE == "threads":
            self.skipTest('test not appropriate for {}'.format(self.TYPE))

        # By default uses the current process's daemon flag.
        proc0 = self.Process(target=self._test)
        self.assertEqual(proc0.daemon, self.current_process().daemon)
        proc1 = self.Process(target=self._test, daemon=True)
        self.assertTrue(proc1.daemon)
        proc2 = self.Process(target=self._test, daemon=False)
        self.assertFalse(proc2.daemon)

    @classmethod
    def _test(cls, q, *args, **kwds):
        current = cls.current_process()
        q.put(args)
        q.put(kwds)
        q.put(current.name)
        if cls.TYPE != 'threads':
            q.put(bytes(current.authkey))
            q.put(current.pid)

    def test_process(self):
        q = self.Queue(1)
        e = self.Event()
        args = (q, 1, 2)
        kwargs = {'hello':23, 'bye':2.54}
        name = 'SomeProcess'
        p = self.Process(
            target=self._test, args=args, kwargs=kwargs, name=name
            )
        p.daemon = True
        current = self.current_process()

        if self.TYPE != 'threads':
            self.assertEqual(p.authkey, current.authkey)
        self.assertEqual(p.is_alive(), False)
        self.assertEqual(p.daemon, True)
        self.assertNotIn(p, self.active_children())
        self.assertTrue(type(self.active_children()) is list)
        self.assertEqual(p.exitcode, None)

        p.start()

        self.assertEqual(p.exitcode, None)
        self.assertEqual(p.is_alive(), True)
        self.assertIn(p, self.active_children())

        self.assertEqual(q.get(), args[1:])
        self.assertEqual(q.get(), kwargs)
        self.assertEqual(q.get(), p.name)
        if self.TYPE != 'threads':
            self.assertEqual(q.get(), current.authkey)
            self.assertEqual(q.get(), p.pid)

        p.join()

        self.assertEqual(p.exitcode, 0)
        self.assertEqual(p.is_alive(), False)
        self.assertNotIn(p, self.active_children())

    @classmethod
    def _test_terminate(cls):
        time.sleep(100)

    def test_terminate(self):
        if self.TYPE == 'threads':
            self.skipTest('test not appropriate for {}'.format(self.TYPE))

        p = self.Process(target=self._test_terminate)
        p.daemon = True
        p.start()

        self.assertEqual(p.is_alive(), True)
        self.assertIn(p, self.active_children())
        self.assertEqual(p.exitcode, None)

        join = TimingWrapper(p.join)

        self.assertEqual(join(0), None)
        self.assertTimingAlmostEqual(join.elapsed, 0.0)
        self.assertEqual(p.is_alive(), True)

        self.assertEqual(join(-1), None)
        self.assertTimingAlmostEqual(join.elapsed, 0.0)
        self.assertEqual(p.is_alive(), True)

        # XXX maybe terminating too soon causes the problems on Gentoo...
        time.sleep(1)

        p.terminate()

        if hasattr(signal, 'alarm'):
            # On the Gentoo buildbot waitpid() often seems to block forever.
            # We use alarm() to interrupt it if it blocks for too long.
            def handler(*args):
                raise RuntimeError('join took too long: %s' % p)
            old_handler = signal.signal(signal.SIGALRM, handler)
            try:
                signal.alarm(10)
                self.assertEqual(join(), None)
            finally:
                signal.alarm(0)
                signal.signal(signal.SIGALRM, old_handler)
        else:
            self.assertEqual(join(), None)

        self.assertTimingAlmostEqual(join.elapsed, 0.0)

        self.assertEqual(p.is_alive(), False)
        self.assertNotIn(p, self.active_children())

        p.join()

        # XXX sometimes get p.exitcode == 0 on Windows ...
        #self.assertEqual(p.exitcode, -signal.SIGTERM)

    def test_cpu_count(self):
        try:
            cpus = multiprocessing.cpu_count()
        except NotImplementedError:
            cpus = 1
        self.assertTrue(type(cpus) is int)
        self.assertTrue(cpus >= 1)

    def test_active_children(self):
        self.assertEqual(type(self.active_children()), list)

        p = self.Process(target=time.sleep, args=(DELTA,))
        self.assertNotIn(p, self.active_children())

        p.daemon = True
        p.start()
        self.assertIn(p, self.active_children())

        p.join()
        self.assertNotIn(p, self.active_children())

    @classmethod
    def _test_recursion(cls, wconn, id):
        wconn.send(id)
        if len(id) < 2:
            for i in range(2):
                p = cls.Process(
                    target=cls._test_recursion, args=(wconn, id+[i])
                    )
                p.start()
                p.join()

    def test_recursion(self):
        rconn, wconn = self.Pipe(duplex=False)
        self._test_recursion(wconn, [])

        time.sleep(DELTA)
        result = []
        while rconn.poll():
            result.append(rconn.recv())

        expected = [
            [],
              [0],
                [0, 0],
                [0, 1],
              [1],
                [1, 0],
                [1, 1]
            ]
        self.assertEqual(result, expected)

    @classmethod
    def _test_sentinel(cls, event):
        event.wait(10.0)

    def test_sentinel(self):
        if self.TYPE == "threads":
            self.skipTest('test not appropriate for {}'.format(self.TYPE))
        event = self.Event()
        p = self.Process(target=self._test_sentinel, args=(event,))
        with self.assertRaises(ValueError):
            p.sentinel
        p.start()
        self.addCleanup(p.join)
        sentinel = p.sentinel
        self.assertIsInstance(sentinel, int)
        self.assertFalse(wait_for_handle(sentinel, timeout=0.0))
        event.set()
        p.join()
        self.assertTrue(wait_for_handle(sentinel, timeout=1))

#
#
#

class _UpperCaser(multiprocessing.Process):

    def __init__(self):
        multiprocessing.Process.__init__(self)
        self.child_conn, self.parent_conn = multiprocessing.Pipe()

    def run(self):
        self.parent_conn.close()
        for s in iter(self.child_conn.recv, None):
            self.child_conn.send(s.upper())
        self.child_conn.close()

    def submit(self, s):
        assert type(s) is str
        self.parent_conn.send(s)
        return self.parent_conn.recv()

    def stop(self):
        self.parent_conn.send(None)
        self.parent_conn.close()
        self.child_conn.close()

class _TestSubclassingProcess(BaseTestCase):

    ALLOWED_TYPES = ('processes',)

    def test_subclassing(self):
        uppercaser = _UpperCaser()
        uppercaser.daemon = True
        uppercaser.start()
        self.assertEqual(uppercaser.submit('hello'), 'HELLO')
        self.assertEqual(uppercaser.submit('world'), 'WORLD')
        uppercaser.stop()
        uppercaser.join()

    def test_stderr_flush(self):
        # sys.stderr is flushed at process shutdown (issue #13812)
        if self.TYPE == "threads":
            self.skipTest('test not appropriate for {}'.format(self.TYPE))

        testfn = test.support.TESTFN
        self.addCleanup(test.support.unlink, testfn)
        proc = self.Process(target=self._test_stderr_flush, args=(testfn,))
        proc.start()
        proc.join()
        with open(testfn, 'r') as f:
            err = f.read()
            # The whole traceback was printed
            self.assertIn("ZeroDivisionError", err)
            self.assertIn("test_multiprocessing.py", err)
            self.assertIn("1/0 # MARKER", err)

    @classmethod
    def _test_stderr_flush(cls, testfn):
        sys.stderr = open(testfn, 'w')
        1/0 # MARKER


    @classmethod
    def _test_sys_exit(cls, reason, testfn):
        sys.stderr = open(testfn, 'w')
        sys.exit(reason)

    def test_sys_exit(self):
        # See Issue 13854
        if self.TYPE == 'threads':
            self.skipTest('test not appropriate for {}'.format(self.TYPE))

        testfn = test.support.TESTFN
        self.addCleanup(test.support.unlink, testfn)

        for reason, code in (([1, 2, 3], 1), ('ignore this', 1)):
            p = self.Process(target=self._test_sys_exit, args=(reason, testfn))
            p.daemon = True
            p.start()
            p.join(5)
            self.assertEqual(p.exitcode, code)

            with open(testfn, 'r') as f:
                self.assertEqual(f.read().rstrip(), str(reason))

        for reason in (True, False, 8):
            p = self.Process(target=sys.exit, args=(reason,))
            p.daemon = True
            p.start()
            p.join(5)
            self.assertEqual(p.exitcode, reason)

#
#
#

def queue_empty(q):
    if hasattr(q, 'empty'):
        return q.empty()
    else:
        return q.qsize() == 0

def queue_full(q, maxsize):
    if hasattr(q, 'full'):
        return q.full()
    else:
        return q.qsize() == maxsize


class _TestQueue(BaseTestCase):


    @classmethod
    def _test_put(cls, queue, child_can_start, parent_can_continue):
        child_can_start.wait()
        for i in range(6):
            queue.get()
        parent_can_continue.set()

    def test_put(self):
        MAXSIZE = 6
        queue = self.Queue(maxsize=MAXSIZE)
        child_can_start = self.Event()
        parent_can_continue = self.Event()

        proc = self.Process(
            target=self._test_put,
            args=(queue, child_can_start, parent_can_continue)
            )
        proc.daemon = True
        proc.start()

        self.assertEqual(queue_empty(queue), True)
        self.assertEqual(queue_full(queue, MAXSIZE), False)

        queue.put(1)
        queue.put(2, True)
        queue.put(3, True, None)
        queue.put(4, False)
        queue.put(5, False, None)
        queue.put_nowait(6)

        # the values may be in buffer but not yet in pipe so sleep a bit
        time.sleep(DELTA)

        self.assertEqual(queue_empty(queue), False)
        self.assertEqual(queue_full(queue, MAXSIZE), True)

        put = TimingWrapper(queue.put)
        put_nowait = TimingWrapper(queue.put_nowait)

        self.assertRaises(pyqueue.Full, put, 7, False)
        self.assertTimingAlmostEqual(put.elapsed, 0)

        self.assertRaises(pyqueue.Full, put, 7, False, None)
        self.assertTimingAlmostEqual(put.elapsed, 0)

        self.assertRaises(pyqueue.Full, put_nowait, 7)
        self.assertTimingAlmostEqual(put_nowait.elapsed, 0)

        self.assertRaises(pyqueue.Full, put, 7, True, TIMEOUT1)
        self.assertTimingAlmostEqual(put.elapsed, TIMEOUT1)

        self.assertRaises(pyqueue.Full, put, 7, False, TIMEOUT2)
        self.assertTimingAlmostEqual(put.elapsed, 0)

        self.assertRaises(pyqueue.Full, put, 7, True, timeout=TIMEOUT3)
        self.assertTimingAlmostEqual(put.elapsed, TIMEOUT3)

        child_can_start.set()
        parent_can_continue.wait()

        self.assertEqual(queue_empty(queue), True)
        self.assertEqual(queue_full(queue, MAXSIZE), False)

        proc.join()

    @classmethod
    def _test_get(cls, queue, child_can_start, parent_can_continue):
        child_can_start.wait()
        #queue.put(1)
        queue.put(2)
        queue.put(3)
        queue.put(4)
        queue.put(5)
        parent_can_continue.set()

    def test_get(self):
        queue = self.Queue()
        child_can_start = self.Event()
        parent_can_continue = self.Event()

        proc = self.Process(
            target=self._test_get,
            args=(queue, child_can_start, parent_can_continue)
            )
        proc.daemon = True
        proc.start()

        self.assertEqual(queue_empty(queue), True)

        child_can_start.set()
        parent_can_continue.wait()

        time.sleep(DELTA)
        self.assertEqual(queue_empty(queue), False)

        # Hangs unexpectedly, remove for now
        #self.assertEqual(queue.get(), 1)
        self.assertEqual(queue.get(True, None), 2)
        self.assertEqual(queue.get(True), 3)
        self.assertEqual(queue.get(timeout=1), 4)
        self.assertEqual(queue.get_nowait(), 5)

        self.assertEqual(queue_empty(queue), True)

        get = TimingWrapper(queue.get)
        get_nowait = TimingWrapper(queue.get_nowait)

        self.assertRaises(pyqueue.Empty, get, False)
        self.assertTimingAlmostEqual(get.elapsed, 0)

        self.assertRaises(pyqueue.Empty, get, False, None)
        self.assertTimingAlmostEqual(get.elapsed, 0)

        self.assertRaises(pyqueue.Empty, get_nowait)
        self.assertTimingAlmostEqual(get_nowait.elapsed, 0)

        self.assertRaises(pyqueue.Empty, get, True, TIMEOUT1)
        self.assertTimingAlmostEqual(get.elapsed, TIMEOUT1)

        self.assertRaises(pyqueue.Empty, get, False, TIMEOUT2)
        self.assertTimingAlmostEqual(get.elapsed, 0)

        self.assertRaises(pyqueue.Empty, get, timeout=TIMEOUT3)
        self.assertTimingAlmostEqual(get.elapsed, TIMEOUT3)

        proc.join()

    @classmethod
    def _test_fork(cls, queue):
        for i in range(10, 20):
            queue.put(i)
        # note that at this point the items may only be buffered, so the
        # process cannot shutdown until the feeder thread has finished
        # pushing items onto the pipe.

    def test_fork(self):
        # Old versions of Queue would fail to create a new feeder
        # thread for a forked process if the original process had its
        # own feeder thread.  This test checks that this no longer
        # happens.

        queue = self.Queue()

        # put items on queue so that main process starts a feeder thread
        for i in range(10):
            queue.put(i)

        # wait to make sure thread starts before we fork a new process
        time.sleep(DELTA)

        # fork process
        p = self.Process(target=self._test_fork, args=(queue,))
        p.daemon = True
        p.start()

        # check that all expected items are in the queue
        for i in range(20):
            self.assertEqual(queue.get(), i)
        self.assertRaises(pyqueue.Empty, queue.get, False)

        p.join()

    def test_qsize(self):
        q = self.Queue()
        try:
            self.assertEqual(q.qsize(), 0)
        except NotImplementedError:
            self.skipTest('qsize method not implemented')
        q.put(1)
        self.assertEqual(q.qsize(), 1)
        q.put(5)
        self.assertEqual(q.qsize(), 2)
        q.get()
        self.assertEqual(q.qsize(), 1)
        q.get()
        self.assertEqual(q.qsize(), 0)

    @classmethod
    def _test_task_done(cls, q):
        for obj in iter(q.get, None):
            time.sleep(DELTA)
            q.task_done()

    def test_task_done(self):
        queue = self.JoinableQueue()

        workers = [self.Process(target=self._test_task_done, args=(queue,))
                   for i in range(4)]

        for p in workers:
            p.daemon = True
            p.start()

        for i in range(10):
            queue.put(i)

        queue.join()

        for p in workers:
            queue.put(None)

        for p in workers:
            p.join()

    def test_timeout(self):
        q = multiprocessing.Queue()
        start = time.time()
        self.assertRaises(pyqueue.Empty, q.get, True, 0.2)
        delta = time.time() - start
        self.assertGreaterEqual(delta, 0.18)

#
#
#

class _TestLock(BaseTestCase):

    def test_lock(self):
        lock = self.Lock()
        self.assertEqual(lock.acquire(), True)
        self.assertEqual(lock.acquire(False), False)
        self.assertEqual(lock.release(), None)
        self.assertRaises((ValueError, threading.ThreadError), lock.release)

    def test_rlock(self):
        lock = self.RLock()
        self.assertEqual(lock.acquire(), True)
        self.assertEqual(lock.acquire(), True)
        self.assertEqual(lock.acquire(), True)
        self.assertEqual(lock.release(), None)
        self.assertEqual(lock.release(), None)
        self.assertEqual(lock.release(), None)
        self.assertRaises((AssertionError, RuntimeError), lock.release)

    def test_lock_context(self):
        with self.Lock():
            pass


class _TestSemaphore(BaseTestCase):

    def _test_semaphore(self, sem):
        self.assertReturnsIfImplemented(2, get_value, sem)
        self.assertEqual(sem.acquire(), True)
        self.assertReturnsIfImplemented(1, get_value, sem)
        self.assertEqual(sem.acquire(), True)
        self.assertReturnsIfImplemented(0, get_value, sem)
        self.assertEqual(sem.acquire(False), False)
        self.assertReturnsIfImplemented(0, get_value, sem)
        self.assertEqual(sem.release(), None)
        self.assertReturnsIfImplemented(1, get_value, sem)
        self.assertEqual(sem.release(), None)
        self.assertReturnsIfImplemented(2, get_value, sem)

    def test_semaphore(self):
        sem = self.Semaphore(2)
        self._test_semaphore(sem)
        self.assertEqual(sem.release(), None)
        self.assertReturnsIfImplemented(3, get_value, sem)
        self.assertEqual(sem.release(), None)
        self.assertReturnsIfImplemented(4, get_value, sem)

    def test_bounded_semaphore(self):
        sem = self.BoundedSemaphore(2)
        self._test_semaphore(sem)
        # Currently fails on OS/X
        #if HAVE_GETVALUE:
        #    self.assertRaises(ValueError, sem.release)
        #    self.assertReturnsIfImplemented(2, get_value, sem)

    def test_timeout(self):
        if self.TYPE != 'processes':
            self.skipTest('test not appropriate for {}'.format(self.TYPE))

        sem = self.Semaphore(0)
        acquire = TimingWrapper(sem.acquire)

        self.assertEqual(acquire(False), False)
        self.assertTimingAlmostEqual(acquire.elapsed, 0.0)

        self.assertEqual(acquire(False, None), False)
        self.assertTimingAlmostEqual(acquire.elapsed, 0.0)

        self.assertEqual(acquire(False, TIMEOUT1), False)
        self.assertTimingAlmostEqual(acquire.elapsed, 0)

        self.assertEqual(acquire(True, TIMEOUT2), False)
        self.assertTimingAlmostEqual(acquire.elapsed, TIMEOUT2)

        self.assertEqual(acquire(timeout=TIMEOUT3), False)
        self.assertTimingAlmostEqual(acquire.elapsed, TIMEOUT3)


class _TestCondition(BaseTestCase):

    @classmethod
    def f(cls, cond, sleeping, woken, timeout=None):
        cond.acquire()
        sleeping.release()
        cond.wait(timeout)
        woken.release()
        cond.release()

    def check_invariant(self, cond):
        # this is only supposed to succeed when there are no sleepers
        if self.TYPE == 'processes':
            try:
                sleepers = (cond._sleeping_count.get_value() -
                            cond._woken_count.get_value())
                self.assertEqual(sleepers, 0)
                self.assertEqual(cond._wait_semaphore.get_value(), 0)
            except NotImplementedError:
                pass

    def test_notify(self):
        cond = self.Condition()
        sleeping = self.Semaphore(0)
        woken = self.Semaphore(0)

        p = self.Process(target=self.f, args=(cond, sleeping, woken))
        p.daemon = True
        p.start()

        p = threading.Thread(target=self.f, args=(cond, sleeping, woken))
        p.daemon = True
        p.start()

        # wait for both children to start sleeping
        sleeping.acquire()
        sleeping.acquire()

        # check no process/thread has woken up
        time.sleep(DELTA)
        self.assertReturnsIfImplemented(0, get_value, woken)

        # wake up one process/thread
        cond.acquire()
        cond.notify()
        cond.release()

        # check one process/thread has woken up
        time.sleep(DELTA)
        self.assertReturnsIfImplemented(1, get_value, woken)

        # wake up another
        cond.acquire()
        cond.notify()
        cond.release()

        # check other has woken up
        time.sleep(DELTA)
        self.assertReturnsIfImplemented(2, get_value, woken)

        # check state is not mucked up
        self.check_invariant(cond)
        p.join()

    def test_notify_all(self):
        cond = self.Condition()
        sleeping = self.Semaphore(0)
        woken = self.Semaphore(0)

        # start some threads/processes which will timeout
        for i in range(3):
            p = self.Process(target=self.f,
                             args=(cond, sleeping, woken, TIMEOUT1))
            p.daemon = True
            p.start()

            t = threading.Thread(target=self.f,
                                 args=(cond, sleeping, woken, TIMEOUT1))
            t.daemon = True
            t.start()

        # wait for them all to sleep
        for i in range(6):
            sleeping.acquire()

        # check they have all timed out
        for i in range(6):
            woken.acquire()
        self.assertReturnsIfImplemented(0, get_value, woken)

        # check state is not mucked up
        self.check_invariant(cond)

        # start some more threads/processes
        for i in range(3):
            p = self.Process(target=self.f, args=(cond, sleeping, woken))
            p.daemon = True
            p.start()

            t = threading.Thread(target=self.f, args=(cond, sleeping, woken))
            t.daemon = True
            t.start()

        # wait for them to all sleep
        for i in range(6):
            sleeping.acquire()

        # check no process/thread has woken up
        time.sleep(DELTA)
        self.assertReturnsIfImplemented(0, get_value, woken)

        # wake them all up
        cond.acquire()
        cond.notify_all()
        cond.release()

        # check they have all woken
        for i in range(10):
            try:
                if get_value(woken) == 6:
                    break
            except NotImplementedError:
                break
            time.sleep(DELTA)
        self.assertReturnsIfImplemented(6, get_value, woken)

        # check state is not mucked up
        self.check_invariant(cond)

    def test_timeout(self):
        cond = self.Condition()
        wait = TimingWrapper(cond.wait)
        cond.acquire()
        res = wait(TIMEOUT1)
        cond.release()
        self.assertEqual(res, False)
        self.assertTimingAlmostEqual(wait.elapsed, TIMEOUT1)

    @classmethod
    def _test_waitfor_f(cls, cond, state):
        with cond:
            state.value = 0
            cond.notify()
            result = cond.wait_for(lambda : state.value==4)
            if not result or state.value != 4:
                sys.exit(1)

    @unittest.skipUnless(HAS_SHAREDCTYPES, 'needs sharedctypes')
    def test_waitfor(self):
        # based on test in test/lock_tests.py
        cond = self.Condition()
        state = self.Value('i', -1)

        p = self.Process(target=self._test_waitfor_f, args=(cond, state))
        p.daemon = True
        p.start()

        with cond:
            result = cond.wait_for(lambda : state.value==0)
            self.assertTrue(result)
            self.assertEqual(state.value, 0)

        for i in range(4):
            time.sleep(0.01)
            with cond:
                state.value += 1
                cond.notify()

        p.join(5)
        self.assertFalse(p.is_alive())
        self.assertEqual(p.exitcode, 0)

    @classmethod
    def _test_waitfor_timeout_f(cls, cond, state, success, sem):
        sem.release()
        with cond:
            expected = 0.1
            dt = time.time()
            result = cond.wait_for(lambda : state.value==4, timeout=expected)
            dt = time.time() - dt
            # borrow logic in assertTimeout() from test/lock_tests.py
            if not result and expected * 0.6 < dt < expected * 10.0:
                success.value = True

    @unittest.skipUnless(HAS_SHAREDCTYPES, 'needs sharedctypes')
    def test_waitfor_timeout(self):
        # based on test in test/lock_tests.py
        cond = self.Condition()
        state = self.Value('i', 0)
        success = self.Value('i', False)
        sem = self.Semaphore(0)

        p = self.Process(target=self._test_waitfor_timeout_f,
                         args=(cond, state, success, sem))
        p.daemon = True
        p.start()
        self.assertTrue(sem.acquire(timeout=10))

        # Only increment 3 times, so state == 4 is never reached.
        for i in range(3):
            time.sleep(0.01)
            with cond:
                state.value += 1
                cond.notify()

        p.join(5)
        self.assertTrue(success.value)

    @classmethod
    def _test_wait_result(cls, c, pid):
        with c:
            c.notify()
        time.sleep(1)
        if pid is not None:
            os.kill(pid, signal.SIGINT)

    def test_wait_result(self):
        if isinstance(self, ProcessesMixin) and sys.platform != 'win32':
            pid = os.getpid()
        else:
            pid = None

        c = self.Condition()
        with c:
            self.assertFalse(c.wait(0))
            self.assertFalse(c.wait(0.1))

            p = self.Process(target=self._test_wait_result, args=(c, pid))
            p.start()

            self.assertTrue(c.wait(10))
            if pid is not None:
                self.assertRaises(KeyboardInterrupt, c.wait, 10)

            p.join()


class _TestEvent(BaseTestCase):

    @classmethod
    def _test_event(cls, event):
        time.sleep(TIMEOUT2)
        event.set()

    def test_event(self):
        event = self.Event()
        wait = TimingWrapper(event.wait)

        # Removed temporarily, due to API shear, this does not
        # work with threading._Event objects. is_set == isSet
        self.assertEqual(event.is_set(), False)

        # Removed, threading.Event.wait() will return the value of the __flag
        # instead of None. API Shear with the semaphore backed mp.Event
        self.assertEqual(wait(0.0), False)
        self.assertTimingAlmostEqual(wait.elapsed, 0.0)
        self.assertEqual(wait(TIMEOUT1), False)
        self.assertTimingAlmostEqual(wait.elapsed, TIMEOUT1)

        event.set()

        # See note above on the API differences
        self.assertEqual(event.is_set(), True)
        self.assertEqual(wait(), True)
        self.assertTimingAlmostEqual(wait.elapsed, 0.0)
        self.assertEqual(wait(TIMEOUT1), True)
        self.assertTimingAlmostEqual(wait.elapsed, 0.0)
        # self.assertEqual(event.is_set(), True)

        event.clear()

        #self.assertEqual(event.is_set(), False)

        p = self.Process(target=self._test_event, args=(event,))
        p.daemon = True
        p.start()
        self.assertEqual(wait(), True)

#
# Tests for Barrier - adapted from tests in test/lock_tests.py
#

# Many of the tests for threading.Barrier use a list as an atomic
# counter: a value is appended to increment the counter, and the
# length of the list gives the value.  We use the class DummyList
# for the same purpose.

class _DummyList(object):

    def __init__(self):
        wrapper = multiprocessing.heap.BufferWrapper(struct.calcsize('i'))
        lock = multiprocessing.Lock()
        self.__setstate__((wrapper, lock))
        self._lengthbuf[0] = 0

    def __setstate__(self, state):
        (self._wrapper, self._lock) = state
        self._lengthbuf = self._wrapper.create_memoryview().cast('i')

    def __getstate__(self):
        return (self._wrapper, self._lock)

    def append(self, _):
        with self._lock:
            self._lengthbuf[0] += 1

    def __len__(self):
        with self._lock:
            return self._lengthbuf[0]

def _wait():
    # A crude wait/yield function not relying on synchronization primitives.
    time.sleep(0.01)


class Bunch(object):
    """
    A bunch of threads.
    """
    def __init__(self, namespace, f, args, n, wait_before_exit=False):
        """
        Construct a bunch of `n` threads running the same function `f`.
        If `wait_before_exit` is True, the threads won't terminate until
        do_finish() is called.
        """
        self.f = f
        self.args = args
        self.n = n
        self.started = namespace.DummyList()
        self.finished = namespace.DummyList()
        self._can_exit = namespace.Event()
        if not wait_before_exit:
            self._can_exit.set()
        for i in range(n):
            p = namespace.Process(target=self.task)
            p.daemon = True
            p.start()

    def task(self):
        pid = os.getpid()
        self.started.append(pid)
        try:
            self.f(*self.args)
        finally:
            self.finished.append(pid)
            self._can_exit.wait(30)
            assert self._can_exit.is_set()

    def wait_for_started(self):
        while len(self.started) < self.n:
            _wait()

    def wait_for_finished(self):
        while len(self.finished) < self.n:
            _wait()

    def do_finish(self):
        self._can_exit.set()


class AppendTrue(object):
    def __init__(self, obj):
        self.obj = obj
    def __call__(self):
        self.obj.append(True)


class _TestBarrier(BaseTestCase):
    """
    Tests for Barrier objects.
    """
    N = 5
    defaultTimeout = 30.0  # XXX Slow Windows buildbots need generous timeout

    def setUp(self):
        self.barrier = self.Barrier(self.N, timeout=self.defaultTimeout)

    def tearDown(self):
        self.barrier.abort()
        self.barrier = None

    def DummyList(self):
        if self.TYPE == 'threads':
            return []
        elif self.TYPE == 'manager':
            return self.manager.list()
        else:
            return _DummyList()

    def run_threads(self, f, args):
        b = Bunch(self, f, args, self.N-1)
        f(*args)
        b.wait_for_finished()

    @classmethod
    def multipass(cls, barrier, results, n):
        m = barrier.parties
        assert m == cls.N
        for i in range(n):
            results[0].append(True)
            assert len(results[1]) == i * m
            barrier.wait()
            results[1].append(True)
            assert len(results[0]) == (i + 1) * m
            barrier.wait()
        try:
            assert barrier.n_waiting == 0
        except NotImplementedError:
            pass
        assert not barrier.broken

    def test_barrier(self, passes=1):
        """
        Test that a barrier is passed in lockstep
        """
        results = [self.DummyList(), self.DummyList()]
        self.run_threads(self.multipass, (self.barrier, results, passes))

    def test_barrier_10(self):
        """
        Test that a barrier works for 10 consecutive runs
        """
        return self.test_barrier(10)

    @classmethod
    def _test_wait_return_f(cls, barrier, queue):
        res = barrier.wait()
        queue.put(res)

    def test_wait_return(self):
        """
        test the return value from barrier.wait
        """
        queue = self.Queue()
        self.run_threads(self._test_wait_return_f, (self.barrier, queue))
        results = [queue.get() for i in range(self.N)]
        self.assertEqual(results.count(0), 1)

    @classmethod
    def _test_action_f(cls, barrier, results):
        barrier.wait()
        if len(results) != 1:
            raise RuntimeError

    def test_action(self):
        """
        Test the 'action' callback
        """
        results = self.DummyList()
        barrier = self.Barrier(self.N, action=AppendTrue(results))
        self.run_threads(self._test_action_f, (barrier, results))
        self.assertEqual(len(results), 1)

    @classmethod
    def _test_abort_f(cls, barrier, results1, results2):
        try:
            i = barrier.wait()
            if i == cls.N//2:
                raise RuntimeError
            barrier.wait()
            results1.append(True)
        except threading.BrokenBarrierError:
            results2.append(True)
        except RuntimeError:
            barrier.abort()

    def test_abort(self):
        """
        Test that an abort will put the barrier in a broken state
        """
        results1 = self.DummyList()
        results2 = self.DummyList()
        self.run_threads(self._test_abort_f,
                         (self.barrier, results1, results2))
        self.assertEqual(len(results1), 0)
        self.assertEqual(len(results2), self.N-1)
        self.assertTrue(self.barrier.broken)

    @classmethod
    def _test_reset_f(cls, barrier, results1, results2, results3):
        i = barrier.wait()
        if i == cls.N//2:
            # Wait until the other threads are all in the barrier.
            while barrier.n_waiting < cls.N-1:
                time.sleep(0.001)
            barrier.reset()
        else:
            try:
                barrier.wait()
                results1.append(True)
            except threading.BrokenBarrierError:
                results2.append(True)
        # Now, pass the barrier again
        barrier.wait()
        results3.append(True)

    def test_reset(self):
        """
        Test that a 'reset' on a barrier frees the waiting threads
        """
        results1 = self.DummyList()
        results2 = self.DummyList()
        results3 = self.DummyList()
        self.run_threads(self._test_reset_f,
                         (self.barrier, results1, results2, results3))
        self.assertEqual(len(results1), 0)
        self.assertEqual(len(results2), self.N-1)
        self.assertEqual(len(results3), self.N)

    @classmethod
    def _test_abort_and_reset_f(cls, barrier, barrier2,
                                results1, results2, results3):
        try:
            i = barrier.wait()
            if i == cls.N//2:
                raise RuntimeError
            barrier.wait()
            results1.append(True)
        except threading.BrokenBarrierError:
            results2.append(True)
        except RuntimeError:
            barrier.abort()
        # Synchronize and reset the barrier.  Must synchronize first so
        # that everyone has left it when we reset, and after so that no
        # one enters it before the reset.
        if barrier2.wait() == cls.N//2:
            barrier.reset()
        barrier2.wait()
        barrier.wait()
        results3.append(True)

    def test_abort_and_reset(self):
        """
        Test that a barrier can be reset after being broken.
        """
        results1 = self.DummyList()
        results2 = self.DummyList()
        results3 = self.DummyList()
        barrier2 = self.Barrier(self.N)

        self.run_threads(self._test_abort_and_reset_f,
                         (self.barrier, barrier2, results1, results2, results3))
        self.assertEqual(len(results1), 0)
        self.assertEqual(len(results2), self.N-1)
        self.assertEqual(len(results3), self.N)

    @classmethod
    def _test_timeout_f(cls, barrier, results):
        i = barrier.wait()
        if i == cls.N//2:
            # One thread is late!
            time.sleep(1.0)
        try:
            barrier.wait(0.5)
        except threading.BrokenBarrierError:
            results.append(True)

    def test_timeout(self):
        """
        Test wait(timeout)
        """
        results = self.DummyList()
        self.run_threads(self._test_timeout_f, (self.barrier, results))
        self.assertEqual(len(results), self.barrier.parties)

    @classmethod
    def _test_default_timeout_f(cls, barrier, results):
        i = barrier.wait(cls.defaultTimeout)
        if i == cls.N//2:
            # One thread is later than the default timeout
            time.sleep(1.0)
        try:
            barrier.wait()
        except threading.BrokenBarrierError:
            results.append(True)

    def test_default_timeout(self):
        """
        Test the barrier's default timeout
        """
        barrier = self.Barrier(self.N, timeout=0.5)
        results = self.DummyList()
        self.run_threads(self._test_default_timeout_f, (barrier, results))
        self.assertEqual(len(results), barrier.parties)

    def test_single_thread(self):
        b = self.Barrier(1)
        b.wait()
        b.wait()

    @classmethod
    def _test_thousand_f(cls, barrier, passes, conn, lock):
        for i in range(passes):
            barrier.wait()
            with lock:
                conn.send(i)

    def test_thousand(self):
        if self.TYPE == 'manager':
            self.skipTest('test not appropriate for {}'.format(self.TYPE))
        passes = 1000
        lock = self.Lock()
        conn, child_conn = self.Pipe(False)
        for j in range(self.N):
            p = self.Process(target=self._test_thousand_f,
                           args=(self.barrier, passes, child_conn, lock))
            p.start()

        for i in range(passes):
            for j in range(self.N):
                self.assertEqual(conn.recv(), i)

#
#
#

class _TestValue(BaseTestCase):

    ALLOWED_TYPES = ('processes',)

    codes_values = [
        ('i', 4343, 24234),
        ('d', 3.625, -4.25),
        ('h', -232, 234),
        ('c', latin('x'), latin('y'))
        ]

    def setUp(self):
        if not HAS_SHAREDCTYPES:
            self.skipTest("requires multiprocessing.sharedctypes")

    @classmethod
    def _test(cls, values):
        for sv, cv in zip(values, cls.codes_values):
            sv.value = cv[2]


    def test_value(self, raw=False):
        if raw:
            values = [self.RawValue(code, value)
                      for code, value, _ in self.codes_values]
        else:
            values = [self.Value(code, value)
                      for code, value, _ in self.codes_values]

        for sv, cv in zip(values, self.codes_values):
            self.assertEqual(sv.value, cv[1])

        proc = self.Process(target=self._test, args=(values,))
        proc.daemon = True
        proc.start()
        proc.join()

        for sv, cv in zip(values, self.codes_values):
            self.assertEqual(sv.value, cv[2])

    def test_rawvalue(self):
        self.test_value(raw=True)

    def test_getobj_getlock(self):
        val1 = self.Value('i', 5)
        lock1 = val1.get_lock()
        obj1 = val1.get_obj()

        val2 = self.Value('i', 5, lock=None)
        lock2 = val2.get_lock()
        obj2 = val2.get_obj()

        lock = self.Lock()
        val3 = self.Value('i', 5, lock=lock)
        lock3 = val3.get_lock()
        obj3 = val3.get_obj()
        self.assertEqual(lock, lock3)

        arr4 = self.Value('i', 5, lock=False)
        self.assertFalse(hasattr(arr4, 'get_lock'))
        self.assertFalse(hasattr(arr4, 'get_obj'))

        self.assertRaises(AttributeError, self.Value, 'i', 5, lock='navalue')

        arr5 = self.RawValue('i', 5)
        self.assertFalse(hasattr(arr5, 'get_lock'))
        self.assertFalse(hasattr(arr5, 'get_obj'))


class _TestArray(BaseTestCase):

    ALLOWED_TYPES = ('processes',)

    @classmethod
    def f(cls, seq):
        for i in range(1, len(seq)):
            seq[i] += seq[i-1]

    @unittest.skipIf(c_int is None, "requires _ctypes")
    def test_array(self, raw=False):
        seq = [680, 626, 934, 821, 150, 233, 548, 982, 714, 831]
        if raw:
            arr = self.RawArray('i', seq)
        else:
            arr = self.Array('i', seq)

        self.assertEqual(len(arr), len(seq))
        self.assertEqual(arr[3], seq[3])
        self.assertEqual(list(arr[2:7]), list(seq[2:7]))

        arr[4:8] = seq[4:8] = array.array('i', [1, 2, 3, 4])

        self.assertEqual(list(arr[:]), seq)

        self.f(seq)

        p = self.Process(target=self.f, args=(arr,))
        p.daemon = True
        p.start()
        p.join()

        self.assertEqual(list(arr[:]), seq)

    @unittest.skipIf(c_int is None, "requires _ctypes")
    def test_array_from_size(self):
        size = 10
        # Test for zeroing (see issue #11675).
        # The repetition below strengthens the test by increasing the chances
        # of previously allocated non-zero memory being used for the new array
        # on the 2nd and 3rd loops.
        for _ in range(3):
            arr = self.Array('i', size)
            self.assertEqual(len(arr), size)
            self.assertEqual(list(arr), [0] * size)
            arr[:] = range(10)
            self.assertEqual(list(arr), list(range(10)))
            del arr

    @unittest.skipIf(c_int is None, "requires _ctypes")
    def test_rawarray(self):
        self.test_array(raw=True)

    @unittest.skipIf(c_int is None, "requires _ctypes")
    def test_getobj_getlock_obj(self):
        arr1 = self.Array('i', list(range(10)))
        lock1 = arr1.get_lock()
        obj1 = arr1.get_obj()

        arr2 = self.Array('i', list(range(10)), lock=None)
        lock2 = arr2.get_lock()
        obj2 = arr2.get_obj()

        lock = self.Lock()
        arr3 = self.Array('i', list(range(10)), lock=lock)
        lock3 = arr3.get_lock()
        obj3 = arr3.get_obj()
        self.assertEqual(lock, lock3)

        arr4 = self.Array('i', range(10), lock=False)
        self.assertFalse(hasattr(arr4, 'get_lock'))
        self.assertFalse(hasattr(arr4, 'get_obj'))
        self.assertRaises(AttributeError,
                          self.Array, 'i', range(10), lock='notalock')

        arr5 = self.RawArray('i', range(10))
        self.assertFalse(hasattr(arr5, 'get_lock'))
        self.assertFalse(hasattr(arr5, 'get_obj'))

#
#
#

class _TestContainers(BaseTestCase):

    ALLOWED_TYPES = ('manager',)

    def test_list(self):
        a = self.list(list(range(10)))
        self.assertEqual(a[:], list(range(10)))

        b = self.list()
        self.assertEqual(b[:], [])

        b.extend(list(range(5)))
        self.assertEqual(b[:], list(range(5)))

        self.assertEqual(b[2], 2)
        self.assertEqual(b[2:10], [2,3,4])

        b *= 2
        self.assertEqual(b[:], [0, 1, 2, 3, 4, 0, 1, 2, 3, 4])

        self.assertEqual(b + [5, 6], [0, 1, 2, 3, 4, 0, 1, 2, 3, 4, 5, 6])

        self.assertEqual(a[:], list(range(10)))

        d = [a, b]
        e = self.list(d)
        self.assertEqual(
            e[:],
            [[0, 1, 2, 3, 4, 5, 6, 7, 8, 9], [0, 1, 2, 3, 4, 0, 1, 2, 3, 4]]
            )

        f = self.list([a])
        a.append('hello')
        self.assertEqual(f[:], [[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 'hello']])

    def test_dict(self):
        d = self.dict()
        indices = list(range(65, 70))
        for i in indices:
            d[i] = chr(i)
        self.assertEqual(d.copy(), dict((i, chr(i)) for i in indices))
        self.assertEqual(sorted(d.keys()), indices)
        self.assertEqual(sorted(d.values()), [chr(i) for i in indices])
        self.assertEqual(sorted(d.items()), [(i, chr(i)) for i in indices])

    def test_namespace(self):
        n = self.Namespace()
        n.name = 'Bob'
        n.job = 'Builder'
        n._hidden = 'hidden'
        self.assertEqual((n.name, n.job), ('Bob', 'Builder'))
        del n.job
        self.assertEqual(str(n), "Namespace(name='Bob')")
        self.assertTrue(hasattr(n, 'name'))
        self.assertTrue(not hasattr(n, 'job'))

#
#
#

def sqr(x, wait=0.0):
    time.sleep(wait)
    return x*x

def mul(x, y):
    return x*y

class _TestPool(BaseTestCase):

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls.pool = cls.Pool(4)

    @classmethod
    def tearDownClass(cls):
        cls.pool.terminate()
        cls.pool.join()
        cls.pool = None
        super().tearDownClass()

    def test_apply(self):
        papply = self.pool.apply
        self.assertEqual(papply(sqr, (5,)), sqr(5))
        self.assertEqual(papply(sqr, (), {'x':3}), sqr(x=3))

    def test_map(self):
        pmap = self.pool.map
        self.assertEqual(pmap(sqr, list(range(10))), list(map(sqr, list(range(10)))))
        self.assertEqual(pmap(sqr, list(range(100)), chunksize=20),
                         list(map(sqr, list(range(100)))))

    def test_starmap(self):
        psmap = self.pool.starmap
        tuples = list(zip(range(10), range(9,-1, -1)))
        self.assertEqual(psmap(mul, tuples),
                         list(itertools.starmap(mul, tuples)))
        tuples = list(zip(range(100), range(99,-1, -1)))
        self.assertEqual(psmap(mul, tuples, chunksize=20),
                         list(itertools.starmap(mul, tuples)))

    def test_starmap_async(self):
        tuples = list(zip(range(100), range(99,-1, -1)))
        self.assertEqual(self.pool.starmap_async(mul, tuples).get(),
                         list(itertools.starmap(mul, tuples)))

    def test_map_async(self):
        self.assertEqual(self.pool.map_async(sqr, list(range(10))).get(),
                         list(map(sqr, list(range(10)))))

    def test_map_async_callbacks(self):
        call_args = self.manager.list() if self.TYPE == 'manager' else []
        self.pool.map_async(int, ['1'],
                            callback=call_args.append,
                            error_callback=call_args.append).wait()
        self.assertEqual(1, len(call_args))
        self.assertEqual([1], call_args[0])
        self.pool.map_async(int, ['a'],
                            callback=call_args.append,
                            error_callback=call_args.append).wait()
        self.assertEqual(2, len(call_args))
        self.assertIsInstance(call_args[1], ValueError)

    def test_map_unplicklable(self):
        # Issue #19425 -- failure to pickle should not cause a hang
        if self.TYPE == 'threads':
            self.skipTest('test not appropriate for {}'.format(self.TYPE))
        class A(object):
            def __reduce__(self):
                raise RuntimeError('cannot pickle')
        with self.assertRaises(RuntimeError):
            self.pool.map(sqr, [A()]*10)

    def test_map_chunksize(self):
        try:
            self.pool.map_async(sqr, [], chunksize=1).get(timeout=TIMEOUT1)
        except multiprocessing.TimeoutError:
            self.fail("pool.map_async with chunksize stalled on null list")

    def test_async(self):
        res = self.pool.apply_async(sqr, (7, TIMEOUT1,))
        get = TimingWrapper(res.get)
        self.assertEqual(get(), 49)
        self.assertTimingAlmostEqual(get.elapsed, TIMEOUT1)

    def test_async_timeout(self):
        res = self.pool.apply_async(sqr, (6, TIMEOUT2 + 1.0))
        get = TimingWrapper(res.get)
        self.assertRaises(multiprocessing.TimeoutError, get, timeout=TIMEOUT2)
        self.assertTimingAlmostEqual(get.elapsed, TIMEOUT2)

    def test_imap(self):
        it = self.pool.imap(sqr, list(range(10)))
        self.assertEqual(list(it), list(map(sqr, list(range(10)))))

        it = self.pool.imap(sqr, list(range(10)))
        for i in range(10):
            self.assertEqual(next(it), i*i)
        self.assertRaises(StopIteration, it.__next__)

        it = self.pool.imap(sqr, list(range(1000)), chunksize=100)
        for i in range(1000):
            self.assertEqual(next(it), i*i)
        self.assertRaises(StopIteration, it.__next__)

    def test_imap_unordered(self):
        it = self.pool.imap_unordered(sqr, list(range(1000)))
        self.assertEqual(sorted(it), list(map(sqr, list(range(1000)))))

        it = self.pool.imap_unordered(sqr, list(range(1000)), chunksize=53)
        self.assertEqual(sorted(it), list(map(sqr, list(range(1000)))))

    def test_make_pool(self):
        self.assertRaises(ValueError, multiprocessing.Pool, -1)
        self.assertRaises(ValueError, multiprocessing.Pool, 0)

        p = multiprocessing.Pool(3)
        self.assertEqual(3, len(p._pool))
        p.close()
        p.join()

    def test_terminate(self):
        result = self.pool.map_async(
            time.sleep, [0.1 for i in range(10000)], chunksize=1
            )
        self.pool.terminate()
        join = TimingWrapper(self.pool.join)
        join()
        self.assertLess(join.elapsed, 0.5)

    def test_empty_iterable(self):
        # See Issue 12157
        p = self.Pool(1)

        self.assertEqual(p.map(sqr, []), [])
        self.assertEqual(list(p.imap(sqr, [])), [])
        self.assertEqual(list(p.imap_unordered(sqr, [])), [])
        self.assertEqual(p.map_async(sqr, []).get(), [])

        p.close()
        p.join()

    def test_context(self):
        if self.TYPE == 'processes':
            L = list(range(10))
            expected = [sqr(i) for i in L]
            with multiprocessing.Pool(2) as p:
                r = p.map_async(sqr, L)
                self.assertEqual(r.get(), expected)
            self.assertRaises(ValueError, p.map_async, sqr, L)

    @classmethod
    def _test_traceback(cls):
        raise RuntimeError(123) # some comment

    def test_traceback(self):
        # We want ensure that the traceback from the child process is
        # contained in the traceback raised in the main process.
        if self.TYPE == 'processes':
            with self.Pool(1) as p:
                try:
                    p.apply(self._test_traceback)
                except Exception as e:
                    exc = e
                else:
                    raise AssertionError('expected RuntimeError')
            self.assertIs(type(exc), RuntimeError)
            self.assertEqual(exc.args, (123,))
            cause = exc.__cause__
            self.assertIs(type(cause), multiprocessing.pool.RemoteTraceback)
            self.assertIn('raise RuntimeError(123) # some comment', cause.tb)

            with test.support.captured_stderr() as f1:
                try:
                    raise exc
                except RuntimeError:
                    sys.excepthook(*sys.exc_info())
            self.assertIn('raise RuntimeError(123) # some comment',
                          f1.getvalue())

    @classmethod
    def _test_wrapped_exception(cls):
        raise RuntimeError('foo')

    def test_wrapped_exception(self):
        # Issue #20980: Should not wrap exception when using thread pool
        with self.Pool(1) as p:
            with self.assertRaises(RuntimeError):
                p.apply(self._test_wrapped_exception)


def raising():
    raise KeyError("key")

def unpickleable_result():
    return lambda: 42

class _TestPoolWorkerErrors(BaseTestCase):
    ALLOWED_TYPES = ('processes', )

    def test_async_error_callback(self):
        p = multiprocessing.Pool(2)

        scratchpad = [None]
        def errback(exc):
            scratchpad[0] = exc

        res = p.apply_async(raising, error_callback=errback)
        self.assertRaises(KeyError, res.get)
        self.assertTrue(scratchpad[0])
        self.assertIsInstance(scratchpad[0], KeyError)

        p.close()
        p.join()

    def test_unpickleable_result(self):
        from multiprocessing.pool import MaybeEncodingError
        p = multiprocessing.Pool(2)

        # Make sure we don't lose pool processes because of encoding errors.
        for iteration in range(20):

            scratchpad = [None]
            def errback(exc):
                scratchpad[0] = exc

            res = p.apply_async(unpickleable_result, error_callback=errback)
            self.assertRaises(MaybeEncodingError, res.get)
            wrapped = scratchpad[0]
            self.assertTrue(wrapped)
            self.assertIsInstance(scratchpad[0], MaybeEncodingError)
            self.assertIsNotNone(wrapped.exc)
            self.assertIsNotNone(wrapped.value)

        p.close()
        p.join()

class _TestPoolWorkerLifetime(BaseTestCase):
    ALLOWED_TYPES = ('processes', )

    def test_pool_worker_lifetime(self):
        p = multiprocessing.Pool(3, maxtasksperchild=10)
        self.assertEqual(3, len(p._pool))
        origworkerpids = [w.pid for w in p._pool]
        # Run many tasks so each worker gets replaced (hopefully)
        results = []
        for i in range(100):
            results.append(p.apply_async(sqr, (i, )))
        # Fetch the results and verify we got the right answers,
        # also ensuring all the tasks have completed.
        for (j, res) in enumerate(results):
            self.assertEqual(res.get(), sqr(j))
        # Refill the pool
        p._repopulate_pool()
        # Wait until all workers are alive
        # (countdown * DELTA = 5 seconds max startup process time)
        countdown = 50
        while countdown and not all(w.is_alive() for w in p._pool):
            countdown -= 1
            time.sleep(DELTA)
        finalworkerpids = [w.pid for w in p._pool]
        # All pids should be assigned.  See issue #7805.
        self.assertNotIn(None, origworkerpids)
        self.assertNotIn(None, finalworkerpids)
        # Finally, check that the worker pids have changed
        self.assertNotEqual(sorted(origworkerpids), sorted(finalworkerpids))
        p.close()
        p.join()

    def test_pool_worker_lifetime_early_close(self):
        # Issue #10332: closing a pool whose workers have limited lifetimes
        # before all the tasks completed would make join() hang.
        p = multiprocessing.Pool(3, maxtasksperchild=1)
        results = []
        for i in range(6):
            results.append(p.apply_async(sqr, (i, 0.3)))
        p.close()
        p.join()
        # check the results
        for (j, res) in enumerate(results):
            self.assertEqual(res.get(), sqr(j))

#
# Test of creating a customized manager class
#

from multiprocessing.managers import BaseManager, BaseProxy, RemoteError

class FooBar(object):
    def f(self):
        return 'f()'
    def g(self):
        raise ValueError
    def _h(self):
        return '_h()'

def baz():
    for i in range(10):
        yield i*i

class IteratorProxy(BaseProxy):
    _exposed_ = ('__next__',)
    def __iter__(self):
        return self
    def __next__(self):
        return self._callmethod('__next__')

class MyManager(BaseManager):
    pass

MyManager.register('Foo', callable=FooBar)
MyManager.register('Bar', callable=FooBar, exposed=('f', '_h'))
MyManager.register('baz', callable=baz, proxytype=IteratorProxy)


class _TestMyManager(BaseTestCase):

    ALLOWED_TYPES = ('manager',)

    def test_mymanager(self):
        manager = MyManager()
        manager.start()
        self.common(manager)
        manager.shutdown()

        # If the manager process exited cleanly then the exitcode
        # will be zero.  Otherwise (after a short timeout)
        # terminate() is used, resulting in an exitcode of -SIGTERM.
        self.assertEqual(manager._process.exitcode, 0)

    def test_mymanager_context(self):
        with MyManager() as manager:
            self.common(manager)
        self.assertEqual(manager._process.exitcode, 0)

    def test_mymanager_context_prestarted(self):
        manager = MyManager()
        manager.start()
        with manager:
            self.common(manager)
        self.assertEqual(manager._process.exitcode, 0)

    def common(self, manager):
        foo = manager.Foo()
        bar = manager.Bar()
        baz = manager.baz()

        foo_methods = [name for name in ('f', 'g', '_h') if hasattr(foo, name)]
        bar_methods = [name for name in ('f', 'g', '_h') if hasattr(bar, name)]

        self.assertEqual(foo_methods, ['f', 'g'])
        self.assertEqual(bar_methods, ['f', '_h'])

        self.assertEqual(foo.f(), 'f()')
        self.assertRaises(ValueError, foo.g)
        self.assertEqual(foo._callmethod('f'), 'f()')
        self.assertRaises(RemoteError, foo._callmethod, '_h')

        self.assertEqual(bar.f(), 'f()')
        self.assertEqual(bar._h(), '_h()')
        self.assertEqual(bar._callmethod('f'), 'f()')
        self.assertEqual(bar._callmethod('_h'), '_h()')

        self.assertEqual(list(baz), [i*i for i in range(10)])


#
# Test of connecting to a remote server and using xmlrpclib for serialization
#

_queue = pyqueue.Queue()
def get_queue():
    return _queue

class QueueManager(BaseManager):
    '''manager class used by server process'''
QueueManager.register('get_queue', callable=get_queue)

class QueueManager2(BaseManager):
    '''manager class which specifies the same interface as QueueManager'''
QueueManager2.register('get_queue')


SERIALIZER = 'xmlrpclib'

class _TestRemoteManager(BaseTestCase):

    ALLOWED_TYPES = ('manager',)

    @classmethod
    def _putter(cls, address, authkey):
        manager = QueueManager2(
            address=address, authkey=authkey, serializer=SERIALIZER
            )
        manager.connect()
        queue = manager.get_queue()
        queue.put(('hello world', None, True, 2.25))

    def test_remote(self):
        authkey = os.urandom(32)

        manager = QueueManager(
            address=(test.support.HOST, 0), authkey=authkey, serializer=SERIALIZER
            )
        manager.start()

        p = self.Process(target=self._putter, args=(manager.address, authkey))
        p.daemon = True
        p.start()

        manager2 = QueueManager2(
            address=manager.address, authkey=authkey, serializer=SERIALIZER
            )
        manager2.connect()
        queue = manager2.get_queue()

        # Note that xmlrpclib will deserialize object as a list not a tuple
        self.assertEqual(queue.get(), ['hello world', None, True, 2.25])

        # Because we are using xmlrpclib for serialization instead of
        # pickle this will cause a serialization error.
        self.assertRaises(Exception, queue.put, time.sleep)

        # Make queue finalizer run before the server is stopped
        del queue
        manager.shutdown()

class _TestManagerRestart(BaseTestCase):

    @classmethod
    def _putter(cls, address, authkey):
        manager = QueueManager(
            address=address, authkey=authkey, serializer=SERIALIZER)
        manager.connect()
        queue = manager.get_queue()
        queue.put('hello world')

    def test_rapid_restart(self):
        authkey = os.urandom(32)
        manager = QueueManager(
            address=(test.support.HOST, 0), authkey=authkey, serializer=SERIALIZER)
        srvr = manager.get_server()
        addr = srvr.address
        # Close the connection.Listener socket which gets opened as a part
        # of manager.get_server(). It's not needed for the test.
        srvr.listener.close()
        manager.start()

        p = self.Process(target=self._putter, args=(manager.address, authkey))
        p.daemon = True
        p.start()
        queue = manager.get_queue()
        self.assertEqual(queue.get(), 'hello world')
        del queue
        manager.shutdown()
        manager = QueueManager(
            address=addr, authkey=authkey, serializer=SERIALIZER)
        try:
            manager.start()
        except OSError as e:
            if e.errno != errno.EADDRINUSE:
                raise
            # Retry after some time, in case the old socket was lingering
            # (sporadic failure on buildbots)
            time.sleep(1.0)
            manager = QueueManager(
                address=addr, authkey=authkey, serializer=SERIALIZER)
        manager.shutdown()

#
#
#

SENTINEL = latin('')

class _TestConnection(BaseTestCase):

    ALLOWED_TYPES = ('processes', 'threads')

    @classmethod
    def _echo(cls, conn):
        for msg in iter(conn.recv_bytes, SENTINEL):
            conn.send_bytes(msg)
        conn.close()

    def test_connection(self):
        conn, child_conn = self.Pipe()

        p = self.Process(target=self._echo, args=(child_conn,))
        p.daemon = True
        p.start()

        seq = [1, 2.25, None]
        msg = latin('hello world')
        longmsg = msg * 10
        arr = array.array('i', list(range(4)))

        if self.TYPE == 'processes':
            self.assertEqual(type(conn.fileno()), int)

        self.assertEqual(conn.send(seq), None)
        self.assertEqual(conn.recv(), seq)

        self.assertEqual(conn.send_bytes(msg), None)
        self.assertEqual(conn.recv_bytes(), msg)

        if self.TYPE == 'processes':
            buffer = array.array('i', [0]*10)
            expected = list(arr) + [0] * (10 - len(arr))
            self.assertEqual(conn.send_bytes(arr), None)
            self.assertEqual(conn.recv_bytes_into(buffer),
                             len(arr) * buffer.itemsize)
            self.assertEqual(list(buffer), expected)

            buffer = array.array('i', [0]*10)
            expected = [0] * 3 + list(arr) + [0] * (10 - 3 - len(arr))
            self.assertEqual(conn.send_bytes(arr), None)
            self.assertEqual(conn.recv_bytes_into(buffer, 3 * buffer.itemsize),
                             len(arr) * buffer.itemsize)
            self.assertEqual(list(buffer), expected)

            buffer = bytearray(latin(' ' * 40))
            self.assertEqual(conn.send_bytes(longmsg), None)
            try:
                res = conn.recv_bytes_into(buffer)
            except multiprocessing.BufferTooShort as e:
                self.assertEqual(e.args, (longmsg,))
            else:
                self.fail('expected BufferTooShort, got %s' % res)

        poll = TimingWrapper(conn.poll)

        self.assertEqual(poll(), False)
        self.assertTimingAlmostEqual(poll.elapsed, 0)

        self.assertEqual(poll(-1), False)
        self.assertTimingAlmostEqual(poll.elapsed, 0)

        self.assertEqual(poll(TIMEOUT1), False)
        self.assertTimingAlmostEqual(poll.elapsed, TIMEOUT1)

        conn.send(None)
        time.sleep(.1)

        self.assertEqual(poll(TIMEOUT1), True)
        self.assertTimingAlmostEqual(poll.elapsed, 0)

        self.assertEqual(conn.recv(), None)

        really_big_msg = latin('X') * (1024 * 1024 * 16)   # 16Mb
        conn.send_bytes(really_big_msg)
        self.assertEqual(conn.recv_bytes(), really_big_msg)

        conn.send_bytes(SENTINEL)                          # tell child to quit
        child_conn.close()

        if self.TYPE == 'processes':
            self.assertEqual(conn.readable, True)
            self.assertEqual(conn.writable, True)
            self.assertRaises(EOFError, conn.recv)
            self.assertRaises(EOFError, conn.recv_bytes)

        p.join()

    def test_duplex_false(self):
        reader, writer = self.Pipe(duplex=False)
        self.assertEqual(writer.send(1), None)
        self.assertEqual(reader.recv(), 1)
        if self.TYPE == 'processes':
            self.assertEqual(reader.readable, True)
            self.assertEqual(reader.writable, False)
            self.assertEqual(writer.readable, False)
            self.assertEqual(writer.writable, True)
            self.assertRaises(OSError, reader.send, 2)
            self.assertRaises(OSError, writer.recv)
            self.assertRaises(OSError, writer.poll)

    def test_spawn_close(self):
        # We test that a pipe connection can be closed by parent
        # process immediately after child is spawned.  On Windows this
        # would have sometimes failed on old versions because
        # child_conn would be closed before the child got a chance to
        # duplicate it.
        conn, child_conn = self.Pipe()

        p = self.Process(target=self._echo, args=(child_conn,))
        p.daemon = True
        p.start()
        child_conn.close()    # this might complete before child initializes

        msg = latin('hello')
        conn.send_bytes(msg)
        self.assertEqual(conn.recv_bytes(), msg)

        conn.send_bytes(SENTINEL)
        conn.close()
        p.join()

    def test_sendbytes(self):
        if self.TYPE != 'processes':
            self.skipTest('test not appropriate for {}'.format(self.TYPE))

        msg = latin('abcdefghijklmnopqrstuvwxyz')
        a, b = self.Pipe()

        a.send_bytes(msg)
        self.assertEqual(b.recv_bytes(), msg)

        a.send_bytes(msg, 5)
        self.assertEqual(b.recv_bytes(), msg[5:])

        a.send_bytes(msg, 7, 8)
        self.assertEqual(b.recv_bytes(), msg[7:7+8])

        a.send_bytes(msg, 26)
        self.assertEqual(b.recv_bytes(), latin(''))

        a.send_bytes(msg, 26, 0)
        self.assertEqual(b.recv_bytes(), latin(''))

        self.assertRaises(ValueError, a.send_bytes, msg, 27)

        self.assertRaises(ValueError, a.send_bytes, msg, 22, 5)

        self.assertRaises(ValueError, a.send_bytes, msg, 26, 1)

        self.assertRaises(ValueError, a.send_bytes, msg, -1)

        self.assertRaises(ValueError, a.send_bytes, msg, 4, -1)

    @classmethod
    def _is_fd_assigned(cls, fd):
        try:
            os.fstat(fd)
        except OSError as e:
            if e.errno == errno.EBADF:
                return False
            raise
        else:
            return True

    @classmethod
    def _writefd(cls, conn, data, create_dummy_fds=False):
        if create_dummy_fds:
            for i in range(0, 256):
                if not cls._is_fd_assigned(i):
                    os.dup2(conn.fileno(), i)
        fd = reduction.recv_handle(conn)
        if msvcrt:
            fd = msvcrt.open_osfhandle(fd, os.O_WRONLY)
        os.write(fd, data)
        os.close(fd)

    @unittest.skipUnless(HAS_REDUCTION, "test needs multiprocessing.reduction")
    def test_fd_transfer(self):
        if self.TYPE != 'processes':
            self.skipTest("only makes sense with processes")
        conn, child_conn = self.Pipe(duplex=True)

        p = self.Process(target=self._writefd, args=(child_conn, b"foo"))
        p.daemon = True
        p.start()
        self.addCleanup(test.support.unlink, test.support.TESTFN)
        with open(test.support.TESTFN, "wb") as f:
            fd = f.fileno()
            if msvcrt:
                fd = msvcrt.get_osfhandle(fd)
            reduction.send_handle(conn, fd, p.pid)
        p.join()
        with open(test.support.TESTFN, "rb") as f:
            self.assertEqual(f.read(), b"foo")

    @unittest.skipUnless(HAS_REDUCTION, "test needs multiprocessing.reduction")
    @unittest.skipIf(sys.platform == "win32",
                     "test semantics don't make sense on Windows")
    @unittest.skipIf(MAXFD <= 256,
                     "largest assignable fd number is too small")
    @unittest.skipUnless(hasattr(os, "dup2"),
                         "test needs os.dup2()")
    def test_large_fd_transfer(self):
        # With fd > 256 (issue #11657)
        if self.TYPE != 'processes':
            self.skipTest("only makes sense with processes")
        conn, child_conn = self.Pipe(duplex=True)

        p = self.Process(target=self._writefd, args=(child_conn, b"bar", True))
        p.daemon = True
        p.start()
        self.addCleanup(test.support.unlink, test.support.TESTFN)
        with open(test.support.TESTFN, "wb") as f:
            fd = f.fileno()
            for newfd in range(256, MAXFD):
                if not self._is_fd_assigned(newfd):
                    break
            else:
                self.fail("could not find an unassigned large file descriptor")
            os.dup2(fd, newfd)
            try:
                reduction.send_handle(conn, newfd, p.pid)
            finally:
                os.close(newfd)
        p.join()
        with open(test.support.TESTFN, "rb") as f:
            self.assertEqual(f.read(), b"bar")

    @classmethod
    def _send_data_without_fd(self, conn):
        os.write(conn.fileno(), b"\0")

    @unittest.skipUnless(HAS_REDUCTION, "test needs multiprocessing.reduction")
    @unittest.skipIf(sys.platform == "win32", "doesn't make sense on Windows")
    def test_missing_fd_transfer(self):
        # Check that exception is raised when received data is not
        # accompanied by a file descriptor in ancillary data.
        if self.TYPE != 'processes':
            self.skipTest("only makes sense with processes")
        conn, child_conn = self.Pipe(duplex=True)

        p = self.Process(target=self._send_data_without_fd, args=(child_conn,))
        p.daemon = True
        p.start()
        self.assertRaises(RuntimeError, reduction.recv_handle, conn)
        p.join()

    def test_context(self):
        a, b = self.Pipe()

        with a, b:
            a.send(1729)
            self.assertEqual(b.recv(), 1729)
            if self.TYPE == 'processes':
                self.assertFalse(a.closed)
                self.assertFalse(b.closed)

        if self.TYPE == 'processes':
            self.assertTrue(a.closed)
            self.assertTrue(b.closed)
            self.assertRaises(OSError, a.recv)
            self.assertRaises(OSError, b.recv)

class _TestListener(BaseTestCase):

    ALLOWED_TYPES = ('processes',)

    def test_multiple_bind(self):
        for family in self.connection.families:
            l = self.connection.Listener(family=family)
            self.addCleanup(l.close)
            self.assertRaises(OSError, self.connection.Listener,
                              l.address, family)

    def test_context(self):
        with self.connection.Listener() as l:
            with self.connection.Client(l.address) as c:
                with l.accept() as d:
                    c.send(1729)
                    self.assertEqual(d.recv(), 1729)

        if self.TYPE == 'processes':
            self.assertRaises(OSError, l.accept)

class _TestListenerClient(BaseTestCase):

    ALLOWED_TYPES = ('processes', 'threads')

    @classmethod
    def _test(cls, address):
        conn = cls.connection.Client(address)
        conn.send('hello')
        conn.close()

    def test_listener_client(self):
        for family in self.connection.families:
            l = self.connection.Listener(family=family)
            p = self.Process(target=self._test, args=(l.address,))
            p.daemon = True
            p.start()
            conn = l.accept()
            self.assertEqual(conn.recv(), 'hello')
            p.join()
            l.close()

    def test_issue14725(self):
        l = self.connection.Listener()
        p = self.Process(target=self._test, args=(l.address,))
        p.daemon = True
        p.start()
        time.sleep(1)
        # On Windows the client process should by now have connected,
        # written data and closed the pipe handle by now.  This causes
        # ConnectNamdedPipe() to fail with ERROR_NO_DATA.  See Issue
        # 14725.
        conn = l.accept()
        self.assertEqual(conn.recv(), 'hello')
        conn.close()
        p.join()
        l.close()

    def test_issue16955(self):
        for fam in self.connection.families:
            l = self.connection.Listener(family=fam)
            c = self.connection.Client(l.address)
            a = l.accept()
            a.send_bytes(b"hello")
            self.assertTrue(c.poll(1))
            a.close()
            c.close()
            l.close()

class _TestPoll(BaseTestCase):

    ALLOWED_TYPES = ('processes', 'threads')

    def test_empty_string(self):
        a, b = self.Pipe()
        self.assertEqual(a.poll(), False)
        b.send_bytes(b'')
        self.assertEqual(a.poll(), True)
        self.assertEqual(a.poll(), True)

    @classmethod
    def _child_strings(cls, conn, strings):
        for s in strings:
            time.sleep(0.1)
            conn.send_bytes(s)
        conn.close()

    def test_strings(self):
        strings = (b'hello', b'', b'a', b'b', b'', b'bye', b'', b'lop')
        a, b = self.Pipe()
        p = self.Process(target=self._child_strings, args=(b, strings))
        p.start()

        for s in strings:
            for i in range(200):
                if a.poll(0.01):
                    break
            x = a.recv_bytes()
            self.assertEqual(s, x)

        p.join()

    @classmethod
    def _child_boundaries(cls, r):
        # Polling may "pull" a message in to the child process, but we
        # don't want it to pull only part of a message, as that would
        # corrupt the pipe for any other processes which might later
        # read from it.
        r.poll(5)

    def test_boundaries(self):
        r, w = self.Pipe(False)
        p = self.Process(target=self._child_boundaries, args=(r,))
        p.start()
        time.sleep(2)
        L = [b"first", b"second"]
        for obj in L:
            w.send_bytes(obj)
        w.close()
        p.join()
        self.assertIn(r.recv_bytes(), L)

    @classmethod
    def _child_dont_merge(cls, b):
        b.send_bytes(b'a')
        b.send_bytes(b'b')
        b.send_bytes(b'cd')

    def test_dont_merge(self):
        a, b = self.Pipe()
        self.assertEqual(a.poll(0.0), False)
        self.assertEqual(a.poll(0.1), False)

        p = self.Process(target=self._child_dont_merge, args=(b,))
        p.start()

        self.assertEqual(a.recv_bytes(), b'a')
        self.assertEqual(a.poll(1.0), True)
        self.assertEqual(a.poll(1.0), True)
        self.assertEqual(a.recv_bytes(), b'b')
        self.assertEqual(a.poll(1.0), True)
        self.assertEqual(a.poll(1.0), True)
        self.assertEqual(a.poll(0.0), True)
        self.assertEqual(a.recv_bytes(), b'cd')

        p.join()

#
# Test of sending connection and socket objects between processes
#

@unittest.skipUnless(HAS_REDUCTION, "test needs multiprocessing.reduction")
class _TestPicklingConnections(BaseTestCase):

    ALLOWED_TYPES = ('processes',)

    @classmethod
    def tearDownClass(cls):
        from multiprocessing import resource_sharer
        resource_sharer.stop(timeout=5)

    @classmethod
    def _listener(cls, conn, families):
        for fam in families:
            l = cls.connection.Listener(family=fam)
            conn.send(l.address)
            new_conn = l.accept()
            conn.send(new_conn)
            new_conn.close()
            l.close()

        l = socket.socket()
        l.bind((test.support.HOST, 0))
        l.listen(1)
        conn.send(l.getsockname())
        new_conn, addr = l.accept()
        conn.send(new_conn)
        new_conn.close()
        l.close()

        conn.recv()

    @classmethod
    def _remote(cls, conn):
        for (address, msg) in iter(conn.recv, None):
            client = cls.connection.Client(address)
            client.send(msg.upper())
            client.close()

        address, msg = conn.recv()
        client = socket.socket()
        client.connect(address)
        client.sendall(msg.upper())
        client.close()

        conn.close()

    def test_pickling(self):
        families = self.connection.families

        lconn, lconn0 = self.Pipe()
        lp = self.Process(target=self._listener, args=(lconn0, families))
        lp.daemon = True
        lp.start()
        lconn0.close()

        rconn, rconn0 = self.Pipe()
        rp = self.Process(target=self._remote, args=(rconn0,))
        rp.daemon = True
        rp.start()
        rconn0.close()

        for fam in families:
            msg = ('This connection uses family %s' % fam).encode('ascii')
            address = lconn.recv()
            rconn.send((address, msg))
            new_conn = lconn.recv()
            self.assertEqual(new_conn.recv(), msg.upper())

        rconn.send(None)

        msg = latin('This connection uses a normal socket')
        address = lconn.recv()
        rconn.send((address, msg))
        new_conn = lconn.recv()
        buf = []
        while True:
            s = new_conn.recv(100)
            if not s:
                break
            buf.append(s)
        buf = b''.join(buf)
        self.assertEqual(buf, msg.upper())
        new_conn.close()

        lconn.send(None)

        rconn.close()
        lconn.close()

        lp.join()
        rp.join()

    @classmethod
    def child_access(cls, conn):
        w = conn.recv()
        w.send('all is well')
        w.close()

        r = conn.recv()
        msg = r.recv()
        conn.send(msg*2)

        conn.close()

    def test_access(self):
        # On Windows, if we do not specify a destination pid when
        # using DupHandle then we need to be careful to use the
        # correct access flags for DuplicateHandle(), or else
        # DupHandle.detach() will raise PermissionError.  For example,
        # for a read only pipe handle we should use
        # access=FILE_GENERIC_READ.  (Unfortunately
        # DUPLICATE_SAME_ACCESS does not work.)
        conn, child_conn = self.Pipe()
        p = self.Process(target=self.child_access, args=(child_conn,))
        p.daemon = True
        p.start()
        child_conn.close()

        r, w = self.Pipe(duplex=False)
        conn.send(w)
        w.close()
        self.assertEqual(r.recv(), 'all is well')
        r.close()

        r, w = self.Pipe(duplex=False)
        conn.send(r)
        r.close()
        w.send('foobar')
        w.close()
        self.assertEqual(conn.recv(), 'foobar'*2)

#
#
#

class _TestHeap(BaseTestCase):

    ALLOWED_TYPES = ('processes',)

    def test_heap(self):
        iterations = 5000
        maxblocks = 50
        blocks = []

        # create and destroy lots of blocks of different sizes
        for i in range(iterations):
            size = int(random.lognormvariate(0, 1) * 1000)
            b = multiprocessing.heap.BufferWrapper(size)
            blocks.append(b)
            if len(blocks) > maxblocks:
                i = random.randrange(maxblocks)
                del blocks[i]

        # get the heap object
        heap = multiprocessing.heap.BufferWrapper._heap

        # verify the state of the heap
        all = []
        occupied = 0
        heap._lock.acquire()
        self.addCleanup(heap._lock.release)
        for L in list(heap._len_to_seq.values()):
            for arena, start, stop in L:
                all.append((heap._arenas.index(arena), start, stop,
                            stop-start, 'free'))
        for arena, start, stop in heap._allocated_blocks:
            all.append((heap._arenas.index(arena), start, stop,
                        stop-start, 'occupied'))
            occupied += (stop-start)

        all.sort()

        for i in range(len(all)-1):
            (arena, start, stop) = all[i][:3]
            (narena, nstart, nstop) = all[i+1][:3]
            self.assertTrue((arena != narena and nstart == 0) or
                            (stop == nstart))

    def test_free_from_gc(self):
        # Check that freeing of blocks by the garbage collector doesn't deadlock
        # (issue #12352).
        # Make sure the GC is enabled, and set lower collection thresholds to
        # make collections more frequent (and increase the probability of
        # deadlock).
        if not gc.isenabled():
            gc.enable()
            self.addCleanup(gc.disable)
        thresholds = gc.get_threshold()
        self.addCleanup(gc.set_threshold, *thresholds)
        gc.set_threshold(10)

        # perform numerous block allocations, with cyclic references to make
        # sure objects are collected asynchronously by the gc
        for i in range(5000):
            a = multiprocessing.heap.BufferWrapper(1)
            b = multiprocessing.heap.BufferWrapper(1)
            # circular references
            a.buddy = b
            b.buddy = a

#
#
#

class _Foo(Structure):
    _fields_ = [
        ('x', c_int),
        ('y', c_double)
        ]

class _TestSharedCTypes(BaseTestCase):

    ALLOWED_TYPES = ('processes',)

    def setUp(self):
        if not HAS_SHAREDCTYPES:
            self.skipTest("requires multiprocessing.sharedctypes")

    @classmethod
    def _double(cls, x, y, foo, arr, string):
        x.value *= 2
        y.value *= 2
        foo.x *= 2
        foo.y *= 2
        string.value *= 2
        for i in range(len(arr)):
            arr[i] *= 2

    def test_sharedctypes(self, lock=False):
        x = Value('i', 7, lock=lock)
        y = Value(c_double, 1.0/3.0, lock=lock)
        foo = Value(_Foo, 3, 2, lock=lock)
        arr = self.Array('d', list(range(10)), lock=lock)
        string = self.Array('c', 20, lock=lock)
        string.value = latin('hello')

        p = self.Process(target=self._double, args=(x, y, foo, arr, string))
        p.daemon = True
        p.start()
        p.join()

        self.assertEqual(x.value, 14)
        self.assertAlmostEqual(y.value, 2.0/3.0)
        self.assertEqual(foo.x, 6)
        self.assertAlmostEqual(foo.y, 4.0)
        for i in range(10):
            self.assertAlmostEqual(arr[i], i*2)
        self.assertEqual(string.value, latin('hellohello'))

    def test_synchronize(self):
        self.test_sharedctypes(lock=True)

    def test_copy(self):
        foo = _Foo(2, 5.0)
        bar = copy(foo)
        foo.x = 0
        foo.y = 0
        self.assertEqual(bar.x, 2)
        self.assertAlmostEqual(bar.y, 5.0)

#
#
#

class _TestFinalize(BaseTestCase):

    ALLOWED_TYPES = ('processes',)

    @classmethod
    def _test_finalize(cls, conn):
        class Foo(object):
            pass

        a = Foo()
        util.Finalize(a, conn.send, args=('a',))
        del a           # triggers callback for a

        b = Foo()
        close_b = util.Finalize(b, conn.send, args=('b',))
        close_b()       # triggers callback for b
        close_b()       # does nothing because callback has already been called
        del b           # does nothing because callback has already been called

        c = Foo()
        util.Finalize(c, conn.send, args=('c',))

        d10 = Foo()
        util.Finalize(d10, conn.send, args=('d10',), exitpriority=1)

        d01 = Foo()
        util.Finalize(d01, conn.send, args=('d01',), exitpriority=0)
        d02 = Foo()
        util.Finalize(d02, conn.send, args=('d02',), exitpriority=0)
        d03 = Foo()
        util.Finalize(d03, conn.send, args=('d03',), exitpriority=0)

        util.Finalize(None, conn.send, args=('e',), exitpriority=-10)

        util.Finalize(None, conn.send, args=('STOP',), exitpriority=-100)

        # call multiprocessing's cleanup function then exit process without
        # garbage collecting locals
        util._exit_function()
        conn.close()
        os._exit(0)

    def test_finalize(self):
        conn, child_conn = self.Pipe()

        p = self.Process(target=self._test_finalize, args=(child_conn,))
        p.daemon = True
        p.start()
        p.join()

        result = [obj for obj in iter(conn.recv, 'STOP')]
        self.assertEqual(result, ['a', 'b', 'd10', 'd03', 'd02', 'd01', 'e'])

#
# Test that from ... import * works for each module
#

class _TestImportStar(unittest.TestCase):

    def get_module_names(self):
        import glob
        folder = os.path.dirname(multiprocessing.__file__)
        pattern = os.path.join(folder, '*.py')
        files = glob.glob(pattern)
        modules = [os.path.splitext(os.path.split(f)[1])[0] for f in files]
        modules = ['multiprocessing.' + m for m in modules]
        modules.remove('multiprocessing.__init__')
        modules.append('multiprocessing')
        return modules

    def test_import(self):
        modules = self.get_module_names()
        if sys.platform == 'win32':
            modules.remove('multiprocessing.popen_fork')
            modules.remove('multiprocessing.popen_forkserver')
            modules.remove('multiprocessing.popen_spawn_posix')
        else:
            modules.remove('multiprocessing.popen_spawn_win32')
            if not HAS_REDUCTION:
                modules.remove('multiprocessing.popen_forkserver')

        if c_int is None:
            # This module requires _ctypes
            modules.remove('multiprocessing.sharedctypes')

        for name in modules:
            __import__(name)
            mod = sys.modules[name]
            self.assertTrue(hasattr(mod, '__all__'), name)

            for attr in mod.__all__:
                self.assertTrue(
                    hasattr(mod, attr),
                    '%r does not have attribute %r' % (mod, attr)
                    )

#
# Quick test that logging works -- does not test logging output
#

class _TestLogging(BaseTestCase):

    ALLOWED_TYPES = ('processes',)

    def test_enable_logging(self):
        logger = multiprocessing.get_logger()
        logger.setLevel(util.SUBWARNING)
        self.assertTrue(logger is not None)
        logger.debug('this will not be printed')
        logger.info('nor will this')
        logger.setLevel(LOG_LEVEL)

    @classmethod
    def _test_level(cls, conn):
        logger = multiprocessing.get_logger()
        conn.send(logger.getEffectiveLevel())

    def test_level(self):
        LEVEL1 = 32
        LEVEL2 = 37

        logger = multiprocessing.get_logger()
        root_logger = logging.getLogger()
        root_level = root_logger.level

        reader, writer = multiprocessing.Pipe(duplex=False)

        logger.setLevel(LEVEL1)
        p = self.Process(target=self._test_level, args=(writer,))
        p.daemon = True
        p.start()
        self.assertEqual(LEVEL1, reader.recv())

        logger.setLevel(logging.NOTSET)
        root_logger.setLevel(LEVEL2)
        p = self.Process(target=self._test_level, args=(writer,))
        p.daemon = True
        p.start()
        self.assertEqual(LEVEL2, reader.recv())

        root_logger.setLevel(root_level)
        logger.setLevel(level=LOG_LEVEL)


# class _TestLoggingProcessName(BaseTestCase):
#
#     def handle(self, record):
#         assert record.processName == multiprocessing.current_process().name
#         self.__handled = True
#
#     def test_logging(self):
#         handler = logging.Handler()
#         handler.handle = self.handle
#         self.__handled = False
#         # Bypass getLogger() and side-effects
#         logger = logging.getLoggerClass()(
#                 'multiprocessing.test.TestLoggingProcessName')
#         logger.addHandler(handler)
#         logger.propagate = False
#
#         logger.warn('foo')
#         assert self.__handled

#
# Check that Process.join() retries if os.waitpid() fails with EINTR
#

class _TestPollEintr(BaseTestCase):

    ALLOWED_TYPES = ('processes',)

    @classmethod
    def _killer(cls, pid):
        time.sleep(0.1)
        os.kill(pid, signal.SIGUSR1)

    @unittest.skipUnless(hasattr(signal, 'SIGUSR1'), 'requires SIGUSR1')
    def test_poll_eintr(self):
        got_signal = [False]
        def record(*args):
            got_signal[0] = True
        pid = os.getpid()
        oldhandler = signal.signal(signal.SIGUSR1, record)
        try:
            killer = self.Process(target=self._killer, args=(pid,))
            killer.start()
            try:
                p = self.Process(target=time.sleep, args=(2,))
                p.start()
                p.join()
            finally:
                killer.join()
            self.assertTrue(got_signal[0])
            self.assertEqual(p.exitcode, 0)
        finally:
            signal.signal(signal.SIGUSR1, oldhandler)

#
# Test to verify handle verification, see issue 3321
#

class TestInvalidHandle(unittest.TestCase):

    @unittest.skipIf(WIN32, "skipped on Windows")
    def test_invalid_handles(self):
        conn = multiprocessing.connection.Connection(44977608)
        # check that poll() doesn't crash
        try:
            conn.poll()
        except (ValueError, OSError):
            pass
        finally:
            # Hack private attribute _handle to avoid printing an error
            # in conn.__del__
            conn._handle = None
        self.assertRaises((ValueError, OSError),
                          multiprocessing.connection.Connection, -1)



class OtherTest(unittest.TestCase):
    # TODO: add more tests for deliver/answer challenge.
    def test_deliver_challenge_auth_failure(self):
        class _FakeConnection(object):
            def recv_bytes(self, size):
                return b'something bogus'
            def send_bytes(self, data):
                pass
        self.assertRaises(multiprocessing.AuthenticationError,
                          multiprocessing.connection.deliver_challenge,
                          _FakeConnection(), b'abc')

    def test_answer_challenge_auth_failure(self):
        class _FakeConnection(object):
            def __init__(self):
                self.count = 0
            def recv_bytes(self, size):
                self.count += 1
                if self.count == 1:
                    return multiprocessing.connection.CHALLENGE
                elif self.count == 2:
                    return b'something bogus'
                return b''
            def send_bytes(self, data):
                pass
        self.assertRaises(multiprocessing.AuthenticationError,
                          multiprocessing.connection.answer_challenge,
                          _FakeConnection(), b'abc')

#
# Test Manager.start()/Pool.__init__() initializer feature - see issue 5585
#

def initializer(ns):
    ns.test += 1

class TestInitializers(unittest.TestCase):
    def setUp(self):
        self.mgr = multiprocessing.Manager()
        self.ns = self.mgr.Namespace()
        self.ns.test = 0

    def tearDown(self):
        self.mgr.shutdown()
        self.mgr.join()

    def test_manager_initializer(self):
        m = multiprocessing.managers.SyncManager()
        self.assertRaises(TypeError, m.start, 1)
        m.start(initializer, (self.ns,))
        self.assertEqual(self.ns.test, 1)
        m.shutdown()
        m.join()

    def test_pool_initializer(self):
        self.assertRaises(TypeError, multiprocessing.Pool, initializer=1)
        p = multiprocessing.Pool(1, initializer, (self.ns,))
        p.close()
        p.join()
        self.assertEqual(self.ns.test, 1)

#
# Issue 5155, 5313, 5331: Test process in processes
# Verifies os.close(sys.stdin.fileno) vs. sys.stdin.close() behavior
#

def _this_sub_process(q):
    try:
        item = q.get(block=False)
    except pyqueue.Empty:
        pass

def _test_process(q):
    queue = multiprocessing.Queue()
    subProc = multiprocessing.Process(target=_this_sub_process, args=(queue,))
    subProc.daemon = True
    subProc.start()
    subProc.join()

def _afunc(x):
    return x*x

def pool_in_process():
    pool = multiprocessing.Pool(processes=4)
    x = pool.map(_afunc, [1, 2, 3, 4, 5, 6, 7])
    pool.close()
    pool.join()

class _file_like(object):
    def __init__(self, delegate):
        self._delegate = delegate
        self._pid = None

    @property
    def cache(self):
        pid = os.getpid()
        # There are no race conditions since fork keeps only the running thread
        if pid != self._pid:
            self._pid = pid
            self._cache = []
        return self._cache

    def write(self, data):
        self.cache.append(data)

    def flush(self):
        self._delegate.write(''.join(self.cache))
        self._cache = []

class TestStdinBadfiledescriptor(unittest.TestCase):

    def test_queue_in_process(self):
        queue = multiprocessing.Queue()
        proc = multiprocessing.Process(target=_test_process, args=(queue,))
        proc.start()
        proc.join()

    def test_pool_in_process(self):
        p = multiprocessing.Process(target=pool_in_process)
        p.start()
        p.join()

    def test_flushing(self):
        sio = io.StringIO()
        flike = _file_like(sio)
        flike.write('foo')
        proc = multiprocessing.Process(target=lambda: flike.flush())
        flike.flush()
        assert sio.getvalue() == 'foo'


class TestWait(unittest.TestCase):

    @classmethod
    def _child_test_wait(cls, w, slow):
        for i in range(10):
            if slow:
                time.sleep(random.random()*0.1)
            w.send((i, os.getpid()))
        w.close()

    def test_wait(self, slow=False):
        from multiprocessing.connection import wait
        readers = []
        procs = []
        messages = []

        for i in range(4):
            r, w = multiprocessing.Pipe(duplex=False)
            p = multiprocessing.Process(target=self._child_test_wait, args=(w, slow))
            p.daemon = True
            p.start()
            w.close()
            readers.append(r)
            procs.append(p)
            self.addCleanup(p.join)

        while readers:
            for r in wait(readers):
                try:
                    msg = r.recv()
                except EOFError:
                    readers.remove(r)
                    r.close()
                else:
                    messages.append(msg)

        messages.sort()
        expected = sorted((i, p.pid) for i in range(10) for p in procs)
        self.assertEqual(messages, expected)

    @classmethod
    def _child_test_wait_socket(cls, address, slow):
        s = socket.socket()
        s.connect(address)
        for i in range(10):
            if slow:
                time.sleep(random.random()*0.1)
            s.sendall(('%s\n' % i).encode('ascii'))
        s.close()

    def test_wait_socket(self, slow=False):
        from multiprocessing.connection import wait
        l = socket.socket()
        l.bind((test.support.HOST, 0))
        l.listen(4)
        addr = l.getsockname()
        readers = []
        procs = []
        dic = {}

        for i in range(4):
            p = multiprocessing.Process(target=self._child_test_wait_socket,
                                        args=(addr, slow))
            p.daemon = True
            p.start()
            procs.append(p)
            self.addCleanup(p.join)

        for i in range(4):
            r, _ = l.accept()
            readers.append(r)
            dic[r] = []
        l.close()

        while readers:
            for r in wait(readers):
                msg = r.recv(32)
                if not msg:
                    readers.remove(r)
                    r.close()
                else:
                    dic[r].append(msg)

        expected = ''.join('%s\n' % i for i in range(10)).encode('ascii')
        for v in dic.values():
            self.assertEqual(b''.join(v), expected)

    def test_wait_slow(self):
        self.test_wait(True)

    def test_wait_socket_slow(self):
        self.test_wait_socket(True)

    def test_wait_timeout(self):
        from multiprocessing.connection import wait

        expected = 5
        a, b = multiprocessing.Pipe()

        start = time.time()
        res = wait([a, b], expected)
        delta = time.time() - start

        self.assertEqual(res, [])
        self.assertLess(delta, expected * 2)
        self.assertGreater(delta, expected * 0.5)

        b.send(None)

        start = time.time()
        res = wait([a, b], 20)
        delta = time.time() - start

        self.assertEqual(res, [a])
        self.assertLess(delta, 0.4)

    @classmethod
    def signal_and_sleep(cls, sem, period):
        sem.release()
        time.sleep(period)

    def test_wait_integer(self):
        from multiprocessing.connection import wait

        expected = 3
        sorted_ = lambda l: sorted(l, key=lambda x: id(x))
        sem = multiprocessing.Semaphore(0)
        a, b = multiprocessing.Pipe()
        p = multiprocessing.Process(target=self.signal_and_sleep,
                                    args=(sem, expected))

        p.start()
        self.assertIsInstance(p.sentinel, int)
        self.assertTrue(sem.acquire(timeout=20))

        start = time.time()
        res = wait([a, p.sentinel, b], expected + 20)
        delta = time.time() - start

        self.assertEqual(res, [p.sentinel])
        self.assertLess(delta, expected + 2)
        self.assertGreater(delta, expected - 2)

        a.send(None)

        start = time.time()
        res = wait([a, p.sentinel, b], 20)
        delta = time.time() - start

        self.assertEqual(sorted_(res), sorted_([p.sentinel, b]))
        self.assertLess(delta, 0.4)

        b.send(None)

        start = time.time()
        res = wait([a, p.sentinel, b], 20)
        delta = time.time() - start

        self.assertEqual(sorted_(res), sorted_([a, p.sentinel, b]))
        self.assertLess(delta, 0.4)

        p.terminate()
        p.join()

    def test_neg_timeout(self):
        from multiprocessing.connection import wait
        a, b = multiprocessing.Pipe()
        t = time.time()
        res = wait([a], timeout=-1)
        t = time.time() - t
        self.assertEqual(res, [])
        self.assertLess(t, 1)
        a.close()
        b.close()

#
# Issue 14151: Test invalid family on invalid environment
#

class TestInvalidFamily(unittest.TestCase):

    @unittest.skipIf(WIN32, "skipped on Windows")
    def test_invalid_family(self):
        with self.assertRaises(ValueError):
            multiprocessing.connection.Listener(r'\\.\test')

    @unittest.skipUnless(WIN32, "skipped on non-Windows platforms")
    def test_invalid_family_win32(self):
        with self.assertRaises(ValueError):
            multiprocessing.connection.Listener('/var/test.pipe')

#
# Issue 12098: check sys.flags of child matches that for parent
#

class TestFlags(unittest.TestCase):
    @classmethod
    def run_in_grandchild(cls, conn):
        conn.send(tuple(sys.flags))

    @classmethod
    def run_in_child(cls):
        import json
        r, w = multiprocessing.Pipe(duplex=False)
        p = multiprocessing.Process(target=cls.run_in_grandchild, args=(w,))
        p.start()
        grandchild_flags = r.recv()
        p.join()
        r.close()
        w.close()
        flags = (tuple(sys.flags), grandchild_flags)
        print(json.dumps(flags))

    def test_flags(self):
        import json, subprocess
        # start child process using unusual flags
        prog = ('from test._test_multiprocessing import TestFlags; ' +
                'TestFlags.run_in_child()')
        data = subprocess.check_output(
            [sys.executable, '-E', '-S', '-O', '-c', prog])
        child_flags, grandchild_flags = json.loads(data.decode('ascii'))
        self.assertEqual(child_flags, grandchild_flags)

#
# Test interaction with socket timeouts - see Issue #6056
#

class TestTimeouts(unittest.TestCase):
    @classmethod
    def _test_timeout(cls, child, address):
        time.sleep(1)
        child.send(123)
        child.close()
        conn = multiprocessing.connection.Client(address)
        conn.send(456)
        conn.close()

    def test_timeout(self):
        old_timeout = socket.getdefaulttimeout()
        try:
            socket.setdefaulttimeout(0.1)
            parent, child = multiprocessing.Pipe(duplex=True)
            l = multiprocessing.connection.Listener(family='AF_INET')
            p = multiprocessing.Process(target=self._test_timeout,
                                        args=(child, l.address))
            p.start()
            child.close()
            self.assertEqual(parent.recv(), 123)
            parent.close()
            conn = l.accept()
            self.assertEqual(conn.recv(), 456)
            conn.close()
            l.close()
            p.join(10)
        finally:
            socket.setdefaulttimeout(old_timeout)

#
# Test what happens with no "if __name__ == '__main__'"
#

class TestNoForkBomb(unittest.TestCase):
    def test_noforkbomb(self):
        sm = multiprocessing.get_start_method()
        name = os.path.join(os.path.dirname(__file__), 'mp_fork_bomb.py')
        if sm != 'fork':
            rc, out, err = test.script_helper.assert_python_failure(name, sm)
            self.assertEqual('', out.decode('ascii'))
            self.assertIn('RuntimeError', err.decode('ascii'))
        else:
            rc, out, err = test.script_helper.assert_python_ok(name, sm)
            self.assertEqual('123', out.decode('ascii').rstrip())
            self.assertEqual('', err.decode('ascii'))

#
# Issue #17555: ForkAwareThreadLock
#

class TestForkAwareThreadLock(unittest.TestCase):
    # We recurisvely start processes.  Issue #17555 meant that the
    # after fork registry would get duplicate entries for the same
    # lock.  The size of the registry at generation n was ~2**n.

    @classmethod
    def child(cls, n, conn):
        if n > 1:
            p = multiprocessing.Process(target=cls.child, args=(n-1, conn))
            p.start()
            conn.close()
            p.join(timeout=5)
        else:
            conn.send(len(util._afterfork_registry))
        conn.close()

    def test_lock(self):
        r, w = multiprocessing.Pipe(False)
        l = util.ForkAwareThreadLock()
        old_size = len(util._afterfork_registry)
        p = multiprocessing.Process(target=self.child, args=(5, w))
        p.start()
        w.close()
        new_size = r.recv()
        p.join(timeout=5)
        self.assertLessEqual(new_size, old_size)

#
# Check that non-forked child processes do not inherit unneeded fds/handles
#

class TestCloseFds(unittest.TestCase):

    def get_high_socket_fd(self):
        if WIN32:
            # The child process will not have any socket handles, so
            # calling socket.fromfd() should produce WSAENOTSOCK even
            # if there is a handle of the same number.
            return socket.socket().detach()
        else:
            # We want to produce a socket with an fd high enough that a
            # freshly created child process will not have any fds as high.
            fd = socket.socket().detach()
            to_close = []
            while fd < 50:
                to_close.append(fd)
                fd = os.dup(fd)
            for x in to_close:
                os.close(x)
            return fd

    def close(self, fd):
        if WIN32:
            socket.socket(fileno=fd).close()
        else:
            os.close(fd)

    @classmethod
    def _test_closefds(cls, conn, fd):
        try:
            s = socket.fromfd(fd, socket.AF_INET, socket.SOCK_STREAM)
        except Exception as e:
            conn.send(e)
        else:
            s.close()
            conn.send(None)

    def test_closefd(self):
        if not HAS_REDUCTION:
            raise unittest.SkipTest('requires fd pickling')

        reader, writer = multiprocessing.Pipe()
        fd = self.get_high_socket_fd()
        try:
            p = multiprocessing.Process(target=self._test_closefds,
                                        args=(writer, fd))
            p.start()
            writer.close()
            e = reader.recv()
            p.join(timeout=5)
        finally:
            self.close(fd)
            writer.close()
            reader.close()

        if multiprocessing.get_start_method() == 'fork':
            self.assertIs(e, None)
        else:
            WSAENOTSOCK = 10038
            self.assertIsInstance(e, OSError)
            self.assertTrue(e.errno == errno.EBADF or
                            e.winerror == WSAENOTSOCK, e)

#
# Issue #17097: EINTR should be ignored by recv(), send(), accept() etc
#

class TestIgnoreEINTR(unittest.TestCase):

    @classmethod
    def _test_ignore(cls, conn):
        def handler(signum, frame):
            pass
        signal.signal(signal.SIGUSR1, handler)
        conn.send('ready')
        x = conn.recv()
        conn.send(x)
        conn.send_bytes(b'x'*(1024*1024))   # sending 1 MB should block

    @unittest.skipUnless(hasattr(signal, 'SIGUSR1'), 'requires SIGUSR1')
    def test_ignore(self):
        conn, child_conn = multiprocessing.Pipe()
        try:
            p = multiprocessing.Process(target=self._test_ignore,
                                        args=(child_conn,))
            p.daemon = True
            p.start()
            child_conn.close()
            self.assertEqual(conn.recv(), 'ready')
            time.sleep(0.1)
            os.kill(p.pid, signal.SIGUSR1)
            time.sleep(0.1)
            conn.send(1234)
            self.assertEqual(conn.recv(), 1234)
            time.sleep(0.1)
            os.kill(p.pid, signal.SIGUSR1)
            self.assertEqual(conn.recv_bytes(), b'x'*(1024*1024))
            time.sleep(0.1)
            p.join()
        finally:
            conn.close()

    @classmethod
    def _test_ignore_listener(cls, conn):
        def handler(signum, frame):
            pass
        signal.signal(signal.SIGUSR1, handler)
        with multiprocessing.connection.Listener() as l:
            conn.send(l.address)
            a = l.accept()
            a.send('welcome')

    @unittest.skipUnless(hasattr(signal, 'SIGUSR1'), 'requires SIGUSR1')
    def test_ignore_listener(self):
        conn, child_conn = multiprocessing.Pipe()
        try:
            p = multiprocessing.Process(target=self._test_ignore_listener,
                                        args=(child_conn,))
            p.daemon = True
            p.start()
            child_conn.close()
            address = conn.recv()
            time.sleep(0.1)
            os.kill(p.pid, signal.SIGUSR1)
            time.sleep(0.1)
            client = multiprocessing.connection.Client(address)
            self.assertEqual(client.recv(), 'welcome')
            p.join()
        finally:
            conn.close()

class TestStartMethod(unittest.TestCase):
    @classmethod
    def _check_context(cls, conn):
        conn.send(multiprocessing.get_start_method())

    def check_context(self, ctx):
        r, w = ctx.Pipe(duplex=False)
        p = ctx.Process(target=self._check_context, args=(w,))
        p.start()
        w.close()
        child_method = r.recv()
        r.close()
        p.join()
        self.assertEqual(child_method, ctx.get_start_method())

    def test_context(self):
        for method in ('fork', 'spawn', 'forkserver'):
            try:
                ctx = multiprocessing.get_context(method)
            except ValueError:
                continue
            self.assertEqual(ctx.get_start_method(), method)
            self.assertIs(ctx.get_context(), ctx)
            self.assertRaises(ValueError, ctx.set_start_method, 'spawn')
            self.assertRaises(ValueError, ctx.set_start_method, None)
            self.check_context(ctx)

    def test_set_get(self):
        multiprocessing.set_forkserver_preload(PRELOAD)
        count = 0
        old_method = multiprocessing.get_start_method()
        try:
            for method in ('fork', 'spawn', 'forkserver'):
                try:
                    multiprocessing.set_start_method(method, force=True)
                except ValueError:
                    continue
                self.assertEqual(multiprocessing.get_start_method(), method)
                ctx = multiprocessing.get_context()
                self.assertEqual(ctx.get_start_method(), method)
                self.assertTrue(type(ctx).__name__.lower().startswith(method))
                self.assertTrue(
                    ctx.Process.__name__.lower().startswith(method))
                self.check_context(multiprocessing)
                count += 1
        finally:
            multiprocessing.set_start_method(old_method, force=True)
        self.assertGreaterEqual(count, 1)

    def test_get_all(self):
        methods = multiprocessing.get_all_start_methods()
        if sys.platform == 'win32':
            self.assertEqual(methods, ['spawn'])
        else:
            self.assertTrue(methods == ['fork', 'spawn'] or
                            methods == ['fork', 'spawn', 'forkserver'])

#
# Check that killing process does not leak named semaphores
#

@unittest.skipIf(sys.platform == "win32",
                 "test semantics don't make sense on Windows")
class TestSemaphoreTracker(unittest.TestCase):
    def test_semaphore_tracker(self):
        import subprocess
        cmd = '''if 1:
            import multiprocessing as mp, time, os
            mp.set_start_method("spawn")
            lock1 = mp.Lock()
            lock2 = mp.Lock()
            os.write(%d, lock1._semlock.name.encode("ascii") + b"\\n")
            os.write(%d, lock2._semlock.name.encode("ascii") + b"\\n")
            time.sleep(10)
        '''
        r, w = os.pipe()
        p = subprocess.Popen([sys.executable,
                             '-c', cmd % (w, w)],
                             pass_fds=[w],
                             stderr=subprocess.PIPE)
        os.close(w)
        with open(r, 'rb', closefd=True) as f:
            name1 = f.readline().rstrip().decode('ascii')
            name2 = f.readline().rstrip().decode('ascii')
        _multiprocessing.sem_unlink(name1)
        p.terminate()
        p.wait()
        time.sleep(2.0)
        with self.assertRaises(OSError) as ctx:
            _multiprocessing.sem_unlink(name2)
        # docs say it should be ENOENT, but OSX seems to give EINVAL
        self.assertIn(ctx.exception.errno, (errno.ENOENT, errno.EINVAL))
        err = p.stderr.read().decode('utf-8')
        p.stderr.close()
        expected = 'semaphore_tracker: There appear to be 2 leaked semaphores'
        self.assertRegex(err, expected)
        self.assertRegex(err, 'semaphore_tracker: %r: \[Errno' % name1)

#
# Mixins
#

class ProcessesMixin(object):
    TYPE = 'processes'
    Process = multiprocessing.Process
    connection = multiprocessing.connection
    current_process = staticmethod(multiprocessing.current_process)
    active_children = staticmethod(multiprocessing.active_children)
    Pool = staticmethod(multiprocessing.Pool)
    Pipe = staticmethod(multiprocessing.Pipe)
    Queue = staticmethod(multiprocessing.Queue)
    JoinableQueue = staticmethod(multiprocessing.JoinableQueue)
    Lock = staticmethod(multiprocessing.Lock)
    RLock = staticmethod(multiprocessing.RLock)
    Semaphore = staticmethod(multiprocessing.Semaphore)
    BoundedSemaphore = staticmethod(multiprocessing.BoundedSemaphore)
    Condition = staticmethod(multiprocessing.Condition)
    Event = staticmethod(multiprocessing.Event)
    Barrier = staticmethod(multiprocessing.Barrier)
    Value = staticmethod(multiprocessing.Value)
    Array = staticmethod(multiprocessing.Array)
    RawValue = staticmethod(multiprocessing.RawValue)
    RawArray = staticmethod(multiprocessing.RawArray)


class ManagerMixin(object):
    TYPE = 'manager'
    Process = multiprocessing.Process
    Queue = property(operator.attrgetter('manager.Queue'))
    JoinableQueue = property(operator.attrgetter('manager.JoinableQueue'))
    Lock = property(operator.attrgetter('manager.Lock'))
    RLock = property(operator.attrgetter('manager.RLock'))
    Semaphore = property(operator.attrgetter('manager.Semaphore'))
    BoundedSemaphore = property(operator.attrgetter('manager.BoundedSemaphore'))
    Condition = property(operator.attrgetter('manager.Condition'))
    Event = property(operator.attrgetter('manager.Event'))
    Barrier = property(operator.attrgetter('manager.Barrier'))
    Value = property(operator.attrgetter('manager.Value'))
    Array = property(operator.attrgetter('manager.Array'))
    list = property(operator.attrgetter('manager.list'))
    dict = property(operator.attrgetter('manager.dict'))
    Namespace = property(operator.attrgetter('manager.Namespace'))

    @classmethod
    def Pool(cls, *args, **kwds):
        return cls.manager.Pool(*args, **kwds)

    @classmethod
    def setUpClass(cls):
        cls.manager = multiprocessing.Manager()

    @classmethod
    def tearDownClass(cls):
        # only the manager process should be returned by active_children()
        # but this can take a bit on slow machines, so wait a few seconds
        # if there are other children too (see #17395)
        t = 0.01
        while len(multiprocessing.active_children()) > 1 and t < 5:
            time.sleep(t)
            t *= 2
        gc.collect()                       # do garbage collection
        if cls.manager._number_of_objects() != 0:
            # This is not really an error since some tests do not
            # ensure that all processes which hold a reference to a
            # managed object have been joined.
            print('Shared objects which still exist at manager shutdown:')
            print(cls.manager._debug_info())
        cls.manager.shutdown()
        cls.manager.join()
        cls.manager = None


class ThreadsMixin(object):
    TYPE = 'threads'
    Process = multiprocessing.dummy.Process
    connection = multiprocessing.dummy.connection
    current_process = staticmethod(multiprocessing.dummy.current_process)
    active_children = staticmethod(multiprocessing.dummy.active_children)
    Pool = staticmethod(multiprocessing.Pool)
    Pipe = staticmethod(multiprocessing.dummy.Pipe)
    Queue = staticmethod(multiprocessing.dummy.Queue)
    JoinableQueue = staticmethod(multiprocessing.dummy.JoinableQueue)
    Lock = staticmethod(multiprocessing.dummy.Lock)
    RLock = staticmethod(multiprocessing.dummy.RLock)
    Semaphore = staticmethod(multiprocessing.dummy.Semaphore)
    BoundedSemaphore = staticmethod(multiprocessing.dummy.BoundedSemaphore)
    Condition = staticmethod(multiprocessing.dummy.Condition)
    Event = staticmethod(multiprocessing.dummy.Event)
    Barrier = staticmethod(multiprocessing.dummy.Barrier)
    Value = staticmethod(multiprocessing.dummy.Value)
    Array = staticmethod(multiprocessing.dummy.Array)

#
# Functions used to create test cases from the base ones in this module
#

def install_tests_in_module_dict(remote_globs, start_method):
    __module__ = remote_globs['__name__']
    local_globs = globals()
    ALL_TYPES = {'processes', 'threads', 'manager'}

    for name, base in local_globs.items():
        if not isinstance(base, type):
            continue
        if issubclass(base, BaseTestCase):
            if base is BaseTestCase:
                continue
            assert set(base.ALLOWED_TYPES) <= ALL_TYPES, base.ALLOWED_TYPES
            for type_ in base.ALLOWED_TYPES:
                newname = 'With' + type_.capitalize() + name[1:]
                Mixin = local_globs[type_.capitalize() + 'Mixin']
                class Temp(base, Mixin, unittest.TestCase):
                    pass
                Temp.__name__ = Temp.__qualname__ = newname
                Temp.__module__ = __module__
                remote_globs[newname] = Temp
        elif issubclass(base, unittest.TestCase):
            class Temp(base, object):
                pass
            Temp.__name__ = Temp.__qualname__ = name
            Temp.__module__ = __module__
            remote_globs[name] = Temp

    dangling = [None, None]
    old_start_method = [None]

    def setUpModule():
        multiprocessing.set_forkserver_preload(PRELOAD)
        multiprocessing.process._cleanup()
        dangling[0] = multiprocessing.process._dangling.copy()
        dangling[1] = threading._dangling.copy()
        old_start_method[0] = multiprocessing.get_start_method(allow_none=True)
        try:
            multiprocessing.set_start_method(start_method, force=True)
        except ValueError:
            raise unittest.SkipTest(start_method +
                                    ' start method not supported')

        if sys.platform.startswith("linux"):
            try:
                lock = multiprocessing.RLock()
            except OSError:
                raise unittest.SkipTest("OSError raises on RLock creation, "
                                        "see issue 3111!")
        check_enough_semaphores()
        util.get_temp_dir()     # creates temp directory
        multiprocessing.get_logger().setLevel(LOG_LEVEL)

    def tearDownModule():
        multiprocessing.set_start_method(old_start_method[0], force=True)
        # pause a bit so we don't get warning about dangling threads/processes
        time.sleep(0.5)
        multiprocessing.process._cleanup()
        gc.collect()
        tmp = set(multiprocessing.process._dangling) - set(dangling[0])
        if tmp:
            print('Dangling processes:', tmp, file=sys.stderr)
        del tmp
        tmp = set(threading._dangling) - set(dangling[1])
        if tmp:
            print('Dangling threads:', tmp, file=sys.stderr)

    remote_globs['setUpModule'] = setUpModule
    remote_globs['tearDownModule'] = tearDownModule
