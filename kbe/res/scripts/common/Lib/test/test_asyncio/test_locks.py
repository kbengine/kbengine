"""Tests for lock.py"""

import unittest
from unittest import mock
import re

import asyncio
from asyncio import test_utils


STR_RGX_REPR = (
    r'^<(?P<class>.*?) object at (?P<address>.*?)'
    r'\[(?P<extras>'
    r'(set|unset|locked|unlocked)(,value:\d)?(,waiters:\d+)?'
    r')\]>\Z'
)
RGX_REPR = re.compile(STR_RGX_REPR)


class LockTests(unittest.TestCase):

    def setUp(self):
        self.loop = test_utils.TestLoop()
        asyncio.set_event_loop(None)

    def tearDown(self):
        self.loop.close()

    def test_ctor_loop(self):
        loop = mock.Mock()
        lock = asyncio.Lock(loop=loop)
        self.assertIs(lock._loop, loop)

        lock = asyncio.Lock(loop=self.loop)
        self.assertIs(lock._loop, self.loop)

    def test_ctor_noloop(self):
        try:
            asyncio.set_event_loop(self.loop)
            lock = asyncio.Lock()
            self.assertIs(lock._loop, self.loop)
        finally:
            asyncio.set_event_loop(None)

    def test_repr(self):
        lock = asyncio.Lock(loop=self.loop)
        self.assertTrue(repr(lock).endswith('[unlocked]>'))
        self.assertTrue(RGX_REPR.match(repr(lock)))

        @asyncio.coroutine
        def acquire_lock():
            yield from lock

        self.loop.run_until_complete(acquire_lock())
        self.assertTrue(repr(lock).endswith('[locked]>'))
        self.assertTrue(RGX_REPR.match(repr(lock)))

    def test_lock(self):
        lock = asyncio.Lock(loop=self.loop)

        @asyncio.coroutine
        def acquire_lock():
            return (yield from lock)

        res = self.loop.run_until_complete(acquire_lock())

        self.assertTrue(res)
        self.assertTrue(lock.locked())

        lock.release()
        self.assertFalse(lock.locked())

    def test_acquire(self):
        lock = asyncio.Lock(loop=self.loop)
        result = []

        self.assertTrue(self.loop.run_until_complete(lock.acquire()))

        @asyncio.coroutine
        def c1(result):
            if (yield from lock.acquire()):
                result.append(1)
            return True

        @asyncio.coroutine
        def c2(result):
            if (yield from lock.acquire()):
                result.append(2)
            return True

        @asyncio.coroutine
        def c3(result):
            if (yield from lock.acquire()):
                result.append(3)
            return True

        t1 = asyncio.Task(c1(result), loop=self.loop)
        t2 = asyncio.Task(c2(result), loop=self.loop)

        test_utils.run_briefly(self.loop)
        self.assertEqual([], result)

        lock.release()
        test_utils.run_briefly(self.loop)
        self.assertEqual([1], result)

        test_utils.run_briefly(self.loop)
        self.assertEqual([1], result)

        t3 = asyncio.Task(c3(result), loop=self.loop)

        lock.release()
        test_utils.run_briefly(self.loop)
        self.assertEqual([1, 2], result)

        lock.release()
        test_utils.run_briefly(self.loop)
        self.assertEqual([1, 2, 3], result)

        self.assertTrue(t1.done())
        self.assertTrue(t1.result())
        self.assertTrue(t2.done())
        self.assertTrue(t2.result())
        self.assertTrue(t3.done())
        self.assertTrue(t3.result())

    def test_acquire_cancel(self):
        lock = asyncio.Lock(loop=self.loop)
        self.assertTrue(self.loop.run_until_complete(lock.acquire()))

        task = asyncio.Task(lock.acquire(), loop=self.loop)
        self.loop.call_soon(task.cancel)
        self.assertRaises(
            asyncio.CancelledError,
            self.loop.run_until_complete, task)
        self.assertFalse(lock._waiters)

    def test_cancel_race(self):
        # Several tasks:
        # - A acquires the lock
        # - B is blocked in aqcuire()
        # - C is blocked in aqcuire()
        #
        # Now, concurrently:
        # - B is cancelled
        # - A releases the lock
        #
        # If B's waiter is marked cancelled but not yet removed from
        # _waiters, A's release() call will crash when trying to set
        # B's waiter; instead, it should move on to C's waiter.

        # Setup: A has the lock, b and c are waiting.
        lock = asyncio.Lock(loop=self.loop)

        @asyncio.coroutine
        def lockit(name, blocker):
            yield from lock.acquire()
            try:
                if blocker is not None:
                    yield from blocker
            finally:
                lock.release()

        fa = asyncio.Future(loop=self.loop)
        ta = asyncio.Task(lockit('A', fa), loop=self.loop)
        test_utils.run_briefly(self.loop)
        self.assertTrue(lock.locked())
        tb = asyncio.Task(lockit('B', None), loop=self.loop)
        test_utils.run_briefly(self.loop)
        self.assertEqual(len(lock._waiters), 1)
        tc = asyncio.Task(lockit('C', None), loop=self.loop)
        test_utils.run_briefly(self.loop)
        self.assertEqual(len(lock._waiters), 2)

        # Create the race and check.
        # Without the fix this failed at the last assert.
        fa.set_result(None)
        tb.cancel()
        self.assertTrue(lock._waiters[0].cancelled())
        test_utils.run_briefly(self.loop)
        self.assertFalse(lock.locked())
        self.assertTrue(ta.done())
        self.assertTrue(tb.cancelled())
        self.assertTrue(tc.done())

    def test_release_not_acquired(self):
        lock = asyncio.Lock(loop=self.loop)

        self.assertRaises(RuntimeError, lock.release)

    def test_release_no_waiters(self):
        lock = asyncio.Lock(loop=self.loop)
        self.loop.run_until_complete(lock.acquire())
        self.assertTrue(lock.locked())

        lock.release()
        self.assertFalse(lock.locked())

    def test_context_manager(self):
        lock = asyncio.Lock(loop=self.loop)

        @asyncio.coroutine
        def acquire_lock():
            return (yield from lock)

        with self.loop.run_until_complete(acquire_lock()):
            self.assertTrue(lock.locked())

        self.assertFalse(lock.locked())

    def test_context_manager_cant_reuse(self):
        lock = asyncio.Lock(loop=self.loop)

        @asyncio.coroutine
        def acquire_lock():
            return (yield from lock)

        # This spells "yield from lock" outside a generator.
        cm = self.loop.run_until_complete(acquire_lock())
        with cm:
            self.assertTrue(lock.locked())

        self.assertFalse(lock.locked())

        with self.assertRaises(AttributeError):
            with cm:
                pass

    def test_context_manager_no_yield(self):
        lock = asyncio.Lock(loop=self.loop)

        try:
            with lock:
                self.fail('RuntimeError is not raised in with expression')
        except RuntimeError as err:
            self.assertEqual(
                str(err),
                '"yield from" should be used as context manager expression')

        self.assertFalse(lock.locked())


class EventTests(unittest.TestCase):

    def setUp(self):
        self.loop = test_utils.TestLoop()
        asyncio.set_event_loop(None)

    def tearDown(self):
        self.loop.close()

    def test_ctor_loop(self):
        loop = mock.Mock()
        ev = asyncio.Event(loop=loop)
        self.assertIs(ev._loop, loop)

        ev = asyncio.Event(loop=self.loop)
        self.assertIs(ev._loop, self.loop)

    def test_ctor_noloop(self):
        try:
            asyncio.set_event_loop(self.loop)
            ev = asyncio.Event()
            self.assertIs(ev._loop, self.loop)
        finally:
            asyncio.set_event_loop(None)

    def test_repr(self):
        ev = asyncio.Event(loop=self.loop)
        self.assertTrue(repr(ev).endswith('[unset]>'))
        match = RGX_REPR.match(repr(ev))
        self.assertEqual(match.group('extras'), 'unset')

        ev.set()
        self.assertTrue(repr(ev).endswith('[set]>'))
        self.assertTrue(RGX_REPR.match(repr(ev)))

        ev._waiters.append(mock.Mock())
        self.assertTrue('waiters:1' in repr(ev))
        self.assertTrue(RGX_REPR.match(repr(ev)))

    def test_wait(self):
        ev = asyncio.Event(loop=self.loop)
        self.assertFalse(ev.is_set())

        result = []

        @asyncio.coroutine
        def c1(result):
            if (yield from ev.wait()):
                result.append(1)

        @asyncio.coroutine
        def c2(result):
            if (yield from ev.wait()):
                result.append(2)

        @asyncio.coroutine
        def c3(result):
            if (yield from ev.wait()):
                result.append(3)

        t1 = asyncio.Task(c1(result), loop=self.loop)
        t2 = asyncio.Task(c2(result), loop=self.loop)

        test_utils.run_briefly(self.loop)
        self.assertEqual([], result)

        t3 = asyncio.Task(c3(result), loop=self.loop)

        ev.set()
        test_utils.run_briefly(self.loop)
        self.assertEqual([3, 1, 2], result)

        self.assertTrue(t1.done())
        self.assertIsNone(t1.result())
        self.assertTrue(t2.done())
        self.assertIsNone(t2.result())
        self.assertTrue(t3.done())
        self.assertIsNone(t3.result())

    def test_wait_on_set(self):
        ev = asyncio.Event(loop=self.loop)
        ev.set()

        res = self.loop.run_until_complete(ev.wait())
        self.assertTrue(res)

    def test_wait_cancel(self):
        ev = asyncio.Event(loop=self.loop)

        wait = asyncio.Task(ev.wait(), loop=self.loop)
        self.loop.call_soon(wait.cancel)
        self.assertRaises(
            asyncio.CancelledError,
            self.loop.run_until_complete, wait)
        self.assertFalse(ev._waiters)

    def test_clear(self):
        ev = asyncio.Event(loop=self.loop)
        self.assertFalse(ev.is_set())

        ev.set()
        self.assertTrue(ev.is_set())

        ev.clear()
        self.assertFalse(ev.is_set())

    def test_clear_with_waiters(self):
        ev = asyncio.Event(loop=self.loop)
        result = []

        @asyncio.coroutine
        def c1(result):
            if (yield from ev.wait()):
                result.append(1)
            return True

        t = asyncio.Task(c1(result), loop=self.loop)
        test_utils.run_briefly(self.loop)
        self.assertEqual([], result)

        ev.set()
        ev.clear()
        self.assertFalse(ev.is_set())

        ev.set()
        ev.set()
        self.assertEqual(1, len(ev._waiters))

        test_utils.run_briefly(self.loop)
        self.assertEqual([1], result)
        self.assertEqual(0, len(ev._waiters))

        self.assertTrue(t.done())
        self.assertTrue(t.result())


class ConditionTests(unittest.TestCase):

    def setUp(self):
        self.loop = test_utils.TestLoop()
        asyncio.set_event_loop(None)

    def tearDown(self):
        self.loop.close()

    def test_ctor_loop(self):
        loop = mock.Mock()
        cond = asyncio.Condition(loop=loop)
        self.assertIs(cond._loop, loop)

        cond = asyncio.Condition(loop=self.loop)
        self.assertIs(cond._loop, self.loop)

    def test_ctor_noloop(self):
        try:
            asyncio.set_event_loop(self.loop)
            cond = asyncio.Condition()
            self.assertIs(cond._loop, self.loop)
        finally:
            asyncio.set_event_loop(None)

    def test_wait(self):
        cond = asyncio.Condition(loop=self.loop)
        result = []

        @asyncio.coroutine
        def c1(result):
            yield from cond.acquire()
            if (yield from cond.wait()):
                result.append(1)
            return True

        @asyncio.coroutine
        def c2(result):
            yield from cond.acquire()
            if (yield from cond.wait()):
                result.append(2)
            return True

        @asyncio.coroutine
        def c3(result):
            yield from cond.acquire()
            if (yield from cond.wait()):
                result.append(3)
            return True

        t1 = asyncio.Task(c1(result), loop=self.loop)
        t2 = asyncio.Task(c2(result), loop=self.loop)
        t3 = asyncio.Task(c3(result), loop=self.loop)

        test_utils.run_briefly(self.loop)
        self.assertEqual([], result)
        self.assertFalse(cond.locked())

        self.assertTrue(self.loop.run_until_complete(cond.acquire()))
        cond.notify()
        test_utils.run_briefly(self.loop)
        self.assertEqual([], result)
        self.assertTrue(cond.locked())

        cond.release()
        test_utils.run_briefly(self.loop)
        self.assertEqual([1], result)
        self.assertTrue(cond.locked())

        cond.notify(2)
        test_utils.run_briefly(self.loop)
        self.assertEqual([1], result)
        self.assertTrue(cond.locked())

        cond.release()
        test_utils.run_briefly(self.loop)
        self.assertEqual([1, 2], result)
        self.assertTrue(cond.locked())

        cond.release()
        test_utils.run_briefly(self.loop)
        self.assertEqual([1, 2, 3], result)
        self.assertTrue(cond.locked())

        self.assertTrue(t1.done())
        self.assertTrue(t1.result())
        self.assertTrue(t2.done())
        self.assertTrue(t2.result())
        self.assertTrue(t3.done())
        self.assertTrue(t3.result())

    def test_wait_cancel(self):
        cond = asyncio.Condition(loop=self.loop)
        self.loop.run_until_complete(cond.acquire())

        wait = asyncio.Task(cond.wait(), loop=self.loop)
        self.loop.call_soon(wait.cancel)
        self.assertRaises(
            asyncio.CancelledError,
            self.loop.run_until_complete, wait)
        self.assertFalse(cond._waiters)
        self.assertTrue(cond.locked())

    def test_wait_unacquired(self):
        cond = asyncio.Condition(loop=self.loop)
        self.assertRaises(
            RuntimeError,
            self.loop.run_until_complete, cond.wait())

    def test_wait_for(self):
        cond = asyncio.Condition(loop=self.loop)
        presult = False

        def predicate():
            return presult

        result = []

        @asyncio.coroutine
        def c1(result):
            yield from cond.acquire()
            if (yield from cond.wait_for(predicate)):
                result.append(1)
                cond.release()
            return True

        t = asyncio.Task(c1(result), loop=self.loop)

        test_utils.run_briefly(self.loop)
        self.assertEqual([], result)

        self.loop.run_until_complete(cond.acquire())
        cond.notify()
        cond.release()
        test_utils.run_briefly(self.loop)
        self.assertEqual([], result)

        presult = True
        self.loop.run_until_complete(cond.acquire())
        cond.notify()
        cond.release()
        test_utils.run_briefly(self.loop)
        self.assertEqual([1], result)

        self.assertTrue(t.done())
        self.assertTrue(t.result())

    def test_wait_for_unacquired(self):
        cond = asyncio.Condition(loop=self.loop)

        # predicate can return true immediately
        res = self.loop.run_until_complete(cond.wait_for(lambda: [1, 2, 3]))
        self.assertEqual([1, 2, 3], res)

        self.assertRaises(
            RuntimeError,
            self.loop.run_until_complete,
            cond.wait_for(lambda: False))

    def test_notify(self):
        cond = asyncio.Condition(loop=self.loop)
        result = []

        @asyncio.coroutine
        def c1(result):
            yield from cond.acquire()
            if (yield from cond.wait()):
                result.append(1)
                cond.release()
            return True

        @asyncio.coroutine
        def c2(result):
            yield from cond.acquire()
            if (yield from cond.wait()):
                result.append(2)
                cond.release()
            return True

        @asyncio.coroutine
        def c3(result):
            yield from cond.acquire()
            if (yield from cond.wait()):
                result.append(3)
                cond.release()
            return True

        t1 = asyncio.Task(c1(result), loop=self.loop)
        t2 = asyncio.Task(c2(result), loop=self.loop)
        t3 = asyncio.Task(c3(result), loop=self.loop)

        test_utils.run_briefly(self.loop)
        self.assertEqual([], result)

        self.loop.run_until_complete(cond.acquire())
        cond.notify(1)
        cond.release()
        test_utils.run_briefly(self.loop)
        self.assertEqual([1], result)

        self.loop.run_until_complete(cond.acquire())
        cond.notify(1)
        cond.notify(2048)
        cond.release()
        test_utils.run_briefly(self.loop)
        self.assertEqual([1, 2, 3], result)

        self.assertTrue(t1.done())
        self.assertTrue(t1.result())
        self.assertTrue(t2.done())
        self.assertTrue(t2.result())
        self.assertTrue(t3.done())
        self.assertTrue(t3.result())

    def test_notify_all(self):
        cond = asyncio.Condition(loop=self.loop)

        result = []

        @asyncio.coroutine
        def c1(result):
            yield from cond.acquire()
            if (yield from cond.wait()):
                result.append(1)
                cond.release()
            return True

        @asyncio.coroutine
        def c2(result):
            yield from cond.acquire()
            if (yield from cond.wait()):
                result.append(2)
                cond.release()
            return True

        t1 = asyncio.Task(c1(result), loop=self.loop)
        t2 = asyncio.Task(c2(result), loop=self.loop)

        test_utils.run_briefly(self.loop)
        self.assertEqual([], result)

        self.loop.run_until_complete(cond.acquire())
        cond.notify_all()
        cond.release()
        test_utils.run_briefly(self.loop)
        self.assertEqual([1, 2], result)

        self.assertTrue(t1.done())
        self.assertTrue(t1.result())
        self.assertTrue(t2.done())
        self.assertTrue(t2.result())

    def test_notify_unacquired(self):
        cond = asyncio.Condition(loop=self.loop)
        self.assertRaises(RuntimeError, cond.notify)

    def test_notify_all_unacquired(self):
        cond = asyncio.Condition(loop=self.loop)
        self.assertRaises(RuntimeError, cond.notify_all)

    def test_repr(self):
        cond = asyncio.Condition(loop=self.loop)
        self.assertTrue('unlocked' in repr(cond))
        self.assertTrue(RGX_REPR.match(repr(cond)))

        self.loop.run_until_complete(cond.acquire())
        self.assertTrue('locked' in repr(cond))

        cond._waiters.append(mock.Mock())
        self.assertTrue('waiters:1' in repr(cond))
        self.assertTrue(RGX_REPR.match(repr(cond)))

        cond._waiters.append(mock.Mock())
        self.assertTrue('waiters:2' in repr(cond))
        self.assertTrue(RGX_REPR.match(repr(cond)))

    def test_context_manager(self):
        cond = asyncio.Condition(loop=self.loop)

        @asyncio.coroutine
        def acquire_cond():
            return (yield from cond)

        with self.loop.run_until_complete(acquire_cond()):
            self.assertTrue(cond.locked())

        self.assertFalse(cond.locked())

    def test_context_manager_no_yield(self):
        cond = asyncio.Condition(loop=self.loop)

        try:
            with cond:
                self.fail('RuntimeError is not raised in with expression')
        except RuntimeError as err:
            self.assertEqual(
                str(err),
                '"yield from" should be used as context manager expression')

        self.assertFalse(cond.locked())


class SemaphoreTests(unittest.TestCase):

    def setUp(self):
        self.loop = test_utils.TestLoop()
        asyncio.set_event_loop(None)

    def tearDown(self):
        self.loop.close()

    def test_ctor_loop(self):
        loop = mock.Mock()
        sem = asyncio.Semaphore(loop=loop)
        self.assertIs(sem._loop, loop)

        sem = asyncio.Semaphore(loop=self.loop)
        self.assertIs(sem._loop, self.loop)

    def test_ctor_noloop(self):
        try:
            asyncio.set_event_loop(self.loop)
            sem = asyncio.Semaphore()
            self.assertIs(sem._loop, self.loop)
        finally:
            asyncio.set_event_loop(None)

    def test_initial_value_zero(self):
        sem = asyncio.Semaphore(0, loop=self.loop)
        self.assertTrue(sem.locked())

    def test_repr(self):
        sem = asyncio.Semaphore(loop=self.loop)
        self.assertTrue(repr(sem).endswith('[unlocked,value:1]>'))
        self.assertTrue(RGX_REPR.match(repr(sem)))

        self.loop.run_until_complete(sem.acquire())
        self.assertTrue(repr(sem).endswith('[locked]>'))
        self.assertTrue('waiters' not in repr(sem))
        self.assertTrue(RGX_REPR.match(repr(sem)))

        sem._waiters.append(mock.Mock())
        self.assertTrue('waiters:1' in repr(sem))
        self.assertTrue(RGX_REPR.match(repr(sem)))

        sem._waiters.append(mock.Mock())
        self.assertTrue('waiters:2' in repr(sem))
        self.assertTrue(RGX_REPR.match(repr(sem)))

    def test_semaphore(self):
        sem = asyncio.Semaphore(loop=self.loop)
        self.assertEqual(1, sem._value)

        @asyncio.coroutine
        def acquire_lock():
            return (yield from sem)

        res = self.loop.run_until_complete(acquire_lock())

        self.assertTrue(res)
        self.assertTrue(sem.locked())
        self.assertEqual(0, sem._value)

        sem.release()
        self.assertFalse(sem.locked())
        self.assertEqual(1, sem._value)

    def test_semaphore_value(self):
        self.assertRaises(ValueError, asyncio.Semaphore, -1)

    def test_acquire(self):
        sem = asyncio.Semaphore(3, loop=self.loop)
        result = []

        self.assertTrue(self.loop.run_until_complete(sem.acquire()))
        self.assertTrue(self.loop.run_until_complete(sem.acquire()))
        self.assertFalse(sem.locked())

        @asyncio.coroutine
        def c1(result):
            yield from sem.acquire()
            result.append(1)
            return True

        @asyncio.coroutine
        def c2(result):
            yield from sem.acquire()
            result.append(2)
            return True

        @asyncio.coroutine
        def c3(result):
            yield from sem.acquire()
            result.append(3)
            return True

        @asyncio.coroutine
        def c4(result):
            yield from sem.acquire()
            result.append(4)
            return True

        t1 = asyncio.Task(c1(result), loop=self.loop)
        t2 = asyncio.Task(c2(result), loop=self.loop)
        t3 = asyncio.Task(c3(result), loop=self.loop)

        test_utils.run_briefly(self.loop)
        self.assertEqual([1], result)
        self.assertTrue(sem.locked())
        self.assertEqual(2, len(sem._waiters))
        self.assertEqual(0, sem._value)

        t4 = asyncio.Task(c4(result), loop=self.loop)

        sem.release()
        sem.release()
        self.assertEqual(2, sem._value)

        test_utils.run_briefly(self.loop)
        self.assertEqual(0, sem._value)
        self.assertEqual([1, 2, 3], result)
        self.assertTrue(sem.locked())
        self.assertEqual(1, len(sem._waiters))
        self.assertEqual(0, sem._value)

        self.assertTrue(t1.done())
        self.assertTrue(t1.result())
        self.assertTrue(t2.done())
        self.assertTrue(t2.result())
        self.assertTrue(t3.done())
        self.assertTrue(t3.result())
        self.assertFalse(t4.done())

        # cleanup locked semaphore
        sem.release()

    def test_acquire_cancel(self):
        sem = asyncio.Semaphore(loop=self.loop)
        self.loop.run_until_complete(sem.acquire())

        acquire = asyncio.Task(sem.acquire(), loop=self.loop)
        self.loop.call_soon(acquire.cancel)
        self.assertRaises(
            asyncio.CancelledError,
            self.loop.run_until_complete, acquire)
        self.assertFalse(sem._waiters)

    def test_release_not_acquired(self):
        sem = asyncio.BoundedSemaphore(loop=self.loop)

        self.assertRaises(ValueError, sem.release)

    def test_release_no_waiters(self):
        sem = asyncio.Semaphore(loop=self.loop)
        self.loop.run_until_complete(sem.acquire())
        self.assertTrue(sem.locked())

        sem.release()
        self.assertFalse(sem.locked())

    def test_context_manager(self):
        sem = asyncio.Semaphore(2, loop=self.loop)

        @asyncio.coroutine
        def acquire_lock():
            return (yield from sem)

        with self.loop.run_until_complete(acquire_lock()):
            self.assertFalse(sem.locked())
            self.assertEqual(1, sem._value)

            with self.loop.run_until_complete(acquire_lock()):
                self.assertTrue(sem.locked())

        self.assertEqual(2, sem._value)

    def test_context_manager_no_yield(self):
        sem = asyncio.Semaphore(2, loop=self.loop)

        try:
            with sem:
                self.fail('RuntimeError is not raised in with expression')
        except RuntimeError as err:
            self.assertEqual(
                str(err),
                '"yield from" should be used as context manager expression')

        self.assertEqual(2, sem._value)


if __name__ == '__main__':
    unittest.main()
