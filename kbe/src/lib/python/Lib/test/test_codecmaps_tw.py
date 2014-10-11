#
# test_codecmaps_tw.py
#   Codec mapping tests for ROC encodings
#

from test import support
from test import multibytecodec_support
import unittest

class TestBIG5Map(multibytecodec_support.TestBase_Mapping,
                  unittest.TestCase):
    encoding = 'big5'
    mapfileurl = 'http://www.unicode.org/Public/MAPPINGS/OBSOLETE/' \
                 'EASTASIA/OTHER/BIG5.TXT'

class TestCP950Map(multibytecodec_support.TestBase_Mapping,
                   unittest.TestCase):
    encoding = 'cp950'
    mapfileurl = 'http://www.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/' \
                 'WINDOWS/CP950.TXT'
    pass_enctest = [
        (b'\xa2\xcc', '\u5341'),
        (b'\xa2\xce', '\u5345'),
    ]
    codectests = (
        (b"\xFFxy", "replace",  "\ufffdxy"),
    )

def test_main():
    support.run_unittest(__name__)

if __name__ == "__main__":
    test_main()
