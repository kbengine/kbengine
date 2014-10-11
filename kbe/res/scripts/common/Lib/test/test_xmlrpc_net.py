import collections.abc
import errno
import socket
import sys
import unittest
from test import support

import xmlrpc.client as xmlrpclib

class PythonBuildersTest(unittest.TestCase):

    def test_python_builders(self):
        # Get the list of builders from the XMLRPC buildbot interface at
        # python.org.
        server = xmlrpclib.ServerProxy("http://buildbot.python.org/all/xmlrpc/")
        try:
            builders = server.getAllBuilders()
        except OSError as e:
            self.skipTest("network error: %s" % e)
        self.addCleanup(lambda: server('close')())

        # Perform a minimal sanity check on the result, just to be sure
        # the request means what we think it means.
        self.assertIsInstance(builders, collections.abc.Sequence)
        self.assertTrue([x for x in builders if "3.x" in x], builders)


def test_main():
    support.requires("network")
    support.run_unittest(PythonBuildersTest)

if __name__ == "__main__":
    test_main()
