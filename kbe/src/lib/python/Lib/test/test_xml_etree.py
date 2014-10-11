# IMPORTANT: the same tests are run from "test_xml_etree_c" in order
# to ensure consistency between the C implementation and the Python
# implementation.
#
# For this purpose, the module-level "ET" symbol is temporarily
# monkey-patched when running the "test_xml_etree_c" test suite.

import html
import io
import operator
import pickle
import sys
import types
import unittest
import weakref

from itertools import product
from test import support
from test.support import TESTFN, findfile, import_fresh_module, gc_collect

# pyET is the pure-Python implementation.
#
# ET is pyET in test_xml_etree and is the C accelerated version in
# test_xml_etree_c.
pyET = None
ET = None

SIMPLE_XMLFILE = findfile("simple.xml", subdir="xmltestdata")
try:
    SIMPLE_XMLFILE.encode("utf-8")
except UnicodeEncodeError:
    raise unittest.SkipTest("filename is not encodable to utf8")
SIMPLE_NS_XMLFILE = findfile("simple-ns.xml", subdir="xmltestdata")

SAMPLE_XML = """\
<body>
  <tag class='a'>text</tag>
  <tag class='b' />
  <section>
    <tag class='b' id='inner'>subtext</tag>
  </section>
</body>
"""

SAMPLE_SECTION = """\
<section>
  <tag class='b' id='inner'>subtext</tag>
  <nexttag />
  <nextsection>
    <tag />
  </nextsection>
</section>
"""

SAMPLE_XML_NS = """
<body xmlns="http://effbot.org/ns">
  <tag>text</tag>
  <tag />
  <section>
    <tag>subtext</tag>
  </section>
</body>
"""

SAMPLE_XML_NS_ELEMS = """
<root>
<h:table xmlns:h="hello">
  <h:tr>
    <h:td>Apples</h:td>
    <h:td>Bananas</h:td>
  </h:tr>
</h:table>

<f:table xmlns:f="foo">
  <f:name>African Coffee Table</f:name>
  <f:width>80</f:width>
  <f:length>120</f:length>
</f:table>
</root>
"""

ENTITY_XML = """\
<!DOCTYPE points [
<!ENTITY % user-entities SYSTEM 'user-entities.xml'>
%user-entities;
]>
<document>&entity;</document>
"""


class ModuleTest(unittest.TestCase):
    # TODO: this should be removed once we get rid of the global module vars

    def test_sanity(self):
        # Import sanity.

        from xml.etree import ElementTree
        from xml.etree import ElementInclude
        from xml.etree import ElementPath


def serialize(elem, to_string=True, encoding='unicode', **options):
    if encoding != 'unicode':
        file = io.BytesIO()
    else:
        file = io.StringIO()
    tree = ET.ElementTree(elem)
    tree.write(file, encoding=encoding, **options)
    if to_string:
        return file.getvalue()
    else:
        file.seek(0)
        return file

def summarize_list(seq):
    return [elem.tag for elem in seq]


class ElementTestCase:
    @classmethod
    def setUpClass(cls):
        cls.modules = {pyET, ET}

    def pickleRoundTrip(self, obj, name, dumper, loader):
        save_m = sys.modules[name]
        try:
            sys.modules[name] = dumper
            temp = pickle.dumps(obj)
            sys.modules[name] = loader
            result = pickle.loads(temp)
        except pickle.PicklingError as pe:
            # pyET must be second, because pyET may be (equal to) ET.
            human = dict([(ET, "cET"), (pyET, "pyET")])
            raise support.TestFailed("Failed to round-trip %r from %r to %r"
                                     % (obj,
                                        human.get(dumper, dumper),
                                        human.get(loader, loader))) from pe
        finally:
            sys.modules[name] = save_m
        return result

    def assertEqualElements(self, alice, bob):
        self.assertIsInstance(alice, (ET.Element, pyET.Element))
        self.assertIsInstance(bob, (ET.Element, pyET.Element))
        self.assertEqual(len(list(alice)), len(list(bob)))
        for x, y in zip(alice, bob):
            self.assertEqualElements(x, y)
        properties = operator.attrgetter('tag', 'tail', 'text', 'attrib')
        self.assertEqual(properties(alice), properties(bob))

# --------------------------------------------------------------------
# element tree tests

class ElementTreeTest(unittest.TestCase):

    def serialize_check(self, elem, expected):
        self.assertEqual(serialize(elem), expected)

    def test_interface(self):
        # Test element tree interface.

        def check_string(string):
            len(string)
            for char in string:
                self.assertEqual(len(char), 1,
                        msg="expected one-character string, got %r" % char)
            new_string = string + ""
            new_string = string + " "
            string[:0]

        def check_mapping(mapping):
            len(mapping)
            keys = mapping.keys()
            items = mapping.items()
            for key in keys:
                item = mapping[key]
            mapping["key"] = "value"
            self.assertEqual(mapping["key"], "value",
                    msg="expected value string, got %r" % mapping["key"])

        def check_element(element):
            self.assertTrue(ET.iselement(element), msg="not an element")
            self.assertTrue(hasattr(element, "tag"), msg="no tag member")
            self.assertTrue(hasattr(element, "attrib"), msg="no attrib member")
            self.assertTrue(hasattr(element, "text"), msg="no text member")
            self.assertTrue(hasattr(element, "tail"), msg="no tail member")

            check_string(element.tag)
            check_mapping(element.attrib)
            if element.text is not None:
                check_string(element.text)
            if element.tail is not None:
                check_string(element.tail)
            for elem in element:
                check_element(elem)

        element = ET.Element("tag")
        check_element(element)
        tree = ET.ElementTree(element)
        check_element(tree.getroot())
        element = ET.Element("t\xe4g", key="value")
        tree = ET.ElementTree(element)
        self.assertRegex(repr(element), r"^<Element 't\xe4g' at 0x.*>$")
        element = ET.Element("tag", key="value")

        # Make sure all standard element methods exist.

        def check_method(method):
            self.assertTrue(hasattr(method, '__call__'),
                    msg="%s not callable" % method)

        check_method(element.append)
        check_method(element.extend)
        check_method(element.insert)
        check_method(element.remove)
        check_method(element.getchildren)
        check_method(element.find)
        check_method(element.iterfind)
        check_method(element.findall)
        check_method(element.findtext)
        check_method(element.clear)
        check_method(element.get)
        check_method(element.set)
        check_method(element.keys)
        check_method(element.items)
        check_method(element.iter)
        check_method(element.itertext)
        check_method(element.getiterator)

        # These methods return an iterable. See bug 6472.

        def check_iter(it):
            check_method(it.__next__)

        check_iter(element.iterfind("tag"))
        check_iter(element.iterfind("*"))
        check_iter(tree.iterfind("tag"))
        check_iter(tree.iterfind("*"))

        # These aliases are provided:

        self.assertEqual(ET.XML, ET.fromstring)
        self.assertEqual(ET.PI, ET.ProcessingInstruction)

    def test_simpleops(self):
        # Basic method sanity checks.

        elem = ET.XML("<body><tag/></body>")
        self.serialize_check(elem, '<body><tag /></body>')
        e = ET.Element("tag2")
        elem.append(e)
        self.serialize_check(elem, '<body><tag /><tag2 /></body>')
        elem.remove(e)
        self.serialize_check(elem, '<body><tag /></body>')
        elem.insert(0, e)
        self.serialize_check(elem, '<body><tag2 /><tag /></body>')
        elem.remove(e)
        elem.extend([e])
        self.serialize_check(elem, '<body><tag /><tag2 /></body>')
        elem.remove(e)

        element = ET.Element("tag", key="value")
        self.serialize_check(element, '<tag key="value" />') # 1
        subelement = ET.Element("subtag")
        element.append(subelement)
        self.serialize_check(element, '<tag key="value"><subtag /></tag>') # 2
        element.insert(0, subelement)
        self.serialize_check(element,
                '<tag key="value"><subtag /><subtag /></tag>') # 3
        element.remove(subelement)
        self.serialize_check(element, '<tag key="value"><subtag /></tag>') # 4
        element.remove(subelement)
        self.serialize_check(element, '<tag key="value" />') # 5
        with self.assertRaises(ValueError) as cm:
            element.remove(subelement)
        self.assertEqual(str(cm.exception), 'list.remove(x): x not in list')
        self.serialize_check(element, '<tag key="value" />') # 6
        element[0:0] = [subelement, subelement, subelement]
        self.serialize_check(element[1], '<subtag />')
        self.assertEqual(element[1:9], [element[1], element[2]])
        self.assertEqual(element[:9:2], [element[0], element[2]])
        del element[1:2]
        self.serialize_check(element,
                '<tag key="value"><subtag /><subtag /></tag>')

    def test_cdata(self):
        # Test CDATA handling (etc).

        self.serialize_check(ET.XML("<tag>hello</tag>"),
                '<tag>hello</tag>')
        self.serialize_check(ET.XML("<tag>&#104;&#101;&#108;&#108;&#111;</tag>"),
                '<tag>hello</tag>')
        self.serialize_check(ET.XML("<tag><![CDATA[hello]]></tag>"),
                '<tag>hello</tag>')

    def test_file_init(self):
        stringfile = io.BytesIO(SAMPLE_XML.encode("utf-8"))
        tree = ET.ElementTree(file=stringfile)
        self.assertEqual(tree.find("tag").tag, 'tag')
        self.assertEqual(tree.find("section/tag").tag, 'tag')

        tree = ET.ElementTree(file=SIMPLE_XMLFILE)
        self.assertEqual(tree.find("element").tag, 'element')
        self.assertEqual(tree.find("element/../empty-element").tag,
                'empty-element')

    def test_path_cache(self):
        # Check that the path cache behaves sanely.

        from xml.etree import ElementPath

        elem = ET.XML(SAMPLE_XML)
        for i in range(10): ET.ElementTree(elem).find('./'+str(i))
        cache_len_10 = len(ElementPath._cache)
        for i in range(10): ET.ElementTree(elem).find('./'+str(i))
        self.assertEqual(len(ElementPath._cache), cache_len_10)
        for i in range(20): ET.ElementTree(elem).find('./'+str(i))
        self.assertGreater(len(ElementPath._cache), cache_len_10)
        for i in range(600): ET.ElementTree(elem).find('./'+str(i))
        self.assertLess(len(ElementPath._cache), 500)

    def test_copy(self):
        # Test copy handling (etc).

        import copy
        e1 = ET.XML("<tag>hello<foo/></tag>")
        e2 = copy.copy(e1)
        e3 = copy.deepcopy(e1)
        e1.find("foo").tag = "bar"
        self.serialize_check(e1, '<tag>hello<bar /></tag>')
        self.serialize_check(e2, '<tag>hello<bar /></tag>')
        self.serialize_check(e3, '<tag>hello<foo /></tag>')

    def test_attrib(self):
        # Test attribute handling.

        elem = ET.Element("tag")
        elem.get("key") # 1.1
        self.assertEqual(elem.get("key", "default"), 'default') # 1.2

        elem.set("key", "value")
        self.assertEqual(elem.get("key"), 'value') # 1.3

        elem = ET.Element("tag", key="value")
        self.assertEqual(elem.get("key"), 'value') # 2.1
        self.assertEqual(elem.attrib, {'key': 'value'}) # 2.2

        attrib = {"key": "value"}
        elem = ET.Element("tag", attrib)
        attrib.clear() # check for aliasing issues
        self.assertEqual(elem.get("key"), 'value') # 3.1
        self.assertEqual(elem.attrib, {'key': 'value'}) # 3.2

        attrib = {"key": "value"}
        elem = ET.Element("tag", **attrib)
        attrib.clear() # check for aliasing issues
        self.assertEqual(elem.get("key"), 'value') # 4.1
        self.assertEqual(elem.attrib, {'key': 'value'}) # 4.2

        elem = ET.Element("tag", {"key": "other"}, key="value")
        self.assertEqual(elem.get("key"), 'value') # 5.1
        self.assertEqual(elem.attrib, {'key': 'value'}) # 5.2

        elem = ET.Element('test')
        elem.text = "aa"
        elem.set('testa', 'testval')
        elem.set('testb', 'test2')
        self.assertEqual(ET.tostring(elem),
                b'<test testa="testval" testb="test2">aa</test>')
        self.assertEqual(sorted(elem.keys()), ['testa', 'testb'])
        self.assertEqual(sorted(elem.items()),
                [('testa', 'testval'), ('testb', 'test2')])
        self.assertEqual(elem.attrib['testb'], 'test2')
        elem.attrib['testb'] = 'test1'
        elem.attrib['testc'] = 'test2'
        self.assertEqual(ET.tostring(elem),
                b'<test testa="testval" testb="test1" testc="test2">aa</test>')

    def test_makeelement(self):
        # Test makeelement handling.

        elem = ET.Element("tag")
        attrib = {"key": "value"}
        subelem = elem.makeelement("subtag", attrib)
        self.assertIsNot(subelem.attrib, attrib, msg="attrib aliasing")
        elem.append(subelem)
        self.serialize_check(elem, '<tag><subtag key="value" /></tag>')

        elem.clear()
        self.serialize_check(elem, '<tag />')
        elem.append(subelem)
        self.serialize_check(elem, '<tag><subtag key="value" /></tag>')
        elem.extend([subelem, subelem])
        self.serialize_check(elem,
            '<tag><subtag key="value" /><subtag key="value" /><subtag key="value" /></tag>')
        elem[:] = [subelem]
        self.serialize_check(elem, '<tag><subtag key="value" /></tag>')
        elem[:] = tuple([subelem])
        self.serialize_check(elem, '<tag><subtag key="value" /></tag>')

    def test_parsefile(self):
        # Test parsing from file.

        tree = ET.parse(SIMPLE_XMLFILE)
        stream = io.StringIO()
        tree.write(stream, encoding='unicode')
        self.assertEqual(stream.getvalue(),
                '<root>\n'
                '   <element key="value">text</element>\n'
                '   <element>text</element>tail\n'
                '   <empty-element />\n'
                '</root>')
        tree = ET.parse(SIMPLE_NS_XMLFILE)
        stream = io.StringIO()
        tree.write(stream, encoding='unicode')
        self.assertEqual(stream.getvalue(),
                '<ns0:root xmlns:ns0="namespace">\n'
                '   <ns0:element key="value">text</ns0:element>\n'
                '   <ns0:element>text</ns0:element>tail\n'
                '   <ns0:empty-element />\n'
                '</ns0:root>')

        with open(SIMPLE_XMLFILE) as f:
            data = f.read()

        parser = ET.XMLParser()
        self.assertRegex(parser.version, r'^Expat ')
        parser.feed(data)
        self.serialize_check(parser.close(),
                '<root>\n'
                '   <element key="value">text</element>\n'
                '   <element>text</element>tail\n'
                '   <empty-element />\n'
                '</root>')

        target = ET.TreeBuilder()
        parser = ET.XMLParser(target=target)
        parser.feed(data)
        self.serialize_check(parser.close(),
                '<root>\n'
                '   <element key="value">text</element>\n'
                '   <element>text</element>tail\n'
                '   <empty-element />\n'
                '</root>')

    def test_parseliteral(self):
        element = ET.XML("<html><body>text</body></html>")
        self.assertEqual(ET.tostring(element, encoding='unicode'),
                '<html><body>text</body></html>')
        element = ET.fromstring("<html><body>text</body></html>")
        self.assertEqual(ET.tostring(element, encoding='unicode'),
                '<html><body>text</body></html>')
        sequence = ["<html><body>", "text</bo", "dy></html>"]
        element = ET.fromstringlist(sequence)
        self.assertEqual(ET.tostring(element),
                b'<html><body>text</body></html>')
        self.assertEqual(b"".join(ET.tostringlist(element)),
                b'<html><body>text</body></html>')
        self.assertEqual(ET.tostring(element, "ascii"),
                b"<?xml version='1.0' encoding='ascii'?>\n"
                b"<html><body>text</body></html>")
        _, ids = ET.XMLID("<html><body>text</body></html>")
        self.assertEqual(len(ids), 0)
        _, ids = ET.XMLID("<html><body id='body'>text</body></html>")
        self.assertEqual(len(ids), 1)
        self.assertEqual(ids["body"].tag, 'body')

    def test_iterparse(self):
        # Test iterparse interface.

        iterparse = ET.iterparse

        context = iterparse(SIMPLE_XMLFILE)
        action, elem = next(context)
        self.assertEqual((action, elem.tag), ('end', 'element'))
        self.assertEqual([(action, elem.tag) for action, elem in context], [
                ('end', 'element'),
                ('end', 'empty-element'),
                ('end', 'root'),
            ])
        self.assertEqual(context.root.tag, 'root')

        context = iterparse(SIMPLE_NS_XMLFILE)
        self.assertEqual([(action, elem.tag) for action, elem in context], [
                ('end', '{namespace}element'),
                ('end', '{namespace}element'),
                ('end', '{namespace}empty-element'),
                ('end', '{namespace}root'),
            ])

        events = ()
        context = iterparse(SIMPLE_XMLFILE, events)
        self.assertEqual([(action, elem.tag) for action, elem in context], [])

        events = ()
        context = iterparse(SIMPLE_XMLFILE, events=events)
        self.assertEqual([(action, elem.tag) for action, elem in context], [])

        events = ("start", "end")
        context = iterparse(SIMPLE_XMLFILE, events)
        self.assertEqual([(action, elem.tag) for action, elem in context], [
                ('start', 'root'),
                ('start', 'element'),
                ('end', 'element'),
                ('start', 'element'),
                ('end', 'element'),
                ('start', 'empty-element'),
                ('end', 'empty-element'),
                ('end', 'root'),
            ])

        events = ("start", "end", "start-ns", "end-ns")
        context = iterparse(SIMPLE_NS_XMLFILE, events)
        self.assertEqual([(action, elem.tag) if action in ("start", "end")
                                             else (action, elem)
                          for action, elem in context], [
                ('start-ns', ('', 'namespace')),
                ('start', '{namespace}root'),
                ('start', '{namespace}element'),
                ('end', '{namespace}element'),
                ('start', '{namespace}element'),
                ('end', '{namespace}element'),
                ('start', '{namespace}empty-element'),
                ('end', '{namespace}empty-element'),
                ('end', '{namespace}root'),
                ('end-ns', None),
            ])

        events = ('start-ns', 'end-ns')
        context = iterparse(io.StringIO(r"<root xmlns=''/>"), events)
        res = [action for action, elem in context]
        self.assertEqual(res, ['start-ns', 'end-ns'])

        events = ("start", "end", "bogus")
        with self.assertRaises(ValueError) as cm:
            with open(SIMPLE_XMLFILE, "rb") as f:
                iterparse(f, events)
        self.assertEqual(str(cm.exception), "unknown event 'bogus'")

        source = io.BytesIO(
            b"<?xml version='1.0' encoding='iso-8859-1'?>\n"
            b"<body xmlns='http://&#233;ffbot.org/ns'\n"
            b"      xmlns:cl\xe9='http://effbot.org/ns'>text</body>\n")
        events = ("start-ns",)
        context = iterparse(source, events)
        self.assertEqual([(action, elem) for action, elem in context], [
                ('start-ns', ('', 'http://\xe9ffbot.org/ns')),
                ('start-ns', ('cl\xe9', 'http://effbot.org/ns')),
            ])

        source = io.StringIO("<document />junk")
        it = iterparse(source)
        action, elem = next(it)
        self.assertEqual((action, elem.tag), ('end', 'document'))
        with self.assertRaises(ET.ParseError) as cm:
            next(it)
        self.assertEqual(str(cm.exception),
                'junk after document element: line 1, column 12')

    def test_writefile(self):
        elem = ET.Element("tag")
        elem.text = "text"
        self.serialize_check(elem, '<tag>text</tag>')
        ET.SubElement(elem, "subtag").text = "subtext"
        self.serialize_check(elem, '<tag>text<subtag>subtext</subtag></tag>')

        # Test tag suppression
        elem.tag = None
        self.serialize_check(elem, 'text<subtag>subtext</subtag>')
        elem.insert(0, ET.Comment("comment"))
        self.serialize_check(elem,
                'text<!--comment--><subtag>subtext</subtag>')     # assumes 1.3

        elem[0] = ET.PI("key", "value")
        self.serialize_check(elem, 'text<?key value?><subtag>subtext</subtag>')

    def test_custom_builder(self):
        # Test parser w. custom builder.

        with open(SIMPLE_XMLFILE) as f:
            data = f.read()
        class Builder(list):
            def start(self, tag, attrib):
                self.append(("start", tag))
            def end(self, tag):
                self.append(("end", tag))
            def data(self, text):
                pass
        builder = Builder()
        parser = ET.XMLParser(target=builder)
        parser.feed(data)
        self.assertEqual(builder, [
                ('start', 'root'),
                ('start', 'element'),
                ('end', 'element'),
                ('start', 'element'),
                ('end', 'element'),
                ('start', 'empty-element'),
                ('end', 'empty-element'),
                ('end', 'root'),
            ])

        with open(SIMPLE_NS_XMLFILE) as f:
            data = f.read()
        class Builder(list):
            def start(self, tag, attrib):
                self.append(("start", tag))
            def end(self, tag):
                self.append(("end", tag))
            def data(self, text):
                pass
            def pi(self, target, data):
                self.append(("pi", target, data))
            def comment(self, data):
                self.append(("comment", data))
        builder = Builder()
        parser = ET.XMLParser(target=builder)
        parser.feed(data)
        self.assertEqual(builder, [
                ('pi', 'pi', 'data'),
                ('comment', ' comment '),
                ('start', '{namespace}root'),
                ('start', '{namespace}element'),
                ('end', '{namespace}element'),
                ('start', '{namespace}element'),
                ('end', '{namespace}element'),
                ('start', '{namespace}empty-element'),
                ('end', '{namespace}empty-element'),
                ('end', '{namespace}root'),
            ])


    def test_getchildren(self):
        # Test Element.getchildren()

        with open(SIMPLE_XMLFILE, "rb") as f:
            tree = ET.parse(f)
        self.assertEqual([summarize_list(elem.getchildren())
                          for elem in tree.getroot().iter()], [
                ['element', 'element', 'empty-element'],
                [],
                [],
                [],
            ])
        self.assertEqual([summarize_list(elem.getchildren())
                          for elem in tree.getiterator()], [
                ['element', 'element', 'empty-element'],
                [],
                [],
                [],
            ])

        elem = ET.XML(SAMPLE_XML)
        self.assertEqual(len(elem.getchildren()), 3)
        self.assertEqual(len(elem[2].getchildren()), 1)
        self.assertEqual(elem[:], elem.getchildren())
        child1 = elem[0]
        child2 = elem[2]
        del elem[1:2]
        self.assertEqual(len(elem.getchildren()), 2)
        self.assertEqual(child1, elem[0])
        self.assertEqual(child2, elem[1])
        elem[0:2] = [child2, child1]
        self.assertEqual(child2, elem[0])
        self.assertEqual(child1, elem[1])
        self.assertNotEqual(child1, elem[0])
        elem.clear()
        self.assertEqual(elem.getchildren(), [])

    def test_writestring(self):
        elem = ET.XML("<html><body>text</body></html>")
        self.assertEqual(ET.tostring(elem), b'<html><body>text</body></html>')
        elem = ET.fromstring("<html><body>text</body></html>")
        self.assertEqual(ET.tostring(elem), b'<html><body>text</body></html>')

    def test_encoding(self):
        def check(encoding, body=''):
            xml = ("<?xml version='1.0' encoding='%s'?><xml>%s</xml>" %
                   (encoding, body))
            self.assertEqual(ET.XML(xml.encode(encoding)).text, body)
            self.assertEqual(ET.XML(xml).text, body)
        check("ascii", 'a')
        check("us-ascii", 'a')
        check("iso-8859-1", '\xbd')
        check("iso-8859-15", '\u20ac')
        check("cp437", '\u221a')
        check("mac-roman", '\u02da')

        def xml(encoding):
            return "<?xml version='1.0' encoding='%s'?><xml />" % encoding
        def bxml(encoding):
            return xml(encoding).encode(encoding)
        supported_encodings = [
            'ascii', 'utf-8', 'utf-8-sig', 'utf-16', 'utf-16be', 'utf-16le',
            'iso8859-1', 'iso8859-2', 'iso8859-3', 'iso8859-4', 'iso8859-5',
            'iso8859-6', 'iso8859-7', 'iso8859-8', 'iso8859-9', 'iso8859-10',
            'iso8859-13', 'iso8859-14', 'iso8859-15', 'iso8859-16',
            'cp437', 'cp720', 'cp737', 'cp775', 'cp850', 'cp852',
            'cp855', 'cp856', 'cp857', 'cp858', 'cp860', 'cp861', 'cp862',
            'cp863', 'cp865', 'cp866', 'cp869', 'cp874', 'cp1006', 'cp1125',
            'cp1250', 'cp1251', 'cp1252', 'cp1253', 'cp1254', 'cp1255',
            'cp1256', 'cp1257', 'cp1258',
            'mac-cyrillic', 'mac-greek', 'mac-iceland', 'mac-latin2',
            'mac-roman', 'mac-turkish',
            'iso2022-jp', 'iso2022-jp-1', 'iso2022-jp-2', 'iso2022-jp-2004',
            'iso2022-jp-3', 'iso2022-jp-ext',
            'koi8-r', 'koi8-u',
            'hz', 'ptcp154',
        ]
        for encoding in supported_encodings:
            self.assertEqual(ET.tostring(ET.XML(bxml(encoding))), b'<xml />')

        unsupported_ascii_compatible_encodings = [
            'big5', 'big5hkscs',
            'cp932', 'cp949', 'cp950',
            'euc-jp', 'euc-jis-2004', 'euc-jisx0213', 'euc-kr',
            'gb2312', 'gbk', 'gb18030',
            'iso2022-kr', 'johab',
            'shift-jis', 'shift-jis-2004', 'shift-jisx0213',
            'utf-7',
        ]
        for encoding in unsupported_ascii_compatible_encodings:
            self.assertRaises(ValueError, ET.XML, bxml(encoding))

        unsupported_ascii_incompatible_encodings = [
            'cp037', 'cp424', 'cp500', 'cp864', 'cp875', 'cp1026', 'cp1140',
            'utf_32', 'utf_32_be', 'utf_32_le',
        ]
        for encoding in unsupported_ascii_incompatible_encodings:
            self.assertRaises(ET.ParseError, ET.XML, bxml(encoding))

        self.assertRaises(ValueError, ET.XML, xml('undefined').encode('ascii'))
        self.assertRaises(LookupError, ET.XML, xml('xxx').encode('ascii'))

    def test_methods(self):
        # Test serialization methods.

        e = ET.XML("<html><link/><script>1 &lt; 2</script></html>")
        e.tail = "\n"
        self.assertEqual(serialize(e),
                '<html><link /><script>1 &lt; 2</script></html>\n')
        self.assertEqual(serialize(e, method=None),
                '<html><link /><script>1 &lt; 2</script></html>\n')
        self.assertEqual(serialize(e, method="xml"),
                '<html><link /><script>1 &lt; 2</script></html>\n')
        self.assertEqual(serialize(e, method="html"),
                '<html><link><script>1 < 2</script></html>\n')
        self.assertEqual(serialize(e, method="text"), '1 < 2\n')

    def test_issue18347(self):
        e = ET.XML('<html><CamelCase>text</CamelCase></html>')
        self.assertEqual(serialize(e),
                '<html><CamelCase>text</CamelCase></html>')
        self.assertEqual(serialize(e, method="html"),
                '<html><CamelCase>text</CamelCase></html>')

    def test_entity(self):
        # Test entity handling.

        # 1) good entities

        e = ET.XML("<document title='&#x8230;'>test</document>")
        self.assertEqual(serialize(e, encoding="us-ascii"),
                b'<document title="&#33328;">test</document>')
        self.serialize_check(e, '<document title="\u8230">test</document>')

        # 2) bad entities

        with self.assertRaises(ET.ParseError) as cm:
            ET.XML("<document>&entity;</document>")
        self.assertEqual(str(cm.exception),
                'undefined entity: line 1, column 10')

        with self.assertRaises(ET.ParseError) as cm:
            ET.XML(ENTITY_XML)
        self.assertEqual(str(cm.exception),
                'undefined entity &entity;: line 5, column 10')

        # 3) custom entity

        parser = ET.XMLParser()
        parser.entity["entity"] = "text"
        parser.feed(ENTITY_XML)
        root = parser.close()
        self.serialize_check(root, '<document>text</document>')

    def test_namespace(self):
        # Test namespace issues.

        # 1) xml namespace

        elem = ET.XML("<tag xml:lang='en' />")
        self.serialize_check(elem, '<tag xml:lang="en" />') # 1.1

        # 2) other "well-known" namespaces

        elem = ET.XML("<rdf:RDF xmlns:rdf='http://www.w3.org/1999/02/22-rdf-syntax-ns#' />")
        self.serialize_check(elem,
            '<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" />') # 2.1

        elem = ET.XML("<html:html xmlns:html='http://www.w3.org/1999/xhtml' />")
        self.serialize_check(elem,
            '<html:html xmlns:html="http://www.w3.org/1999/xhtml" />') # 2.2

        elem = ET.XML("<soap:Envelope xmlns:soap='http://schemas.xmlsoap.org/soap/envelope' />")
        self.serialize_check(elem,
            '<ns0:Envelope xmlns:ns0="http://schemas.xmlsoap.org/soap/envelope" />') # 2.3

        # 3) unknown namespaces
        elem = ET.XML(SAMPLE_XML_NS)
        self.serialize_check(elem,
            '<ns0:body xmlns:ns0="http://effbot.org/ns">\n'
            '  <ns0:tag>text</ns0:tag>\n'
            '  <ns0:tag />\n'
            '  <ns0:section>\n'
            '    <ns0:tag>subtext</ns0:tag>\n'
            '  </ns0:section>\n'
            '</ns0:body>')

    def test_qname(self):
        # Test QName handling.

        # 1) decorated tags

        elem = ET.Element("{uri}tag")
        self.serialize_check(elem, '<ns0:tag xmlns:ns0="uri" />') # 1.1
        elem = ET.Element(ET.QName("{uri}tag"))
        self.serialize_check(elem, '<ns0:tag xmlns:ns0="uri" />') # 1.2
        elem = ET.Element(ET.QName("uri", "tag"))
        self.serialize_check(elem, '<ns0:tag xmlns:ns0="uri" />') # 1.3
        elem = ET.Element(ET.QName("uri", "tag"))
        subelem = ET.SubElement(elem, ET.QName("uri", "tag1"))
        subelem = ET.SubElement(elem, ET.QName("uri", "tag2"))
        self.serialize_check(elem,
            '<ns0:tag xmlns:ns0="uri"><ns0:tag1 /><ns0:tag2 /></ns0:tag>') # 1.4

        # 2) decorated attributes

        elem.clear()
        elem.attrib["{uri}key"] = "value"
        self.serialize_check(elem,
            '<ns0:tag xmlns:ns0="uri" ns0:key="value" />') # 2.1

        elem.clear()
        elem.attrib[ET.QName("{uri}key")] = "value"
        self.serialize_check(elem,
            '<ns0:tag xmlns:ns0="uri" ns0:key="value" />') # 2.2

        # 3) decorated values are not converted by default, but the
        # QName wrapper can be used for values

        elem.clear()
        elem.attrib["{uri}key"] = "{uri}value"
        self.serialize_check(elem,
            '<ns0:tag xmlns:ns0="uri" ns0:key="{uri}value" />') # 3.1

        elem.clear()
        elem.attrib["{uri}key"] = ET.QName("{uri}value")
        self.serialize_check(elem,
            '<ns0:tag xmlns:ns0="uri" ns0:key="ns0:value" />') # 3.2

        elem.clear()
        subelem = ET.Element("tag")
        subelem.attrib["{uri1}key"] = ET.QName("{uri2}value")
        elem.append(subelem)
        elem.append(subelem)
        self.serialize_check(elem,
            '<ns0:tag xmlns:ns0="uri" xmlns:ns1="uri1" xmlns:ns2="uri2">'
            '<tag ns1:key="ns2:value" />'
            '<tag ns1:key="ns2:value" />'
            '</ns0:tag>') # 3.3

        # 4) Direct QName tests

        self.assertEqual(str(ET.QName('ns', 'tag')), '{ns}tag')
        self.assertEqual(str(ET.QName('{ns}tag')), '{ns}tag')
        q1 = ET.QName('ns', 'tag')
        q2 = ET.QName('ns', 'tag')
        self.assertEqual(q1, q2)
        q2 = ET.QName('ns', 'other-tag')
        self.assertNotEqual(q1, q2)
        self.assertNotEqual(q1, 'ns:tag')
        self.assertEqual(q1, '{ns}tag')

    def test_doctype_public(self):
        # Test PUBLIC doctype.

        elem = ET.XML('<!DOCTYPE html PUBLIC'
                ' "-//W3C//DTD XHTML 1.0 Transitional//EN"'
                ' "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">'
                '<html>text</html>')

    def test_xpath_tokenizer(self):
        # Test the XPath tokenizer.
        from xml.etree import ElementPath
        def check(p, expected):
            self.assertEqual([op or tag
                              for op, tag in ElementPath.xpath_tokenizer(p)],
                             expected)

        # tests from the xml specification
        check("*", ['*'])
        check("text()", ['text', '()'])
        check("@name", ['@', 'name'])
        check("@*", ['@', '*'])
        check("para[1]", ['para', '[', '1', ']'])
        check("para[last()]", ['para', '[', 'last', '()', ']'])
        check("*/para", ['*', '/', 'para'])
        check("/doc/chapter[5]/section[2]",
              ['/', 'doc', '/', 'chapter', '[', '5', ']',
               '/', 'section', '[', '2', ']'])
        check("chapter//para", ['chapter', '//', 'para'])
        check("//para", ['//', 'para'])
        check("//olist/item", ['//', 'olist', '/', 'item'])
        check(".", ['.'])
        check(".//para", ['.', '//', 'para'])
        check("..", ['..'])
        check("../@lang", ['..', '/', '@', 'lang'])
        check("chapter[title]", ['chapter', '[', 'title', ']'])
        check("employee[@secretary and @assistant]", ['employee',
              '[', '@', 'secretary', '', 'and', '', '@', 'assistant', ']'])

        # additional tests
        check("{http://spam}egg", ['{http://spam}egg'])
        check("./spam.egg", ['.', '/', 'spam.egg'])
        check(".//{http://spam}egg", ['.', '//', '{http://spam}egg'])

    def test_processinginstruction(self):
        # Test ProcessingInstruction directly

        self.assertEqual(ET.tostring(ET.ProcessingInstruction('test', 'instruction')),
                b'<?test instruction?>')
        self.assertEqual(ET.tostring(ET.PI('test', 'instruction')),
                b'<?test instruction?>')

        # Issue #2746

        self.assertEqual(ET.tostring(ET.PI('test', '<testing&>')),
                b'<?test <testing&>?>')
        self.assertEqual(ET.tostring(ET.PI('test', '<testing&>\xe3'), 'latin-1'),
                b"<?xml version='1.0' encoding='latin-1'?>\n"
                b"<?test <testing&>\xe3?>")

    def test_html_empty_elems_serialization(self):
        # issue 15970
        # from http://www.w3.org/TR/html401/index/elements.html
        for element in ['AREA', 'BASE', 'BASEFONT', 'BR', 'COL', 'FRAME', 'HR',
                        'IMG', 'INPUT', 'ISINDEX', 'LINK', 'META', 'PARAM']:
            for elem in [element, element.lower()]:
                expected = '<%s>' % elem
                serialized = serialize(ET.XML('<%s />' % elem), method='html')
                self.assertEqual(serialized, expected)
                serialized = serialize(ET.XML('<%s></%s>' % (elem,elem)),
                                       method='html')
                self.assertEqual(serialized, expected)


class XMLPullParserTest(unittest.TestCase):

    def _feed(self, parser, data, chunk_size=None):
        if chunk_size is None:
            parser.feed(data)
        else:
            for i in range(0, len(data), chunk_size):
                parser.feed(data[i:i+chunk_size])

    def assert_event_tags(self, parser, expected):
        events = parser.read_events()
        self.assertEqual([(action, elem.tag) for action, elem in events],
                         expected)

    def test_simple_xml(self):
        for chunk_size in (None, 1, 5):
            with self.subTest(chunk_size=chunk_size):
                parser = ET.XMLPullParser()
                self.assert_event_tags(parser, [])
                self._feed(parser, "<!-- comment -->\n", chunk_size)
                self.assert_event_tags(parser, [])
                self._feed(parser,
                           "<root>\n  <element key='value'>text</element",
                           chunk_size)
                self.assert_event_tags(parser, [])
                self._feed(parser, ">\n", chunk_size)
                self.assert_event_tags(parser, [('end', 'element')])
                self._feed(parser, "<element>text</element>tail\n", chunk_size)
                self._feed(parser, "<empty-element/>\n", chunk_size)
                self.assert_event_tags(parser, [
                    ('end', 'element'),
                    ('end', 'empty-element'),
                    ])
                self._feed(parser, "</root>\n", chunk_size)
                self.assert_event_tags(parser, [('end', 'root')])
                self.assertIsNone(parser.close())

    def test_feed_while_iterating(self):
        parser = ET.XMLPullParser()
        it = parser.read_events()
        self._feed(parser, "<root>\n  <element key='value'>text</element>\n")
        action, elem = next(it)
        self.assertEqual((action, elem.tag), ('end', 'element'))
        self._feed(parser, "</root>\n")
        action, elem = next(it)
        self.assertEqual((action, elem.tag), ('end', 'root'))
        with self.assertRaises(StopIteration):
            next(it)

    def test_simple_xml_with_ns(self):
        parser = ET.XMLPullParser()
        self.assert_event_tags(parser, [])
        self._feed(parser, "<!-- comment -->\n")
        self.assert_event_tags(parser, [])
        self._feed(parser, "<root xmlns='namespace'>\n")
        self.assert_event_tags(parser, [])
        self._feed(parser, "<element key='value'>text</element")
        self.assert_event_tags(parser, [])
        self._feed(parser, ">\n")
        self.assert_event_tags(parser, [('end', '{namespace}element')])
        self._feed(parser, "<element>text</element>tail\n")
        self._feed(parser, "<empty-element/>\n")
        self.assert_event_tags(parser, [
            ('end', '{namespace}element'),
            ('end', '{namespace}empty-element'),
            ])
        self._feed(parser, "</root>\n")
        self.assert_event_tags(parser, [('end', '{namespace}root')])
        self.assertIsNone(parser.close())

    def test_ns_events(self):
        parser = ET.XMLPullParser(events=('start-ns', 'end-ns'))
        self._feed(parser, "<!-- comment -->\n")
        self._feed(parser, "<root xmlns='namespace'>\n")
        self.assertEqual(
            list(parser.read_events()),
            [('start-ns', ('', 'namespace'))])
        self._feed(parser, "<element key='value'>text</element")
        self._feed(parser, ">\n")
        self._feed(parser, "<element>text</element>tail\n")
        self._feed(parser, "<empty-element/>\n")
        self._feed(parser, "</root>\n")
        self.assertEqual(list(parser.read_events()), [('end-ns', None)])
        self.assertIsNone(parser.close())

    def test_events(self):
        parser = ET.XMLPullParser(events=())
        self._feed(parser, "<root/>\n")
        self.assert_event_tags(parser, [])

        parser = ET.XMLPullParser(events=('start', 'end'))
        self._feed(parser, "<!-- comment -->\n")
        self.assert_event_tags(parser, [])
        self._feed(parser, "<root>\n")
        self.assert_event_tags(parser, [('start', 'root')])
        self._feed(parser, "<element key='value'>text</element")
        self.assert_event_tags(parser, [('start', 'element')])
        self._feed(parser, ">\n")
        self.assert_event_tags(parser, [('end', 'element')])
        self._feed(parser,
                   "<element xmlns='foo'>text<empty-element/></element>tail\n")
        self.assert_event_tags(parser, [
            ('start', '{foo}element'),
            ('start', '{foo}empty-element'),
            ('end', '{foo}empty-element'),
            ('end', '{foo}element'),
            ])
        self._feed(parser, "</root>")
        self.assertIsNone(parser.close())
        self.assert_event_tags(parser, [('end', 'root')])

        parser = ET.XMLPullParser(events=('start',))
        self._feed(parser, "<!-- comment -->\n")
        self.assert_event_tags(parser, [])
        self._feed(parser, "<root>\n")
        self.assert_event_tags(parser, [('start', 'root')])
        self._feed(parser, "<element key='value'>text</element")
        self.assert_event_tags(parser, [('start', 'element')])
        self._feed(parser, ">\n")
        self.assert_event_tags(parser, [])
        self._feed(parser,
                   "<element xmlns='foo'>text<empty-element/></element>tail\n")
        self.assert_event_tags(parser, [
            ('start', '{foo}element'),
            ('start', '{foo}empty-element'),
            ])
        self._feed(parser, "</root>")
        self.assertIsNone(parser.close())

    def test_events_sequence(self):
        # Test that events can be some sequence that's not just a tuple or list
        eventset = {'end', 'start'}
        parser = ET.XMLPullParser(events=eventset)
        self._feed(parser, "<foo>bar</foo>")
        self.assert_event_tags(parser, [('start', 'foo'), ('end', 'foo')])

        class DummyIter:
            def __init__(self):
                self.events = iter(['start', 'end', 'start-ns'])
            def __iter__(self):
                return self
            def __next__(self):
                return next(self.events)

        parser = ET.XMLPullParser(events=DummyIter())
        self._feed(parser, "<foo>bar</foo>")
        self.assert_event_tags(parser, [('start', 'foo'), ('end', 'foo')])


    def test_unknown_event(self):
        with self.assertRaises(ValueError):
            ET.XMLPullParser(events=('start', 'end', 'bogus'))


#
# xinclude tests (samples from appendix C of the xinclude specification)

XINCLUDE = {}

XINCLUDE["C1.xml"] = """\
<?xml version='1.0'?>
<document xmlns:xi="http://www.w3.org/2001/XInclude">
  <p>120 Mz is adequate for an average home user.</p>
  <xi:include href="disclaimer.xml"/>
</document>
"""

XINCLUDE["disclaimer.xml"] = """\
<?xml version='1.0'?>
<disclaimer>
  <p>The opinions represented herein represent those of the individual
  and should not be interpreted as official policy endorsed by this
  organization.</p>
</disclaimer>
"""

XINCLUDE["C2.xml"] = """\
<?xml version='1.0'?>
<document xmlns:xi="http://www.w3.org/2001/XInclude">
  <p>This document has been accessed
  <xi:include href="count.txt" parse="text"/> times.</p>
</document>
"""

XINCLUDE["count.txt"] = "324387"

XINCLUDE["C2b.xml"] = """\
<?xml version='1.0'?>
<document xmlns:xi="http://www.w3.org/2001/XInclude">
  <p>This document has been <em>accessed</em>
  <xi:include href="count.txt" parse="text"/> times.</p>
</document>
"""

XINCLUDE["C3.xml"] = """\
<?xml version='1.0'?>
<document xmlns:xi="http://www.w3.org/2001/XInclude">
  <p>The following is the source of the "data.xml" resource:</p>
  <example><xi:include href="data.xml" parse="text"/></example>
</document>
"""

XINCLUDE["data.xml"] = """\
<?xml version='1.0'?>
<data>
  <item><![CDATA[Brooks & Shields]]></item>
</data>
"""

XINCLUDE["C5.xml"] = """\
<?xml version='1.0'?>
<div xmlns:xi="http://www.w3.org/2001/XInclude">
  <xi:include href="example.txt" parse="text">
    <xi:fallback>
      <xi:include href="fallback-example.txt" parse="text">
        <xi:fallback><a href="mailto:bob@example.org">Report error</a></xi:fallback>
      </xi:include>
    </xi:fallback>
  </xi:include>
</div>
"""

XINCLUDE["default.xml"] = """\
<?xml version='1.0'?>
<document xmlns:xi="http://www.w3.org/2001/XInclude">
  <p>Example.</p>
  <xi:include href="{}"/>
</document>
""".format(html.escape(SIMPLE_XMLFILE, True))

#
# badly formatted xi:include tags

XINCLUDE_BAD = {}

XINCLUDE_BAD["B1.xml"] = """\
<?xml version='1.0'?>
<document xmlns:xi="http://www.w3.org/2001/XInclude">
  <p>120 Mz is adequate for an average home user.</p>
  <xi:include href="disclaimer.xml" parse="BAD_TYPE"/>
</document>
"""

XINCLUDE_BAD["B2.xml"] = """\
<?xml version='1.0'?>
<div xmlns:xi="http://www.w3.org/2001/XInclude">
    <xi:fallback></xi:fallback>
</div>
"""

class XIncludeTest(unittest.TestCase):

    def xinclude_loader(self, href, parse="xml", encoding=None):
        try:
            data = XINCLUDE[href]
        except KeyError:
            raise OSError("resource not found")
        if parse == "xml":
            data = ET.XML(data)
        return data

    def none_loader(self, href, parser, encoding=None):
        return None

    def _my_loader(self, href, parse):
        # Used to avoid a test-dependency problem where the default loader
        # of ElementInclude uses the pyET parser for cET tests.
        if parse == 'xml':
            with open(href, 'rb') as f:
                return ET.parse(f).getroot()
        else:
            return None

    def test_xinclude_default(self):
        from xml.etree import ElementInclude
        doc = self.xinclude_loader('default.xml')
        ElementInclude.include(doc, self._my_loader)
        self.assertEqual(serialize(doc),
            '<document>\n'
            '  <p>Example.</p>\n'
            '  <root>\n'
            '   <element key="value">text</element>\n'
            '   <element>text</element>tail\n'
            '   <empty-element />\n'
            '</root>\n'
            '</document>')

    def test_xinclude(self):
        from xml.etree import ElementInclude

        # Basic inclusion example (XInclude C.1)
        document = self.xinclude_loader("C1.xml")
        ElementInclude.include(document, self.xinclude_loader)
        self.assertEqual(serialize(document),
            '<document>\n'
            '  <p>120 Mz is adequate for an average home user.</p>\n'
            '  <disclaimer>\n'
            '  <p>The opinions represented herein represent those of the individual\n'
            '  and should not be interpreted as official policy endorsed by this\n'
            '  organization.</p>\n'
            '</disclaimer>\n'
            '</document>') # C1

        # Textual inclusion example (XInclude C.2)
        document = self.xinclude_loader("C2.xml")
        ElementInclude.include(document, self.xinclude_loader)
        self.assertEqual(serialize(document),
            '<document>\n'
            '  <p>This document has been accessed\n'
            '  324387 times.</p>\n'
            '</document>') # C2

        # Textual inclusion after sibling element (based on modified XInclude C.2)
        document = self.xinclude_loader("C2b.xml")
        ElementInclude.include(document, self.xinclude_loader)
        self.assertEqual(serialize(document),
            '<document>\n'
            '  <p>This document has been <em>accessed</em>\n'
            '  324387 times.</p>\n'
            '</document>') # C2b

        # Textual inclusion of XML example (XInclude C.3)
        document = self.xinclude_loader("C3.xml")
        ElementInclude.include(document, self.xinclude_loader)
        self.assertEqual(serialize(document),
            '<document>\n'
            '  <p>The following is the source of the "data.xml" resource:</p>\n'
            "  <example>&lt;?xml version='1.0'?&gt;\n"
            '&lt;data&gt;\n'
            '  &lt;item&gt;&lt;![CDATA[Brooks &amp; Shields]]&gt;&lt;/item&gt;\n'
            '&lt;/data&gt;\n'
            '</example>\n'
            '</document>') # C3

        # Fallback example (XInclude C.5)
        # Note! Fallback support is not yet implemented
        document = self.xinclude_loader("C5.xml")
        with self.assertRaises(OSError) as cm:
            ElementInclude.include(document, self.xinclude_loader)
        self.assertEqual(str(cm.exception), 'resource not found')
        self.assertEqual(serialize(document),
            '<div xmlns:ns0="http://www.w3.org/2001/XInclude">\n'
            '  <ns0:include href="example.txt" parse="text">\n'
            '    <ns0:fallback>\n'
            '      <ns0:include href="fallback-example.txt" parse="text">\n'
            '        <ns0:fallback><a href="mailto:bob@example.org">Report error</a></ns0:fallback>\n'
            '      </ns0:include>\n'
            '    </ns0:fallback>\n'
            '  </ns0:include>\n'
            '</div>') # C5

    def test_xinclude_failures(self):
        from xml.etree import ElementInclude

        # Test failure to locate included XML file.
        document = ET.XML(XINCLUDE["C1.xml"])
        with self.assertRaises(ElementInclude.FatalIncludeError) as cm:
            ElementInclude.include(document, loader=self.none_loader)
        self.assertEqual(str(cm.exception),
                "cannot load 'disclaimer.xml' as 'xml'")

        # Test failure to locate included text file.
        document = ET.XML(XINCLUDE["C2.xml"])
        with self.assertRaises(ElementInclude.FatalIncludeError) as cm:
            ElementInclude.include(document, loader=self.none_loader)
        self.assertEqual(str(cm.exception),
                "cannot load 'count.txt' as 'text'")

        # Test bad parse type.
        document = ET.XML(XINCLUDE_BAD["B1.xml"])
        with self.assertRaises(ElementInclude.FatalIncludeError) as cm:
            ElementInclude.include(document, loader=self.none_loader)
        self.assertEqual(str(cm.exception),
                "unknown parse type in xi:include tag ('BAD_TYPE')")

        # Test xi:fallback outside xi:include.
        document = ET.XML(XINCLUDE_BAD["B2.xml"])
        with self.assertRaises(ElementInclude.FatalIncludeError) as cm:
            ElementInclude.include(document, loader=self.none_loader)
        self.assertEqual(str(cm.exception),
                "xi:fallback tag must be child of xi:include "
                "('{http://www.w3.org/2001/XInclude}fallback')")

# --------------------------------------------------------------------
# reported bugs

class BugsTest(unittest.TestCase):

    def test_bug_xmltoolkit21(self):
        # marshaller gives obscure errors for non-string values

        def check(elem):
            with self.assertRaises(TypeError) as cm:
                serialize(elem)
            self.assertEqual(str(cm.exception),
                    'cannot serialize 123 (type int)')

        elem = ET.Element(123)
        check(elem) # tag

        elem = ET.Element("elem")
        elem.text = 123
        check(elem) # text

        elem = ET.Element("elem")
        elem.tail = 123
        check(elem) # tail

        elem = ET.Element("elem")
        elem.set(123, "123")
        check(elem) # attribute key

        elem = ET.Element("elem")
        elem.set("123", 123)
        check(elem) # attribute value

    def test_bug_xmltoolkit25(self):
        # typo in ElementTree.findtext

        elem = ET.XML(SAMPLE_XML)
        tree = ET.ElementTree(elem)
        self.assertEqual(tree.findtext("tag"), 'text')
        self.assertEqual(tree.findtext("section/tag"), 'subtext')

    def test_bug_xmltoolkit28(self):
        # .//tag causes exceptions

        tree = ET.XML("<doc><table><tbody/></table></doc>")
        self.assertEqual(summarize_list(tree.findall(".//thead")), [])
        self.assertEqual(summarize_list(tree.findall(".//tbody")), ['tbody'])

    def test_bug_xmltoolkitX1(self):
        # dump() doesn't flush the output buffer

        tree = ET.XML("<doc><table><tbody/></table></doc>")
        with support.captured_stdout() as stdout:
            ET.dump(tree)
            self.assertEqual(stdout.getvalue(), '<doc><table><tbody /></table></doc>\n')

    def test_bug_xmltoolkit39(self):
        # non-ascii element and attribute names doesn't work

        tree = ET.XML(b"<?xml version='1.0' encoding='iso-8859-1'?><t\xe4g />")
        self.assertEqual(ET.tostring(tree, "utf-8"), b'<t\xc3\xa4g />')

        tree = ET.XML(b"<?xml version='1.0' encoding='iso-8859-1'?>"
                      b"<tag \xe4ttr='v&#228;lue' />")
        self.assertEqual(tree.attrib, {'\xe4ttr': 'v\xe4lue'})
        self.assertEqual(ET.tostring(tree, "utf-8"),
                b'<tag \xc3\xa4ttr="v\xc3\xa4lue" />')

        tree = ET.XML(b"<?xml version='1.0' encoding='iso-8859-1'?>"
                      b'<t\xe4g>text</t\xe4g>')
        self.assertEqual(ET.tostring(tree, "utf-8"),
                b'<t\xc3\xa4g>text</t\xc3\xa4g>')

        tree = ET.Element("t\u00e4g")
        self.assertEqual(ET.tostring(tree, "utf-8"), b'<t\xc3\xa4g />')

        tree = ET.Element("tag")
        tree.set("\u00e4ttr", "v\u00e4lue")
        self.assertEqual(ET.tostring(tree, "utf-8"),
                b'<tag \xc3\xa4ttr="v\xc3\xa4lue" />')

    def test_bug_xmltoolkit54(self):
        # problems handling internally defined entities

        e = ET.XML("<!DOCTYPE doc [<!ENTITY ldots '&#x8230;'>]>"
                   '<doc>&ldots;</doc>')
        self.assertEqual(serialize(e, encoding="us-ascii"),
                b'<doc>&#33328;</doc>')
        self.assertEqual(serialize(e), '<doc>\u8230</doc>')

    def test_bug_xmltoolkit55(self):
        # make sure we're reporting the first error, not the last

        with self.assertRaises(ET.ParseError) as cm:
            ET.XML(b"<!DOCTYPE doc SYSTEM 'doc.dtd'>"
                   b'<doc>&ldots;&ndots;&rdots;</doc>')
        self.assertEqual(str(cm.exception),
                'undefined entity &ldots;: line 1, column 36')

    def test_bug_xmltoolkit60(self):
        # Handle crash in stream source.

        class ExceptionFile:
            def read(self, x):
                raise OSError

        self.assertRaises(OSError, ET.parse, ExceptionFile())

    def test_bug_xmltoolkit62(self):
        # Don't crash when using custom entities.

        ENTITIES = {'rsquo': '\u2019', 'lsquo': '\u2018'}
        parser = ET.XMLParser()
        parser.entity.update(ENTITIES)
        parser.feed("""<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE patent-application-publication SYSTEM "pap-v15-2001-01-31.dtd" []>
<patent-application-publication>
<subdoc-abstract>
<paragraph id="A-0001" lvl="0">A new cultivar of Begonia plant named &lsquo;BCT9801BEG&rsquo;.</paragraph>
</subdoc-abstract>
</patent-application-publication>""")
        t = parser.close()
        self.assertEqual(t.find('.//paragraph').text,
            'A new cultivar of Begonia plant named \u2018BCT9801BEG\u2019.')

    def test_bug_xmltoolkit63(self):
        # Check reference leak.
        def xmltoolkit63():
            tree = ET.TreeBuilder()
            tree.start("tag", {})
            tree.data("text")
            tree.end("tag")

        xmltoolkit63()
        count = sys.getrefcount(None)
        for i in range(1000):
            xmltoolkit63()
        self.assertEqual(sys.getrefcount(None), count)

    def test_bug_200708_newline(self):
        # Preserve newlines in attributes.

        e = ET.Element('SomeTag', text="def _f():\n  return 3\n")
        self.assertEqual(ET.tostring(e),
                b'<SomeTag text="def _f():&#10;  return 3&#10;" />')
        self.assertEqual(ET.XML(ET.tostring(e)).get("text"),
                'def _f():\n  return 3\n')
        self.assertEqual(ET.tostring(ET.XML(ET.tostring(e))),
                b'<SomeTag text="def _f():&#10;  return 3&#10;" />')

    def test_bug_200708_close(self):
        # Test default builder.
        parser = ET.XMLParser() # default
        parser.feed("<element>some text</element>")
        self.assertEqual(parser.close().tag, 'element')

        # Test custom builder.
        class EchoTarget:
            def close(self):
                return ET.Element("element") # simulate root
        parser = ET.XMLParser(EchoTarget())
        parser.feed("<element>some text</element>")
        self.assertEqual(parser.close().tag, 'element')

    def test_bug_200709_default_namespace(self):
        e = ET.Element("{default}elem")
        s = ET.SubElement(e, "{default}elem")
        self.assertEqual(serialize(e, default_namespace="default"), # 1
                '<elem xmlns="default"><elem /></elem>')

        e = ET.Element("{default}elem")
        s = ET.SubElement(e, "{default}elem")
        s = ET.SubElement(e, "{not-default}elem")
        self.assertEqual(serialize(e, default_namespace="default"), # 2
            '<elem xmlns="default" xmlns:ns1="not-default">'
            '<elem />'
            '<ns1:elem />'
            '</elem>')

        e = ET.Element("{default}elem")
        s = ET.SubElement(e, "{default}elem")
        s = ET.SubElement(e, "elem") # unprefixed name
        with self.assertRaises(ValueError) as cm:
            serialize(e, default_namespace="default") # 3
        self.assertEqual(str(cm.exception),
                'cannot use non-qualified names with default_namespace option')

    def test_bug_200709_register_namespace(self):
        e = ET.Element("{http://namespace.invalid/does/not/exist/}title")
        self.assertEqual(ET.tostring(e),
            b'<ns0:title xmlns:ns0="http://namespace.invalid/does/not/exist/" />')
        ET.register_namespace("foo", "http://namespace.invalid/does/not/exist/")
        e = ET.Element("{http://namespace.invalid/does/not/exist/}title")
        self.assertEqual(ET.tostring(e),
            b'<foo:title xmlns:foo="http://namespace.invalid/does/not/exist/" />')

        # And the Dublin Core namespace is in the default list:

        e = ET.Element("{http://purl.org/dc/elements/1.1/}title")
        self.assertEqual(ET.tostring(e),
            b'<dc:title xmlns:dc="http://purl.org/dc/elements/1.1/" />')

    def test_bug_200709_element_comment(self):
        # Not sure if this can be fixed, really (since the serializer needs
        # ET.Comment, not cET.comment).

        a = ET.Element('a')
        a.append(ET.Comment('foo'))
        self.assertEqual(a[0].tag, ET.Comment)

        a = ET.Element('a')
        a.append(ET.PI('foo'))
        self.assertEqual(a[0].tag, ET.PI)

    def test_bug_200709_element_insert(self):
        a = ET.Element('a')
        b = ET.SubElement(a, 'b')
        c = ET.SubElement(a, 'c')
        d = ET.Element('d')
        a.insert(0, d)
        self.assertEqual(summarize_list(a), ['d', 'b', 'c'])
        a.insert(-1, d)
        self.assertEqual(summarize_list(a), ['d', 'b', 'd', 'c'])

    def test_bug_200709_iter_comment(self):
        a = ET.Element('a')
        b = ET.SubElement(a, 'b')
        comment_b = ET.Comment("TEST-b")
        b.append(comment_b)
        self.assertEqual(summarize_list(a.iter(ET.Comment)), [ET.Comment])

    # --------------------------------------------------------------------
    # reported on bugs.python.org

    def test_bug_1534630(self):
        bob = ET.TreeBuilder()
        e = bob.data("data")
        e = bob.start("tag", {})
        e = bob.end("tag")
        e = bob.close()
        self.assertEqual(serialize(e), '<tag />')

    def test_issue6233(self):
        e = ET.XML(b"<?xml version='1.0' encoding='utf-8'?>"
                   b'<body>t\xc3\xa3g</body>')
        self.assertEqual(ET.tostring(e, 'ascii'),
                b"<?xml version='1.0' encoding='ascii'?>\n"
                b'<body>t&#227;g</body>')
        e = ET.XML(b"<?xml version='1.0' encoding='iso-8859-1'?>"
                   b'<body>t\xe3g</body>')
        self.assertEqual(ET.tostring(e, 'ascii'),
                b"<?xml version='1.0' encoding='ascii'?>\n"
                b'<body>t&#227;g</body>')

    def test_issue3151(self):
        e = ET.XML('<prefix:localname xmlns:prefix="${stuff}"/>')
        self.assertEqual(e.tag, '{${stuff}}localname')
        t = ET.ElementTree(e)
        self.assertEqual(ET.tostring(e), b'<ns0:localname xmlns:ns0="${stuff}" />')

    def test_issue6565(self):
        elem = ET.XML("<body><tag/></body>")
        self.assertEqual(summarize_list(elem), ['tag'])
        newelem = ET.XML(SAMPLE_XML)
        elem[:] = newelem[:]
        self.assertEqual(summarize_list(elem), ['tag', 'tag', 'section'])

    def test_issue10777(self):
        # Registering a namespace twice caused a "dictionary changed size during
        # iteration" bug.

        ET.register_namespace('test10777', 'http://myuri/')
        ET.register_namespace('test10777', 'http://myuri/')


# --------------------------------------------------------------------


class BasicElementTest(ElementTestCase, unittest.TestCase):
    def test_augmentation_type_errors(self):
        e = ET.Element('joe')
        self.assertRaises(TypeError, e.append, 'b')
        self.assertRaises(TypeError, e.extend, [ET.Element('bar'), 'foo'])
        self.assertRaises(TypeError, e.insert, 0, 'foo')

    def test_cyclic_gc(self):
        class Dummy:
            pass

        # Test the shortest cycle: d->element->d
        d = Dummy()
        d.dummyref = ET.Element('joe', attr=d)
        wref = weakref.ref(d)
        del d
        gc_collect()
        self.assertIsNone(wref())

        # A longer cycle: d->e->e2->d
        e = ET.Element('joe')
        d = Dummy()
        d.dummyref = e
        wref = weakref.ref(d)
        e2 = ET.SubElement(e, 'foo', attr=d)
        del d, e, e2
        gc_collect()
        self.assertIsNone(wref())

        # A cycle between Element objects as children of one another
        # e1->e2->e3->e1
        e1 = ET.Element('e1')
        e2 = ET.Element('e2')
        e3 = ET.Element('e3')
        e1.append(e2)
        e2.append(e2)
        e3.append(e1)
        wref = weakref.ref(e1)
        del e1, e2, e3
        gc_collect()
        self.assertIsNone(wref())

    def test_weakref(self):
        flag = False
        def wref_cb(w):
            nonlocal flag
            flag = True
        e = ET.Element('e')
        wref = weakref.ref(e, wref_cb)
        self.assertEqual(wref().tag, 'e')
        del e
        self.assertEqual(flag, True)
        self.assertEqual(wref(), None)

    def test_get_keyword_args(self):
        e1 = ET.Element('foo' , x=1, y=2, z=3)
        self.assertEqual(e1.get('x', default=7), 1)
        self.assertEqual(e1.get('w', default=7), 7)

    def test_pickle(self):
        # issue #16076: the C implementation wasn't pickleable.
        for dumper, loader in product(self.modules, repeat=2):
            e = dumper.Element('foo', bar=42)
            e.text = "text goes here"
            e.tail = "opposite of head"
            dumper.SubElement(e, 'child').append(dumper.Element('grandchild'))
            e.append(dumper.Element('child'))
            e.findall('.//grandchild')[0].set('attr', 'other value')

            e2 = self.pickleRoundTrip(e, 'xml.etree.ElementTree',
                                      dumper, loader)

            self.assertEqual(e2.tag, 'foo')
            self.assertEqual(e2.attrib['bar'], 42)
            self.assertEqual(len(e2), 2)
            self.assertEqualElements(e, e2)

    def test_pickle_issue18997(self):
        for dumper, loader in product(self.modules, repeat=2):
            XMLTEXT = """<?xml version="1.0"?>
                <group><dogs>4</dogs>
                </group>"""
            e1 = dumper.fromstring(XMLTEXT)
            if hasattr(e1, '__getstate__'):
                self.assertEqual(e1.__getstate__()['tag'], 'group')
            e2 = self.pickleRoundTrip(e1, 'xml.etree.ElementTree', dumper, loader)
            self.assertEqual(e2.tag, 'group')
            self.assertEqual(e2[0].tag, 'dogs')


class ElementTreeTypeTest(unittest.TestCase):
    def test_istype(self):
        self.assertIsInstance(ET.ParseError, type)
        self.assertIsInstance(ET.QName, type)
        self.assertIsInstance(ET.ElementTree, type)
        self.assertIsInstance(ET.Element, type)
        self.assertIsInstance(ET.TreeBuilder, type)
        self.assertIsInstance(ET.XMLParser, type)

    def test_Element_subclass_trivial(self):
        class MyElement(ET.Element):
            pass

        mye = MyElement('foo')
        self.assertIsInstance(mye, ET.Element)
        self.assertIsInstance(mye, MyElement)
        self.assertEqual(mye.tag, 'foo')

        # test that attribute assignment works (issue 14849)
        mye.text = "joe"
        self.assertEqual(mye.text, "joe")

    def test_Element_subclass_constructor(self):
        class MyElement(ET.Element):
            def __init__(self, tag, attrib={}, **extra):
                super(MyElement, self).__init__(tag + '__', attrib, **extra)

        mye = MyElement('foo', {'a': 1, 'b': 2}, c=3, d=4)
        self.assertEqual(mye.tag, 'foo__')
        self.assertEqual(sorted(mye.items()),
            [('a', 1), ('b', 2), ('c', 3), ('d', 4)])

    def test_Element_subclass_new_method(self):
        class MyElement(ET.Element):
            def newmethod(self):
                return self.tag

        mye = MyElement('joe')
        self.assertEqual(mye.newmethod(), 'joe')


class ElementFindTest(unittest.TestCase):
    def test_find_simple(self):
        e = ET.XML(SAMPLE_XML)
        self.assertEqual(e.find('tag').tag, 'tag')
        self.assertEqual(e.find('section/tag').tag, 'tag')
        self.assertEqual(e.find('./tag').tag, 'tag')

        e[2] = ET.XML(SAMPLE_SECTION)
        self.assertEqual(e.find('section/nexttag').tag, 'nexttag')

        self.assertEqual(e.findtext('./tag'), 'text')
        self.assertEqual(e.findtext('section/tag'), 'subtext')

        # section/nexttag is found but has no text
        self.assertEqual(e.findtext('section/nexttag'), '')
        self.assertEqual(e.findtext('section/nexttag', 'default'), '')

        # tog doesn't exist and 'default' kicks in
        self.assertIsNone(e.findtext('tog'))
        self.assertEqual(e.findtext('tog', 'default'), 'default')

        # Issue #16922
        self.assertEqual(ET.XML('<tag><empty /></tag>').findtext('empty'), '')

    def test_find_xpath(self):
        LINEAR_XML = '''
        <body>
            <tag class='a'/>
            <tag class='b'/>
            <tag class='c'/>
            <tag class='d'/>
        </body>'''
        e = ET.XML(LINEAR_XML)

        # Test for numeric indexing and last()
        self.assertEqual(e.find('./tag[1]').attrib['class'], 'a')
        self.assertEqual(e.find('./tag[2]').attrib['class'], 'b')
        self.assertEqual(e.find('./tag[last()]').attrib['class'], 'd')
        self.assertEqual(e.find('./tag[last()-1]').attrib['class'], 'c')
        self.assertEqual(e.find('./tag[last()-2]').attrib['class'], 'b')

        self.assertRaisesRegex(SyntaxError, 'XPath', e.find, './tag[0]')
        self.assertRaisesRegex(SyntaxError, 'XPath', e.find, './tag[-1]')
        self.assertRaisesRegex(SyntaxError, 'XPath', e.find, './tag[last()-0]')
        self.assertRaisesRegex(SyntaxError, 'XPath', e.find, './tag[last()+1]')

    def test_findall(self):
        e = ET.XML(SAMPLE_XML)
        e[2] = ET.XML(SAMPLE_SECTION)
        self.assertEqual(summarize_list(e.findall('.')), ['body'])
        self.assertEqual(summarize_list(e.findall('tag')), ['tag', 'tag'])
        self.assertEqual(summarize_list(e.findall('tog')), [])
        self.assertEqual(summarize_list(e.findall('tog/foo')), [])
        self.assertEqual(summarize_list(e.findall('*')),
            ['tag', 'tag', 'section'])
        self.assertEqual(summarize_list(e.findall('.//tag')),
            ['tag'] * 4)
        self.assertEqual(summarize_list(e.findall('section/tag')), ['tag'])
        self.assertEqual(summarize_list(e.findall('section//tag')), ['tag'] * 2)
        self.assertEqual(summarize_list(e.findall('section/*')),
            ['tag', 'nexttag', 'nextsection'])
        self.assertEqual(summarize_list(e.findall('section//*')),
            ['tag', 'nexttag', 'nextsection', 'tag'])
        self.assertEqual(summarize_list(e.findall('section/.//*')),
            ['tag', 'nexttag', 'nextsection', 'tag'])
        self.assertEqual(summarize_list(e.findall('*/*')),
            ['tag', 'nexttag', 'nextsection'])
        self.assertEqual(summarize_list(e.findall('*//*')),
            ['tag', 'nexttag', 'nextsection', 'tag'])
        self.assertEqual(summarize_list(e.findall('*/tag')), ['tag'])
        self.assertEqual(summarize_list(e.findall('*/./tag')), ['tag'])
        self.assertEqual(summarize_list(e.findall('./tag')), ['tag'] * 2)
        self.assertEqual(summarize_list(e.findall('././tag')), ['tag'] * 2)

        self.assertEqual(summarize_list(e.findall('.//tag[@class]')),
            ['tag'] * 3)
        self.assertEqual(summarize_list(e.findall('.//tag[@class="a"]')),
            ['tag'])
        self.assertEqual(summarize_list(e.findall('.//tag[@class="b"]')),
            ['tag'] * 2)
        self.assertEqual(summarize_list(e.findall('.//tag[@id]')),
            ['tag'])
        self.assertEqual(summarize_list(e.findall('.//section[tag]')),
            ['section'])
        self.assertEqual(summarize_list(e.findall('.//section[element]')), [])
        self.assertEqual(summarize_list(e.findall('../tag')), [])
        self.assertEqual(summarize_list(e.findall('section/../tag')),
            ['tag'] * 2)
        self.assertEqual(e.findall('section//'), e.findall('section//*'))

    def test_test_find_with_ns(self):
        e = ET.XML(SAMPLE_XML_NS)
        self.assertEqual(summarize_list(e.findall('tag')), [])
        self.assertEqual(
            summarize_list(e.findall("{http://effbot.org/ns}tag")),
            ['{http://effbot.org/ns}tag'] * 2)
        self.assertEqual(
            summarize_list(e.findall(".//{http://effbot.org/ns}tag")),
            ['{http://effbot.org/ns}tag'] * 3)

    def test_findall_different_nsmaps(self):
        root = ET.XML('''
            <a xmlns:x="X" xmlns:y="Y">
                <x:b><c/></x:b>
                <b/>
                <c><x:b/><b/></c><y:b/>
            </a>''')
        nsmap = {'xx': 'X'}
        self.assertEqual(len(root.findall(".//xx:b", namespaces=nsmap)), 2)
        self.assertEqual(len(root.findall(".//b", namespaces=nsmap)), 2)
        nsmap = {'xx': 'Y'}
        self.assertEqual(len(root.findall(".//xx:b", namespaces=nsmap)), 1)
        self.assertEqual(len(root.findall(".//b", namespaces=nsmap)), 2)

    def test_bad_find(self):
        e = ET.XML(SAMPLE_XML)
        with self.assertRaisesRegex(SyntaxError, 'cannot use absolute path'):
            e.findall('/tag')

    def test_find_through_ElementTree(self):
        e = ET.XML(SAMPLE_XML)
        self.assertEqual(ET.ElementTree(e).find('tag').tag, 'tag')
        self.assertEqual(ET.ElementTree(e).findtext('tag'), 'text')
        self.assertEqual(summarize_list(ET.ElementTree(e).findall('tag')),
            ['tag'] * 2)
        # this produces a warning
        self.assertEqual(summarize_list(ET.ElementTree(e).findall('//tag')),
            ['tag'] * 3)


class ElementIterTest(unittest.TestCase):
    def _ilist(self, elem, tag=None):
        return summarize_list(elem.iter(tag))

    def test_basic(self):
        doc = ET.XML("<html><body>this is a <i>paragraph</i>.</body>..</html>")
        self.assertEqual(self._ilist(doc), ['html', 'body', 'i'])
        self.assertEqual(self._ilist(doc.find('body')), ['body', 'i'])
        self.assertEqual(next(doc.iter()).tag, 'html')
        self.assertEqual(''.join(doc.itertext()), 'this is a paragraph...')
        self.assertEqual(''.join(doc.find('body').itertext()),
            'this is a paragraph.')
        self.assertEqual(next(doc.itertext()), 'this is a ')

        # iterparse should return an iterator
        sourcefile = serialize(doc, to_string=False)
        self.assertEqual(next(ET.iterparse(sourcefile))[0], 'end')

        # With an explitit parser too (issue #9708)
        sourcefile = serialize(doc, to_string=False)
        parser = ET.XMLParser(target=ET.TreeBuilder())
        self.assertEqual(next(ET.iterparse(sourcefile, parser=parser))[0],
                         'end')

        tree = ET.ElementTree(None)
        self.assertRaises(AttributeError, tree.iter)

        # Issue #16913
        doc = ET.XML("<root>a&amp;<sub>b&amp;</sub>c&amp;</root>")
        self.assertEqual(''.join(doc.itertext()), 'a&b&c&')

    def test_corners(self):
        # single root, no subelements
        a = ET.Element('a')
        self.assertEqual(self._ilist(a), ['a'])

        # one child
        b = ET.SubElement(a, 'b')
        self.assertEqual(self._ilist(a), ['a', 'b'])

        # one child and one grandchild
        c = ET.SubElement(b, 'c')
        self.assertEqual(self._ilist(a), ['a', 'b', 'c'])

        # two children, only first with grandchild
        d = ET.SubElement(a, 'd')
        self.assertEqual(self._ilist(a), ['a', 'b', 'c', 'd'])

        # replace first child by second
        a[0] = a[1]
        del a[1]
        self.assertEqual(self._ilist(a), ['a', 'd'])

    def test_iter_by_tag(self):
        doc = ET.XML('''
            <document>
                <house>
                    <room>bedroom1</room>
                    <room>bedroom2</room>
                </house>
                <shed>nothing here
                </shed>
                <house>
                    <room>bedroom8</room>
                </house>
            </document>''')

        self.assertEqual(self._ilist(doc, 'room'), ['room'] * 3)
        self.assertEqual(self._ilist(doc, 'house'), ['house'] * 2)

        # test that iter also accepts 'tag' as a keyword arg
        self.assertEqual(
            summarize_list(doc.iter(tag='room')),
            ['room'] * 3)

        # make sure both tag=None and tag='*' return all tags
        all_tags = ['document', 'house', 'room', 'room',
                    'shed', 'house', 'room']
        self.assertEqual(self._ilist(doc), all_tags)
        self.assertEqual(self._ilist(doc, '*'), all_tags)


class TreeBuilderTest(unittest.TestCase):
    sample1 = ('<!DOCTYPE html PUBLIC'
        ' "-//W3C//DTD XHTML 1.0 Transitional//EN"'
        ' "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">'
        '<html>text<div>subtext</div>tail</html>')

    sample2 = '''<toplevel>sometext</toplevel>'''

    def _check_sample1_element(self, e):
        self.assertEqual(e.tag, 'html')
        self.assertEqual(e.text, 'text')
        self.assertEqual(e.tail, None)
        self.assertEqual(e.attrib, {})
        children = list(e)
        self.assertEqual(len(children), 1)
        child = children[0]
        self.assertEqual(child.tag, 'div')
        self.assertEqual(child.text, 'subtext')
        self.assertEqual(child.tail, 'tail')
        self.assertEqual(child.attrib, {})

    def test_dummy_builder(self):
        class BaseDummyBuilder:
            def close(self):
                return 42

        class DummyBuilder(BaseDummyBuilder):
            data = start = end = lambda *a: None

        parser = ET.XMLParser(target=DummyBuilder())
        parser.feed(self.sample1)
        self.assertEqual(parser.close(), 42)

        parser = ET.XMLParser(target=BaseDummyBuilder())
        parser.feed(self.sample1)
        self.assertEqual(parser.close(), 42)

        parser = ET.XMLParser(target=object())
        parser.feed(self.sample1)
        self.assertIsNone(parser.close())

    def test_treebuilder_elementfactory_none(self):
        parser = ET.XMLParser(target=ET.TreeBuilder(element_factory=None))
        parser.feed(self.sample1)
        e = parser.close()
        self._check_sample1_element(e)

    def test_subclass(self):
        class MyTreeBuilder(ET.TreeBuilder):
            def foobar(self, x):
                return x * 2

        tb = MyTreeBuilder()
        self.assertEqual(tb.foobar(10), 20)

        parser = ET.XMLParser(target=tb)
        parser.feed(self.sample1)

        e = parser.close()
        self._check_sample1_element(e)

    def test_element_factory(self):
        lst = []
        def myfactory(tag, attrib):
            nonlocal lst
            lst.append(tag)
            return ET.Element(tag, attrib)

        tb = ET.TreeBuilder(element_factory=myfactory)
        parser = ET.XMLParser(target=tb)
        parser.feed(self.sample2)
        parser.close()

        self.assertEqual(lst, ['toplevel'])

    def _check_element_factory_class(self, cls):
        tb = ET.TreeBuilder(element_factory=cls)

        parser = ET.XMLParser(target=tb)
        parser.feed(self.sample1)
        e = parser.close()
        self.assertIsInstance(e, cls)
        self._check_sample1_element(e)

    def test_element_factory_subclass(self):
        class MyElement(ET.Element):
            pass
        self._check_element_factory_class(MyElement)

    def test_element_factory_pure_python_subclass(self):
        # Mimick SimpleTAL's behaviour (issue #16089): both versions of
        # TreeBuilder should be able to cope with a subclass of the
        # pure Python Element class.
        base = ET._Element_Py
        # Not from a C extension
        self.assertEqual(base.__module__, 'xml.etree.ElementTree')
        # Force some multiple inheritance with a C class to make things
        # more interesting.
        class MyElement(base, ValueError):
            pass
        self._check_element_factory_class(MyElement)

    def test_doctype(self):
        class DoctypeParser:
            _doctype = None

            def doctype(self, name, pubid, system):
                self._doctype = (name, pubid, system)

            def close(self):
                return self._doctype

        parser = ET.XMLParser(target=DoctypeParser())
        parser.feed(self.sample1)

        self.assertEqual(parser.close(),
            ('html', '-//W3C//DTD XHTML 1.0 Transitional//EN',
             'http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd'))


class XMLParserTest(unittest.TestCase):
    sample1 = b'<file><line>22</line></file>'
    sample2 = (b'<!DOCTYPE html PUBLIC'
        b' "-//W3C//DTD XHTML 1.0 Transitional//EN"'
        b' "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">'
        b'<html>text</html>')
    sample3 = ('<?xml version="1.0" encoding="iso-8859-1"?>\n'
        '<money value="$\xa3\u20ac\U0001017b">$\xa3\u20ac\U0001017b</money>')

    def _check_sample_element(self, e):
        self.assertEqual(e.tag, 'file')
        self.assertEqual(e[0].tag, 'line')
        self.assertEqual(e[0].text, '22')

    def test_constructor_args(self):
        # Positional args. The first (html) is not supported, but should be
        # nevertheless correctly accepted.
        parser = ET.XMLParser(None, ET.TreeBuilder(), 'utf-8')
        parser.feed(self.sample1)
        self._check_sample_element(parser.close())

        # Now as keyword args.
        parser2 = ET.XMLParser(encoding='utf-8',
                               html=[{}],
                               target=ET.TreeBuilder())
        parser2.feed(self.sample1)
        self._check_sample_element(parser2.close())

    def test_subclass(self):
        class MyParser(ET.XMLParser):
            pass
        parser = MyParser()
        parser.feed(self.sample1)
        self._check_sample_element(parser.close())

    def test_subclass_doctype(self):
        _doctype = None
        class MyParserWithDoctype(ET.XMLParser):
            def doctype(self, name, pubid, system):
                nonlocal _doctype
                _doctype = (name, pubid, system)

        parser = MyParserWithDoctype()
        with self.assertWarns(DeprecationWarning):
            parser.feed(self.sample2)
        parser.close()
        self.assertEqual(_doctype,
            ('html', '-//W3C//DTD XHTML 1.0 Transitional//EN',
             'http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd'))

    def test_parse_string(self):
        parser = ET.XMLParser(target=ET.TreeBuilder())
        parser.feed(self.sample3)
        e = parser.close()
        self.assertEqual(e.tag, 'money')
        self.assertEqual(e.attrib['value'], '$\xa3\u20ac\U0001017b')
        self.assertEqual(e.text, '$\xa3\u20ac\U0001017b')


class NamespaceParseTest(unittest.TestCase):
    def test_find_with_namespace(self):
        nsmap = {'h': 'hello', 'f': 'foo'}
        doc = ET.fromstring(SAMPLE_XML_NS_ELEMS)

        self.assertEqual(len(doc.findall('{hello}table', nsmap)), 1)
        self.assertEqual(len(doc.findall('.//{hello}td', nsmap)), 2)
        self.assertEqual(len(doc.findall('.//{foo}name', nsmap)), 1)


class ElementSlicingTest(unittest.TestCase):
    def _elem_tags(self, elemlist):
        return [e.tag for e in elemlist]

    def _subelem_tags(self, elem):
        return self._elem_tags(list(elem))

    def _make_elem_with_children(self, numchildren):
        """Create an Element with a tag 'a', with the given amount of children
           named 'a0', 'a1' ... and so on.

        """
        e = ET.Element('a')
        for i in range(numchildren):
            ET.SubElement(e, 'a%s' % i)
        return e

    def test_getslice_single_index(self):
        e = self._make_elem_with_children(10)

        self.assertEqual(e[1].tag, 'a1')
        self.assertEqual(e[-2].tag, 'a8')

        self.assertRaises(IndexError, lambda: e[12])

    def test_getslice_range(self):
        e = self._make_elem_with_children(6)

        self.assertEqual(self._elem_tags(e[3:]), ['a3', 'a4', 'a5'])
        self.assertEqual(self._elem_tags(e[3:6]), ['a3', 'a4', 'a5'])
        self.assertEqual(self._elem_tags(e[3:16]), ['a3', 'a4', 'a5'])
        self.assertEqual(self._elem_tags(e[3:5]), ['a3', 'a4'])
        self.assertEqual(self._elem_tags(e[3:-1]), ['a3', 'a4'])
        self.assertEqual(self._elem_tags(e[:2]), ['a0', 'a1'])

    def test_getslice_steps(self):
        e = self._make_elem_with_children(10)

        self.assertEqual(self._elem_tags(e[8:10:1]), ['a8', 'a9'])
        self.assertEqual(self._elem_tags(e[::3]), ['a0', 'a3', 'a6', 'a9'])
        self.assertEqual(self._elem_tags(e[::8]), ['a0', 'a8'])
        self.assertEqual(self._elem_tags(e[1::8]), ['a1', 'a9'])

    def test_getslice_negative_steps(self):
        e = self._make_elem_with_children(4)

        self.assertEqual(self._elem_tags(e[::-1]), ['a3', 'a2', 'a1', 'a0'])
        self.assertEqual(self._elem_tags(e[::-2]), ['a3', 'a1'])

    def test_delslice(self):
        e = self._make_elem_with_children(4)
        del e[0:2]
        self.assertEqual(self._subelem_tags(e), ['a2', 'a3'])

        e = self._make_elem_with_children(4)
        del e[0:]
        self.assertEqual(self._subelem_tags(e), [])

        e = self._make_elem_with_children(4)
        del e[::-1]
        self.assertEqual(self._subelem_tags(e), [])

        e = self._make_elem_with_children(4)
        del e[::-2]
        self.assertEqual(self._subelem_tags(e), ['a0', 'a2'])

        e = self._make_elem_with_children(4)
        del e[1::2]
        self.assertEqual(self._subelem_tags(e), ['a0', 'a2'])

        e = self._make_elem_with_children(2)
        del e[::2]
        self.assertEqual(self._subelem_tags(e), ['a1'])


class IOTest(unittest.TestCase):
    def tearDown(self):
        support.unlink(TESTFN)

    def test_encoding(self):
        # Test encoding issues.
        elem = ET.Element("tag")
        elem.text = "abc"
        self.assertEqual(serialize(elem), '<tag>abc</tag>')
        self.assertEqual(serialize(elem, encoding="utf-8"),
                b'<tag>abc</tag>')
        self.assertEqual(serialize(elem, encoding="us-ascii"),
                b'<tag>abc</tag>')
        for enc in ("iso-8859-1", "utf-16", "utf-32"):
            self.assertEqual(serialize(elem, encoding=enc),
                    ("<?xml version='1.0' encoding='%s'?>\n"
                     "<tag>abc</tag>" % enc).encode(enc))

        elem = ET.Element("tag")
        elem.text = "<&\"\'>"
        self.assertEqual(serialize(elem), '<tag>&lt;&amp;"\'&gt;</tag>')
        self.assertEqual(serialize(elem, encoding="utf-8"),
                b'<tag>&lt;&amp;"\'&gt;</tag>')
        self.assertEqual(serialize(elem, encoding="us-ascii"),
                b'<tag>&lt;&amp;"\'&gt;</tag>')
        for enc in ("iso-8859-1", "utf-16", "utf-32"):
            self.assertEqual(serialize(elem, encoding=enc),
                    ("<?xml version='1.0' encoding='%s'?>\n"
                     "<tag>&lt;&amp;\"'&gt;</tag>" % enc).encode(enc))

        elem = ET.Element("tag")
        elem.attrib["key"] = "<&\"\'>"
        self.assertEqual(serialize(elem), '<tag key="&lt;&amp;&quot;\'&gt;" />')
        self.assertEqual(serialize(elem, encoding="utf-8"),
                b'<tag key="&lt;&amp;&quot;\'&gt;" />')
        self.assertEqual(serialize(elem, encoding="us-ascii"),
                b'<tag key="&lt;&amp;&quot;\'&gt;" />')
        for enc in ("iso-8859-1", "utf-16", "utf-32"):
            self.assertEqual(serialize(elem, encoding=enc),
                    ("<?xml version='1.0' encoding='%s'?>\n"
                     "<tag key=\"&lt;&amp;&quot;'&gt;\" />" % enc).encode(enc))

        elem = ET.Element("tag")
        elem.text = '\xe5\xf6\xf6<>'
        self.assertEqual(serialize(elem), '<tag>\xe5\xf6\xf6&lt;&gt;</tag>')
        self.assertEqual(serialize(elem, encoding="utf-8"),
                b'<tag>\xc3\xa5\xc3\xb6\xc3\xb6&lt;&gt;</tag>')
        self.assertEqual(serialize(elem, encoding="us-ascii"),
                b'<tag>&#229;&#246;&#246;&lt;&gt;</tag>')
        for enc in ("iso-8859-1", "utf-16", "utf-32"):
            self.assertEqual(serialize(elem, encoding=enc),
                    ("<?xml version='1.0' encoding='%s'?>\n"
                     "<tag>åöö&lt;&gt;</tag>" % enc).encode(enc))

        elem = ET.Element("tag")
        elem.attrib["key"] = '\xe5\xf6\xf6<>'
        self.assertEqual(serialize(elem), '<tag key="\xe5\xf6\xf6&lt;&gt;" />')
        self.assertEqual(serialize(elem, encoding="utf-8"),
                b'<tag key="\xc3\xa5\xc3\xb6\xc3\xb6&lt;&gt;" />')
        self.assertEqual(serialize(elem, encoding="us-ascii"),
                b'<tag key="&#229;&#246;&#246;&lt;&gt;" />')
        for enc in ("iso-8859-1", "utf-16", "utf-16le", "utf-16be", "utf-32"):
            self.assertEqual(serialize(elem, encoding=enc),
                    ("<?xml version='1.0' encoding='%s'?>\n"
                     "<tag key=\"åöö&lt;&gt;\" />" % enc).encode(enc))

    def test_write_to_filename(self):
        tree = ET.ElementTree(ET.XML('''<site />'''))
        tree.write(TESTFN)
        with open(TESTFN, 'rb') as f:
            self.assertEqual(f.read(), b'''<site />''')

    def test_write_to_text_file(self):
        tree = ET.ElementTree(ET.XML('''<site />'''))
        with open(TESTFN, 'w', encoding='utf-8') as f:
            tree.write(f, encoding='unicode')
            self.assertFalse(f.closed)
        with open(TESTFN, 'rb') as f:
            self.assertEqual(f.read(), b'''<site />''')

    def test_write_to_binary_file(self):
        tree = ET.ElementTree(ET.XML('''<site />'''))
        with open(TESTFN, 'wb') as f:
            tree.write(f)
            self.assertFalse(f.closed)
        with open(TESTFN, 'rb') as f:
            self.assertEqual(f.read(), b'''<site />''')

    def test_write_to_binary_file_with_bom(self):
        tree = ET.ElementTree(ET.XML('''<site />'''))
        # test BOM writing to buffered file
        with open(TESTFN, 'wb') as f:
            tree.write(f, encoding='utf-16')
            self.assertFalse(f.closed)
        with open(TESTFN, 'rb') as f:
            self.assertEqual(f.read(),
                    '''<?xml version='1.0' encoding='utf-16'?>\n'''
                    '''<site />'''.encode("utf-16"))
        # test BOM writing to non-buffered file
        with open(TESTFN, 'wb', buffering=0) as f:
            tree.write(f, encoding='utf-16')
            self.assertFalse(f.closed)
        with open(TESTFN, 'rb') as f:
            self.assertEqual(f.read(),
                    '''<?xml version='1.0' encoding='utf-16'?>\n'''
                    '''<site />'''.encode("utf-16"))

    def test_read_from_stringio(self):
        tree = ET.ElementTree()
        stream = io.StringIO('''<?xml version="1.0"?><site></site>''')
        tree.parse(stream)
        self.assertEqual(tree.getroot().tag, 'site')

    def test_write_to_stringio(self):
        tree = ET.ElementTree(ET.XML('''<site />'''))
        stream = io.StringIO()
        tree.write(stream, encoding='unicode')
        self.assertEqual(stream.getvalue(), '''<site />''')

    def test_read_from_bytesio(self):
        tree = ET.ElementTree()
        raw = io.BytesIO(b'''<?xml version="1.0"?><site></site>''')
        tree.parse(raw)
        self.assertEqual(tree.getroot().tag, 'site')

    def test_write_to_bytesio(self):
        tree = ET.ElementTree(ET.XML('''<site />'''))
        raw = io.BytesIO()
        tree.write(raw)
        self.assertEqual(raw.getvalue(), b'''<site />''')

    class dummy:
        pass

    def test_read_from_user_text_reader(self):
        stream = io.StringIO('''<?xml version="1.0"?><site></site>''')
        reader = self.dummy()
        reader.read = stream.read
        tree = ET.ElementTree()
        tree.parse(reader)
        self.assertEqual(tree.getroot().tag, 'site')

    def test_write_to_user_text_writer(self):
        tree = ET.ElementTree(ET.XML('''<site />'''))
        stream = io.StringIO()
        writer = self.dummy()
        writer.write = stream.write
        tree.write(writer, encoding='unicode')
        self.assertEqual(stream.getvalue(), '''<site />''')

    def test_read_from_user_binary_reader(self):
        raw = io.BytesIO(b'''<?xml version="1.0"?><site></site>''')
        reader = self.dummy()
        reader.read = raw.read
        tree = ET.ElementTree()
        tree.parse(reader)
        self.assertEqual(tree.getroot().tag, 'site')
        tree = ET.ElementTree()

    def test_write_to_user_binary_writer(self):
        tree = ET.ElementTree(ET.XML('''<site />'''))
        raw = io.BytesIO()
        writer = self.dummy()
        writer.write = raw.write
        tree.write(writer)
        self.assertEqual(raw.getvalue(), b'''<site />''')

    def test_write_to_user_binary_writer_with_bom(self):
        tree = ET.ElementTree(ET.XML('''<site />'''))
        raw = io.BytesIO()
        writer = self.dummy()
        writer.write = raw.write
        writer.seekable = lambda: True
        writer.tell = raw.tell
        tree.write(writer, encoding="utf-16")
        self.assertEqual(raw.getvalue(),
                '''<?xml version='1.0' encoding='utf-16'?>\n'''
                '''<site />'''.encode("utf-16"))

    def test_tostringlist_invariant(self):
        root = ET.fromstring('<tag>foo</tag>')
        self.assertEqual(
            ET.tostring(root, 'unicode'),
            ''.join(ET.tostringlist(root, 'unicode')))
        self.assertEqual(
            ET.tostring(root, 'utf-16'),
            b''.join(ET.tostringlist(root, 'utf-16')))

    def test_short_empty_elements(self):
        root = ET.fromstring('<tag>a<x />b<y></y>c</tag>')
        self.assertEqual(
            ET.tostring(root, 'unicode'),
            '<tag>a<x />b<y />c</tag>')
        self.assertEqual(
            ET.tostring(root, 'unicode', short_empty_elements=True),
            '<tag>a<x />b<y />c</tag>')
        self.assertEqual(
            ET.tostring(root, 'unicode', short_empty_elements=False),
            '<tag>a<x></x>b<y></y>c</tag>')


class ParseErrorTest(unittest.TestCase):
    def test_subclass(self):
        self.assertIsInstance(ET.ParseError(), SyntaxError)

    def _get_error(self, s):
        try:
            ET.fromstring(s)
        except ET.ParseError as e:
            return e

    def test_error_position(self):
        self.assertEqual(self._get_error('foo').position, (1, 0))
        self.assertEqual(self._get_error('<tag>&foo;</tag>').position, (1, 5))
        self.assertEqual(self._get_error('foobar<').position, (1, 6))

    def test_error_code(self):
        import xml.parsers.expat.errors as ERRORS
        self.assertEqual(self._get_error('foo').code,
                ERRORS.codes[ERRORS.XML_ERROR_SYNTAX])


class KeywordArgsTest(unittest.TestCase):
    # Test various issues with keyword arguments passed to ET.Element
    # constructor and methods
    def test_issue14818(self):
        x = ET.XML("<a>foo</a>")
        self.assertEqual(x.find('a', None),
                         x.find(path='a', namespaces=None))
        self.assertEqual(x.findtext('a', None, None),
                         x.findtext(path='a', default=None, namespaces=None))
        self.assertEqual(x.findall('a', None),
                         x.findall(path='a', namespaces=None))
        self.assertEqual(list(x.iterfind('a', None)),
                         list(x.iterfind(path='a', namespaces=None)))

        self.assertEqual(ET.Element('a').attrib, {})
        elements = [
            ET.Element('a', dict(href="#", id="foo")),
            ET.Element('a', attrib=dict(href="#", id="foo")),
            ET.Element('a', dict(href="#"), id="foo"),
            ET.Element('a', href="#", id="foo"),
            ET.Element('a', dict(href="#", id="foo"), href="#", id="foo"),
        ]
        for e in elements:
            self.assertEqual(e.tag, 'a')
            self.assertEqual(e.attrib, dict(href="#", id="foo"))

        e2 = ET.SubElement(elements[0], 'foobar', attrib={'key1': 'value1'})
        self.assertEqual(e2.attrib['key1'], 'value1')

        with self.assertRaisesRegex(TypeError, 'must be dict, not str'):
            ET.Element('a', "I'm not a dict")
        with self.assertRaisesRegex(TypeError, 'must be dict, not str'):
            ET.Element('a', attrib="I'm not a dict")

# --------------------------------------------------------------------

class NoAcceleratorTest(unittest.TestCase):
    def setUp(self):
        if not pyET:
            raise unittest.SkipTest('only for the Python version')

    # Test that the C accelerator was not imported for pyET
    def test_correct_import_pyET(self):
        # The type of methods defined in Python code is types.FunctionType,
        # while the type of methods defined inside _elementtree is
        # <class 'wrapper_descriptor'>
        self.assertIsInstance(pyET.Element.__init__, types.FunctionType)
        self.assertIsInstance(pyET.XMLParser.__init__, types.FunctionType)

# --------------------------------------------------------------------


class CleanContext(object):
    """Provide default namespace mapping and path cache."""
    checkwarnings = None

    def __init__(self, quiet=False):
        if sys.flags.optimize >= 2:
            # under -OO, doctests cannot be run and therefore not all warnings
            # will be emitted
            quiet = True
        deprecations = (
            # Search behaviour is broken if search path starts with "/".
            ("This search is broken in 1.3 and earlier, and will be fixed "
             "in a future version.  If you rely on the current behaviour, "
             "change it to '.+'", FutureWarning),
            # Element.getchildren() and Element.getiterator() are deprecated.
            ("This method will be removed in future versions.  "
             "Use .+ instead.", DeprecationWarning),
            ("This method will be removed in future versions.  "
             "Use .+ instead.", PendingDeprecationWarning))
        self.checkwarnings = support.check_warnings(*deprecations, quiet=quiet)

    def __enter__(self):
        from xml.etree import ElementPath
        self._nsmap = ET.register_namespace._namespace_map
        # Copy the default namespace mapping
        self._nsmap_copy = self._nsmap.copy()
        # Copy the path cache (should be empty)
        self._path_cache = ElementPath._cache
        ElementPath._cache = self._path_cache.copy()
        self.checkwarnings.__enter__()

    def __exit__(self, *args):
        from xml.etree import ElementPath
        # Restore mapping and path cache
        self._nsmap.clear()
        self._nsmap.update(self._nsmap_copy)
        ElementPath._cache = self._path_cache
        self.checkwarnings.__exit__(*args)


def test_main(module=None):
    # When invoked without a module, runs the Python ET tests by loading pyET.
    # Otherwise, uses the given module as the ET.
    global pyET
    pyET = import_fresh_module('xml.etree.ElementTree',
                               blocked=['_elementtree'])
    if module is None:
        module = pyET

    global ET
    ET = module

    test_classes = [
        ModuleTest,
        ElementSlicingTest,
        BasicElementTest,
        ElementTreeTest,
        IOTest,
        ParseErrorTest,
        XIncludeTest,
        ElementTreeTypeTest,
        ElementFindTest,
        ElementIterTest,
        TreeBuilderTest,
        XMLParserTest,
        XMLPullParserTest,
        BugsTest,
        ]

    # These tests will only run for the pure-Python version that doesn't import
    # _elementtree. We can't use skipUnless here, because pyET is filled in only
    # after the module is loaded.
    if pyET is not ET:
        test_classes.extend([
            NoAcceleratorTest,
            ])

    try:
        # XXX the C module should give the same warnings as the Python module
        with CleanContext(quiet=(pyET is not ET)):
            support.run_unittest(*test_classes)
    finally:
        # don't interfere with subsequent tests
        ET = pyET = None


if __name__ == '__main__':
    test_main()
