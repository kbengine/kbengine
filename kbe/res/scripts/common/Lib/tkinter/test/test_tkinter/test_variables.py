import unittest

from tkinter import Variable, StringVar, IntVar, DoubleVar, BooleanVar, Tk


class Var(Variable):

    _default = "default"
    side_effect = False

    def set(self, value):
        self.side_effect = True
        super().set(value)


class TestBase(unittest.TestCase):

    def setUp(self):
        self.root = Tk()

    def tearDown(self):
        self.root.destroy()


class TestVariable(TestBase):

    def info_exists(self, *args):
        return self.root.getboolean(self.root.call("info", "exists", *args))

    def test_default(self):
        v = Variable(self.root)
        self.assertEqual("", v.get())
        self.assertRegex(str(v), r"^PY_VAR(\d+)$")

    def test_name_and_value(self):
        v = Variable(self.root, "sample string", "varname")
        self.assertEqual("sample string", v.get())
        self.assertEqual("varname", str(v))

    def test___del__(self):
        self.assertFalse(self.info_exists("varname"))
        v = Variable(self.root, "sample string", "varname")
        self.assertTrue(self.info_exists("varname"))
        del v
        self.assertFalse(self.info_exists("varname"))

    def test_dont_unset_not_existing(self):
        self.assertFalse(self.info_exists("varname"))
        v1 = Variable(self.root, name="name")
        v2 = Variable(self.root, name="name")
        del v1
        self.assertFalse(self.info_exists("name"))
        # shouldn't raise exception
        del v2
        self.assertFalse(self.info_exists("name"))

    def test___eq__(self):
        # values doesn't matter, only class and name are checked
        v1 = Variable(self.root, name="abc")
        v2 = Variable(self.root, name="abc")
        self.assertEqual(v1, v2)

        v3 = Variable(self.root, name="abc")
        v4 = StringVar(self.root, name="abc")
        self.assertNotEqual(v3, v4)

    def test_invalid_name(self):
        with self.assertRaises(TypeError):
            Variable(self.root, name=123)

    def test_null_in_name(self):
        with self.assertRaises(ValueError):
            Variable(self.root, name='var\x00name')
        with self.assertRaises(ValueError):
            self.root.globalsetvar('var\x00name', "value")
        with self.assertRaises(ValueError):
            self.root.globalsetvar(b'var\x00name', "value")
        with self.assertRaises(ValueError):
            self.root.setvar('var\x00name', "value")
        with self.assertRaises(ValueError):
            self.root.setvar(b'var\x00name', "value")

    def test_initialize(self):
        v = Var()
        self.assertFalse(v.side_effect)
        v.set("value")
        self.assertTrue(v.side_effect)


class TestStringVar(TestBase):

    def test_default(self):
        v = StringVar(self.root)
        self.assertEqual("", v.get())

    def test_get(self):
        v = StringVar(self.root, "abc", "name")
        self.assertEqual("abc", v.get())
        self.root.globalsetvar("name", "value")
        self.assertEqual("value", v.get())

    def test_get_null(self):
        v = StringVar(self.root, "abc\x00def", "name")
        self.assertEqual("abc\x00def", v.get())
        self.root.globalsetvar("name", "val\x00ue")
        self.assertEqual("val\x00ue", v.get())


class TestIntVar(TestBase):

    def test_default(self):
        v = IntVar(self.root)
        self.assertEqual(0, v.get())

    def test_get(self):
        v = IntVar(self.root, 123, "name")
        self.assertEqual(123, v.get())
        self.root.globalsetvar("name", "345")
        self.assertEqual(345, v.get())

    def test_invalid_value(self):
        v = IntVar(self.root, name="name")
        self.root.globalsetvar("name", "value")
        with self.assertRaises(ValueError):
            v.get()
        self.root.globalsetvar("name", "345.0")
        with self.assertRaises(ValueError):
            v.get()


class TestDoubleVar(TestBase):

    def test_default(self):
        v = DoubleVar(self.root)
        self.assertEqual(0.0, v.get())

    def test_get(self):
        v = DoubleVar(self.root, 1.23, "name")
        self.assertAlmostEqual(1.23, v.get())
        self.root.globalsetvar("name", "3.45")
        self.assertAlmostEqual(3.45, v.get())

    def test_get_from_int(self):
        v = DoubleVar(self.root, 1.23, "name")
        self.assertAlmostEqual(1.23, v.get())
        self.root.globalsetvar("name", "3.45")
        self.assertAlmostEqual(3.45, v.get())
        self.root.globalsetvar("name", "456")
        self.assertAlmostEqual(456, v.get())

    def test_invalid_value(self):
        v = DoubleVar(self.root, name="name")
        self.root.globalsetvar("name", "value")
        with self.assertRaises(ValueError):
            v.get()


class TestBooleanVar(TestBase):

    def test_default(self):
        v = BooleanVar(self.root)
        self.assertEqual(False, v.get())

    def test_get(self):
        v = BooleanVar(self.root, True, "name")
        self.assertAlmostEqual(True, v.get())
        self.root.globalsetvar("name", "0")
        self.assertAlmostEqual(False, v.get())

    def test_invalid_value_domain(self):
        v = BooleanVar(self.root, name="name")
        self.root.globalsetvar("name", "value")
        with self.assertRaises(ValueError):
            v.get()
        self.root.globalsetvar("name", "1.0")
        with self.assertRaises(ValueError):
            v.get()


tests_gui = (TestVariable, TestStringVar, TestIntVar,
             TestDoubleVar, TestBooleanVar)


if __name__ == "__main__":
    from test.support import run_unittest
    run_unittest(*tests_gui)
