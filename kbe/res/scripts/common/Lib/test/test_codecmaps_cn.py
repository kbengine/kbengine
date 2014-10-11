#
# test_codecmaps_cn.py
#   Codec mapping tests for PRC encodings
#

from test import support
from test import multibytecodec_support
import unittest

class TestGB2312Map(multibytecodec_support.TestBase_Mapping,
                   unittest.TestCase):
    encoding = 'gb2312'
    mapfileurl = 'http://people.freebsd.org/~perky/i18n/EUC-CN.TXT'

class TestGBKMap(multibytecodec_support.TestBase_Mapping,
                   unittest.TestCase):
    encoding = 'gbk'
    mapfileurl = 'http://www.unicode.org/Public/MAPPINGS/VENDORS/' \
                 'MICSFT/WINDOWS/CP936.TXT'

class TestGB18030Map(multibytecodec_support.TestBase_Mapping,
                     unittest.TestCase):
    encoding = 'gb18030'
    mapfileurl = 'http://source.icu-project.org/repos/icu/data/' \
                 'trunk/charset/data/xml/gb-18030-2000.xml'


def test_main():
    support.run_unittest(__name__)

if __name__ == "__main__":
    test_main()
