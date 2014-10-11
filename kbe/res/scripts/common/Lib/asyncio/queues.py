"""Queues"""

__all__ = ['Queue', 'PriorityQueue', 'LifoQueue', 'JoinableQueue',
           'QueueFull', 'QueueEmpty']

import collections
import heapq

from . import events
from . import futures
from . import locks
from .tasks import coroutine


class QueueEmpty(Exception):
    'Exception raised by Queue.get(block=0)/get_nowait().'
    pass


class QueueFull(Exception):
    'Exception raised by Queue.put(block=0)/put_nowait().'
    pass


class Queue:
    """A queue, useful for coordinating producer and consumer coroutines.

    If maxsize is less than or equal to zero, the queue size is infinite. If it
    is an integer greater than 0, then "yield from put()" will block when the
    queue reaches maxsize, until an item is removed by get().

    Unlike the standard library Queue, you can reliably know this Queue's size
    with qsize(), since your single-threaded asyncio application won't be
    interrupted between calling qsize() and doing an operation on the Queue.
    """

    def __init__(self, maxsize=0, *, loop=None):
        if loop is None:
            self._loop = events.get_event_loop()
        else:
            self._loop = loop
        self._maxsize = maxsize

        # Futures.
        self._getters = collections.deque()
        # Pairs of (item, Future).
        self._putters = collections.deque()
        self._init(maxsize)

    def _init(self, maxsize):
        self._queue = collections.deque()

    def _get(self):
        return self._queue.popleft()

    def _put(self, item):
        self._queue.append(item)

    def __repr__(self):
        return '<{} at {:#x} {}>'.format(
            type(self).__name__, id(self), self._format())

    def __str__(self):
        return '<{} {}>'.format(type(self).__name__, self._format())

    def _format(self):
        result = 'maxsize={!r}'.format(self._maxsize)
        if getattr(self, '_queue', None):
            result += ' _queue={!r}'.format(list(self._queue))
        if self._getters:
            result += ' _getters[{}]'.format(len(self._getters))
        if self._putters:
            result += ' _putters[{}]'.format(len(self._putters))
        return result

    def _consume_done_getters(self):
        # Delete waiters at the head of the get() queue who've timed out.
        while self._getters and self._getters[0].done():
            self._getters.popleft()

    def _consume_done_putters(self):
        # Delete waiters at the head of the put() queue who've timed out.
        while self._putters and self._putters[0][1].done():
            self._putters.popleft()

    def qsize(self):
        """Number of items in the queue."""
        return len(self._queue)

    @property
    def maxsize(self):
        """Number of items allowed in the queue."""
        return self._maxsize

    def empty(self):
        """Return True if the queue is empty, False otherwise."""
        return not self._queue

    def full(self):
        """Return True if there are maxsize items in the queue.

        Note: if the Queue was initialized with maxsize=0 (the default),
        then full() is never True.
        """
        if self._maxsize <= 0:
            return False
        else:
            return self.qsize() == self._maxsize

    @coroutine
    def put(self, item):
        """Put an item into the queue.

        If you yield from put(), wait until a free slot is available
        before adding item.
        """
        self._consume_done_getters()
        if self._getters:
            assert not self._queue, (
                'queue non-empty, why are getters waiting?')

            getter = self._getters.popleft()

            # Use _put and _get instead of passing item straight to getter, in
            # case a subclass has logic that must run (e.g. JoinableQueue).
            self._put(item)
            getter.set_result(self._get())

        elif self._maxsize > 0 and self._maxsize == self.qsize():
            waiter = futures.Future(loop=self._loop)

            self._putters.append((item, waiter))
            yield from waiter

        else:
            self._put(item)

    def put_nowait(self, item):
        """Put an item into the queue without blocking.

        If no free slot is immediately available, raise QueueFull.
        """
        self._consume_done_getters()
        if self._getters:
            assert not self._queue, (
                'queue non-empty, why are getters waiting?')

            getter = self._getters.popleft()

            # Use _put and _get instead of passing item straight to getter, in
            # case a subclass has logic that must run (e.g. JoinableQueue).
            self._put(item)
            getter.set_result(self._get())

        elif self._maxsize > 0 and self._maxsize == self.qsize():
            raise QueueFull
        else:
            self._put(item)

    @coroutine
    def get(self):
        """Remove and return an item from the queue.

        If you yield from get(), wait until a item is available.
        """
        self._consume_done_putters()
        if self._putters:
            assert self.full(), 'queue not full, why are putters waiting?'
            item, putter = self._putters.popleft()
            self._put(item)

            # When a getter runs and frees up a slot so this putter can
            # run, we need to defer the put for a tick to ensure that
            # getters and putters alternate perfectly. See
            # ChannelTest.test_wait.
            self._loop.call_soon(putter.set_result, None)

            return self._get()

        elif self.qsize():
            return self._get()
        else:
            waiter = futures.Future(loop=self._loop)

            self._getters.append(waiter)
            return (yield from waiter)

    def get_nowait(self):
        """Remove and return an item from the queue.

        Return an item if one is immediately available, else raise QueueEmpty.
        """
        self._consume_done_putters()
        if self._putters:
            assert self.full(), 'queue not full, why are putters waiting?'
            item, putter = self._putters.popleft()
            self._put(item)
            # Wake putter on next tick.
            putter.set_result(None)

            return self._get()

        elif self.qsize():
            return self._get()
        else:
            raise QueueEmpty


class PriorityQueue(Queue):
    """A subclass of Queue; retrieves entries in priority order (lowest first).

    Entries are typically tuples of the form: (priority number, data).
    """

    def _init(self, maxsize):
        self._queue = []

    def _put(self, item, heappush=heapq.heappush):
        heappush(self._queue, item)

    def _get(self, heappop=heapq.heappop):
        return heappop(self._queue)


class LifoQueue(Queue):
    """A subclass of Queue that retrieves most recently added entries first."""

    def _init(self, maxsize):
        self._queue = []

    def _put(self, item):
        self._queue.append(item)

    def _get(self):
        return self._queue.pop()


class JoinableQueue(Queue):
    """A subclass of Queue with task_done() and join() methods."""

    def __init__(self, maxsize=0, *, loop=None):
        super().__init__(maxsize=maxsize, loop=loop)
        self._unfinished_tasks = 0
        self._finished = locks.Event(loop=self._loop)
        self._finished.set()

    def _format(self):
        result = Queue._format(self)
        if self._unfinished_tasks:
            result += ' tasks={}'.format(self._unfinished_tasks)
        return result

    def _put(self, item):
        super()._put(item)
        self._unfinished_tasks += 1
        self._finished.clear()

    def task_done(self):
        """Indicate that a formerly enqueued task is complete.

        Used by queue consumers. For each get() used to fetch a task,
        a subsequent call to task_done() tells the queue that the processing
        on the task is complete.

        If a join() is currently blocking, it will resume when all items have
        been processed (meaning that a task_done() call was received for every
        item that had been put() into the queue).

        Raises ValueError if called more times than there were items placed in
        the queue.
        """
        if self._unfinished_tasks <= 0:
            raise ValueError('task_done() called too many times')
        self._unfinished_tasks -= 1
        if self._unfinished_tasks == 0:
            self._finished.set()

    @coroutine
    def join(self):
        """Block until all items in the queue have been gotten and processed.

        The count of unfinished tasks goes up whenever an item is added to the
        queue. The count goes down whenever a consumer thread calls task_done()
        to indicate that the item was retrieved and all work on it is complete.
        When the count of unfinished tasks drops to zero, join() unblocks.
        """
        if self._unfinished_tasks > 0:
            yield from self._finished.wait()
