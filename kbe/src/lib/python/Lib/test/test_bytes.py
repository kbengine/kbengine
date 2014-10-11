"""Unit tests for the bytes and bytearray types.

XXX This is a mess.  Common tests should be moved to buffer_tests.py,
which itself ought to be unified with string_tests.py (and the latter
should be modernized).
"""

import os
import re
import sys
import copy
import functools
import pickle
import tempfile
import unittest
import test.support
import test.string_tests
import test.buffer_tests


if sys.flags.bytes_warning:
    def check_bytes_warnings(func):
        @functools.wraps(func)
        def wrapper(*args, **kw):
            with test.support.check_warnings(('', BytesWarning)):
                return func(*args, **kw)
        return wrapper
else:
    # no-op
    def check_bytes_warnings(func):
        return func


class Indexable:
    def __init__(self, value=0):
        self.value = value
    def __index__(self):
        return self.value


class BaseBytesTest:

    def test_basics(self):
        b = self.type2test()
        self.assertEqual(type(b), self.type2test)
        self.assertEqual(b.__class__, self.type2test)

    def test_copy(self):
        a = self.type2test(b"abcd")
        for copy_method in (copy.copy, copy.deepcopy):
            b = copy_method(a)
            self.assertEqual(a, b)
            self.assertEqual(type(a), type(b))

    def test_empty_sequence(self):
        b = self.type2test()
        self.assertEqual(len(b), 0)
        self.assertRaises(IndexError, lambda: b[0])
        self.assertRaises(IndexError, lambda: b[1])
        self.assertRaises(IndexError, lambda: b[sys.maxsize])
        self.assertRaises(IndexError, lambda: b[sys.maxsize+1])
        self.assertRaises(IndexError, lambda: b[10**100])
        self.assertRaises(IndexError, lambda: b[-1])
        self.assertRaises(IndexError, lambda: b[-2])
        self.assertRaises(IndexError, lambda: b[-sys.maxsize])
        self.assertRaises(IndexError, lambda: b[-sys.maxsize-1])
        self.assertRaises(IndexError, lambda: b[-sys.maxsize-2])
        self.assertRaises(IndexError, lambda: b[-10**100])

    def test_from_list(self):
        ints = list(range(256))
        b = self.type2test(i for i in ints)
        self.assertEqual(len(b), 256)
        self.assertEqual(list(b), ints)

    def test_from_index(self):
        b = self.type2test([Indexable(), Indexable(1), Indexable(254),
                            Indexable(255)])
        self.assertEqual(list(b), [0, 1, 254, 255])
        self.assertRaises(ValueError, self.type2test, [Indexable(-1)])
        self.assertRaises(ValueError, self.type2test, [Indexable(256)])

    def test_from_ssize(self):
        self.assertEqual(self.type2test(0), b'')
        self.assertEqual(self.type2test(1), b'\x00')
        self.assertEqual(self.type2test(5), b'\x00\x00\x00\x00\x00')
        self.assertRaises(ValueError, self.type2test, -1)

        self.assertEqual(self.type2test('0', 'ascii'), b'0')
        self.assertEqual(self.type2test(b'0'), b'0')
        self.assertRaises(OverflowError, self.type2test, sys.maxsize + 1)

    def test_constructor_type_errors(self):
        self.assertRaises(TypeError, self.type2test, 0.0)
        class C:
            pass
        self.assertRaises(TypeError, self.type2test, ["0"])
        self.assertRaises(TypeError, self.type2test, [0.0])
        self.assertRaises(TypeError, self.type2test, [None])
        self.assertRaises(TypeError, self.type2test, [C()])

    def test_constructor_value_errors(self):
        self.assertRaises(ValueError, self.type2test, [-1])
        self.assertRaises(ValueError, self.type2test, [-sys.maxsize])
        self.assertRaises(ValueError, self.type2test, [-sys.maxsize-1])
        self.assertRaises(ValueError, self.type2test, [-sys.maxsize-2])
        self.assertRaises(ValueError, self.type2test, [-10**100])
        self.assertRaises(ValueError, self.type2test, [256])
        self.assertRaises(ValueError, self.type2test, [257])
        self.assertRaises(ValueError, self.type2test, [sys.maxsize])
        self.assertRaises(ValueError, self.type2test, [sys.maxsize+1])
        self.assertRaises(ValueError, self.type2test, [10**100])

    def test_compare(self):
        b1 = self.type2test([1, 2, 3])
        b2 = self.type2test([1, 2, 3])
        b3 = self.type2test([1, 3])

        self.assertEqual(b1, b2)
        self.assertTrue(b2 != b3)
        self.assertTrue(b1 <= b2)
        self.assertTrue(b1 <= b3)
        self.assertTrue(b1 <  b3)
        self.assertTrue(b1 >= b2)
        self.assertTrue(b3 >= b2)
        self.assertTrue(b3 >  b2)

        self.assertFalse(b1 != b2)
        self.assertFalse(b2 == b3)
        self.assertFalse(b1 >  b2)
        self.assertFalse(b1 >  b3)
        self.assertFalse(b1 >= b3)
        self.assertFalse(b1 <  b2)
        self.assertFalse(b3 <  b2)
        self.assertFalse(b3 <= b2)

    @check_bytes_warnings
    def test_compare_to_str(self):
        # Byte comparisons with unicode should always fail!
        # Test this for all expected byte orders and Unicode character
        # sizes.
        self.assertEqual(self.type2test(b"\0a\0b\0c") == "abc", False)
        self.assertEqual(self.type2test(b"\0\0\0a\0\0\0b\0\0\0c") == "abc",
                            False)
        self.assertEqual(self.type2test(b"a\0b\0c\0") == "abc", False)
        self.assertEqual(self.type2test(b"a\0\0\0b\0\0\0c\0\0\0") == "abc",
                            False)
        self.assertEqual(self.type2test() == str(), False)
        self.assertEqual(self.type2test() != str(), True)

    def test_reversed(self):
        input = list(map(ord, "Hello"))
        b = self.type2test(input)
        output = list(reversed(b))
        input.reverse()
        self.assertEqual(output, input)

    def test_getslice(self):
        def by(s):
            return self.type2test(map(ord, s))
        b = by("Hello, world")

        self.assertEqual(b[:5], by("Hello"))
        self.assertEqual(b[1:5], by("ello"))
        self.assertEqual(b[5:7], by(", "))
        self.assertEqual(b[7:], by("world"))
        self.assertEqual(b[7:12], by("world"))
        self.assertEqual(b[7:100], by("world"))

        self.assertEqual(b[:-7], by("Hello"))
        self.assertEqual(b[-11:-7], by("ello"))
        self.assertEqual(b[-7:-5], by(", "))
        self.assertEqual(b[-5:], by("world"))
        self.assertEqual(b[-5:12], by("world"))
        self.assertEqual(b[-5:100], by("world"))
        self.assertEqual(b[-100:5], by("Hello"))

    def test_extended_getslice(self):
        # Test extended slicing by comparing with list slicing.
        L = list(range(255))
        b = self.type2test(L)
        indices = (0, None, 1, 3, 19, 100, -1, -2, -31, -100)
        for start in indices:
            for stop in indices:
                # Skip step 0 (invalid)
                for step in indices[1:]:
                    self.assertEqual(b[start:stop:step], self.type2test(L[start:stop:step]))

    def test_encoding(self):
        sample = "Hello world\n\u1234\u5678\u9abc"
        for enc in ("utf-8", "utf-16"):
            b = self.type2test(sample, enc)
            self.assertEqual(b, self.type2test(sample.encode(enc)))
        self.assertRaises(UnicodeEncodeError, self.type2test, sample, "latin-1")
        b = self.type2test(sample, "latin-1", "ignore")
        self.assertEqual(b, self.type2test(sample[:-3], "utf-8"))

    def test_decode(self):
        sample = "Hello world\n\u1234\u5678\u9abc\def0\def0"
        for enc in ("utf-8", "utf-16"):
            b = self.type2test(sample, enc)
            self.assertEqual(b.decode(enc), sample)
        sample = "Hello world\n\x80\x81\xfe\xff"
        b = self.type2test(sample, "latin-1")
        self.assertRaises(UnicodeDecodeError, b.decode, "utf-8")
        self.assertEqual(b.decode("utf-8", "ignore"), "Hello world\n")
        self.assertEqual(b.decode(errors="ignore", encoding="utf-8"),
                         "Hello world\n")
        # Default encoding is utf-8
        self.assertEqual(self.type2test(b'\xe2\x98\x83').decode(), '\u2603')

    def test_from_int(self):
        b = self.type2test(0)
        self.assertEqual(b, self.type2test())
        b = self.type2test(10)
        self.assertEqual(b, self.type2test([0]*10))
        b = self.type2test(10000)
        self.assertEqual(b, self.type2test([0]*10000))

    def test_concat(self):
        b1 = self.type2test(b"abc")
        b2 = self.type2test(b"def")
        self.assertEqual(b1 + b2, b"abcdef")
        self.assertEqual(b1 + bytes(b"def"), b"abcdef")
        self.assertEqual(bytes(b"def") + b1, b"defabc")
        self.assertRaises(TypeError, lambda: b1 + "def")
        self.assertRaises(TypeError, lambda: "abc" + b2)

    def test_repeat(self):
        for b in b"abc", self.type2test(b"abc"):
            self.assertEqual(b * 3, b"abcabcabc")
            self.assertEqual(b * 0, b"")
            self.assertEqual(b * -1, b"")
            self.assertRaises(TypeError, lambda: b * 3.14)
            self.assertRaises(TypeError, lambda: 3.14 * b)
            # XXX Shouldn't bytes and bytearray agree on what to raise?
            with self.assertRaises((OverflowError, MemoryError)):
                c = b * sys.maxsize
            with self.assertRaises((OverflowError, MemoryError)):
                b *= sys.maxsize

    def test_repeat_1char(self):
        self.assertEqual(self.type2test(b'x')*100, self.type2test([ord('x')]*100))

    def test_contains(self):
        b = self.type2test(b"abc")
        self.assertIn(ord('a'), b)
        self.assertIn(int(ord('a')), b)
        self.assertNotIn(200, b)
        self.assertRaises(ValueError, lambda: 300 in b)
        self.assertRaises(ValueError, lambda: -1 in b)
        self.assertRaises(TypeError, lambda: None in b)
        self.assertRaises(TypeError, lambda: float(ord('a')) in b)
        self.assertRaises(TypeError, lambda: "a" in b)
        for f in bytes, bytearray:
            self.assertIn(f(b""), b)
            self.assertIn(f(b"a"), b)
            self.assertIn(f(b"b"), b)
            self.assertIn(f(b"c"), b)
            self.assertIn(f(b"ab"), b)
            self.assertIn(f(b"bc"), b)
            self.assertIn(f(b"abc"), b)
            self.assertNotIn(f(b"ac"), b)
            self.assertNotIn(f(b"d"), b)
            self.assertNotIn(f(b"dab"), b)
            self.assertNotIn(f(b"abd"), b)

    def test_fromhex(self):
        self.assertRaises(TypeError, self.type2test.fromhex)
        self.assertRaises(TypeError, self.type2test.fromhex, 1)
        self.assertEqual(self.type2test.fromhex(''), self.type2test())
        b = bytearray([0x1a, 0x2b, 0x30])
        self.assertEqual(self.type2test.fromhex('1a2B30'), b)
        self.assertEqual(self.type2test.fromhex('  1A 2B  30   '), b)
        self.assertEqual(self.type2test.fromhex('0000'), b'\0\0')
        self.assertRaises(TypeError, self.type2test.fromhex, b'1B')
        self.assertRaises(ValueError, self.type2test.fromhex, 'a')
        self.assertRaises(ValueError, self.type2test.fromhex, 'rt')
        self.assertRaises(ValueError, self.type2test.fromhex, '1a b cd')
        self.assertRaises(ValueError, self.type2test.fromhex, '\x00')
        self.assertRaises(ValueError, self.type2test.fromhex, '12   \x00   34')

    def test_join(self):
        self.assertEqual(self.type2test(b"").join([]), b"")
        self.assertEqual(self.type2test(b"").join([b""]), b"")
        for lst in [[b"abc"], [b"a", b"bc"], [b"ab", b"c"], [b"a", b"b", b"c"]]:
            lst = list(map(self.type2test, lst))
            self.assertEqual(self.type2test(b"").join(lst), b"abc")
            self.assertEqual(self.type2test(b"").join(tuple(lst)), b"abc")
            self.assertEqual(self.type2test(b"").join(iter(lst)), b"abc")
        dot_join = self.type2test(b".:").join
        self.assertEqual(dot_join([b"ab", b"cd"]), b"ab.:cd")
        self.assertEqual(dot_join([memoryview(b"ab"), b"cd"]), b"ab.:cd")
        self.assertEqual(dot_join([b"ab", memoryview(b"cd")]), b"ab.:cd")
        self.assertEqual(dot_join([bytearray(b"ab"), b"cd"]), b"ab.:cd")
        self.assertEqual(dot_join([b"ab", bytearray(b"cd")]), b"ab.:cd")
        # Stress it with many items
        seq = [b"abc"] * 1000
        expected = b"abc" + b".:abc" * 999
        self.assertEqual(dot_join(seq), expected)
        # Error handling and cleanup when some item in the middle of the
        # sequence has the wrong type.
        with self.assertRaises(TypeError):
            dot_join([bytearray(b"ab"), "cd", b"ef"])
        with self.assertRaises(TypeError):
            dot_join([memoryview(b"ab"), "cd", b"ef"])

    def test_count(self):
        b = self.type2test(b'mississippi')
        i = 105
        p = 112
        w = 119

        self.assertEqual(b.count(b'i'), 4)
        self.assertEqual(b.count(b'ss'), 2)
        self.assertEqual(b.count(b'w'), 0)

        self.assertEqual(b.count(i), 4)
        self.assertEqual(b.count(w), 0)

        self.assertEqual(b.count(b'i', 6), 2)
        self.assertEqual(b.count(b'p', 6), 2)
        self.assertEqual(b.count(b'i', 1, 3), 1)
        self.assertEqual(b.count(b'p', 7, 9), 1)

        self.assertEqual(b.count(i, 6), 2)
        self.assertEqual(b.count(p, 6), 2)
        self.assertEqual(b.count(i, 1, 3), 1)
        self.assertEqual(b.count(p, 7, 9), 1)

    def test_startswith(self):
        b = self.type2test(b'hello')
        self.assertFalse(self.type2test().startswith(b"anything"))
        self.assertTrue(b.startswith(b"hello"))
        self.assertTrue(b.startswith(b"hel"))
        self.assertTrue(b.startswith(b"h"))
        self.assertFalse(b.startswith(b"hellow"))
        self.assertFalse(b.startswith(b"ha"))
        with self.assertRaises(TypeError) as cm:
            b.startswith([b'h'])
        exc = str(cm.exception)
        self.assertIn('bytes', exc)
        self.assertIn('tuple', exc)

    def test_endswith(self):
        b = self.type2test(b'hello')
        self.assertFalse(bytearray().endswith(b"anything"))
        self.assertTrue(b.endswith(b"hello"))
        self.assertTrue(b.endswith(b"llo"))
        self.assertTrue(b.endswith(b"o"))
        self.assertFalse(b.endswith(b"whello"))
        self.assertFalse(b.endswith(b"no"))
        with self.assertRaises(TypeError) as cm:
            b.endswith([b'o'])
        exc = str(cm.exception)
        self.assertIn('bytes', exc)
        self.assertIn('tuple', exc)

    def test_find(self):
        b = self.type2test(b'mississippi')
        i = 105
        w = 119

        self.assertEqual(b.find(b'ss'), 2)
        self.assertEqual(b.find(b'w'), -1)
        self.assertEqual(b.find(b'mississippian'), -1)

        self.assertEqual(b.find(i), 1)
        self.assertEqual(b.find(w), -1)

        self.assertEqual(b.find(b'ss', 3), 5)
        self.assertEqual(b.find(b'ss', 1, 7), 2)
        self.assertEqual(b.find(b'ss', 1, 3), -1)

        self.assertEqual(b.find(i, 6), 7)
        self.assertEqual(b.find(i, 1, 3), 1)
        self.assertEqual(b.find(w, 1, 3), -1)

        for index in (-1, 256, sys.maxsize + 1):
            self.assertRaisesRegex(
                ValueError, r'byte must be in range\(0, 256\)',
                b.find, index)

    def test_rfind(self):
        b = self.type2test(b'mississippi')
        i = 105
        w = 119

        self.assertEqual(b.rfind(b'ss'), 5)
        self.assertEqual(b.rfind(b'w'), -1)
        self.assertEqual(b.rfind(b'mississippian'), -1)

        self.assertEqual(b.rfind(i), 10)
        self.assertEqual(b.rfind(w), -1)

        self.assertEqual(b.rfind(b'ss', 3), 5)
        self.assertEqual(b.rfind(b'ss', 0, 6), 2)

        self.assertEqual(b.rfind(i, 1, 3), 1)
        self.assertEqual(b.rfind(i, 3, 9), 7)
        self.assertEqual(b.rfind(w, 1, 3), -1)

    def test_index(self):
        b = self.type2test(b'mississippi')
        i = 105
        w = 119

        self.assertEqual(b.index(b'ss'), 2)
        self.assertRaises(ValueError, b.index, b'w')
        self.assertRaises(ValueError, b.index, b'mississippian')

        self.assertEqual(b.index(i), 1)
        self.assertRaises(ValueError, b.index, w)

        self.assertEqual(b.index(b'ss', 3), 5)
        self.assertEqual(b.index(b'ss', 1, 7), 2)
        self.assertRaises(ValueError, b.index, b'ss', 1, 3)

        self.assertEqual(b.index(i, 6), 7)
        self.assertEqual(b.index(i, 1, 3), 1)
        self.assertRaises(ValueError, b.index, w, 1, 3)

    def test_rindex(self):
        b = self.type2test(b'mississippi')
        i = 105
        w = 119

        self.assertEqual(b.rindex(b'ss'), 5)
        self.assertRaises(ValueError, b.rindex, b'w')
        self.assertRaises(ValueError, b.rindex, b'mississippian')

        self.assertEqual(b.rindex(i), 10)
        self.assertRaises(ValueError, b.rindex, w)

        self.assertEqual(b.rindex(b'ss', 3), 5)
        self.assertEqual(b.rindex(b'ss', 0, 6), 2)

        self.assertEqual(b.rindex(i, 1, 3), 1)
        self.assertEqual(b.rindex(i, 3, 9), 7)
        self.assertRaises(ValueError, b.rindex, w, 1, 3)

    def test_replace(self):
        b = self.type2test(b'mississippi')
        self.assertEqual(b.replace(b'i', b'a'), b'massassappa')
        self.assertEqual(b.replace(b'ss', b'x'), b'mixixippi')

    def test_split(self):
        b = self.type2test(b'mississippi')
        self.assertEqual(b.split(b'i'), [b'm', b'ss', b'ss', b'pp', b''])
        self.assertEqual(b.split(b'ss'), [b'mi', b'i', b'ippi'])
        self.assertEqual(b.split(b'w'), [b])
        # with keyword args
        b = self.type2test(b'a|b|c|d')
        self.assertEqual(b.split(sep=b'|'), [b'a', b'b', b'c', b'd'])
        self.assertEqual(b.split(b'|', maxsplit=1), [b'a', b'b|c|d'])
        self.assertEqual(b.split(sep=b'|', maxsplit=1), [b'a', b'b|c|d'])
        self.assertEqual(b.split(maxsplit=1, sep=b'|'), [b'a', b'b|c|d'])
        b = self.type2test(b'a b c d')
        self.assertEqual(b.split(maxsplit=1), [b'a', b'b c d'])

    def test_split_whitespace(self):
        for b in (b'  arf  barf  ', b'arf\tbarf', b'arf\nbarf', b'arf\rbarf',
                  b'arf\fbarf', b'arf\vbarf'):
            b = self.type2test(b)
            self.assertEqual(b.split(), [b'arf', b'barf'])
            self.assertEqual(b.split(None), [b'arf', b'barf'])
            self.assertEqual(b.split(None, 2), [b'arf', b'barf'])
        for b in (b'a\x1Cb', b'a\x1Db', b'a\x1Eb', b'a\x1Fb'):
            b = self.type2test(b)
            self.assertEqual(b.split(), [b])
        self.assertEqual(self.type2test(b'  a  bb  c  ').split(None, 0), [b'a  bb  c  '])
        self.assertEqual(self.type2test(b'  a  bb  c  ').split(None, 1), [b'a', b'bb  c  '])
        self.assertEqual(self.type2test(b'  a  bb  c  ').split(None, 2), [b'a', b'bb', b'c  '])
        self.assertEqual(self.type2test(b'  a  bb  c  ').split(None, 3), [b'a', b'bb', b'c'])

    def test_split_string_error(self):
        self.assertRaises(TypeError, self.type2test(b'a b').split, ' ')

    def test_split_unicodewhitespace(self):
        b = self.type2test(b"\x09\x0A\x0B\x0C\x0D\x1C\x1D\x1E\x1F")
        self.assertEqual(b.split(), [b'\x1c\x1d\x1e\x1f'])

    def test_rsplit(self):
        b = self.type2test(b'mississippi')
        self.assertEqual(b.rsplit(b'i'), [b'm', b'ss', b'ss', b'pp', b''])
        self.assertEqual(b.rsplit(b'ss'), [b'mi', b'i', b'ippi'])
        self.assertEqual(b.rsplit(b'w'), [b])
        # with keyword args
        b = self.type2test(b'a|b|c|d')
        self.assertEqual(b.rsplit(sep=b'|'), [b'a', b'b', b'c', b'd'])
        self.assertEqual(b.rsplit(b'|', maxsplit=1), [b'a|b|c', b'd'])
        self.assertEqual(b.rsplit(sep=b'|', maxsplit=1), [b'a|b|c', b'd'])
        self.assertEqual(b.rsplit(maxsplit=1, sep=b'|'), [b'a|b|c', b'd'])
        b = self.type2test(b'a b c d')
        self.assertEqual(b.rsplit(maxsplit=1), [b'a b c', b'd'])

    def test_rsplit_whitespace(self):
        for b in (b'  arf  barf  ', b'arf\tbarf', b'arf\nbarf', b'arf\rbarf',
                  b'arf\fbarf', b'arf\vbarf'):
            b = self.type2test(b)
            self.assertEqual(b.rsplit(), [b'arf', b'barf'])
            self.assertEqual(b.rsplit(None), [b'arf', b'barf'])
            self.assertEqual(b.rsplit(None, 2), [b'arf', b'barf'])
        self.assertEqual(self.type2test(b'  a  bb  c  ').rsplit(None, 0), [b'  a  bb  c'])
        self.assertEqual(self.type2test(b'  a  bb  c  ').rsplit(None, 1), [b'  a  bb', b'c'])
        self.assertEqual(self.type2test(b'  a  bb  c  ').rsplit(None, 2), [b'  a', b'bb', b'c'])
        self.assertEqual(self.type2test(b'  a  bb  c  ').rsplit(None, 3), [b'a', b'bb', b'c'])

    def test_rsplit_string_error(self):
        self.assertRaises(TypeError, self.type2test(b'a b').rsplit, ' ')

    def test_rsplit_unicodewhitespace(self):
        b = self.type2test(b"\x09\x0A\x0B\x0C\x0D\x1C\x1D\x1E\x1F")
        self.assertEqual(b.rsplit(), [b'\x1c\x1d\x1e\x1f'])

    def test_partition(self):
        b = self.type2test(b'mississippi')
        self.assertEqual(b.partition(b'ss'), (b'mi', b'ss', b'issippi'))
        self.assertEqual(b.partition(b'w'), (b'mississippi', b'', b''))

    def test_rpartition(self):
        b = self.type2test(b'mississippi')
        self.assertEqual(b.rpartition(b'ss'), (b'missi', b'ss', b'ippi'))
        self.assertEqual(b.rpartition(b'i'), (b'mississipp', b'i', b''))
        self.assertEqual(b.rpartition(b'w'), (b'', b'', b'mississippi'))

    def test_pickling(self):
        for proto in range(pickle.HIGHEST_PROTOCOL + 1):
            for b in b"", b"a", b"abc", b"\xffab\x80", b"\0\0\377\0\0":
                b = self.type2test(b)
                ps = pickle.dumps(b, proto)
                q = pickle.loads(ps)
                self.assertEqual(b, q)

    def test_iterator_pickling(self):
        for b in b"", b"a", b"abc", b"\xffab\x80", b"\0\0\377\0\0":
            it = itorg = iter(self.type2test(b))
            data = list(self.type2test(b))
            d = pickle.dumps(it)
            it = pickle.loads(d)
            self.assertEqual(type(itorg), type(it))
            self.assertEqual(list(it), data)

            it = pickle.loads(d)
            try:
                next(it)
            except StopIteration:
                continue
            d = pickle.dumps(it)
            it = pickle.loads(d)
            self.assertEqual(list(it), data[1:])

    def test_strip(self):
        b = self.type2test(b'mississippi')
        self.assertEqual(b.strip(b'i'), b'mississipp')
        self.assertEqual(b.strip(b'm'), b'ississippi')
        self.assertEqual(b.strip(b'pi'), b'mississ')
        self.assertEqual(b.strip(b'im'), b'ssissipp')
        self.assertEqual(b.strip(b'pim'), b'ssiss')
        self.assertEqual(b.strip(b), b'')

    def test_lstrip(self):
        b = self.type2test(b'mississippi')
        self.assertEqual(b.lstrip(b'i'), b'mississippi')
        self.assertEqual(b.lstrip(b'm'), b'ississippi')
        self.assertEqual(b.lstrip(b'pi'), b'mississippi')
        self.assertEqual(b.lstrip(b'im'), b'ssissippi')
        self.assertEqual(b.lstrip(b'pim'), b'ssissippi')

    def test_rstrip(self):
        b = self.type2test(b'mississippi')
        self.assertEqual(b.rstrip(b'i'), b'mississipp')
        self.assertEqual(b.rstrip(b'm'), b'mississippi')
        self.assertEqual(b.rstrip(b'pi'), b'mississ')
        self.assertEqual(b.rstrip(b'im'), b'mississipp')
        self.assertEqual(b.rstrip(b'pim'), b'mississ')

    def test_strip_whitespace(self):
        b = self.type2test(b' \t\n\r\f\vabc \t\n\r\f\v')
        self.assertEqual(b.strip(), b'abc')
        self.assertEqual(b.lstrip(), b'abc \t\n\r\f\v')
        self.assertEqual(b.rstrip(), b' \t\n\r\f\vabc')

    def test_strip_bytearray(self):
        self.assertEqual(self.type2test(b'abc').strip(memoryview(b'ac')), b'b')
        self.assertEqual(self.type2test(b'abc').lstrip(memoryview(b'ac')), b'bc')
        self.assertEqual(self.type2test(b'abc').rstrip(memoryview(b'ac')), b'ab')

    def test_strip_string_error(self):
        self.assertRaises(TypeError, self.type2test(b'abc').strip, 'b')
        self.assertRaises(TypeError, self.type2test(b'abc').lstrip, 'b')
        self.assertRaises(TypeError, self.type2test(b'abc').rstrip, 'b')

    def test_center(self):
        # Fill character can be either bytes or bytearray (issue 12380)
        b = self.type2test(b'abc')
        for fill_type in (bytes, bytearray):
            self.assertEqual(b.center(7, fill_type(b'-')),
                             self.type2test(b'--abc--'))

    def test_ljust(self):
        # Fill character can be either bytes or bytearray (issue 12380)
        b = self.type2test(b'abc')
        for fill_type in (bytes, bytearray):
            self.assertEqual(b.ljust(7, fill_type(b'-')),
                             self.type2test(b'abc----'))

    def test_rjust(self):
        # Fill character can be either bytes or bytearray (issue 12380)
        b = self.type2test(b'abc')
        for fill_type in (bytes, bytearray):
            self.assertEqual(b.rjust(7, fill_type(b'-')),
                             self.type2test(b'----abc'))

    def test_ord(self):
        b = self.type2test(b'\0A\x7f\x80\xff')
        self.assertEqual([ord(b[i:i+1]) for i in range(len(b))],
                         [0, 65, 127, 128, 255])

    def test_maketrans(self):
        transtable = b'\000\001\002\003\004\005\006\007\010\011\012\013\014\015\016\017\020\021\022\023\024\025\026\027\030\031\032\033\034\035\036\037 !"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`xyzdefghijklmnopqrstuvwxyz{|}~\177\200\201\202\203\204\205\206\207\210\211\212\213\214\215\216\217\220\221\222\223\224\225\226\227\230\231\232\233\234\235\236\237\240\241\242\243\244\245\246\247\250\251\252\253\254\255\256\257\260\261\262\263\264\265\266\267\270\271\272\273\274\275\276\277\300\301\302\303\304\305\306\307\310\311\312\313\314\315\316\317\320\321\322\323\324\325\326\327\330\331\332\333\334\335\336\337\340\341\342\343\344\345\346\347\350\351\352\353\354\355\356\357\360\361\362\363\364\365\366\367\370\371\372\373\374\375\376\377'
        self.assertEqual(self.type2test.maketrans(b'abc', b'xyz'), transtable)
        transtable = b'\000\001\002\003\004\005\006\007\010\011\012\013\014\015\016\017\020\021\022\023\024\025\026\027\030\031\032\033\034\035\036\037 !"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\177\200\201\202\203\204\205\206\207\210\211\212\213\214\215\216\217\220\221\222\223\224\225\226\227\230\231\232\233\234\235\236\237\240\241\242\243\244\245\246\247\250\251\252\253\254\255\256\257\260\261\262\263\264\265\266\267\270\271\272\273\274\275\276\277\300\301\302\303\304\305\306\307\310\311\312\313\314\315\316\317\320\321\322\323\324\325\326\327\330\331\332\333\334\335\336\337\340\341\342\343\344\345\346\347\350\351\352\353\354\355\356\357\360\361\362\363\364\365\366\367\370\371\372\373\374xyz'
        self.assertEqual(self.type2test.maketrans(b'\375\376\377', b'xyz'), transtable)
        self.assertRaises(ValueError, self.type2test.maketrans, b'abc', b'xyzq')
        self.assertRaises(TypeError, self.type2test.maketrans, 'abc', 'def')

    def test_none_arguments(self):
        # issue 11828
        b = self.type2test(b'hello')
        l = self.type2test(b'l')
        h = self.type2test(b'h')
        x = self.type2test(b'x')
        o = self.type2test(b'o')

        self.assertEqual(2, b.find(l, None))
        self.assertEqual(3, b.find(l, -2, None))
        self.assertEqual(2, b.find(l, None, -2))
        self.assertEqual(0, b.find(h, None, None))

        self.assertEqual(3, b.rfind(l, None))
        self.assertEqual(3, b.rfind(l, -2, None))
        self.assertEqual(2, b.rfind(l, None, -2))
        self.assertEqual(0, b.rfind(h, None, None))

        self.assertEqual(2, b.index(l, None))
        self.assertEqual(3, b.index(l, -2, None))
        self.assertEqual(2, b.index(l, None, -2))
        self.assertEqual(0, b.index(h, None, None))

        self.assertEqual(3, b.rindex(l, None))
        self.assertEqual(3, b.rindex(l, -2, None))
        self.assertEqual(2, b.rindex(l, None, -2))
        self.assertEqual(0, b.rindex(h, None, None))

        self.assertEqual(2, b.count(l, None))
        self.assertEqual(1, b.count(l, -2, None))
        self.assertEqual(1, b.count(l, None, -2))
        self.assertEqual(0, b.count(x, None, None))

        self.assertEqual(True, b.endswith(o, None))
        self.assertEqual(True, b.endswith(o, -2, None))
        self.assertEqual(True, b.endswith(l, None, -2))
        self.assertEqual(False, b.endswith(x, None, None))

        self.assertEqual(True, b.startswith(h, None))
        self.assertEqual(True, b.startswith(l, -2, None))
        self.assertEqual(True, b.startswith(h, None, -2))
        self.assertEqual(False, b.startswith(x, None, None))

    def test_integer_arguments_out_of_byte_range(self):
        b = self.type2test(b'hello')

        for method in (b.count, b.find, b.index, b.rfind, b.rindex):
            self.assertRaises(ValueError, method, -1)
            self.assertRaises(ValueError, method, 256)
            self.assertRaises(ValueError, method, 9999)

    def test_find_etc_raise_correct_error_messages(self):
        # issue 11828
        b = self.type2test(b'hello')
        x = self.type2test(b'x')
        self.assertRaisesRegex(TypeError, r'\bfind\b', b.find,
                                x, None, None, None)
        self.assertRaisesRegex(TypeError, r'\brfind\b', b.rfind,
                                x, None, None, None)
        self.assertRaisesRegex(TypeError, r'\bindex\b', b.index,
                                x, None, None, None)
        self.assertRaisesRegex(TypeError, r'\brindex\b', b.rindex,
                                x, None, None, None)
        self.assertRaisesRegex(TypeError, r'\bcount\b', b.count,
                                x, None, None, None)
        self.assertRaisesRegex(TypeError, r'\bstartswith\b', b.startswith,
                                x, None, None, None)
        self.assertRaisesRegex(TypeError, r'\bendswith\b', b.endswith,
                                x, None, None, None)


class BytesTest(BaseBytesTest, unittest.TestCase):
    type2test = bytes

    def test_buffer_is_readonly(self):
        fd = os.dup(sys.stdin.fileno())
        with open(fd, "rb", buffering=0) as f:
            self.assertRaises(TypeError, f.readinto, b"")

    def test_custom(self):
        class A:
            def __bytes__(self):
                return b'abc'
        self.assertEqual(bytes(A()), b'abc')
        class A: pass
        self.assertRaises(TypeError, bytes, A())
        class A:
            def __bytes__(self):
                return None
        self.assertRaises(TypeError, bytes, A())
        class A:
            def __bytes__(self):
                return b'a'
            def __index__(self):
                return 42
        self.assertEqual(bytes(A()), b'a')

    # Test PyBytes_FromFormat()
    def test_from_format(self):
        test.support.import_module('ctypes')
        from ctypes import pythonapi, py_object, c_int, c_char_p
        PyBytes_FromFormat = pythonapi.PyBytes_FromFormat
        PyBytes_FromFormat.restype = py_object

        self.assertEqual(PyBytes_FromFormat(b'format'),
                         b'format')

        self.assertEqual(PyBytes_FromFormat(b'%'), b'%')
        self.assertEqual(PyBytes_FromFormat(b'%%'), b'%')
        self.assertEqual(PyBytes_FromFormat(b'%%s'), b'%s')
        self.assertEqual(PyBytes_FromFormat(b'[%%]'), b'[%]')
        self.assertEqual(PyBytes_FromFormat(b'%%%c', c_int(ord('_'))), b'%_')

        self.assertEqual(PyBytes_FromFormat(b'c:%c', c_int(255)),
                         b'c:\xff')
        self.assertEqual(PyBytes_FromFormat(b's:%s', c_char_p(b'cstr')),
                         b's:cstr')

        # Issue #19969
        self.assertRaises(OverflowError,
                          PyBytes_FromFormat, b'%c', c_int(-1))
        self.assertRaises(OverflowError,
                          PyBytes_FromFormat, b'%c', c_int(256))


class ByteArrayTest(BaseBytesTest, unittest.TestCase):
    type2test = bytearray

    def test_nohash(self):
        self.assertRaises(TypeError, hash, bytearray())

    def test_bytearray_api(self):
        short_sample = b"Hello world\n"
        sample = short_sample + b"\0"*(20 - len(short_sample))
        tfn = tempfile.mktemp()
        try:
            # Prepare
            with open(tfn, "wb") as f:
                f.write(short_sample)
            # Test readinto
            with open(tfn, "rb") as f:
                b = bytearray(20)
                n = f.readinto(b)
            self.assertEqual(n, len(short_sample))
            self.assertEqual(list(b), list(sample))
            # Test writing in binary mode
            with open(tfn, "wb") as f:
                f.write(b)
            with open(tfn, "rb") as f:
                self.assertEqual(f.read(), sample)
            # Text mode is ambiguous; don't test
        finally:
            try:
                os.remove(tfn)
            except OSError:
                pass

    def test_reverse(self):
        b = bytearray(b'hello')
        self.assertEqual(b.reverse(), None)
        self.assertEqual(b, b'olleh')
        b = bytearray(b'hello1') # test even number of items
        b.reverse()
        self.assertEqual(b, b'1olleh')
        b = bytearray()
        b.reverse()
        self.assertFalse(b)

    def test_clear(self):
        b = bytearray(b'python')
        b.clear()
        self.assertEqual(b, b'')

        b = bytearray(b'')
        b.clear()
        self.assertEqual(b, b'')

        b = bytearray(b'')
        b.append(ord('r'))
        b.clear()
        b.append(ord('p'))
        self.assertEqual(b, b'p')

    def test_copy(self):
        b = bytearray(b'abc')
        bb = b.copy()
        self.assertEqual(bb, b'abc')

        b = bytearray(b'')
        bb = b.copy()
        self.assertEqual(bb, b'')

        # test that it's indeed a copy and not a reference
        b = bytearray(b'abc')
        bb = b.copy()
        self.assertEqual(b, bb)
        self.assertIsNot(b, bb)
        bb.append(ord('d'))
        self.assertEqual(bb, b'abcd')
        self.assertEqual(b, b'abc')

    def test_regexps(self):
        def by(s):
            return bytearray(map(ord, s))
        b = by("Hello, world")
        self.assertEqual(re.findall(br"\w+", b), [by("Hello"), by("world")])

    def test_setitem(self):
        b = bytearray([1, 2, 3])
        b[1] = 100
        self.assertEqual(b, bytearray([1, 100, 3]))
        b[-1] = 200
        self.assertEqual(b, bytearray([1, 100, 200]))
        b[0] = Indexable(10)
        self.assertEqual(b, bytearray([10, 100, 200]))
        try:
            b[3] = 0
            self.fail("Didn't raise IndexError")
        except IndexError:
            pass
        try:
            b[-10] = 0
            self.fail("Didn't raise IndexError")
        except IndexError:
            pass
        try:
            b[0] = 256
            self.fail("Didn't raise ValueError")
        except ValueError:
            pass
        try:
            b[0] = Indexable(-1)
            self.fail("Didn't raise ValueError")
        except ValueError:
            pass
        try:
            b[0] = None
            self.fail("Didn't raise TypeError")
        except TypeError:
            pass

    def test_delitem(self):
        b = bytearray(range(10))
        del b[0]
        self.assertEqual(b, bytearray(range(1, 10)))
        del b[-1]
        self.assertEqual(b, bytearray(range(1, 9)))
        del b[4]
        self.assertEqual(b, bytearray([1, 2, 3, 4, 6, 7, 8]))

    def test_setslice(self):
        b = bytearray(range(10))
        self.assertEqual(list(b), list(range(10)))

        b[0:5] = bytearray([1, 1, 1, 1, 1])
        self.assertEqual(b, bytearray([1, 1, 1, 1, 1, 5, 6, 7, 8, 9]))

        del b[0:-5]
        self.assertEqual(b, bytearray([5, 6, 7, 8, 9]))

        b[0:0] = bytearray([0, 1, 2, 3, 4])
        self.assertEqual(b, bytearray(range(10)))

        b[-7:-3] = bytearray([100, 101])
        self.assertEqual(b, bytearray([0, 1, 2, 100, 101, 7, 8, 9]))

        b[3:5] = [3, 4, 5, 6]
        self.assertEqual(b, bytearray(range(10)))

        b[3:0] = [42, 42, 42]
        self.assertEqual(b, bytearray([0, 1, 2, 42, 42, 42, 3, 4, 5, 6, 7, 8, 9]))

        b[3:] = b'foo'
        self.assertEqual(b, bytearray([0, 1, 2, 102, 111, 111]))

        b[:3] = memoryview(b'foo')
        self.assertEqual(b, bytearray([102, 111, 111, 102, 111, 111]))

        b[3:4] = []
        self.assertEqual(b, bytearray([102, 111, 111, 111, 111]))

        for elem in [5, -5, 0, int(10e20), 'str', 2.3,
                     ['a', 'b'], [b'a', b'b'], [[]]]:
            with self.assertRaises(TypeError):
                b[3:4] = elem

        for elem in [[254, 255, 256], [-256, 9000]]:
            with self.assertRaises(ValueError):
                b[3:4] = elem

    def test_setslice_extend(self):
        # Exercise the resizing logic (see issue #19087)
        b = bytearray(range(100))
        self.assertEqual(list(b), list(range(100)))
        del b[:10]
        self.assertEqual(list(b), list(range(10, 100)))
        b.extend(range(100, 110))
        self.assertEqual(list(b), list(range(10, 110)))

    def test_extended_set_del_slice(self):
        indices = (0, None, 1, 3, 19, 300, 1<<333, -1, -2, -31, -300)
        for start in indices:
            for stop in indices:
                # Skip invalid step 0
                for step in indices[1:]:
                    L = list(range(255))
                    b = bytearray(L)
                    # Make sure we have a slice of exactly the right length,
                    # but with different data.
                    data = L[start:stop:step]
                    data.reverse()
                    L[start:stop:step] = data
                    b[start:stop:step] = data
                    self.assertEqual(b, bytearray(L))

                    del L[start:stop:step]
                    del b[start:stop:step]
                    self.assertEqual(b, bytearray(L))

    def test_setslice_trap(self):
        # This test verifies that we correctly handle assigning self
        # to a slice of self (the old Lambert Meertens trap).
        b = bytearray(range(256))
        b[8:] = b
        self.assertEqual(b, bytearray(list(range(8)) + list(range(256))))

    def test_iconcat(self):
        b = bytearray(b"abc")
        b1 = b
        b += b"def"
        self.assertEqual(b, b"abcdef")
        self.assertEqual(b, b1)
        self.assertTrue(b is b1)
        b += b"xyz"
        self.assertEqual(b, b"abcdefxyz")
        try:
            b += ""
        except TypeError:
            pass
        else:
            self.fail("bytes += unicode didn't raise TypeError")

    def test_irepeat(self):
        b = bytearray(b"abc")
        b1 = b
        b *= 3
        self.assertEqual(b, b"abcabcabc")
        self.assertEqual(b, b1)
        self.assertTrue(b is b1)

    def test_irepeat_1char(self):
        b = bytearray(b"x")
        b1 = b
        b *= 100
        self.assertEqual(b, b"x"*100)
        self.assertEqual(b, b1)
        self.assertTrue(b is b1)

    def test_alloc(self):
        b = bytearray()
        alloc = b.__alloc__()
        self.assertTrue(alloc >= 0)
        seq = [alloc]
        for i in range(100):
            b += b"x"
            alloc = b.__alloc__()
            self.assertTrue(alloc >= len(b))
            if alloc not in seq:
                seq.append(alloc)

    def test_extend(self):
        orig = b'hello'
        a = bytearray(orig)
        a.extend(a)
        self.assertEqual(a, orig + orig)
        self.assertEqual(a[5:], orig)
        a = bytearray(b'')
        # Test iterators that don't have a __length_hint__
        a.extend(map(int, orig * 25))
        a.extend(int(x) for x in orig * 25)
        self.assertEqual(a, orig * 50)
        self.assertEqual(a[-5:], orig)
        a = bytearray(b'')
        a.extend(iter(map(int, orig * 50)))
        self.assertEqual(a, orig * 50)
        self.assertEqual(a[-5:], orig)
        a = bytearray(b'')
        a.extend(list(map(int, orig * 50)))
        self.assertEqual(a, orig * 50)
        self.assertEqual(a[-5:], orig)
        a = bytearray(b'')
        self.assertRaises(ValueError, a.extend, [0, 1, 2, 256])
        self.assertRaises(ValueError, a.extend, [0, 1, 2, -1])
        self.assertEqual(len(a), 0)
        a = bytearray(b'')
        a.extend([Indexable(ord('a'))])
        self.assertEqual(a, b'a')

    def test_remove(self):
        b = bytearray(b'hello')
        b.remove(ord('l'))
        self.assertEqual(b, b'helo')
        b.remove(ord('l'))
        self.assertEqual(b, b'heo')
        self.assertRaises(ValueError, lambda: b.remove(ord('l')))
        self.assertRaises(ValueError, lambda: b.remove(400))
        self.assertRaises(TypeError, lambda: b.remove('e'))
        # remove first and last
        b.remove(ord('o'))
        b.remove(ord('h'))
        self.assertEqual(b, b'e')
        self.assertRaises(TypeError, lambda: b.remove(b'e'))
        b.remove(Indexable(ord('e')))
        self.assertEqual(b, b'')

    def test_pop(self):
        b = bytearray(b'world')
        self.assertEqual(b.pop(), ord('d'))
        self.assertEqual(b.pop(0), ord('w'))
        self.assertEqual(b.pop(-2), ord('r'))
        self.assertRaises(IndexError, lambda: b.pop(10))
        self.assertRaises(IndexError, lambda: bytearray().pop())
        # test for issue #6846
        self.assertEqual(bytearray(b'\xff').pop(), 0xff)

    def test_nosort(self):
        self.assertRaises(AttributeError, lambda: bytearray().sort())

    def test_append(self):
        b = bytearray(b'hell')
        b.append(ord('o'))
        self.assertEqual(b, b'hello')
        self.assertEqual(b.append(100), None)
        b = bytearray()
        b.append(ord('A'))
        self.assertEqual(len(b), 1)
        self.assertRaises(TypeError, lambda: b.append(b'o'))
        b = bytearray()
        b.append(Indexable(ord('A')))
        self.assertEqual(b, b'A')

    def test_insert(self):
        b = bytearray(b'msssspp')
        b.insert(1, ord('i'))
        b.insert(4, ord('i'))
        b.insert(-2, ord('i'))
        b.insert(1000, ord('i'))
        self.assertEqual(b, b'mississippi')
        self.assertRaises(TypeError, lambda: b.insert(0, b'1'))
        b = bytearray()
        b.insert(0, Indexable(ord('A')))
        self.assertEqual(b, b'A')

    def test_copied(self):
        # Issue 4348.  Make sure that operations that don't mutate the array
        # copy the bytes.
        b = bytearray(b'abc')
        self.assertFalse(b is b.replace(b'abc', b'cde', 0))

        t = bytearray([i for i in range(256)])
        x = bytearray(b'')
        self.assertFalse(x is x.translate(t))

    def test_partition_bytearray_doesnt_share_nullstring(self):
        a, b, c = bytearray(b"x").partition(b"y")
        self.assertEqual(b, b"")
        self.assertEqual(c, b"")
        self.assertTrue(b is not c)
        b += b"!"
        self.assertEqual(c, b"")
        a, b, c = bytearray(b"x").partition(b"y")
        self.assertEqual(b, b"")
        self.assertEqual(c, b"")
        # Same for rpartition
        b, c, a = bytearray(b"x").rpartition(b"y")
        self.assertEqual(b, b"")
        self.assertEqual(c, b"")
        self.assertTrue(b is not c)
        b += b"!"
        self.assertEqual(c, b"")
        c, b, a = bytearray(b"x").rpartition(b"y")
        self.assertEqual(b, b"")
        self.assertEqual(c, b"")

    def test_resize_forbidden(self):
        # #4509: can't resize a bytearray when there are buffer exports, even
        # if it wouldn't reallocate the underlying buffer.
        # Furthermore, no destructive changes to the buffer may be applied
        # before raising the error.
        b = bytearray(range(10))
        v = memoryview(b)
        def resize(n):
            b[1:-1] = range(n + 1, 2*n - 1)
        resize(10)
        orig = b[:]
        self.assertRaises(BufferError, resize, 11)
        self.assertEqual(b, orig)
        self.assertRaises(BufferError, resize, 9)
        self.assertEqual(b, orig)
        self.assertRaises(BufferError, resize, 0)
        self.assertEqual(b, orig)
        # Other operations implying resize
        self.assertRaises(BufferError, b.pop, 0)
        self.assertEqual(b, orig)
        self.assertRaises(BufferError, b.remove, b[1])
        self.assertEqual(b, orig)
        def delitem():
            del b[1]
        self.assertRaises(BufferError, delitem)
        self.assertEqual(b, orig)
        # deleting a non-contiguous slice
        def delslice():
            b[1:-1:2] = b""
        self.assertRaises(BufferError, delslice)
        self.assertEqual(b, orig)


class AssortedBytesTest(unittest.TestCase):
    #
    # Test various combinations of bytes and bytearray
    #

    @check_bytes_warnings
    def test_repr_str(self):
        for f in str, repr:
            self.assertEqual(f(bytearray()), "bytearray(b'')")
            self.assertEqual(f(bytearray([0])), "bytearray(b'\\x00')")
            self.assertEqual(f(bytearray([0, 1, 254, 255])),
                             "bytearray(b'\\x00\\x01\\xfe\\xff')")
            self.assertEqual(f(b"abc"), "b'abc'")
            self.assertEqual(f(b"'"), '''b"'"''') # '''
            self.assertEqual(f(b"'\""), r"""b'\'"'""") # '

    def test_compare_bytes_to_bytearray(self):
        self.assertEqual(b"abc" == bytes(b"abc"), True)
        self.assertEqual(b"ab" != bytes(b"abc"), True)
        self.assertEqual(b"ab" <= bytes(b"abc"), True)
        self.assertEqual(b"ab" < bytes(b"abc"), True)
        self.assertEqual(b"abc" >= bytes(b"ab"), True)
        self.assertEqual(b"abc" > bytes(b"ab"), True)

        self.assertEqual(b"abc" != bytes(b"abc"), False)
        self.assertEqual(b"ab" == bytes(b"abc"), False)
        self.assertEqual(b"ab" > bytes(b"abc"), False)
        self.assertEqual(b"ab" >= bytes(b"abc"), False)
        self.assertEqual(b"abc" < bytes(b"ab"), False)
        self.assertEqual(b"abc" <= bytes(b"ab"), False)

        self.assertEqual(bytes(b"abc") == b"abc", True)
        self.assertEqual(bytes(b"ab") != b"abc", True)
        self.assertEqual(bytes(b"ab") <= b"abc", True)
        self.assertEqual(bytes(b"ab") < b"abc", True)
        self.assertEqual(bytes(b"abc") >= b"ab", True)
        self.assertEqual(bytes(b"abc") > b"ab", True)

        self.assertEqual(bytes(b"abc") != b"abc", False)
        self.assertEqual(bytes(b"ab") == b"abc", False)
        self.assertEqual(bytes(b"ab") > b"abc", False)
        self.assertEqual(bytes(b"ab") >= b"abc", False)
        self.assertEqual(bytes(b"abc") < b"ab", False)
        self.assertEqual(bytes(b"abc") <= b"ab", False)

    @test.support.requires_docstrings
    def test_doc(self):
        self.assertIsNotNone(bytearray.__doc__)
        self.assertTrue(bytearray.__doc__.startswith("bytearray("), bytearray.__doc__)
        self.assertIsNotNone(bytes.__doc__)
        self.assertTrue(bytes.__doc__.startswith("bytes("), bytes.__doc__)

    def test_from_bytearray(self):
        sample = bytes(b"Hello world\n\x80\x81\xfe\xff")
        buf = memoryview(sample)
        b = bytearray(buf)
        self.assertEqual(b, bytearray(sample))

    @check_bytes_warnings
    def test_to_str(self):
        self.assertEqual(str(b''), "b''")
        self.assertEqual(str(b'x'), "b'x'")
        self.assertEqual(str(b'\x80'), "b'\\x80'")
        self.assertEqual(str(bytearray(b'')), "bytearray(b'')")
        self.assertEqual(str(bytearray(b'x')), "bytearray(b'x')")
        self.assertEqual(str(bytearray(b'\x80')), "bytearray(b'\\x80')")

    def test_literal(self):
        tests =  [
            (b"Wonderful spam", "Wonderful spam"),
            (br"Wonderful spam too", "Wonderful spam too"),
            (b"\xaa\x00\000\200", "\xaa\x00\000\200"),
            (br"\xaa\x00\000\200", r"\xaa\x00\000\200"),
        ]
        for b, s in tests:
            self.assertEqual(b, bytearray(s, 'latin-1'))
        for c in range(128, 256):
            self.assertRaises(SyntaxError, eval,
                              'b"%s"' % chr(c))

    def test_translate(self):
        b = b'hello'
        ba = bytearray(b)
        rosetta = bytearray(range(0, 256))
        rosetta[ord('o')] = ord('e')
        c = b.translate(rosetta, b'l')
        self.assertEqual(b, b'hello')
        self.assertEqual(c, b'hee')
        c = ba.translate(rosetta, b'l')
        self.assertEqual(ba, b'hello')
        self.assertEqual(c, b'hee')
        c = b.translate(None, b'e')
        self.assertEqual(c, b'hllo')
        c = ba.translate(None, b'e')
        self.assertEqual(c, b'hllo')
        self.assertRaises(TypeError, b.translate, None, None)
        self.assertRaises(TypeError, ba.translate, None, None)

    def test_split_bytearray(self):
        self.assertEqual(b'a b'.split(memoryview(b' ')), [b'a', b'b'])

    def test_rsplit_bytearray(self):
        self.assertEqual(b'a b'.rsplit(memoryview(b' ')), [b'a', b'b'])

    def test_return_self(self):
        # bytearray.replace must always return a new bytearray
        b = bytearray()
        self.assertFalse(b.replace(b'', b'') is b)

    def test_compare(self):
        if sys.flags.bytes_warning:
            def bytes_warning():
                return test.support.check_warnings(('', BytesWarning))
            with bytes_warning():
                b'' == ''
            with bytes_warning():
                b'' != ''
            with bytes_warning():
                bytearray(b'') == ''
            with bytes_warning():
                bytearray(b'') != ''
        else:
            self.skipTest("BytesWarning is needed for this test: use -bb option")

    # Optimizations:
    # __iter__? (optimization)
    # __reversed__? (optimization)

    # XXX More string methods?  (Those that don't use character properties)

    # There are tests in string_tests.py that are more
    # comprehensive for things like split, partition, etc.
    # Unfortunately they are all bundled with tests that
    # are not appropriate for bytes

    # I've started porting some of those into bytearray_tests.py, we should port
    # the rest that make sense (the code can be cleaned up to use modern
    # unittest methods at the same time).

class BytearrayPEP3137Test(unittest.TestCase,
                       test.buffer_tests.MixinBytesBufferCommonTests):
    def marshal(self, x):
        return bytearray(x)

    def test_returns_new_copy(self):
        val = self.marshal(b'1234')
        # On immutable types these MAY return a reference to themselves
        # but on mutable types like bytearray they MUST return a new copy.
        for methname in ('zfill', 'rjust', 'ljust', 'center'):
            method = getattr(val, methname)
            newval = method(3)
            self.assertEqual(val, newval)
            self.assertTrue(val is not newval,
                            methname+' returned self on a mutable object')
        for expr in ('val.split()[0]', 'val.rsplit()[0]',
                     'val.partition(b".")[0]', 'val.rpartition(b".")[2]',
                     'val.splitlines()[0]', 'val.replace(b"", b"")'):
            newval = eval(expr)
            self.assertEqual(val, newval)
            self.assertTrue(val is not newval,
                            expr+' returned val on a mutable object')
        sep = self.marshal(b'')
        newval = sep.join([val])
        self.assertEqual(val, newval)
        self.assertIsNot(val, newval)


class FixedStringTest(test.string_tests.BaseTest):

    def fixtype(self, obj):
        if isinstance(obj, str):
            return obj.encode("utf-8")
        return super().fixtype(obj)

    # Currently the bytes containment testing uses a single integer
    # value. This may not be the final design, but until then the
    # bytes section with in a bytes containment not valid
    def test_contains(self):
        pass
    def test_expandtabs(self):
        pass
    def test_upper(self):
        pass
    def test_lower(self):
        pass

class ByteArrayAsStringTest(FixedStringTest, unittest.TestCase):
    type2test = bytearray
    contains_bytes = True

class BytesAsStringTest(FixedStringTest, unittest.TestCase):
    type2test = bytes
    contains_bytes = True


class SubclassTest:

    def test_basic(self):
        self.assertTrue(issubclass(self.subclass2test, self.type2test))
        self.assertIsInstance(self.subclass2test(), self.type2test)

        a, b = b"abcd", b"efgh"
        _a, _b = self.subclass2test(a), self.subclass2test(b)

        # test comparison operators with subclass instances
        self.assertTrue(_a == _a)
        self.assertTrue(_a != _b)
        self.assertTrue(_a < _b)
        self.assertTrue(_a <= _b)
        self.assertTrue(_b >= _a)
        self.assertTrue(_b > _a)
        self.assertTrue(_a is not a)

        # test concat of subclass instances
        self.assertEqual(a + b, _a + _b)
        self.assertEqual(a + b, a + _b)
        self.assertEqual(a + b, _a + b)

        # test repeat
        self.assertTrue(a*5 == _a*5)

    def test_join(self):
        # Make sure join returns a NEW object for single item sequences
        # involving a subclass.
        # Make sure that it is of the appropriate type.
        s1 = self.subclass2test(b"abcd")
        s2 = self.type2test().join([s1])
        self.assertTrue(s1 is not s2)
        self.assertTrue(type(s2) is self.type2test, type(s2))

        # Test reverse, calling join on subclass
        s3 = s1.join([b"abcd"])
        self.assertTrue(type(s3) is self.type2test)

    def test_pickle(self):
        a = self.subclass2test(b"abcd")
        a.x = 10
        a.y = self.subclass2test(b"efgh")
        for proto in range(pickle.HIGHEST_PROTOCOL + 1):
            b = pickle.loads(pickle.dumps(a, proto))
            self.assertNotEqual(id(a), id(b))
            self.assertEqual(a, b)
            self.assertEqual(a.x, b.x)
            self.assertEqual(a.y, b.y)
            self.assertEqual(type(a), type(b))
            self.assertEqual(type(a.y), type(b.y))

    def test_copy(self):
        a = self.subclass2test(b"abcd")
        a.x = 10
        a.y = self.subclass2test(b"efgh")
        for copy_method in (copy.copy, copy.deepcopy):
            b = copy_method(a)
            self.assertNotEqual(id(a), id(b))
            self.assertEqual(a, b)
            self.assertEqual(a.x, b.x)
            self.assertEqual(a.y, b.y)
            self.assertEqual(type(a), type(b))
            self.assertEqual(type(a.y), type(b.y))


class ByteArraySubclass(bytearray):
    pass

class BytesSubclass(bytes):
    pass

class ByteArraySubclassTest(SubclassTest, unittest.TestCase):
    type2test = bytearray
    subclass2test = ByteArraySubclass

    def test_init_override(self):
        class subclass(bytearray):
            def __init__(me, newarg=1, *args, **kwargs):
                bytearray.__init__(me, *args, **kwargs)
        x = subclass(4, b"abcd")
        x = subclass(4, source=b"abcd")
        self.assertEqual(x, b"abcd")
        x = subclass(newarg=4, source=b"abcd")
        self.assertEqual(x, b"abcd")


class BytesSubclassTest(SubclassTest, unittest.TestCase):
    type2test = bytes
    subclass2test = BytesSubclass


if __name__ == "__main__":
    unittest.main()
