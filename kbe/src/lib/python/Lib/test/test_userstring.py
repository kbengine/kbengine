# UserString is a wrapper around the native builtin string type.
# UserString instances should behave similar to builtin string objects.

import string
import unittest
from test import support, string_tests

from collections import UserString

class UserStringTest(
    string_tests.CommonTest,
    string_tests.MixinStrUnicodeUserStringTest,
    unittest.TestCase
    ):

    type2test = UserString

    # Overwrite the three testing methods, because UserString
    # can't cope with arguments propagated to UserString
    # (and we don't test with subclasses)
    def checkequal(self, result, object, methodname, *args, **kwargs):
        result = self.fixtype(result)
        object = self.fixtype(object)
        # we don't fix the arguments, because UserString can't cope with it
        realresult = getattr(object, methodname)(*args, **kwargs)
        self.assertEqual(
            result,
            realresult
        )

    def checkraises(self, exc, object, methodname, *args):
        object = self.fixtype(object)
        # we don't fix the arguments, because UserString can't cope with it
        self.assertRaises(
            exc,
            getattr(object, methodname),
            *args
        )

    def checkcall(self, object, methodname, *args):
        object = self.fixtype(object)
        # we don't fix the arguments, because UserString can't cope with it
        getattr(object, methodname)(*args)


if __name__ == "__main__":
    unittest.main()
