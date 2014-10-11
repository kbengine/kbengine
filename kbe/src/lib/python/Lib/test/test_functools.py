import abc
import collections
from itertools import permutations
import pickle
from random import choice
import sys
from test import support
import unittest
from weakref import proxy

import functools

py_functools = support.import_fresh_module('functools', blocked=['_functools'])
c_functools = support.import_fresh_module('functools', fresh=['_functools'])

decimal = support.import_fresh_module('decimal', fresh=['_decimal'])


def capture(*args, **kw):
    """capture all positional and keyword arguments"""
    return args, kw


def signature(part):
    """ return the signature of a partial object """
    return (part.func, part.args, part.keywords, part.__dict__)


class TestPartial:

    def test_basic_examples(self):
        p = self.partial(capture, 1, 2, a=10, b=20)
        self.assertTrue(callable(p))
        self.assertEqual(p(3, 4, b=30, c=40),
                         ((1, 2, 3, 4), dict(a=10, b=30, c=40)))
        p = self.partial(map, lambda x: x*10)
        self.assertEqual(list(p([1,2,3,4])), [10, 20, 30, 40])

    def test_attributes(self):
        p = self.partial(capture, 1, 2, a=10, b=20)
        # attributes should be readable
        self.assertEqual(p.func, capture)
        self.assertEqual(p.args, (1, 2))
        self.assertEqual(p.keywords, dict(a=10, b=20))

    def test_argument_checking(self):
        self.assertRaises(TypeError, self.partial)     # need at least a func arg
        try:
            self.partial(2)()
        except TypeError:
            pass
        else:
            self.fail('First arg not checked for callability')

    def test_protection_of_callers_dict_argument(self):
        # a caller's dictionary should not be altered by partial
        def func(a=10, b=20):
            return a
        d = {'a':3}
        p = self.partial(func, a=5)
        self.assertEqual(p(**d), 3)
        self.assertEqual(d, {'a':3})
        p(b=7)
        self.assertEqual(d, {'a':3})

    def test_arg_combinations(self):
        # exercise special code paths for zero args in either partial
        # object or the caller
        p = self.partial(capture)
        self.assertEqual(p(), ((), {}))
        self.assertEqual(p(1,2), ((1,2), {}))
        p = self.partial(capture, 1, 2)
        self.assertEqual(p(), ((1,2), {}))
        self.assertEqual(p(3,4), ((1,2,3,4), {}))

    def test_kw_combinations(self):
        # exercise special code paths for no keyword args in
        # either the partial object or the caller
        p = self.partial(capture)
        self.assertEqual(p(), ((), {}))
        self.assertEqual(p(a=1), ((), {'a':1}))
        p = self.partial(capture, a=1)
        self.assertEqual(p(), ((), {'a':1}))
        self.assertEqual(p(b=2), ((), {'a':1, 'b':2}))
        # keyword args in the call override those in the partial object
        self.assertEqual(p(a=3, b=2), ((), {'a':3, 'b':2}))

    def test_positional(self):
        # make sure positional arguments are captured correctly
        for args in [(), (0,), (0,1), (0,1,2), (0,1,2,3)]:
            p = self.partial(capture, *args)
            expected = args + ('x',)
            got, empty = p('x')
            self.assertTrue(expected == got and empty == {})

    def test_keyword(self):
        # make sure keyword arguments are captured correctly
        for a in ['a', 0, None, 3.5]:
            p = self.partial(capture, a=a)
            expected = {'a':a,'x':None}
            empty, got = p(x=None)
            self.assertTrue(expected == got and empty == ())

    def test_no_side_effects(self):
        # make sure there are no side effects that affect subsequent calls
        p = self.partial(capture, 0, a=1)
        args1, kw1 = p(1, b=2)
        self.assertTrue(args1 == (0,1) and kw1 == {'a':1,'b':2})
        args2, kw2 = p()
        self.assertTrue(args2 == (0,) and kw2 == {'a':1})

    def test_error_propagation(self):
        def f(x, y):
            x / y
        self.assertRaises(ZeroDivisionError, self.partial(f, 1, 0))
        self.assertRaises(ZeroDivisionError, self.partial(f, 1), 0)
        self.assertRaises(ZeroDivisionError, self.partial(f), 1, 0)
        self.assertRaises(ZeroDivisionError, self.partial(f, y=0), 1)

    def test_weakref(self):
        f = self.partial(int, base=16)
        p = proxy(f)
        self.assertEqual(f.func, p.func)
        f = None
        self.assertRaises(ReferenceError, getattr, p, 'func')

    def test_with_bound_and_unbound_methods(self):
        data = list(map(str, range(10)))
        join = self.partial(str.join, '')
        self.assertEqual(join(data), '0123456789')
        join = self.partial(''.join)
        self.assertEqual(join(data), '0123456789')


@unittest.skipUnless(c_functools, 'requires the C _functools module')
class TestPartialC(TestPartial, unittest.TestCase):
    if c_functools:
        partial = c_functools.partial

    def test_attributes_unwritable(self):
        # attributes should not be writable
        p = self.partial(capture, 1, 2, a=10, b=20)
        self.assertRaises(AttributeError, setattr, p, 'func', map)
        self.assertRaises(AttributeError, setattr, p, 'args', (1, 2))
        self.assertRaises(AttributeError, setattr, p, 'keywords', dict(a=1, b=2))

        p = self.partial(hex)
        try:
            del p.__dict__
        except TypeError:
            pass
        else:
            self.fail('partial object allowed __dict__ to be deleted')

    def test_repr(self):
        args = (object(), object())
        args_repr = ', '.join(repr(a) for a in args)
        #kwargs = {'a': object(), 'b': object()}
        kwargs = {'a': object()}
        kwargs_repr = ', '.join("%s=%r" % (k, v) for k, v in kwargs.items())
        if self.partial is c_functools.partial:
            name = 'functools.partial'
        else:
            name = self.partial.__name__

        f = self.partial(capture)
        self.assertEqual('{}({!r})'.format(name, capture),
                         repr(f))

        f = self.partial(capture, *args)
        self.assertEqual('{}({!r}, {})'.format(name, capture, args_repr),
                         repr(f))

        f = self.partial(capture, **kwargs)
        self.assertEqual('{}({!r}, {})'.format(name, capture, kwargs_repr),
                         repr(f))

        f = self.partial(capture, *args, **kwargs)
        self.assertEqual('{}({!r}, {}, {})'.format(name, capture, args_repr, kwargs_repr),
                         repr(f))

    def test_pickle(self):
        f = self.partial(signature, 'asdf', bar=True)
        f.add_something_to__dict__ = True
        f_copy = pickle.loads(pickle.dumps(f))
        self.assertEqual(signature(f), signature(f_copy))

    # Issue 6083: Reference counting bug
    def test_setstate_refcount(self):
        class BadSequence:
            def __len__(self):
                return 4
            def __getitem__(self, key):
                if key == 0:
                    return max
                elif key == 1:
                    return tuple(range(1000000))
                elif key in (2, 3):
                    return {}
                raise IndexError

        f = self.partial(object)
        self.assertRaisesRegex(SystemError,
                "new style getargs format but argument is not a tuple",
                f.__setstate__, BadSequence())


class TestPartialPy(TestPartial, unittest.TestCase):
    partial = staticmethod(py_functools.partial)


if c_functools:
    class PartialSubclass(c_functools.partial):
        pass


@unittest.skipUnless(c_functools, 'requires the C _functools module')
class TestPartialCSubclass(TestPartialC):
    if c_functools:
        partial = PartialSubclass


class TestPartialMethod(unittest.TestCase):

    class A(object):
        nothing = functools.partialmethod(capture)
        positional = functools.partialmethod(capture, 1)
        keywords = functools.partialmethod(capture, a=2)
        both = functools.partialmethod(capture, 3, b=4)

        nested = functools.partialmethod(positional, 5)

        over_partial = functools.partialmethod(functools.partial(capture, c=6), 7)

        static = functools.partialmethod(staticmethod(capture), 8)
        cls = functools.partialmethod(classmethod(capture), d=9)

    a = A()

    def test_arg_combinations(self):
        self.assertEqual(self.a.nothing(), ((self.a,), {}))
        self.assertEqual(self.a.nothing(5), ((self.a, 5), {}))
        self.assertEqual(self.a.nothing(c=6), ((self.a,), {'c': 6}))
        self.assertEqual(self.a.nothing(5, c=6), ((self.a, 5), {'c': 6}))

        self.assertEqual(self.a.positional(), ((self.a, 1), {}))
        self.assertEqual(self.a.positional(5), ((self.a, 1, 5), {}))
        self.assertEqual(self.a.positional(c=6), ((self.a, 1), {'c': 6}))
        self.assertEqual(self.a.positional(5, c=6), ((self.a, 1, 5), {'c': 6}))

        self.assertEqual(self.a.keywords(), ((self.a,), {'a': 2}))
        self.assertEqual(self.a.keywords(5), ((self.a, 5), {'a': 2}))
        self.assertEqual(self.a.keywords(c=6), ((self.a,), {'a': 2, 'c': 6}))
        self.assertEqual(self.a.keywords(5, c=6), ((self.a, 5), {'a': 2, 'c': 6}))

        self.assertEqual(self.a.both(), ((self.a, 3), {'b': 4}))
        self.assertEqual(self.a.both(5), ((self.a, 3, 5), {'b': 4}))
        self.assertEqual(self.a.both(c=6), ((self.a, 3), {'b': 4, 'c': 6}))
        self.assertEqual(self.a.both(5, c=6), ((self.a, 3, 5), {'b': 4, 'c': 6}))

        self.assertEqual(self.A.both(self.a, 5, c=6), ((self.a, 3, 5), {'b': 4, 'c': 6}))

    def test_nested(self):
        self.assertEqual(self.a.nested(), ((self.a, 1, 5), {}))
        self.assertEqual(self.a.nested(6), ((self.a, 1, 5, 6), {}))
        self.assertEqual(self.a.nested(d=7), ((self.a, 1, 5), {'d': 7}))
        self.assertEqual(self.a.nested(6, d=7), ((self.a, 1, 5, 6), {'d': 7}))

        self.assertEqual(self.A.nested(self.a, 6, d=7), ((self.a, 1, 5, 6), {'d': 7}))

    def test_over_partial(self):
        self.assertEqual(self.a.over_partial(), ((self.a, 7), {'c': 6}))
        self.assertEqual(self.a.over_partial(5), ((self.a, 7, 5), {'c': 6}))
        self.assertEqual(self.a.over_partial(d=8), ((self.a, 7), {'c': 6, 'd': 8}))
        self.assertEqual(self.a.over_partial(5, d=8), ((self.a, 7, 5), {'c': 6, 'd': 8}))

        self.assertEqual(self.A.over_partial(self.a, 5, d=8), ((self.a, 7, 5), {'c': 6, 'd': 8}))

    def test_bound_method_introspection(self):
        obj = self.a
        self.assertIs(obj.both.__self__, obj)
        self.assertIs(obj.nested.__self__, obj)
        self.assertIs(obj.over_partial.__self__, obj)
        self.assertIs(obj.cls.__self__, self.A)
        self.assertIs(self.A.cls.__self__, self.A)

    def test_unbound_method_retrieval(self):
        obj = self.A
        self.assertFalse(hasattr(obj.both, "__self__"))
        self.assertFalse(hasattr(obj.nested, "__self__"))
        self.assertFalse(hasattr(obj.over_partial, "__self__"))
        self.assertFalse(hasattr(obj.static, "__self__"))
        self.assertFalse(hasattr(self.a.static, "__self__"))

    def test_descriptors(self):
        for obj in [self.A, self.a]:
            with self.subTest(obj=obj):
                self.assertEqual(obj.static(), ((8,), {}))
                self.assertEqual(obj.static(5), ((8, 5), {}))
                self.assertEqual(obj.static(d=8), ((8,), {'d': 8}))
                self.assertEqual(obj.static(5, d=8), ((8, 5), {'d': 8}))

                self.assertEqual(obj.cls(), ((self.A,), {'d': 9}))
                self.assertEqual(obj.cls(5), ((self.A, 5), {'d': 9}))
                self.assertEqual(obj.cls(c=8), ((self.A,), {'c': 8, 'd': 9}))
                self.assertEqual(obj.cls(5, c=8), ((self.A, 5), {'c': 8, 'd': 9}))

    def test_overriding_keywords(self):
        self.assertEqual(self.a.keywords(a=3), ((self.a,), {'a': 3}))
        self.assertEqual(self.A.keywords(self.a, a=3), ((self.a,), {'a': 3}))

    def test_invalid_args(self):
        with self.assertRaises(TypeError):
            class B(object):
                method = functools.partialmethod(None, 1)

    def test_repr(self):
        self.assertEqual(repr(vars(self.A)['both']),
                         'functools.partialmethod({}, 3, b=4)'.format(capture))

    def test_abstract(self):
        class Abstract(abc.ABCMeta):

            @abc.abstractmethod
            def add(self, x, y):
                pass

            add5 = functools.partialmethod(add, 5)

        self.assertTrue(Abstract.add.__isabstractmethod__)
        self.assertTrue(Abstract.add5.__isabstractmethod__)

        for func in [self.A.static, self.A.cls, self.A.over_partial, self.A.nested, self.A.both]:
            self.assertFalse(getattr(func, '__isabstractmethod__', False))


class TestUpdateWrapper(unittest.TestCase):

    def check_wrapper(self, wrapper, wrapped,
                      assigned=functools.WRAPPER_ASSIGNMENTS,
                      updated=functools.WRAPPER_UPDATES):
        # Check attributes were assigned
        for name in assigned:
            self.assertIs(getattr(wrapper, name), getattr(wrapped, name))
        # Check attributes were updated
        for name in updated:
            wrapper_attr = getattr(wrapper, name)
            wrapped_attr = getattr(wrapped, name)
            for key in wrapped_attr:
                if name == "__dict__" and key == "__wrapped__":
                    # __wrapped__ is overwritten by the update code
                    continue
                self.assertIs(wrapped_attr[key], wrapper_attr[key])
        # Check __wrapped__
        self.assertIs(wrapper.__wrapped__, wrapped)


    def _default_update(self):
        def f(a:'This is a new annotation'):
            """This is a test"""
            pass
        f.attr = 'This is also a test'
        f.__wrapped__ = "This is a bald faced lie"
        def wrapper(b:'This is the prior annotation'):
            pass
        functools.update_wrapper(wrapper, f)
        return wrapper, f

    def test_default_update(self):
        wrapper, f = self._default_update()
        self.check_wrapper(wrapper, f)
        self.assertIs(wrapper.__wrapped__, f)
        self.assertEqual(wrapper.__name__, 'f')
        self.assertEqual(wrapper.__qualname__, f.__qualname__)
        self.assertEqual(wrapper.attr, 'This is also a test')
        self.assertEqual(wrapper.__annotations__['a'], 'This is a new annotation')
        self.assertNotIn('b', wrapper.__annotations__)

    @unittest.skipIf(sys.flags.optimize >= 2,
                     "Docstrings are omitted with -O2 and above")
    def test_default_update_doc(self):
        wrapper, f = self._default_update()
        self.assertEqual(wrapper.__doc__, 'This is a test')

    def test_no_update(self):
        def f():
            """This is a test"""
            pass
        f.attr = 'This is also a test'
        def wrapper():
            pass
        functools.update_wrapper(wrapper, f, (), ())
        self.check_wrapper(wrapper, f, (), ())
        self.assertEqual(wrapper.__name__, 'wrapper')
        self.assertNotEqual(wrapper.__qualname__, f.__qualname__)
        self.assertEqual(wrapper.__doc__, None)
        self.assertEqual(wrapper.__annotations__, {})
        self.assertFalse(hasattr(wrapper, 'attr'))

    def test_selective_update(self):
        def f():
            pass
        f.attr = 'This is a different test'
        f.dict_attr = dict(a=1, b=2, c=3)
        def wrapper():
            pass
        wrapper.dict_attr = {}
        assign = ('attr',)
        update = ('dict_attr',)
        functools.update_wrapper(wrapper, f, assign, update)
        self.check_wrapper(wrapper, f, assign, update)
        self.assertEqual(wrapper.__name__, 'wrapper')
        self.assertNotEqual(wrapper.__qualname__, f.__qualname__)
        self.assertEqual(wrapper.__doc__, None)
        self.assertEqual(wrapper.attr, 'This is a different test')
        self.assertEqual(wrapper.dict_attr, f.dict_attr)

    def test_missing_attributes(self):
        def f():
            pass
        def wrapper():
            pass
        wrapper.dict_attr = {}
        assign = ('attr',)
        update = ('dict_attr',)
        # Missing attributes on wrapped object are ignored
        functools.update_wrapper(wrapper, f, assign, update)
        self.assertNotIn('attr', wrapper.__dict__)
        self.assertEqual(wrapper.dict_attr, {})
        # Wrapper must have expected attributes for updating
        del wrapper.dict_attr
        with self.assertRaises(AttributeError):
            functools.update_wrapper(wrapper, f, assign, update)
        wrapper.dict_attr = 1
        with self.assertRaises(AttributeError):
            functools.update_wrapper(wrapper, f, assign, update)

    @support.requires_docstrings
    @unittest.skipIf(sys.flags.optimize >= 2,
                     "Docstrings are omitted with -O2 and above")
    def test_builtin_update(self):
        # Test for bug #1576241
        def wrapper():
            pass
        functools.update_wrapper(wrapper, max)
        self.assertEqual(wrapper.__name__, 'max')
        self.assertTrue(wrapper.__doc__.startswith('max('))
        self.assertEqual(wrapper.__annotations__, {})


class TestWraps(TestUpdateWrapper):

    def _default_update(self):
        def f():
            """This is a test"""
            pass
        f.attr = 'This is also a test'
        f.__wrapped__ = "This is still a bald faced lie"
        @functools.wraps(f)
        def wrapper():
            pass
        return wrapper, f

    def test_default_update(self):
        wrapper, f = self._default_update()
        self.check_wrapper(wrapper, f)
        self.assertEqual(wrapper.__name__, 'f')
        self.assertEqual(wrapper.__qualname__, f.__qualname__)
        self.assertEqual(wrapper.attr, 'This is also a test')

    @unittest.skipIf(sys.flags.optimize >= 2,
                     "Docstrings are omitted with -O2 and above")
    def test_default_update_doc(self):
        wrapper, _ = self._default_update()
        self.assertEqual(wrapper.__doc__, 'This is a test')

    def test_no_update(self):
        def f():
            """This is a test"""
            pass
        f.attr = 'This is also a test'
        @functools.wraps(f, (), ())
        def wrapper():
            pass
        self.check_wrapper(wrapper, f, (), ())
        self.assertEqual(wrapper.__name__, 'wrapper')
        self.assertNotEqual(wrapper.__qualname__, f.__qualname__)
        self.assertEqual(wrapper.__doc__, None)
        self.assertFalse(hasattr(wrapper, 'attr'))

    def test_selective_update(self):
        def f():
            pass
        f.attr = 'This is a different test'
        f.dict_attr = dict(a=1, b=2, c=3)
        def add_dict_attr(f):
            f.dict_attr = {}
            return f
        assign = ('attr',)
        update = ('dict_attr',)
        @functools.wraps(f, assign, update)
        @add_dict_attr
        def wrapper():
            pass
        self.check_wrapper(wrapper, f, assign, update)
        self.assertEqual(wrapper.__name__, 'wrapper')
        self.assertNotEqual(wrapper.__qualname__, f.__qualname__)
        self.assertEqual(wrapper.__doc__, None)
        self.assertEqual(wrapper.attr, 'This is a different test')
        self.assertEqual(wrapper.dict_attr, f.dict_attr)


class TestReduce(unittest.TestCase):
    func = functools.reduce

    def test_reduce(self):
        class Squares:
            def __init__(self, max):
                self.max = max
                self.sofar = []

            def __len__(self):
                return len(self.sofar)

            def __getitem__(self, i):
                if not 0 <= i < self.max: raise IndexError
                n = len(self.sofar)
                while n <= i:
                    self.sofar.append(n*n)
                    n += 1
                return self.sofar[i]
        def add(x, y):
            return x + y
        self.assertEqual(self.func(add, ['a', 'b', 'c'], ''), 'abc')
        self.assertEqual(
            self.func(add, [['a', 'c'], [], ['d', 'w']], []),
            ['a','c','d','w']
        )
        self.assertEqual(self.func(lambda x, y: x*y, range(2,8), 1), 5040)
        self.assertEqual(
            self.func(lambda x, y: x*y, range(2,21), 1),
            2432902008176640000
        )
        self.assertEqual(self.func(add, Squares(10)), 285)
        self.assertEqual(self.func(add, Squares(10), 0), 285)
        self.assertEqual(self.func(add, Squares(0), 0), 0)
        self.assertRaises(TypeError, self.func)
        self.assertRaises(TypeError, self.func, 42, 42)
        self.assertRaises(TypeError, self.func, 42, 42, 42)
        self.assertEqual(self.func(42, "1"), "1") # func is never called with one item
        self.assertEqual(self.func(42, "", "1"), "1") # func is never called with one item
        self.assertRaises(TypeError, self.func, 42, (42, 42))
        self.assertRaises(TypeError, self.func, add, []) # arg 2 must not be empty sequence with no initial value
        self.assertRaises(TypeError, self.func, add, "")
        self.assertRaises(TypeError, self.func, add, ())
        self.assertRaises(TypeError, self.func, add, object())

        class TestFailingIter:
            def __iter__(self):
                raise RuntimeError
        self.assertRaises(RuntimeError, self.func, add, TestFailingIter())

        self.assertEqual(self.func(add, [], None), None)
        self.assertEqual(self.func(add, [], 42), 42)

        class BadSeq:
            def __getitem__(self, index):
                raise ValueError
        self.assertRaises(ValueError, self.func, 42, BadSeq())

    # Test reduce()'s use of iterators.
    def test_iterator_usage(self):
        class SequenceClass:
            def __init__(self, n):
                self.n = n
            def __getitem__(self, i):
                if 0 <= i < self.n:
                    return i
                else:
                    raise IndexError

        from operator import add
        self.assertEqual(self.func(add, SequenceClass(5)), 10)
        self.assertEqual(self.func(add, SequenceClass(5), 42), 52)
        self.assertRaises(TypeError, self.func, add, SequenceClass(0))
        self.assertEqual(self.func(add, SequenceClass(0), 42), 42)
        self.assertEqual(self.func(add, SequenceClass(1)), 0)
        self.assertEqual(self.func(add, SequenceClass(1), 42), 42)

        d = {"one": 1, "two": 2, "three": 3}
        self.assertEqual(self.func(add, d), "".join(d.keys()))


class TestCmpToKey:

    def test_cmp_to_key(self):
        def cmp1(x, y):
            return (x > y) - (x < y)
        key = self.cmp_to_key(cmp1)
        self.assertEqual(key(3), key(3))
        self.assertGreater(key(3), key(1))
        self.assertGreaterEqual(key(3), key(3))

        def cmp2(x, y):
            return int(x) - int(y)
        key = self.cmp_to_key(cmp2)
        self.assertEqual(key(4.0), key('4'))
        self.assertLess(key(2), key('35'))
        self.assertLessEqual(key(2), key('35'))
        self.assertNotEqual(key(2), key('35'))

    def test_cmp_to_key_arguments(self):
        def cmp1(x, y):
            return (x > y) - (x < y)
        key = self.cmp_to_key(mycmp=cmp1)
        self.assertEqual(key(obj=3), key(obj=3))
        self.assertGreater(key(obj=3), key(obj=1))
        with self.assertRaises((TypeError, AttributeError)):
            key(3) > 1    # rhs is not a K object
        with self.assertRaises((TypeError, AttributeError)):
            1 < key(3)    # lhs is not a K object
        with self.assertRaises(TypeError):
            key = self.cmp_to_key()             # too few args
        with self.assertRaises(TypeError):
            key = self.cmp_to_key(cmp1, None)   # too many args
        key = self.cmp_to_key(cmp1)
        with self.assertRaises(TypeError):
            key()                                    # too few args
        with self.assertRaises(TypeError):
            key(None, None)                          # too many args

    def test_bad_cmp(self):
        def cmp1(x, y):
            raise ZeroDivisionError
        key = self.cmp_to_key(cmp1)
        with self.assertRaises(ZeroDivisionError):
            key(3) > key(1)

        class BadCmp:
            def __lt__(self, other):
                raise ZeroDivisionError
        def cmp1(x, y):
            return BadCmp()
        with self.assertRaises(ZeroDivisionError):
            key(3) > key(1)

    def test_obj_field(self):
        def cmp1(x, y):
            return (x > y) - (x < y)
        key = self.cmp_to_key(mycmp=cmp1)
        self.assertEqual(key(50).obj, 50)

    def test_sort_int(self):
        def mycmp(x, y):
            return y - x
        self.assertEqual(sorted(range(5), key=self.cmp_to_key(mycmp)),
                         [4, 3, 2, 1, 0])

    def test_sort_int_str(self):
        def mycmp(x, y):
            x, y = int(x), int(y)
            return (x > y) - (x < y)
        values = [5, '3', 7, 2, '0', '1', 4, '10', 1]
        values = sorted(values, key=self.cmp_to_key(mycmp))
        self.assertEqual([int(value) for value in values],
                         [0, 1, 1, 2, 3, 4, 5, 7, 10])

    def test_hash(self):
        def mycmp(x, y):
            return y - x
        key = self.cmp_to_key(mycmp)
        k = key(10)
        self.assertRaises(TypeError, hash, k)
        self.assertNotIsInstance(k, collections.Hashable)


@unittest.skipUnless(c_functools, 'requires the C _functools module')
class TestCmpToKeyC(TestCmpToKey, unittest.TestCase):
    if c_functools:
        cmp_to_key = c_functools.cmp_to_key


class TestCmpToKeyPy(TestCmpToKey, unittest.TestCase):
    cmp_to_key = staticmethod(py_functools.cmp_to_key)


class TestTotalOrdering(unittest.TestCase):

    def test_total_ordering_lt(self):
        @functools.total_ordering
        class A:
            def __init__(self, value):
                self.value = value
            def __lt__(self, other):
                return self.value < other.value
            def __eq__(self, other):
                return self.value == other.value
        self.assertTrue(A(1) < A(2))
        self.assertTrue(A(2) > A(1))
        self.assertTrue(A(1) <= A(2))
        self.assertTrue(A(2) >= A(1))
        self.assertTrue(A(2) <= A(2))
        self.assertTrue(A(2) >= A(2))
        self.assertFalse(A(1) > A(2))

    def test_total_ordering_le(self):
        @functools.total_ordering
        class A:
            def __init__(self, value):
                self.value = value
            def __le__(self, other):
                return self.value <= other.value
            def __eq__(self, other):
                return self.value == other.value
        self.assertTrue(A(1) < A(2))
        self.assertTrue(A(2) > A(1))
        self.assertTrue(A(1) <= A(2))
        self.assertTrue(A(2) >= A(1))
        self.assertTrue(A(2) <= A(2))
        self.assertTrue(A(2) >= A(2))
        self.assertFalse(A(1) >= A(2))

    def test_total_ordering_gt(self):
        @functools.total_ordering
        class A:
            def __init__(self, value):
                self.value = value
            def __gt__(self, other):
                return self.value > other.value
            def __eq__(self, other):
                return self.value == other.value
        self.assertTrue(A(1) < A(2))
        self.assertTrue(A(2) > A(1))
        self.assertTrue(A(1) <= A(2))
        self.assertTrue(A(2) >= A(1))
        self.assertTrue(A(2) <= A(2))
        self.assertTrue(A(2) >= A(2))
        self.assertFalse(A(2) < A(1))

    def test_total_ordering_ge(self):
        @functools.total_ordering
        class A:
            def __init__(self, value):
                self.value = value
            def __ge__(self, other):
                return self.value >= other.value
            def __eq__(self, other):
                return self.value == other.value
        self.assertTrue(A(1) < A(2))
        self.assertTrue(A(2) > A(1))
        self.assertTrue(A(1) <= A(2))
        self.assertTrue(A(2) >= A(1))
        self.assertTrue(A(2) <= A(2))
        self.assertTrue(A(2) >= A(2))
        self.assertFalse(A(2) <= A(1))

    def test_total_ordering_no_overwrite(self):
        # new methods should not overwrite existing
        @functools.total_ordering
        class A(int):
            pass
        self.assertTrue(A(1) < A(2))
        self.assertTrue(A(2) > A(1))
        self.assertTrue(A(1) <= A(2))
        self.assertTrue(A(2) >= A(1))
        self.assertTrue(A(2) <= A(2))
        self.assertTrue(A(2) >= A(2))

    def test_no_operations_defined(self):
        with self.assertRaises(ValueError):
            @functools.total_ordering
            class A:
                pass

    def test_type_error_when_not_implemented(self):
        # bug 10042; ensure stack overflow does not occur
        # when decorated types return NotImplemented
        @functools.total_ordering
        class ImplementsLessThan:
            def __init__(self, value):
                self.value = value
            def __eq__(self, other):
                if isinstance(other, ImplementsLessThan):
                    return self.value == other.value
                return False
            def __lt__(self, other):
                if isinstance(other, ImplementsLessThan):
                    return self.value < other.value
                return NotImplemented

        @functools.total_ordering
        class ImplementsGreaterThan:
            def __init__(self, value):
                self.value = value
            def __eq__(self, other):
                if isinstance(other, ImplementsGreaterThan):
                    return self.value == other.value
                return False
            def __gt__(self, other):
                if isinstance(other, ImplementsGreaterThan):
                    return self.value > other.value
                return NotImplemented

        @functools.total_ordering
        class ImplementsLessThanEqualTo:
            def __init__(self, value):
                self.value = value
            def __eq__(self, other):
                if isinstance(other, ImplementsLessThanEqualTo):
                    return self.value == other.value
                return False
            def __le__(self, other):
                if isinstance(other, ImplementsLessThanEqualTo):
                    return self.value <= other.value
                return NotImplemented

        @functools.total_ordering
        class ImplementsGreaterThanEqualTo:
            def __init__(self, value):
                self.value = value
            def __eq__(self, other):
                if isinstance(other, ImplementsGreaterThanEqualTo):
                    return self.value == other.value
                return False
            def __ge__(self, other):
                if isinstance(other, ImplementsGreaterThanEqualTo):
                    return self.value >= other.value
                return NotImplemented

        @functools.total_ordering
        class ComparatorNotImplemented:
            def __init__(self, value):
                self.value = value
            def __eq__(self, other):
                if isinstance(other, ComparatorNotImplemented):
                    return self.value == other.value
                return False
            def __lt__(self, other):
                return NotImplemented

        with self.subTest("LT < 1"), self.assertRaises(TypeError):
            ImplementsLessThan(-1) < 1

        with self.subTest("LT < LE"), self.assertRaises(TypeError):
            ImplementsLessThan(0) < ImplementsLessThanEqualTo(0)

        with self.subTest("LT < GT"), self.assertRaises(TypeError):
            ImplementsLessThan(1) < ImplementsGreaterThan(1)

        with self.subTest("LE <= LT"), self.assertRaises(TypeError):
            ImplementsLessThanEqualTo(2) <= ImplementsLessThan(2)

        with self.subTest("LE <= GE"), self.assertRaises(TypeError):
            ImplementsLessThanEqualTo(3) <= ImplementsGreaterThanEqualTo(3)

        with self.subTest("GT > GE"), self.assertRaises(TypeError):
            ImplementsGreaterThan(4) > ImplementsGreaterThanEqualTo(4)

        with self.subTest("GT > LT"), self.assertRaises(TypeError):
            ImplementsGreaterThan(5) > ImplementsLessThan(5)

        with self.subTest("GE >= GT"), self.assertRaises(TypeError):
            ImplementsGreaterThanEqualTo(6) >= ImplementsGreaterThan(6)

        with self.subTest("GE >= LE"), self.assertRaises(TypeError):
            ImplementsGreaterThanEqualTo(7) >= ImplementsLessThanEqualTo(7)

        with self.subTest("GE when equal"):
            a = ComparatorNotImplemented(8)
            b = ComparatorNotImplemented(8)
            self.assertEqual(a, b)
            with self.assertRaises(TypeError):
                a >= b

        with self.subTest("LE when equal"):
            a = ComparatorNotImplemented(9)
            b = ComparatorNotImplemented(9)
            self.assertEqual(a, b)
            with self.assertRaises(TypeError):
                a <= b

class TestLRU(unittest.TestCase):

    def test_lru(self):
        def orig(x, y):
            return 3 * x + y
        f = functools.lru_cache(maxsize=20)(orig)
        hits, misses, maxsize, currsize = f.cache_info()
        self.assertEqual(maxsize, 20)
        self.assertEqual(currsize, 0)
        self.assertEqual(hits, 0)
        self.assertEqual(misses, 0)

        domain = range(5)
        for i in range(1000):
            x, y = choice(domain), choice(domain)
            actual = f(x, y)
            expected = orig(x, y)
            self.assertEqual(actual, expected)
        hits, misses, maxsize, currsize = f.cache_info()
        self.assertTrue(hits > misses)
        self.assertEqual(hits + misses, 1000)
        self.assertEqual(currsize, 20)

        f.cache_clear()   # test clearing
        hits, misses, maxsize, currsize = f.cache_info()
        self.assertEqual(hits, 0)
        self.assertEqual(misses, 0)
        self.assertEqual(currsize, 0)
        f(x, y)
        hits, misses, maxsize, currsize = f.cache_info()
        self.assertEqual(hits, 0)
        self.assertEqual(misses, 1)
        self.assertEqual(currsize, 1)

        # Test bypassing the cache
        self.assertIs(f.__wrapped__, orig)
        f.__wrapped__(x, y)
        hits, misses, maxsize, currsize = f.cache_info()
        self.assertEqual(hits, 0)
        self.assertEqual(misses, 1)
        self.assertEqual(currsize, 1)

        # test size zero (which means "never-cache")
        @functools.lru_cache(0)
        def f():
            nonlocal f_cnt
            f_cnt += 1
            return 20
        self.assertEqual(f.cache_info().maxsize, 0)
        f_cnt = 0
        for i in range(5):
            self.assertEqual(f(), 20)
        self.assertEqual(f_cnt, 5)
        hits, misses, maxsize, currsize = f.cache_info()
        self.assertEqual(hits, 0)
        self.assertEqual(misses, 5)
        self.assertEqual(currsize, 0)

        # test size one
        @functools.lru_cache(1)
        def f():
            nonlocal f_cnt
            f_cnt += 1
            return 20
        self.assertEqual(f.cache_info().maxsize, 1)
        f_cnt = 0
        for i in range(5):
            self.assertEqual(f(), 20)
        self.assertEqual(f_cnt, 1)
        hits, misses, maxsize, currsize = f.cache_info()
        self.assertEqual(hits, 4)
        self.assertEqual(misses, 1)
        self.assertEqual(currsize, 1)

        # test size two
        @functools.lru_cache(2)
        def f(x):
            nonlocal f_cnt
            f_cnt += 1
            return x*10
        self.assertEqual(f.cache_info().maxsize, 2)
        f_cnt = 0
        for x in 7, 9, 7, 9, 7, 9, 8, 8, 8, 9, 9, 9, 8, 8, 8, 7:
            #    *  *              *                          *
            self.assertEqual(f(x), x*10)
        self.assertEqual(f_cnt, 4)
        hits, misses, maxsize, currsize = f.cache_info()
        self.assertEqual(hits, 12)
        self.assertEqual(misses, 4)
        self.assertEqual(currsize, 2)

    def test_lru_with_maxsize_none(self):
        @functools.lru_cache(maxsize=None)
        def fib(n):
            if n < 2:
                return n
            return fib(n-1) + fib(n-2)
        self.assertEqual([fib(n) for n in range(16)],
            [0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610])
        self.assertEqual(fib.cache_info(),
            functools._CacheInfo(hits=28, misses=16, maxsize=None, currsize=16))
        fib.cache_clear()
        self.assertEqual(fib.cache_info(),
            functools._CacheInfo(hits=0, misses=0, maxsize=None, currsize=0))

    def test_lru_with_exceptions(self):
        # Verify that user_function exceptions get passed through without
        # creating a hard-to-read chained exception.
        # http://bugs.python.org/issue13177
        for maxsize in (None, 128):
            @functools.lru_cache(maxsize)
            def func(i):
                return 'abc'[i]
            self.assertEqual(func(0), 'a')
            with self.assertRaises(IndexError) as cm:
                func(15)
            self.assertIsNone(cm.exception.__context__)
            # Verify that the previous exception did not result in a cached entry
            with self.assertRaises(IndexError):
                func(15)

    def test_lru_with_types(self):
        for maxsize in (None, 128):
            @functools.lru_cache(maxsize=maxsize, typed=True)
            def square(x):
                return x * x
            self.assertEqual(square(3), 9)
            self.assertEqual(type(square(3)), type(9))
            self.assertEqual(square(3.0), 9.0)
            self.assertEqual(type(square(3.0)), type(9.0))
            self.assertEqual(square(x=3), 9)
            self.assertEqual(type(square(x=3)), type(9))
            self.assertEqual(square(x=3.0), 9.0)
            self.assertEqual(type(square(x=3.0)), type(9.0))
            self.assertEqual(square.cache_info().hits, 4)
            self.assertEqual(square.cache_info().misses, 4)

    def test_lru_with_keyword_args(self):
        @functools.lru_cache()
        def fib(n):
            if n < 2:
                return n
            return fib(n=n-1) + fib(n=n-2)
        self.assertEqual(
            [fib(n=number) for number in range(16)],
            [0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610]
        )
        self.assertEqual(fib.cache_info(),
            functools._CacheInfo(hits=28, misses=16, maxsize=128, currsize=16))
        fib.cache_clear()
        self.assertEqual(fib.cache_info(),
            functools._CacheInfo(hits=0, misses=0, maxsize=128, currsize=0))

    def test_lru_with_keyword_args_maxsize_none(self):
        @functools.lru_cache(maxsize=None)
        def fib(n):
            if n < 2:
                return n
            return fib(n=n-1) + fib(n=n-2)
        self.assertEqual([fib(n=number) for number in range(16)],
            [0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610])
        self.assertEqual(fib.cache_info(),
            functools._CacheInfo(hits=28, misses=16, maxsize=None, currsize=16))
        fib.cache_clear()
        self.assertEqual(fib.cache_info(),
            functools._CacheInfo(hits=0, misses=0, maxsize=None, currsize=0))

    def test_need_for_rlock(self):
        # This will deadlock on an LRU cache that uses a regular lock

        @functools.lru_cache(maxsize=10)
        def test_func(x):
            'Used to demonstrate a reentrant lru_cache call within a single thread'
            return x

        class DoubleEq:
            'Demonstrate a reentrant lru_cache call within a single thread'
            def __init__(self, x):
                self.x = x
            def __hash__(self):
                return self.x
            def __eq__(self, other):
                if self.x == 2:
                    test_func(DoubleEq(1))
                return self.x == other.x

        test_func(DoubleEq(1))                      # Load the cache
        test_func(DoubleEq(2))                      # Load the cache
        self.assertEqual(test_func(DoubleEq(2)),    # Trigger a re-entrant __eq__ call
                         DoubleEq(2))               # Verify the correct return value


class TestSingleDispatch(unittest.TestCase):
    def test_simple_overloads(self):
        @functools.singledispatch
        def g(obj):
            return "base"
        def g_int(i):
            return "integer"
        g.register(int, g_int)
        self.assertEqual(g("str"), "base")
        self.assertEqual(g(1), "integer")
        self.assertEqual(g([1,2,3]), "base")

    def test_mro(self):
        @functools.singledispatch
        def g(obj):
            return "base"
        class A:
            pass
        class C(A):
            pass
        class B(A):
            pass
        class D(C, B):
            pass
        def g_A(a):
            return "A"
        def g_B(b):
            return "B"
        g.register(A, g_A)
        g.register(B, g_B)
        self.assertEqual(g(A()), "A")
        self.assertEqual(g(B()), "B")
        self.assertEqual(g(C()), "A")
        self.assertEqual(g(D()), "B")

    def test_register_decorator(self):
        @functools.singledispatch
        def g(obj):
            return "base"
        @g.register(int)
        def g_int(i):
            return "int %s" % (i,)
        self.assertEqual(g(""), "base")
        self.assertEqual(g(12), "int 12")
        self.assertIs(g.dispatch(int), g_int)
        self.assertIs(g.dispatch(object), g.dispatch(str))
        # Note: in the assert above this is not g.
        # @singledispatch returns the wrapper.

    def test_wrapping_attributes(self):
        @functools.singledispatch
        def g(obj):
            "Simple test"
            return "Test"
        self.assertEqual(g.__name__, "g")
        if sys.flags.optimize < 2:
            self.assertEqual(g.__doc__, "Simple test")

    @unittest.skipUnless(decimal, 'requires _decimal')
    @support.cpython_only
    def test_c_classes(self):
        @functools.singledispatch
        def g(obj):
            return "base"
        @g.register(decimal.DecimalException)
        def _(obj):
            return obj.args
        subn = decimal.Subnormal("Exponent < Emin")
        rnd = decimal.Rounded("Number got rounded")
        self.assertEqual(g(subn), ("Exponent < Emin",))
        self.assertEqual(g(rnd), ("Number got rounded",))
        @g.register(decimal.Subnormal)
        def _(obj):
            return "Too small to care."
        self.assertEqual(g(subn), "Too small to care.")
        self.assertEqual(g(rnd), ("Number got rounded",))

    def test_compose_mro(self):
        # None of the examples in this test depend on haystack ordering.
        c = collections
        mro = functools._compose_mro
        bases = [c.Sequence, c.MutableMapping, c.Mapping, c.Set]
        for haystack in permutations(bases):
            m = mro(dict, haystack)
            self.assertEqual(m, [dict, c.MutableMapping, c.Mapping, c.Sized,
                                 c.Iterable, c.Container, object])
        bases = [c.Container, c.Mapping, c.MutableMapping, c.OrderedDict]
        for haystack in permutations(bases):
            m = mro(c.ChainMap, haystack)
            self.assertEqual(m, [c.ChainMap, c.MutableMapping, c.Mapping,
                                 c.Sized, c.Iterable, c.Container, object])

        # If there's a generic function with implementations registered for
        # both Sized and Container, passing a defaultdict to it results in an
        # ambiguous dispatch which will cause a RuntimeError (see
        # test_mro_conflicts).
        bases = [c.Container, c.Sized, str]
        for haystack in permutations(bases):
            m = mro(c.defaultdict, [c.Sized, c.Container, str])
            self.assertEqual(m, [c.defaultdict, dict, c.Sized, c.Container,
                                 object])

        # MutableSequence below is registered directly on D. In other words, it
        # preceeds MutableMapping which means single dispatch will always
        # choose MutableSequence here.
        class D(c.defaultdict):
            pass
        c.MutableSequence.register(D)
        bases = [c.MutableSequence, c.MutableMapping]
        for haystack in permutations(bases):
            m = mro(D, bases)
            self.assertEqual(m, [D, c.MutableSequence, c.Sequence,
                                 c.defaultdict, dict, c.MutableMapping,
                                 c.Mapping, c.Sized, c.Iterable, c.Container,
                                 object])

        # Container and Callable are registered on different base classes and
        # a generic function supporting both should always pick the Callable
        # implementation if a C instance is passed.
        class C(c.defaultdict):
            def __call__(self):
                pass
        bases = [c.Sized, c.Callable, c.Container, c.Mapping]
        for haystack in permutations(bases):
            m = mro(C, haystack)
            self.assertEqual(m, [C, c.Callable, c.defaultdict, dict, c.Mapping,
                                 c.Sized, c.Iterable, c.Container, object])

    def test_register_abc(self):
        c = collections
        d = {"a": "b"}
        l = [1, 2, 3]
        s = {object(), None}
        f = frozenset(s)
        t = (1, 2, 3)
        @functools.singledispatch
        def g(obj):
            return "base"
        self.assertEqual(g(d), "base")
        self.assertEqual(g(l), "base")
        self.assertEqual(g(s), "base")
        self.assertEqual(g(f), "base")
        self.assertEqual(g(t), "base")
        g.register(c.Sized, lambda obj: "sized")
        self.assertEqual(g(d), "sized")
        self.assertEqual(g(l), "sized")
        self.assertEqual(g(s), "sized")
        self.assertEqual(g(f), "sized")
        self.assertEqual(g(t), "sized")
        g.register(c.MutableMapping, lambda obj: "mutablemapping")
        self.assertEqual(g(d), "mutablemapping")
        self.assertEqual(g(l), "sized")
        self.assertEqual(g(s), "sized")
        self.assertEqual(g(f), "sized")
        self.assertEqual(g(t), "sized")
        g.register(c.ChainMap, lambda obj: "chainmap")
        self.assertEqual(g(d), "mutablemapping")  # irrelevant ABCs registered
        self.assertEqual(g(l), "sized")
        self.assertEqual(g(s), "sized")
        self.assertEqual(g(f), "sized")
        self.assertEqual(g(t), "sized")
        g.register(c.MutableSequence, lambda obj: "mutablesequence")
        self.assertEqual(g(d), "mutablemapping")
        self.assertEqual(g(l), "mutablesequence")
        self.assertEqual(g(s), "sized")
        self.assertEqual(g(f), "sized")
        self.assertEqual(g(t), "sized")
        g.register(c.MutableSet, lambda obj: "mutableset")
        self.assertEqual(g(d), "mutablemapping")
        self.assertEqual(g(l), "mutablesequence")
        self.assertEqual(g(s), "mutableset")
        self.assertEqual(g(f), "sized")
        self.assertEqual(g(t), "sized")
        g.register(c.Mapping, lambda obj: "mapping")
        self.assertEqual(g(d), "mutablemapping")  # not specific enough
        self.assertEqual(g(l), "mutablesequence")
        self.assertEqual(g(s), "mutableset")
        self.assertEqual(g(f), "sized")
        self.assertEqual(g(t), "sized")
        g.register(c.Sequence, lambda obj: "sequence")
        self.assertEqual(g(d), "mutablemapping")
        self.assertEqual(g(l), "mutablesequence")
        self.assertEqual(g(s), "mutableset")
        self.assertEqual(g(f), "sized")
        self.assertEqual(g(t), "sequence")
        g.register(c.Set, lambda obj: "set")
        self.assertEqual(g(d), "mutablemapping")
        self.assertEqual(g(l), "mutablesequence")
        self.assertEqual(g(s), "mutableset")
        self.assertEqual(g(f), "set")
        self.assertEqual(g(t), "sequence")
        g.register(dict, lambda obj: "dict")
        self.assertEqual(g(d), "dict")
        self.assertEqual(g(l), "mutablesequence")
        self.assertEqual(g(s), "mutableset")
        self.assertEqual(g(f), "set")
        self.assertEqual(g(t), "sequence")
        g.register(list, lambda obj: "list")
        self.assertEqual(g(d), "dict")
        self.assertEqual(g(l), "list")
        self.assertEqual(g(s), "mutableset")
        self.assertEqual(g(f), "set")
        self.assertEqual(g(t), "sequence")
        g.register(set, lambda obj: "concrete-set")
        self.assertEqual(g(d), "dict")
        self.assertEqual(g(l), "list")
        self.assertEqual(g(s), "concrete-set")
        self.assertEqual(g(f), "set")
        self.assertEqual(g(t), "sequence")
        g.register(frozenset, lambda obj: "frozen-set")
        self.assertEqual(g(d), "dict")
        self.assertEqual(g(l), "list")
        self.assertEqual(g(s), "concrete-set")
        self.assertEqual(g(f), "frozen-set")
        self.assertEqual(g(t), "sequence")
        g.register(tuple, lambda obj: "tuple")
        self.assertEqual(g(d), "dict")
        self.assertEqual(g(l), "list")
        self.assertEqual(g(s), "concrete-set")
        self.assertEqual(g(f), "frozen-set")
        self.assertEqual(g(t), "tuple")

    def test_c3_abc(self):
        c = collections
        mro = functools._c3_mro
        class A(object):
            pass
        class B(A):
            def __len__(self):
                return 0   # implies Sized
        @c.Container.register
        class C(object):
            pass
        class D(object):
            pass   # unrelated
        class X(D, C, B):
            def __call__(self):
                pass   # implies Callable
        expected = [X, c.Callable, D, C, c.Container, B, c.Sized, A, object]
        for abcs in permutations([c.Sized, c.Callable, c.Container]):
            self.assertEqual(mro(X, abcs=abcs), expected)
        # unrelated ABCs don't appear in the resulting MRO
        many_abcs = [c.Mapping, c.Sized, c.Callable, c.Container, c.Iterable]
        self.assertEqual(mro(X, abcs=many_abcs), expected)

    def test_mro_conflicts(self):
        c = collections
        @functools.singledispatch
        def g(arg):
            return "base"
        class O(c.Sized):
            def __len__(self):
                return 0
        o = O()
        self.assertEqual(g(o), "base")
        g.register(c.Iterable, lambda arg: "iterable")
        g.register(c.Container, lambda arg: "container")
        g.register(c.Sized, lambda arg: "sized")
        g.register(c.Set, lambda arg: "set")
        self.assertEqual(g(o), "sized")
        c.Iterable.register(O)
        self.assertEqual(g(o), "sized")   # because it's explicitly in __mro__
        c.Container.register(O)
        self.assertEqual(g(o), "sized")   # see above: Sized is in __mro__
        c.Set.register(O)
        self.assertEqual(g(o), "set")     # because c.Set is a subclass of
                                          # c.Sized and c.Container
        class P:
            pass
        p = P()
        self.assertEqual(g(p), "base")
        c.Iterable.register(P)
        self.assertEqual(g(p), "iterable")
        c.Container.register(P)
        with self.assertRaises(RuntimeError) as re_one:
            g(p)
        self.assertIn(
            str(re_one.exception),
            (("Ambiguous dispatch: <class 'collections.abc.Container'> "
              "or <class 'collections.abc.Iterable'>"),
             ("Ambiguous dispatch: <class 'collections.abc.Iterable'> "
              "or <class 'collections.abc.Container'>")),
        )
        class Q(c.Sized):
            def __len__(self):
                return 0
        q = Q()
        self.assertEqual(g(q), "sized")
        c.Iterable.register(Q)
        self.assertEqual(g(q), "sized")   # because it's explicitly in __mro__
        c.Set.register(Q)
        self.assertEqual(g(q), "set")     # because c.Set is a subclass of
                                          # c.Sized and c.Iterable
        @functools.singledispatch
        def h(arg):
            return "base"
        @h.register(c.Sized)
        def _(arg):
            return "sized"
        @h.register(c.Container)
        def _(arg):
            return "container"
        # Even though Sized and Container are explicit bases of MutableMapping,
        # this ABC is implicitly registered on defaultdict which makes all of
        # MutableMapping's bases implicit as well from defaultdict's
        # perspective.
        with self.assertRaises(RuntimeError) as re_two:
            h(c.defaultdict(lambda: 0))
        self.assertIn(
            str(re_two.exception),
            (("Ambiguous dispatch: <class 'collections.abc.Container'> "
              "or <class 'collections.abc.Sized'>"),
             ("Ambiguous dispatch: <class 'collections.abc.Sized'> "
              "or <class 'collections.abc.Container'>")),
        )
        class R(c.defaultdict):
            pass
        c.MutableSequence.register(R)
        @functools.singledispatch
        def i(arg):
            return "base"
        @i.register(c.MutableMapping)
        def _(arg):
            return "mapping"
        @i.register(c.MutableSequence)
        def _(arg):
            return "sequence"
        r = R()
        self.assertEqual(i(r), "sequence")
        class S:
            pass
        class T(S, c.Sized):
            def __len__(self):
                return 0
        t = T()
        self.assertEqual(h(t), "sized")
        c.Container.register(T)
        self.assertEqual(h(t), "sized")   # because it's explicitly in the MRO
        class U:
            def __len__(self):
                return 0
        u = U()
        self.assertEqual(h(u), "sized")   # implicit Sized subclass inferred
                                          # from the existence of __len__()
        c.Container.register(U)
        # There is no preference for registered versus inferred ABCs.
        with self.assertRaises(RuntimeError) as re_three:
            h(u)
        self.assertIn(
            str(re_three.exception),
            (("Ambiguous dispatch: <class 'collections.abc.Container'> "
              "or <class 'collections.abc.Sized'>"),
             ("Ambiguous dispatch: <class 'collections.abc.Sized'> "
              "or <class 'collections.abc.Container'>")),
        )
        class V(c.Sized, S):
            def __len__(self):
                return 0
        @functools.singledispatch
        def j(arg):
            return "base"
        @j.register(S)
        def _(arg):
            return "s"
        @j.register(c.Container)
        def _(arg):
            return "container"
        v = V()
        self.assertEqual(j(v), "s")
        c.Container.register(V)
        self.assertEqual(j(v), "container")   # because it ends up right after
                                              # Sized in the MRO

    def test_cache_invalidation(self):
        from collections import UserDict
        class TracingDict(UserDict):
            def __init__(self, *args, **kwargs):
                super(TracingDict, self).__init__(*args, **kwargs)
                self.set_ops = []
                self.get_ops = []
            def __getitem__(self, key):
                result = self.data[key]
                self.get_ops.append(key)
                return result
            def __setitem__(self, key, value):
                self.set_ops.append(key)
                self.data[key] = value
            def clear(self):
                self.data.clear()
        _orig_wkd = functools.WeakKeyDictionary
        td = TracingDict()
        functools.WeakKeyDictionary = lambda: td
        c = collections
        @functools.singledispatch
        def g(arg):
            return "base"
        d = {}
        l = []
        self.assertEqual(len(td), 0)
        self.assertEqual(g(d), "base")
        self.assertEqual(len(td), 1)
        self.assertEqual(td.get_ops, [])
        self.assertEqual(td.set_ops, [dict])
        self.assertEqual(td.data[dict], g.registry[object])
        self.assertEqual(g(l), "base")
        self.assertEqual(len(td), 2)
        self.assertEqual(td.get_ops, [])
        self.assertEqual(td.set_ops, [dict, list])
        self.assertEqual(td.data[dict], g.registry[object])
        self.assertEqual(td.data[list], g.registry[object])
        self.assertEqual(td.data[dict], td.data[list])
        self.assertEqual(g(l), "base")
        self.assertEqual(g(d), "base")
        self.assertEqual(td.get_ops, [list, dict])
        self.assertEqual(td.set_ops, [dict, list])
        g.register(list, lambda arg: "list")
        self.assertEqual(td.get_ops, [list, dict])
        self.assertEqual(len(td), 0)
        self.assertEqual(g(d), "base")
        self.assertEqual(len(td), 1)
        self.assertEqual(td.get_ops, [list, dict])
        self.assertEqual(td.set_ops, [dict, list, dict])
        self.assertEqual(td.data[dict],
                         functools._find_impl(dict, g.registry))
        self.assertEqual(g(l), "list")
        self.assertEqual(len(td), 2)
        self.assertEqual(td.get_ops, [list, dict])
        self.assertEqual(td.set_ops, [dict, list, dict, list])
        self.assertEqual(td.data[list],
                         functools._find_impl(list, g.registry))
        class X:
            pass
        c.MutableMapping.register(X)   # Will not invalidate the cache,
                                       # not using ABCs yet.
        self.assertEqual(g(d), "base")
        self.assertEqual(g(l), "list")
        self.assertEqual(td.get_ops, [list, dict, dict, list])
        self.assertEqual(td.set_ops, [dict, list, dict, list])
        g.register(c.Sized, lambda arg: "sized")
        self.assertEqual(len(td), 0)
        self.assertEqual(g(d), "sized")
        self.assertEqual(len(td), 1)
        self.assertEqual(td.get_ops, [list, dict, dict, list])
        self.assertEqual(td.set_ops, [dict, list, dict, list, dict])
        self.assertEqual(g(l), "list")
        self.assertEqual(len(td), 2)
        self.assertEqual(td.get_ops, [list, dict, dict, list])
        self.assertEqual(td.set_ops, [dict, list, dict, list, dict, list])
        self.assertEqual(g(l), "list")
        self.assertEqual(g(d), "sized")
        self.assertEqual(td.get_ops, [list, dict, dict, list, list, dict])
        self.assertEqual(td.set_ops, [dict, list, dict, list, dict, list])
        g.dispatch(list)
        g.dispatch(dict)
        self.assertEqual(td.get_ops, [list, dict, dict, list, list, dict,
                                      list, dict])
        self.assertEqual(td.set_ops, [dict, list, dict, list, dict, list])
        c.MutableSet.register(X)       # Will invalidate the cache.
        self.assertEqual(len(td), 2)   # Stale cache.
        self.assertEqual(g(l), "list")
        self.assertEqual(len(td), 1)
        g.register(c.MutableMapping, lambda arg: "mutablemapping")
        self.assertEqual(len(td), 0)
        self.assertEqual(g(d), "mutablemapping")
        self.assertEqual(len(td), 1)
        self.assertEqual(g(l), "list")
        self.assertEqual(len(td), 2)
        g.register(dict, lambda arg: "dict")
        self.assertEqual(g(d), "dict")
        self.assertEqual(g(l), "list")
        g._clear_cache()
        self.assertEqual(len(td), 0)
        functools.WeakKeyDictionary = _orig_wkd


def test_main(verbose=None):
    test_classes = (
        TestPartialC,
        TestPartialPy,
        TestPartialCSubclass,
        TestPartialMethod,
        TestUpdateWrapper,
        TestTotalOrdering,
        TestCmpToKeyC,
        TestCmpToKeyPy,
        TestWraps,
        TestReduce,
        TestLRU,
        TestSingleDispatch,
    )
    support.run_unittest(*test_classes)

    # verify reference counting
    if verbose and hasattr(sys, "gettotalrefcount"):
        import gc
        counts = [None] * 5
        for i in range(len(counts)):
            support.run_unittest(*test_classes)
            gc.collect()
            counts[i] = sys.gettotalrefcount()
        print(counts)

if __name__ == '__main__':
    test_main(verbose=True)
