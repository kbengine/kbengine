"""Tests for tasks.py."""

import gc
import os.path
import types
import unittest
import weakref
from test.script_helper import assert_python_ok

import asyncio
from asyncio import test_utils


@asyncio.coroutine
def coroutine_function():
    pass


class Dummy:

    def __repr__(self):
        return 'Dummy()'

    def __call__(self, *args):
        pass


class TaskTests(unittest.TestCase):

    def setUp(self):
        self.loop = test_utils.TestLoop()
        asyncio.set_event_loop(None)

    def tearDown(self):
        self.loop.close()
        gc.collect()

    def test_task_class(self):
        @asyncio.coroutine
        def notmuch():
            return 'ok'
        t = asyncio.Task(notmuch(), loop=self.loop)
        self.loop.run_until_complete(t)
        self.assertTrue(t.done())
        self.assertEqual(t.result(), 'ok')
        self.assertIs(t._loop, self.loop)

        loop = asyncio.new_event_loop()
        t = asyncio.Task(notmuch(), loop=loop)
        self.assertIs(t._loop, loop)
        loop.close()

    def test_async_coroutine(self):
        @asyncio.coroutine
        def notmuch():
            return 'ok'
        t = asyncio.async(notmuch(), loop=self.loop)
        self.loop.run_until_complete(t)
        self.assertTrue(t.done())
        self.assertEqual(t.result(), 'ok')
        self.assertIs(t._loop, self.loop)

        loop = asyncio.new_event_loop()
        t = asyncio.async(notmuch(), loop=loop)
        self.assertIs(t._loop, loop)
        loop.close()

    def test_async_future(self):
        f_orig = asyncio.Future(loop=self.loop)
        f_orig.set_result('ko')

        f = asyncio.async(f_orig)
        self.loop.run_until_complete(f)
        self.assertTrue(f.done())
        self.assertEqual(f.result(), 'ko')
        self.assertIs(f, f_orig)

        loop = asyncio.new_event_loop()

        with self.assertRaises(ValueError):
            f = asyncio.async(f_orig, loop=loop)

        loop.close()

        f = asyncio.async(f_orig, loop=self.loop)
        self.assertIs(f, f_orig)

    def test_async_task(self):
        @asyncio.coroutine
        def notmuch():
            return 'ok'
        t_orig = asyncio.Task(notmuch(), loop=self.loop)
        t = asyncio.async(t_orig)
        self.loop.run_until_complete(t)
        self.assertTrue(t.done())
        self.assertEqual(t.result(), 'ok')
        self.assertIs(t, t_orig)

        loop = asyncio.new_event_loop()

        with self.assertRaises(ValueError):
            t = asyncio.async(t_orig, loop=loop)

        loop.close()

        t = asyncio.async(t_orig, loop=self.loop)
        self.assertIs(t, t_orig)

    def test_async_neither(self):
        with self.assertRaises(TypeError):
            asyncio.async('ok')

    def test_task_repr(self):
        @asyncio.coroutine
        def notmuch():
            yield from []
            return 'abc'

        t = asyncio.Task(notmuch(), loop=self.loop)
        t.add_done_callback(Dummy())
        self.assertEqual(repr(t), 'Task(<notmuch>)<PENDING, [Dummy()]>')
        t.cancel()  # Does not take immediate effect!
        self.assertEqual(repr(t), 'Task(<notmuch>)<CANCELLING, [Dummy()]>')
        self.assertRaises(asyncio.CancelledError,
                          self.loop.run_until_complete, t)
        self.assertEqual(repr(t), 'Task(<notmuch>)<CANCELLED>')
        t = asyncio.Task(notmuch(), loop=self.loop)
        self.loop.run_until_complete(t)
        self.assertEqual(repr(t), "Task(<notmuch>)<result='abc'>")

    def test_task_repr_custom(self):
        @asyncio.coroutine
        def coro():
            pass

        class T(asyncio.Future):
            def __repr__(self):
                return 'T[]'

        class MyTask(asyncio.Task, T):
            def __repr__(self):
                return super().__repr__()

        gen = coro()
        t = MyTask(gen, loop=self.loop)
        self.assertEqual(repr(t), 'T[](<coro>)')
        gen.close()

    def test_task_basics(self):
        @asyncio.coroutine
        def outer():
            a = yield from inner1()
            b = yield from inner2()
            return a+b

        @asyncio.coroutine
        def inner1():
            return 42

        @asyncio.coroutine
        def inner2():
            return 1000

        t = outer()
        self.assertEqual(self.loop.run_until_complete(t), 1042)

    def test_cancel(self):

        def gen():
            when = yield
            self.assertAlmostEqual(10.0, when)
            yield 0

        loop = test_utils.TestLoop(gen)
        self.addCleanup(loop.close)

        @asyncio.coroutine
        def task():
            yield from asyncio.sleep(10.0, loop=loop)
            return 12

        t = asyncio.Task(task(), loop=loop)
        loop.call_soon(t.cancel)
        with self.assertRaises(asyncio.CancelledError):
            loop.run_until_complete(t)
        self.assertTrue(t.done())
        self.assertTrue(t.cancelled())
        self.assertFalse(t.cancel())

    def test_cancel_yield(self):
        @asyncio.coroutine
        def task():
            yield
            yield
            return 12

        t = asyncio.Task(task(), loop=self.loop)
        test_utils.run_briefly(self.loop)  # start coro
        t.cancel()
        self.assertRaises(
            asyncio.CancelledError, self.loop.run_until_complete, t)
        self.assertTrue(t.done())
        self.assertTrue(t.cancelled())
        self.assertFalse(t.cancel())

    def test_cancel_inner_future(self):
        f = asyncio.Future(loop=self.loop)

        @asyncio.coroutine
        def task():
            yield from f
            return 12

        t = asyncio.Task(task(), loop=self.loop)
        test_utils.run_briefly(self.loop)  # start task
        f.cancel()
        with self.assertRaises(asyncio.CancelledError):
            self.loop.run_until_complete(t)
        self.assertTrue(f.cancelled())
        self.assertTrue(t.cancelled())

    def test_cancel_both_task_and_inner_future(self):
        f = asyncio.Future(loop=self.loop)

        @asyncio.coroutine
        def task():
            yield from f
            return 12

        t = asyncio.Task(task(), loop=self.loop)
        test_utils.run_briefly(self.loop)

        f.cancel()
        t.cancel()

        with self.assertRaises(asyncio.CancelledError):
            self.loop.run_until_complete(t)

        self.assertTrue(t.done())
        self.assertTrue(f.cancelled())
        self.assertTrue(t.cancelled())

    def test_cancel_task_catching(self):
        fut1 = asyncio.Future(loop=self.loop)
        fut2 = asyncio.Future(loop=self.loop)

        @asyncio.coroutine
        def task():
            yield from fut1
            try:
                yield from fut2
            except asyncio.CancelledError:
                return 42

        t = asyncio.Task(task(), loop=self.loop)
        test_utils.run_briefly(self.loop)
        self.assertIs(t._fut_waiter, fut1)  # White-box test.
        fut1.set_result(None)
        test_utils.run_briefly(self.loop)
        self.assertIs(t._fut_waiter, fut2)  # White-box test.
        t.cancel()
        self.assertTrue(fut2.cancelled())
        res = self.loop.run_until_complete(t)
        self.assertEqual(res, 42)
        self.assertFalse(t.cancelled())

    def test_cancel_task_ignoring(self):
        fut1 = asyncio.Future(loop=self.loop)
        fut2 = asyncio.Future(loop=self.loop)
        fut3 = asyncio.Future(loop=self.loop)

        @asyncio.coroutine
        def task():
            yield from fut1
            try:
                yield from fut2
            except asyncio.CancelledError:
                pass
            res = yield from fut3
            return res

        t = asyncio.Task(task(), loop=self.loop)
        test_utils.run_briefly(self.loop)
        self.assertIs(t._fut_waiter, fut1)  # White-box test.
        fut1.set_result(None)
        test_utils.run_briefly(self.loop)
        self.assertIs(t._fut_waiter, fut2)  # White-box test.
        t.cancel()
        self.assertTrue(fut2.cancelled())
        test_utils.run_briefly(self.loop)
        self.assertIs(t._fut_waiter, fut3)  # White-box test.
        fut3.set_result(42)
        res = self.loop.run_until_complete(t)
        self.assertEqual(res, 42)
        self.assertFalse(fut3.cancelled())
        self.assertFalse(t.cancelled())

    def test_cancel_current_task(self):
        loop = asyncio.new_event_loop()
        self.addCleanup(loop.close)

        @asyncio.coroutine
        def task():
            t.cancel()
            self.assertTrue(t._must_cancel)  # White-box test.
            # The sleep should be cancelled immediately.
            yield from asyncio.sleep(100, loop=loop)
            return 12

        t = asyncio.Task(task(), loop=loop)
        self.assertRaises(
            asyncio.CancelledError, loop.run_until_complete, t)
        self.assertTrue(t.done())
        self.assertFalse(t._must_cancel)  # White-box test.
        self.assertFalse(t.cancel())

    def test_stop_while_run_in_complete(self):

        def gen():
            when = yield
            self.assertAlmostEqual(0.1, when)
            when = yield 0.1
            self.assertAlmostEqual(0.2, when)
            when = yield 0.1
            self.assertAlmostEqual(0.3, when)
            yield 0.1

        loop = test_utils.TestLoop(gen)
        self.addCleanup(loop.close)

        x = 0
        waiters = []

        @asyncio.coroutine
        def task():
            nonlocal x
            while x < 10:
                waiters.append(asyncio.sleep(0.1, loop=loop))
                yield from waiters[-1]
                x += 1
                if x == 2:
                    loop.stop()

        t = asyncio.Task(task(), loop=loop)
        self.assertRaises(
            RuntimeError, loop.run_until_complete, t)
        self.assertFalse(t.done())
        self.assertEqual(x, 2)
        self.assertAlmostEqual(0.3, loop.time())

        # close generators
        for w in waiters:
            w.close()

    def test_wait_for(self):

        def gen():
            when = yield
            self.assertAlmostEqual(0.2, when)
            when = yield 0
            self.assertAlmostEqual(0.1, when)
            when = yield 0.1

        loop = test_utils.TestLoop(gen)
        self.addCleanup(loop.close)

        foo_running = None

        @asyncio.coroutine
        def foo():
            nonlocal foo_running
            foo_running = True
            try:
                yield from asyncio.sleep(0.2, loop=loop)
            finally:
                foo_running = False
            return 'done'

        fut = asyncio.Task(foo(), loop=loop)

        with self.assertRaises(asyncio.TimeoutError):
            loop.run_until_complete(asyncio.wait_for(fut, 0.1, loop=loop))
        self.assertTrue(fut.done())
        # it should have been cancelled due to the timeout
        self.assertTrue(fut.cancelled())
        self.assertAlmostEqual(0.1, loop.time())
        self.assertEqual(foo_running, False)

    def test_wait_for_blocking(self):
        loop = test_utils.TestLoop()
        self.addCleanup(loop.close)

        @asyncio.coroutine
        def coro():
            return 'done'

        res = loop.run_until_complete(asyncio.wait_for(coro(),
                                                       timeout=None,
                                                       loop=loop))
        self.assertEqual(res, 'done')

    def test_wait_for_with_global_loop(self):

        def gen():
            when = yield
            self.assertAlmostEqual(0.2, when)
            when = yield 0
            self.assertAlmostEqual(0.01, when)
            yield 0.01

        loop = test_utils.TestLoop(gen)
        self.addCleanup(loop.close)

        @asyncio.coroutine
        def foo():
            yield from asyncio.sleep(0.2, loop=loop)
            return 'done'

        asyncio.set_event_loop(loop)
        try:
            fut = asyncio.Task(foo(), loop=loop)
            with self.assertRaises(asyncio.TimeoutError):
                loop.run_until_complete(asyncio.wait_for(fut, 0.01))
        finally:
            asyncio.set_event_loop(None)

        self.assertAlmostEqual(0.01, loop.time())
        self.assertTrue(fut.done())
        self.assertTrue(fut.cancelled())

    def test_wait(self):

        def gen():
            when = yield
            self.assertAlmostEqual(0.1, when)
            when = yield 0
            self.assertAlmostEqual(0.15, when)
            yield 0.15

        loop = test_utils.TestLoop(gen)
        self.addCleanup(loop.close)

        a = asyncio.Task(asyncio.sleep(0.1, loop=loop), loop=loop)
        b = asyncio.Task(asyncio.sleep(0.15, loop=loop), loop=loop)

        @asyncio.coroutine
        def foo():
            done, pending = yield from asyncio.wait([b, a], loop=loop)
            self.assertEqual(done, set([a, b]))
            self.assertEqual(pending, set())
            return 42

        res = loop.run_until_complete(asyncio.Task(foo(), loop=loop))
        self.assertEqual(res, 42)
        self.assertAlmostEqual(0.15, loop.time())

        # Doing it again should take no time and exercise a different path.
        res = loop.run_until_complete(asyncio.Task(foo(), loop=loop))
        self.assertAlmostEqual(0.15, loop.time())
        self.assertEqual(res, 42)

    def test_wait_with_global_loop(self):

        def gen():
            when = yield
            self.assertAlmostEqual(0.01, when)
            when = yield 0
            self.assertAlmostEqual(0.015, when)
            yield 0.015

        loop = test_utils.TestLoop(gen)
        self.addCleanup(loop.close)

        a = asyncio.Task(asyncio.sleep(0.01, loop=loop), loop=loop)
        b = asyncio.Task(asyncio.sleep(0.015, loop=loop), loop=loop)

        @asyncio.coroutine
        def foo():
            done, pending = yield from asyncio.wait([b, a])
            self.assertEqual(done, set([a, b]))
            self.assertEqual(pending, set())
            return 42

        asyncio.set_event_loop(loop)
        try:
            res = loop.run_until_complete(
                asyncio.Task(foo(), loop=loop))
        finally:
            asyncio.set_event_loop(None)

        self.assertEqual(res, 42)

    def test_wait_duplicate_coroutines(self):
        @asyncio.coroutine
        def coro(s):
            return s
        c = coro('test')

        task = asyncio.Task(
            asyncio.wait([c, c, coro('spam')], loop=self.loop),
            loop=self.loop)

        done, pending = self.loop.run_until_complete(task)

        self.assertFalse(pending)
        self.assertEqual(set(f.result() for f in done), {'test', 'spam'})

    def test_wait_errors(self):
        self.assertRaises(
            ValueError, self.loop.run_until_complete,
            asyncio.wait(set(), loop=self.loop))

        self.assertRaises(
            ValueError, self.loop.run_until_complete,
            asyncio.wait([asyncio.sleep(10.0, loop=self.loop)],
                         return_when=-1, loop=self.loop))

    def test_wait_first_completed(self):

        def gen():
            when = yield
            self.assertAlmostEqual(10.0, when)
            when = yield 0
            self.assertAlmostEqual(0.1, when)
            yield 0.1

        loop = test_utils.TestLoop(gen)
        self.addCleanup(loop.close)

        a = asyncio.Task(asyncio.sleep(10.0, loop=loop), loop=loop)
        b = asyncio.Task(asyncio.sleep(0.1, loop=loop), loop=loop)
        task = asyncio.Task(
            asyncio.wait([b, a], return_when=asyncio.FIRST_COMPLETED,
                         loop=loop),
            loop=loop)

        done, pending = loop.run_until_complete(task)
        self.assertEqual({b}, done)
        self.assertEqual({a}, pending)
        self.assertFalse(a.done())
        self.assertTrue(b.done())
        self.assertIsNone(b.result())
        self.assertAlmostEqual(0.1, loop.time())

        # move forward to close generator
        loop.advance_time(10)
        loop.run_until_complete(asyncio.wait([a, b], loop=loop))

    def test_wait_really_done(self):
        # there is possibility that some tasks in the pending list
        # became done but their callbacks haven't all been called yet

        @asyncio.coroutine
        def coro1():
            yield

        @asyncio.coroutine
        def coro2():
            yield
            yield

        a = asyncio.Task(coro1(), loop=self.loop)
        b = asyncio.Task(coro2(), loop=self.loop)
        task = asyncio.Task(
            asyncio.wait([b, a], return_when=asyncio.FIRST_COMPLETED,
                         loop=self.loop),
            loop=self.loop)

        done, pending = self.loop.run_until_complete(task)
        self.assertEqual({a, b}, done)
        self.assertTrue(a.done())
        self.assertIsNone(a.result())
        self.assertTrue(b.done())
        self.assertIsNone(b.result())

    def test_wait_first_exception(self):

        def gen():
            when = yield
            self.assertAlmostEqual(10.0, when)
            yield 0

        loop = test_utils.TestLoop(gen)
        self.addCleanup(loop.close)

        # first_exception, task already has exception
        a = asyncio.Task(asyncio.sleep(10.0, loop=loop), loop=loop)

        @asyncio.coroutine
        def exc():
            raise ZeroDivisionError('err')

        b = asyncio.Task(exc(), loop=loop)
        task = asyncio.Task(
            asyncio.wait([b, a], return_when=asyncio.FIRST_EXCEPTION,
                         loop=loop),
            loop=loop)

        done, pending = loop.run_until_complete(task)
        self.assertEqual({b}, done)
        self.assertEqual({a}, pending)
        self.assertAlmostEqual(0, loop.time())

        # move forward to close generator
        loop.advance_time(10)
        loop.run_until_complete(asyncio.wait([a, b], loop=loop))

    def test_wait_first_exception_in_wait(self):

        def gen():
            when = yield
            self.assertAlmostEqual(10.0, when)
            when = yield 0
            self.assertAlmostEqual(0.01, when)
            yield 0.01

        loop = test_utils.TestLoop(gen)
        self.addCleanup(loop.close)

        # first_exception, exception during waiting
        a = asyncio.Task(asyncio.sleep(10.0, loop=loop), loop=loop)

        @asyncio.coroutine
        def exc():
            yield from asyncio.sleep(0.01, loop=loop)
            raise ZeroDivisionError('err')

        b = asyncio.Task(exc(), loop=loop)
        task = asyncio.wait([b, a], return_when=asyncio.FIRST_EXCEPTION,
                            loop=loop)

        done, pending = loop.run_until_complete(task)
        self.assertEqual({b}, done)
        self.assertEqual({a}, pending)
        self.assertAlmostEqual(0.01, loop.time())

        # move forward to close generator
        loop.advance_time(10)
        loop.run_until_complete(asyncio.wait([a, b], loop=loop))

    def test_wait_with_exception(self):

        def gen():
            when = yield
            self.assertAlmostEqual(0.1, when)
            when = yield 0
            self.assertAlmostEqual(0.15, when)
            yield 0.15

        loop = test_utils.TestLoop(gen)
        self.addCleanup(loop.close)

        a = asyncio.Task(asyncio.sleep(0.1, loop=loop), loop=loop)

        @asyncio.coroutine
        def sleeper():
            yield from asyncio.sleep(0.15, loop=loop)
            raise ZeroDivisionError('really')

        b = asyncio.Task(sleeper(), loop=loop)

        @asyncio.coroutine
        def foo():
            done, pending = yield from asyncio.wait([b, a], loop=loop)
            self.assertEqual(len(done), 2)
            self.assertEqual(pending, set())
            errors = set(f for f in done if f.exception() is not None)
            self.assertEqual(len(errors), 1)

        loop.run_until_complete(asyncio.Task(foo(), loop=loop))
        self.assertAlmostEqual(0.15, loop.time())

        loop.run_until_complete(asyncio.Task(foo(), loop=loop))
        self.assertAlmostEqual(0.15, loop.time())

    def test_wait_with_timeout(self):

        def gen():
            when = yield
            self.assertAlmostEqual(0.1, when)
            when = yield 0
            self.assertAlmostEqual(0.15, when)
            when = yield 0
            self.assertAlmostEqual(0.11, when)
            yield 0.11

        loop = test_utils.TestLoop(gen)
        self.addCleanup(loop.close)

        a = asyncio.Task(asyncio.sleep(0.1, loop=loop), loop=loop)
        b = asyncio.Task(asyncio.sleep(0.15, loop=loop), loop=loop)

        @asyncio.coroutine
        def foo():
            done, pending = yield from asyncio.wait([b, a], timeout=0.11,
                                                    loop=loop)
            self.assertEqual(done, set([a]))
            self.assertEqual(pending, set([b]))

        loop.run_until_complete(asyncio.Task(foo(), loop=loop))
        self.assertAlmostEqual(0.11, loop.time())

        # move forward to close generator
        loop.advance_time(10)
        loop.run_until_complete(asyncio.wait([a, b], loop=loop))

    def test_wait_concurrent_complete(self):

        def gen():
            when = yield
            self.assertAlmostEqual(0.1, when)
            when = yield 0
            self.assertAlmostEqual(0.15, when)
            when = yield 0
            self.assertAlmostEqual(0.1, when)
            yield 0.1

        loop = test_utils.TestLoop(gen)
        self.addCleanup(loop.close)

        a = asyncio.Task(asyncio.sleep(0.1, loop=loop), loop=loop)
        b = asyncio.Task(asyncio.sleep(0.15, loop=loop), loop=loop)

        done, pending = loop.run_until_complete(
            asyncio.wait([b, a], timeout=0.1, loop=loop))

        self.assertEqual(done, set([a]))
        self.assertEqual(pending, set([b]))
        self.assertAlmostEqual(0.1, loop.time())

        # move forward to close generator
        loop.advance_time(10)
        loop.run_until_complete(asyncio.wait([a, b], loop=loop))

    def test_as_completed(self):

        def gen():
            yield 0
            yield 0
            yield 0.01
            yield 0

        loop = test_utils.TestLoop(gen)
        self.addCleanup(loop.close)
        completed = set()
        time_shifted = False

        @asyncio.coroutine
        def sleeper(dt, x):
            nonlocal time_shifted
            yield from asyncio.sleep(dt, loop=loop)
            completed.add(x)
            if not time_shifted and 'a' in completed and 'b' in completed:
                time_shifted = True
                loop.advance_time(0.14)
            return x

        a = sleeper(0.01, 'a')
        b = sleeper(0.01, 'b')
        c = sleeper(0.15, 'c')

        @asyncio.coroutine
        def foo():
            values = []
            for f in asyncio.as_completed([b, c, a], loop=loop):
                values.append((yield from f))
            return values

        res = loop.run_until_complete(asyncio.Task(foo(), loop=loop))
        self.assertAlmostEqual(0.15, loop.time())
        self.assertTrue('a' in res[:2])
        self.assertTrue('b' in res[:2])
        self.assertEqual(res[2], 'c')

        # Doing it again should take no time and exercise a different path.
        res = loop.run_until_complete(asyncio.Task(foo(), loop=loop))
        self.assertAlmostEqual(0.15, loop.time())

    def test_as_completed_with_timeout(self):

        def gen():
            yield
            yield 0
            yield 0
            yield 0.1

        loop = test_utils.TestLoop(gen)
        self.addCleanup(loop.close)

        a = asyncio.sleep(0.1, 'a', loop=loop)
        b = asyncio.sleep(0.15, 'b', loop=loop)

        @asyncio.coroutine
        def foo():
            values = []
            for f in asyncio.as_completed([a, b], timeout=0.12, loop=loop):
                if values:
                    loop.advance_time(0.02)
                try:
                    v = yield from f
                    values.append((1, v))
                except asyncio.TimeoutError as exc:
                    values.append((2, exc))
            return values

        res = loop.run_until_complete(asyncio.Task(foo(), loop=loop))
        self.assertEqual(len(res), 2, res)
        self.assertEqual(res[0], (1, 'a'))
        self.assertEqual(res[1][0], 2)
        self.assertIsInstance(res[1][1], asyncio.TimeoutError)
        self.assertAlmostEqual(0.12, loop.time())

        # move forward to close generator
        loop.advance_time(10)
        loop.run_until_complete(asyncio.wait([a, b], loop=loop))

    def test_as_completed_with_unused_timeout(self):

        def gen():
            yield
            yield 0
            yield 0.01

        loop = test_utils.TestLoop(gen)
        self.addCleanup(loop.close)

        a = asyncio.sleep(0.01, 'a', loop=loop)

        @asyncio.coroutine
        def foo():
            for f in asyncio.as_completed([a], timeout=1, loop=loop):
                v = yield from f
                self.assertEqual(v, 'a')

        loop.run_until_complete(asyncio.Task(foo(), loop=loop))

    def test_as_completed_reverse_wait(self):

        def gen():
            yield 0
            yield 0.05
            yield 0

        loop = test_utils.TestLoop(gen)
        self.addCleanup(loop.close)

        a = asyncio.sleep(0.05, 'a', loop=loop)
        b = asyncio.sleep(0.10, 'b', loop=loop)
        fs = {a, b}
        futs = list(asyncio.as_completed(fs, loop=loop))
        self.assertEqual(len(futs), 2)

        x = loop.run_until_complete(futs[1])
        self.assertEqual(x, 'a')
        self.assertAlmostEqual(0.05, loop.time())
        loop.advance_time(0.05)
        y = loop.run_until_complete(futs[0])
        self.assertEqual(y, 'b')
        self.assertAlmostEqual(0.10, loop.time())

    def test_as_completed_concurrent(self):

        def gen():
            when = yield
            self.assertAlmostEqual(0.05, when)
            when = yield 0
            self.assertAlmostEqual(0.05, when)
            yield 0.05

        loop = test_utils.TestLoop(gen)
        self.addCleanup(loop.close)

        a = asyncio.sleep(0.05, 'a', loop=loop)
        b = asyncio.sleep(0.05, 'b', loop=loop)
        fs = {a, b}
        futs = list(asyncio.as_completed(fs, loop=loop))
        self.assertEqual(len(futs), 2)
        waiter = asyncio.wait(futs, loop=loop)
        done, pending = loop.run_until_complete(waiter)
        self.assertEqual(set(f.result() for f in done), {'a', 'b'})

    def test_as_completed_duplicate_coroutines(self):

        @asyncio.coroutine
        def coro(s):
            return s

        @asyncio.coroutine
        def runner():
            result = []
            c = coro('ham')
            for f in asyncio.as_completed([c, c, coro('spam')],
                                          loop=self.loop):
                result.append((yield from f))
            return result

        fut = asyncio.Task(runner(), loop=self.loop)
        self.loop.run_until_complete(fut)
        result = fut.result()
        self.assertEqual(set(result), {'ham', 'spam'})
        self.assertEqual(len(result), 2)

    def test_sleep(self):

        def gen():
            when = yield
            self.assertAlmostEqual(0.05, when)
            when = yield 0.05
            self.assertAlmostEqual(0.1, when)
            yield 0.05

        loop = test_utils.TestLoop(gen)
        self.addCleanup(loop.close)

        @asyncio.coroutine
        def sleeper(dt, arg):
            yield from asyncio.sleep(dt/2, loop=loop)
            res = yield from asyncio.sleep(dt/2, arg, loop=loop)
            return res

        t = asyncio.Task(sleeper(0.1, 'yeah'), loop=loop)
        loop.run_until_complete(t)
        self.assertTrue(t.done())
        self.assertEqual(t.result(), 'yeah')
        self.assertAlmostEqual(0.1, loop.time())

    def test_sleep_cancel(self):

        def gen():
            when = yield
            self.assertAlmostEqual(10.0, when)
            yield 0

        loop = test_utils.TestLoop(gen)
        self.addCleanup(loop.close)

        t = asyncio.Task(asyncio.sleep(10.0, 'yeah', loop=loop),
                         loop=loop)

        handle = None
        orig_call_later = loop.call_later

        def call_later(self, delay, callback, *args):
            nonlocal handle
            handle = orig_call_later(self, delay, callback, *args)
            return handle

        loop.call_later = call_later
        test_utils.run_briefly(loop)

        self.assertFalse(handle._cancelled)

        t.cancel()
        test_utils.run_briefly(loop)
        self.assertTrue(handle._cancelled)

    def test_task_cancel_sleeping_task(self):

        def gen():
            when = yield
            self.assertAlmostEqual(0.1, when)
            when = yield 0
            self.assertAlmostEqual(5000, when)
            yield 0.1

        loop = test_utils.TestLoop(gen)
        self.addCleanup(loop.close)

        @asyncio.coroutine
        def sleep(dt):
            yield from asyncio.sleep(dt, loop=loop)

        @asyncio.coroutine
        def doit():
            sleeper = asyncio.Task(sleep(5000), loop=loop)
            loop.call_later(0.1, sleeper.cancel)
            try:
                yield from sleeper
            except asyncio.CancelledError:
                return 'cancelled'
            else:
                return 'slept in'

        doer = doit()
        self.assertEqual(loop.run_until_complete(doer), 'cancelled')
        self.assertAlmostEqual(0.1, loop.time())

    def test_task_cancel_waiter_future(self):
        fut = asyncio.Future(loop=self.loop)

        @asyncio.coroutine
        def coro():
            yield from fut

        task = asyncio.Task(coro(), loop=self.loop)
        test_utils.run_briefly(self.loop)
        self.assertIs(task._fut_waiter, fut)

        task.cancel()
        test_utils.run_briefly(self.loop)
        self.assertRaises(
            asyncio.CancelledError, self.loop.run_until_complete, task)
        self.assertIsNone(task._fut_waiter)
        self.assertTrue(fut.cancelled())

    def test_step_in_completed_task(self):
        @asyncio.coroutine
        def notmuch():
            return 'ko'

        gen = notmuch()
        task = asyncio.Task(gen, loop=self.loop)
        task.set_result('ok')

        self.assertRaises(AssertionError, task._step)
        gen.close()

    def test_step_result(self):
        @asyncio.coroutine
        def notmuch():
            yield None
            yield 1
            return 'ko'

        self.assertRaises(
            RuntimeError, self.loop.run_until_complete, notmuch())

    def test_step_result_future(self):
        # If coroutine returns future, task waits on this future.

        class Fut(asyncio.Future):
            def __init__(self, *args, **kwds):
                self.cb_added = False
                super().__init__(*args, **kwds)

            def add_done_callback(self, fn):
                self.cb_added = True
                super().add_done_callback(fn)

        fut = Fut(loop=self.loop)
        result = None

        @asyncio.coroutine
        def wait_for_future():
            nonlocal result
            result = yield from fut

        t = asyncio.Task(wait_for_future(), loop=self.loop)
        test_utils.run_briefly(self.loop)
        self.assertTrue(fut.cb_added)

        res = object()
        fut.set_result(res)
        test_utils.run_briefly(self.loop)
        self.assertIs(res, result)
        self.assertTrue(t.done())
        self.assertIsNone(t.result())

    def test_step_with_baseexception(self):
        @asyncio.coroutine
        def notmutch():
            raise BaseException()

        task = asyncio.Task(notmutch(), loop=self.loop)
        self.assertRaises(BaseException, task._step)

        self.assertTrue(task.done())
        self.assertIsInstance(task.exception(), BaseException)

    def test_baseexception_during_cancel(self):

        def gen():
            when = yield
            self.assertAlmostEqual(10.0, when)
            yield 0

        loop = test_utils.TestLoop(gen)
        self.addCleanup(loop.close)

        @asyncio.coroutine
        def sleeper():
            yield from asyncio.sleep(10, loop=loop)

        base_exc = BaseException()

        @asyncio.coroutine
        def notmutch():
            try:
                yield from sleeper()
            except asyncio.CancelledError:
                raise base_exc

        task = asyncio.Task(notmutch(), loop=loop)
        test_utils.run_briefly(loop)

        task.cancel()
        self.assertFalse(task.done())

        self.assertRaises(BaseException, test_utils.run_briefly, loop)

        self.assertTrue(task.done())
        self.assertFalse(task.cancelled())
        self.assertIs(task.exception(), base_exc)

    def test_iscoroutinefunction(self):
        def fn():
            pass

        self.assertFalse(asyncio.iscoroutinefunction(fn))

        def fn1():
            yield
        self.assertFalse(asyncio.iscoroutinefunction(fn1))

        @asyncio.coroutine
        def fn2():
            yield
        self.assertTrue(asyncio.iscoroutinefunction(fn2))

    def test_yield_vs_yield_from(self):
        fut = asyncio.Future(loop=self.loop)

        @asyncio.coroutine
        def wait_for_future():
            yield fut

        task = wait_for_future()
        with self.assertRaises(RuntimeError):
            self.loop.run_until_complete(task)

        self.assertFalse(fut.done())

    def test_yield_vs_yield_from_generator(self):
        @asyncio.coroutine
        def coro():
            yield

        @asyncio.coroutine
        def wait_for_future():
            gen = coro()
            try:
                yield gen
            finally:
                gen.close()

        task = wait_for_future()
        self.assertRaises(
            RuntimeError,
            self.loop.run_until_complete, task)

    def test_coroutine_non_gen_function(self):
        @asyncio.coroutine
        def func():
            return 'test'

        self.assertTrue(asyncio.iscoroutinefunction(func))

        coro = func()
        self.assertTrue(asyncio.iscoroutine(coro))

        res = self.loop.run_until_complete(coro)
        self.assertEqual(res, 'test')

    def test_coroutine_non_gen_function_return_future(self):
        fut = asyncio.Future(loop=self.loop)

        @asyncio.coroutine
        def func():
            return fut

        @asyncio.coroutine
        def coro():
            fut.set_result('test')

        t1 = asyncio.Task(func(), loop=self.loop)
        t2 = asyncio.Task(coro(), loop=self.loop)
        res = self.loop.run_until_complete(t1)
        self.assertEqual(res, 'test')
        self.assertIsNone(t2.result())

    def test_current_task(self):
        self.assertIsNone(asyncio.Task.current_task(loop=self.loop))

        @asyncio.coroutine
        def coro(loop):
            self.assertTrue(asyncio.Task.current_task(loop=loop) is task)

        task = asyncio.Task(coro(self.loop), loop=self.loop)
        self.loop.run_until_complete(task)
        self.assertIsNone(asyncio.Task.current_task(loop=self.loop))

    def test_current_task_with_interleaving_tasks(self):
        self.assertIsNone(asyncio.Task.current_task(loop=self.loop))

        fut1 = asyncio.Future(loop=self.loop)
        fut2 = asyncio.Future(loop=self.loop)

        @asyncio.coroutine
        def coro1(loop):
            self.assertTrue(asyncio.Task.current_task(loop=loop) is task1)
            yield from fut1
            self.assertTrue(asyncio.Task.current_task(loop=loop) is task1)
            fut2.set_result(True)

        @asyncio.coroutine
        def coro2(loop):
            self.assertTrue(asyncio.Task.current_task(loop=loop) is task2)
            fut1.set_result(True)
            yield from fut2
            self.assertTrue(asyncio.Task.current_task(loop=loop) is task2)

        task1 = asyncio.Task(coro1(self.loop), loop=self.loop)
        task2 = asyncio.Task(coro2(self.loop), loop=self.loop)

        self.loop.run_until_complete(asyncio.wait((task1, task2),
                                                  loop=self.loop))
        self.assertIsNone(asyncio.Task.current_task(loop=self.loop))

    # Some thorough tests for cancellation propagation through
    # coroutines, tasks and wait().

    def test_yield_future_passes_cancel(self):
        # Cancelling outer() cancels inner() cancels waiter.
        proof = 0
        waiter = asyncio.Future(loop=self.loop)

        @asyncio.coroutine
        def inner():
            nonlocal proof
            try:
                yield from waiter
            except asyncio.CancelledError:
                proof += 1
                raise
            else:
                self.fail('got past sleep() in inner()')

        @asyncio.coroutine
        def outer():
            nonlocal proof
            try:
                yield from inner()
            except asyncio.CancelledError:
                proof += 100  # Expect this path.
            else:
                proof += 10

        f = asyncio.async(outer(), loop=self.loop)
        test_utils.run_briefly(self.loop)
        f.cancel()
        self.loop.run_until_complete(f)
        self.assertEqual(proof, 101)
        self.assertTrue(waiter.cancelled())

    def test_yield_wait_does_not_shield_cancel(self):
        # Cancelling outer() makes wait() return early, leaves inner()
        # running.
        proof = 0
        waiter = asyncio.Future(loop=self.loop)

        @asyncio.coroutine
        def inner():
            nonlocal proof
            yield from waiter
            proof += 1

        @asyncio.coroutine
        def outer():
            nonlocal proof
            d, p = yield from asyncio.wait([inner()], loop=self.loop)
            proof += 100

        f = asyncio.async(outer(), loop=self.loop)
        test_utils.run_briefly(self.loop)
        f.cancel()
        self.assertRaises(
            asyncio.CancelledError, self.loop.run_until_complete, f)
        waiter.set_result(None)
        test_utils.run_briefly(self.loop)
        self.assertEqual(proof, 1)

    def test_shield_result(self):
        inner = asyncio.Future(loop=self.loop)
        outer = asyncio.shield(inner)
        inner.set_result(42)
        res = self.loop.run_until_complete(outer)
        self.assertEqual(res, 42)

    def test_shield_exception(self):
        inner = asyncio.Future(loop=self.loop)
        outer = asyncio.shield(inner)
        test_utils.run_briefly(self.loop)
        exc = RuntimeError('expected')
        inner.set_exception(exc)
        test_utils.run_briefly(self.loop)
        self.assertIs(outer.exception(), exc)

    def test_shield_cancel(self):
        inner = asyncio.Future(loop=self.loop)
        outer = asyncio.shield(inner)
        test_utils.run_briefly(self.loop)
        inner.cancel()
        test_utils.run_briefly(self.loop)
        self.assertTrue(outer.cancelled())

    def test_shield_shortcut(self):
        fut = asyncio.Future(loop=self.loop)
        fut.set_result(42)
        res = self.loop.run_until_complete(asyncio.shield(fut))
        self.assertEqual(res, 42)

    def test_shield_effect(self):
        # Cancelling outer() does not affect inner().
        proof = 0
        waiter = asyncio.Future(loop=self.loop)

        @asyncio.coroutine
        def inner():
            nonlocal proof
            yield from waiter
            proof += 1

        @asyncio.coroutine
        def outer():
            nonlocal proof
            yield from asyncio.shield(inner(), loop=self.loop)
            proof += 100

        f = asyncio.async(outer(), loop=self.loop)
        test_utils.run_briefly(self.loop)
        f.cancel()
        with self.assertRaises(asyncio.CancelledError):
            self.loop.run_until_complete(f)
        waiter.set_result(None)
        test_utils.run_briefly(self.loop)
        self.assertEqual(proof, 1)

    def test_shield_gather(self):
        child1 = asyncio.Future(loop=self.loop)
        child2 = asyncio.Future(loop=self.loop)
        parent = asyncio.gather(child1, child2, loop=self.loop)
        outer = asyncio.shield(parent, loop=self.loop)
        test_utils.run_briefly(self.loop)
        outer.cancel()
        test_utils.run_briefly(self.loop)
        self.assertTrue(outer.cancelled())
        child1.set_result(1)
        child2.set_result(2)
        test_utils.run_briefly(self.loop)
        self.assertEqual(parent.result(), [1, 2])

    def test_gather_shield(self):
        child1 = asyncio.Future(loop=self.loop)
        child2 = asyncio.Future(loop=self.loop)
        inner1 = asyncio.shield(child1, loop=self.loop)
        inner2 = asyncio.shield(child2, loop=self.loop)
        parent = asyncio.gather(inner1, inner2, loop=self.loop)
        test_utils.run_briefly(self.loop)
        parent.cancel()
        # This should cancel inner1 and inner2 but bot child1 and child2.
        test_utils.run_briefly(self.loop)
        self.assertIsInstance(parent.exception(), asyncio.CancelledError)
        self.assertTrue(inner1.cancelled())
        self.assertTrue(inner2.cancelled())
        child1.set_result(1)
        child2.set_result(2)
        test_utils.run_briefly(self.loop)

    def test_as_completed_invalid_args(self):
        fut = asyncio.Future(loop=self.loop)

        # as_completed() expects a list of futures, not a future instance
        self.assertRaises(TypeError, self.loop.run_until_complete,
            asyncio.as_completed(fut, loop=self.loop))
        self.assertRaises(TypeError, self.loop.run_until_complete,
            asyncio.as_completed(coroutine_function(), loop=self.loop))

    def test_wait_invalid_args(self):
        fut = asyncio.Future(loop=self.loop)

        # wait() expects a list of futures, not a future instance
        self.assertRaises(TypeError, self.loop.run_until_complete,
            asyncio.wait(fut, loop=self.loop))
        self.assertRaises(TypeError, self.loop.run_until_complete,
            asyncio.wait(coroutine_function(), loop=self.loop))

        # wait() expects at least a future
        self.assertRaises(ValueError, self.loop.run_until_complete,
            asyncio.wait([], loop=self.loop))

    def test_corowrapper_mocks_generator(self):

        def check():
            # A function that asserts various things.
            # Called twice, with different debug flag values.

            @asyncio.coroutine
            def coro():
                # The actual coroutine.
                self.assertTrue(gen.gi_running)
                yield from fut

            # A completed Future used to run the coroutine.
            fut = asyncio.Future(loop=self.loop)
            fut.set_result(None)

            # Call the coroutine.
            gen = coro()

            # Check some properties.
            self.assertTrue(asyncio.iscoroutine(gen))
            self.assertIsInstance(gen.gi_frame, types.FrameType)
            self.assertFalse(gen.gi_running)
            self.assertIsInstance(gen.gi_code, types.CodeType)

            # Run it.
            self.loop.run_until_complete(gen)

            # The frame should have changed.
            self.assertIsNone(gen.gi_frame)

        # Save debug flag.
        old_debug = asyncio.tasks._DEBUG
        try:
            # Test with debug flag cleared.
            asyncio.tasks._DEBUG = False
            check()

            # Test with debug flag set.
            asyncio.tasks._DEBUG = True
            check()

        finally:
            # Restore original debug flag.
            asyncio.tasks._DEBUG = old_debug

    def test_yield_from_corowrapper(self):
        old_debug = asyncio.tasks._DEBUG
        asyncio.tasks._DEBUG = True
        try:
            @asyncio.coroutine
            def t1():
                return (yield from t2())

            @asyncio.coroutine
            def t2():
                f = asyncio.Future(loop=self.loop)
                asyncio.Task(t3(f), loop=self.loop)
                return (yield from f)

            @asyncio.coroutine
            def t3(f):
                f.set_result((1, 2, 3))

            task = asyncio.Task(t1(), loop=self.loop)
            val = self.loop.run_until_complete(task)
            self.assertEqual(val, (1, 2, 3))
        finally:
            asyncio.tasks._DEBUG = old_debug

    def test_yield_from_corowrapper_send(self):
        def foo():
            a = yield
            return a

        def call(arg):
            cw = asyncio.tasks.CoroWrapper(foo(), foo)
            cw.send(None)
            try:
                cw.send(arg)
            except StopIteration as ex:
                return ex.args[0]
            else:
                raise AssertionError('StopIteration was expected')

        self.assertEqual(call((1, 2)), (1, 2))
        self.assertEqual(call('spam'), 'spam')

    def test_corowrapper_weakref(self):
        wd = weakref.WeakValueDictionary()
        def foo(): yield from []
        cw = asyncio.tasks.CoroWrapper(foo(), foo)
        wd['cw'] = cw  # Would fail without __weakref__ slot.
        cw.gen = None  # Suppress warning from __del__.


class GatherTestsBase:

    def setUp(self):
        self.one_loop = test_utils.TestLoop()
        self.other_loop = test_utils.TestLoop()

    def tearDown(self):
        self.one_loop.close()
        self.other_loop.close()

    def _run_loop(self, loop):
        while loop._ready:
            test_utils.run_briefly(loop)

    def _check_success(self, **kwargs):
        a, b, c = [asyncio.Future(loop=self.one_loop) for i in range(3)]
        fut = asyncio.gather(*self.wrap_futures(a, b, c), **kwargs)
        cb = test_utils.MockCallback()
        fut.add_done_callback(cb)
        b.set_result(1)
        a.set_result(2)
        self._run_loop(self.one_loop)
        self.assertEqual(cb.called, False)
        self.assertFalse(fut.done())
        c.set_result(3)
        self._run_loop(self.one_loop)
        cb.assert_called_once_with(fut)
        self.assertEqual(fut.result(), [2, 1, 3])

    def test_success(self):
        self._check_success()
        self._check_success(return_exceptions=False)

    def test_result_exception_success(self):
        self._check_success(return_exceptions=True)

    def test_one_exception(self):
        a, b, c, d, e = [asyncio.Future(loop=self.one_loop) for i in range(5)]
        fut = asyncio.gather(*self.wrap_futures(a, b, c, d, e))
        cb = test_utils.MockCallback()
        fut.add_done_callback(cb)
        exc = ZeroDivisionError()
        a.set_result(1)
        b.set_exception(exc)
        self._run_loop(self.one_loop)
        self.assertTrue(fut.done())
        cb.assert_called_once_with(fut)
        self.assertIs(fut.exception(), exc)
        # Does nothing
        c.set_result(3)
        d.cancel()
        e.set_exception(RuntimeError())
        e.exception()

    def test_return_exceptions(self):
        a, b, c, d = [asyncio.Future(loop=self.one_loop) for i in range(4)]
        fut = asyncio.gather(*self.wrap_futures(a, b, c, d),
                             return_exceptions=True)
        cb = test_utils.MockCallback()
        fut.add_done_callback(cb)
        exc = ZeroDivisionError()
        exc2 = RuntimeError()
        b.set_result(1)
        c.set_exception(exc)
        a.set_result(3)
        self._run_loop(self.one_loop)
        self.assertFalse(fut.done())
        d.set_exception(exc2)
        self._run_loop(self.one_loop)
        self.assertTrue(fut.done())
        cb.assert_called_once_with(fut)
        self.assertEqual(fut.result(), [3, 1, exc, exc2])

    def test_env_var_debug(self):
        path = os.path.dirname(asyncio.__file__)
        path = os.path.normpath(os.path.join(path, '..'))
        code = '\n'.join((
            'import sys',
            'sys.path.insert(0, %r)' % path,
            'import asyncio.tasks',
            'print(asyncio.tasks._DEBUG)'))

        # Test with -E to not fail if the unit test was run with
        # PYTHONASYNCIODEBUG set to a non-empty string
        sts, stdout, stderr = assert_python_ok('-E', '-c', code)
        self.assertEqual(stdout.rstrip(), b'False')

        sts, stdout, stderr = assert_python_ok('-c', code,
                                               PYTHONASYNCIODEBUG='')
        self.assertEqual(stdout.rstrip(), b'False')

        sts, stdout, stderr = assert_python_ok('-c', code,
                                               PYTHONASYNCIODEBUG='1')
        self.assertEqual(stdout.rstrip(), b'True')

        sts, stdout, stderr = assert_python_ok('-E', '-c', code,
                                               PYTHONASYNCIODEBUG='1')
        self.assertEqual(stdout.rstrip(), b'False')


class FutureGatherTests(GatherTestsBase, unittest.TestCase):

    def wrap_futures(self, *futures):
        return futures

    def _check_empty_sequence(self, seq_or_iter):
        asyncio.set_event_loop(self.one_loop)
        self.addCleanup(asyncio.set_event_loop, None)
        fut = asyncio.gather(*seq_or_iter)
        self.assertIsInstance(fut, asyncio.Future)
        self.assertIs(fut._loop, self.one_loop)
        self._run_loop(self.one_loop)
        self.assertTrue(fut.done())
        self.assertEqual(fut.result(), [])
        fut = asyncio.gather(*seq_or_iter, loop=self.other_loop)
        self.assertIs(fut._loop, self.other_loop)

    def test_constructor_empty_sequence(self):
        self._check_empty_sequence([])
        self._check_empty_sequence(())
        self._check_empty_sequence(set())
        self._check_empty_sequence(iter(""))

    def test_constructor_heterogenous_futures(self):
        fut1 = asyncio.Future(loop=self.one_loop)
        fut2 = asyncio.Future(loop=self.other_loop)
        with self.assertRaises(ValueError):
            asyncio.gather(fut1, fut2)
        with self.assertRaises(ValueError):
            asyncio.gather(fut1, loop=self.other_loop)

    def test_constructor_homogenous_futures(self):
        children = [asyncio.Future(loop=self.other_loop) for i in range(3)]
        fut = asyncio.gather(*children)
        self.assertIs(fut._loop, self.other_loop)
        self._run_loop(self.other_loop)
        self.assertFalse(fut.done())
        fut = asyncio.gather(*children, loop=self.other_loop)
        self.assertIs(fut._loop, self.other_loop)
        self._run_loop(self.other_loop)
        self.assertFalse(fut.done())

    def test_one_cancellation(self):
        a, b, c, d, e = [asyncio.Future(loop=self.one_loop) for i in range(5)]
        fut = asyncio.gather(a, b, c, d, e)
        cb = test_utils.MockCallback()
        fut.add_done_callback(cb)
        a.set_result(1)
        b.cancel()
        self._run_loop(self.one_loop)
        self.assertTrue(fut.done())
        cb.assert_called_once_with(fut)
        self.assertFalse(fut.cancelled())
        self.assertIsInstance(fut.exception(), asyncio.CancelledError)
        # Does nothing
        c.set_result(3)
        d.cancel()
        e.set_exception(RuntimeError())
        e.exception()

    def test_result_exception_one_cancellation(self):
        a, b, c, d, e, f = [asyncio.Future(loop=self.one_loop)
                            for i in range(6)]
        fut = asyncio.gather(a, b, c, d, e, f, return_exceptions=True)
        cb = test_utils.MockCallback()
        fut.add_done_callback(cb)
        a.set_result(1)
        zde = ZeroDivisionError()
        b.set_exception(zde)
        c.cancel()
        self._run_loop(self.one_loop)
        self.assertFalse(fut.done())
        d.set_result(3)
        e.cancel()
        rte = RuntimeError()
        f.set_exception(rte)
        res = self.one_loop.run_until_complete(fut)
        self.assertIsInstance(res[2], asyncio.CancelledError)
        self.assertIsInstance(res[4], asyncio.CancelledError)
        res[2] = res[4] = None
        self.assertEqual(res, [1, zde, None, 3, None, rte])
        cb.assert_called_once_with(fut)


class CoroutineGatherTests(GatherTestsBase, unittest.TestCase):

    def setUp(self):
        super().setUp()
        asyncio.set_event_loop(self.one_loop)

    def tearDown(self):
        asyncio.set_event_loop(None)
        super().tearDown()

    def wrap_futures(self, *futures):
        coros = []
        for fut in futures:
            @asyncio.coroutine
            def coro(fut=fut):
                return (yield from fut)
            coros.append(coro())
        return coros

    def test_constructor_loop_selection(self):
        @asyncio.coroutine
        def coro():
            return 'abc'
        gen1 = coro()
        gen2 = coro()
        fut = asyncio.gather(gen1, gen2)
        self.assertIs(fut._loop, self.one_loop)
        gen1.close()
        gen2.close()
        gen3 = coro()
        gen4 = coro()
        fut = asyncio.gather(gen3, gen4, loop=self.other_loop)
        self.assertIs(fut._loop, self.other_loop)
        gen3.close()
        gen4.close()

    def test_duplicate_coroutines(self):
        @asyncio.coroutine
        def coro(s):
            return s
        c = coro('abc')
        fut = asyncio.gather(c, c, coro('def'), c, loop=self.one_loop)
        self._run_loop(self.one_loop)
        self.assertEqual(fut.result(), ['abc', 'abc', 'def', 'abc'])

    def test_cancellation_broadcast(self):
        # Cancelling outer() cancels all children.
        proof = 0
        waiter = asyncio.Future(loop=self.one_loop)

        @asyncio.coroutine
        def inner():
            nonlocal proof
            yield from waiter
            proof += 1

        child1 = asyncio.async(inner(), loop=self.one_loop)
        child2 = asyncio.async(inner(), loop=self.one_loop)
        gatherer = None

        @asyncio.coroutine
        def outer():
            nonlocal proof, gatherer
            gatherer = asyncio.gather(child1, child2, loop=self.one_loop)
            yield from gatherer
            proof += 100

        f = asyncio.async(outer(), loop=self.one_loop)
        test_utils.run_briefly(self.one_loop)
        self.assertTrue(f.cancel())
        with self.assertRaises(asyncio.CancelledError):
            self.one_loop.run_until_complete(f)
        self.assertFalse(gatherer.cancel())
        self.assertTrue(waiter.cancelled())
        self.assertTrue(child1.cancelled())
        self.assertTrue(child2.cancelled())
        test_utils.run_briefly(self.one_loop)
        self.assertEqual(proof, 0)

    def test_exception_marking(self):
        # Test for the first line marked "Mark exception retrieved."

        @asyncio.coroutine
        def inner(f):
            yield from f
            raise RuntimeError('should not be ignored')

        a = asyncio.Future(loop=self.one_loop)
        b = asyncio.Future(loop=self.one_loop)

        @asyncio.coroutine
        def outer():
            yield from asyncio.gather(inner(a), inner(b), loop=self.one_loop)

        f = asyncio.async(outer(), loop=self.one_loop)
        test_utils.run_briefly(self.one_loop)
        a.set_result(None)
        test_utils.run_briefly(self.one_loop)
        b.set_result(None)
        test_utils.run_briefly(self.one_loop)
        self.assertIsInstance(f.exception(), RuntimeError)


if __name__ == '__main__':
    unittest.main()
