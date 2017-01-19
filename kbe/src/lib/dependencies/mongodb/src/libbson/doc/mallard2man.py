#!/usr/bin/env python

#
# mallard2man.py
#
# Copyright (C) 2014 MongoDB, Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

COPYRIGHT_HOLDER = "MongoDB, Inc."
GROUP = "libbson"
BUG_URL = 'https://jira.mongodb.org/browse/CDRIVER'

"""
This script is mean to convert a fairly basic mallard format documentation
page to a groff styled man page.
"""

import os
import re
import sys

import codecs
from datetime import datetime
from xml.etree import ElementTree

INCLUDE = '{http://www.w3.org/2001/XInclude}include'
TITLE = '{http://projectmallard.org/1.0/}title'
SUBTITLE = '{http://projectmallard.org/1.0/}subtitle'
SECTION = '{http://projectmallard.org/1.0/}section'
INFO = '{http://projectmallard.org/1.0/}info'
ITEM = '{http://projectmallard.org/1.0/}item'
LISTING = '{http://projectmallard.org/1.0/}listing'
LIST = '{http://projectmallard.org/1.0/}list'
LINK = '{http://projectmallard.org/1.0/}link'
LINKS = '{http://projectmallard.org/1.0/}links'
SYNOPSIS = '{http://projectmallard.org/1.0/}synopsis'
CODE = '{http://projectmallard.org/1.0/}code'
P = '{http://projectmallard.org/1.0/}p'
SCREEN = '{http://projectmallard.org/1.0/}screen'
EM = '{http://projectmallard.org/1.0/}em'
NOTE = '{http://projectmallard.org/1.0/}note'
TABLE = '{http://projectmallard.org/1.0/}table'
TR = '{http://projectmallard.org/1.0/}tr'
TD = '{http://projectmallard.org/1.0/}td'
OUTPUT = '{http://projectmallard.org/1.0/}output'

# Matches "\" and "-", but not "\-".
replaceables = re.compile(r'(\\(?!-))|((?<!\\)-)')


class Convert(object):
    title = None
    subtitle = None
    sections = None
    relpath = None

    def __init__(self, inFile, outFile, section):
        self.inFile = inFile
        self.relpath = os.path.dirname(inFile)
        self.outFile = outFile
        self.section = section
        self.sections = []

        # Map: section id -> section element.
        self.sections_map = {}

    def _parse(self):
        self.tree = ElementTree.ElementTree()
        self.tree.parse(open(self.inFile))
        self.root = self.tree.getroot()

        # Python's standard ElementTree doesn't store an element's parent on
        # the element. Make a child->parent map.
        try:
            iterator = self.tree.iter()
        except AttributeError:
            # Python 2.6.
            iterator = self.tree.getiterator()
        self.parent_map = dict((c, p) for p in iterator for c in p)

    def _get_parent(self, ele):
        return self.parent_map[ele]

    def _extract(self):
        # Extract the title and subtitle.
        for child in self.root.getchildren():
            if child.tag == TITLE:
                # A title like "Version Checks" can't have spaces, otherwise
                # the "whatis" entry can't be parsed from the man page title.
                self.title = child.text.strip().replace(' ', '_')
            elif child.tag == SUBTITLE:
                self.subtitle = child.text.strip()
            elif child.tag == SECTION:
                if child.get('id'):
                    self.sections_map[child.get('id')] = child
                self.sections.append(child)

        if not self.subtitle and 'description' in self.sections_map:
            # No "subtitle" element, use description section title as subtitle.
            self.subtitle = self._section_text(self.sections_map['description'])

    def _section_text(self, section):
        # Find <section id="description"><p>some text</p></section>.
        for child in section:
            if child.tag != TITLE:
                return self._textify_elem(child)

    def _textify_elem(self, elem):
        return ''.join(elem.itertext()).strip()

    def _writeComment(self, text=''):
        lines = text.split('\n')
        for line in lines:
            self.outFile.write('.\\" ')
            self.outFile.write(line)
            self.outFile.write('\n')

    def _escape_char(self, match):
        c = match.group(0)
        if c == "-":
            return r"\(hy"
        elif c == "\\":
            return "\\e"

        assert False, "invalid char passed to _escape_char: %r" % c

    def _escape(self, text):
        # Avoid "hyphen-used-as-minus-sign" lintian warning about man pages,
        # and escape text like "\0" as "\\0". We'll replace all "-" with "\(hy",
        # which is an explicit hyphen, but leave alone the first line's
        # "name \- description" text.
        return replaceables.sub(self._escape_char, text)

    def _write(self, text):
        self._write_noescape(self._escape(text))

    def _write_noescape(self, text):
        self.outFile.write(text)

    def _writeCommand(self, text):
        self._write(text)
        self._write('\n')

    def _writeLine(self, text):
        if text is not None:
            text = text.strip()
            if text.startswith('.'):
                text = '\\&' + text
            self._write(text)
        self._write('\n')

    def _generateHeader(self):
        year = datetime.utcnow().year
        self._writeComment('This manpage is Copyright (C) %s %s' % (year, COPYRIGHT_HOLDER))
        self._writeComment('')
        self._writeComment(
    "Permission is granted to copy, distribute and/or modify this document\n"
    "under the terms of the GNU Free Documentation License, Version 1.3\n"
    "or any later version published by the Free Software Foundation;\n"
    "with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.\n"
    "A copy of the license is included in the section entitled \"GNU\n"
    "Free Documentation License\".")
        self._writeComment('')

        date = datetime.fromtimestamp(int(os.stat(self.inFile).st_mtime)).strftime('%Y-%m-%d')
        title = self.title.replace('()','').upper()
        self._write('.TH "%s" "%s" "%s" "%s"\n' % (title, self.section, date, GROUP))
        self._write('.SH NAME\n')
        self._write_noescape('%s \\- %s\n' % (self.title, self.subtitle))

    def _generateSection(self, section):
        # Try to render the title first
        for child in section.getchildren():
            if child.tag == TITLE:
                s = child.text.strip().upper()
                self._writeCommand('.SH "%s"' % s)
        for child in section.getchildren():
            self._generateElement(child)
            if child.tail:
                self._writeLine(child.tail)

    def _generateSynopsis(self, synopsis):
        self._writeCommand('.nf')
        for child in synopsis.getchildren():
            self._generateElement(child)
            if child.tail:
                self._writeLine(child.tail)
        self._writeCommand('.fi')

    def _generateCode(self, code):
        text = code.text
        is_synopsis = self._get_parent(code).tag.endswith('synopsis')
        if text and '\n' not in text and not is_synopsis:
            text = text.replace('()', '(%s)' % self.section)
            self._writeCommand('.B ' + text)
        else:
            self._writeCommand('.nf')
            self._writeLine(code.text)
            for child in code.getchildren():
                self._generateElement(child)
            self._writeCommand('.fi')

    def _generateNote(self, note):
        self._writeCommand('.B NOTE')
        self._writeCommand('.RS')
        for child in note.getchildren():
            self._generateElement(child)
            if child.tail:
                self._writeLine(child.tail)
        self._writeCommand('.RE')

    def _generateP(self, p):
        if p.text:
            self._writeLine(p.text)
        for child in p.getchildren():
            self._generateElement(child)
            if child.tail:
                self._writeLine(child.tail)

    def _generateScreen(self, screen):
        for child in screen.getchildren():
            self._generateElement(child)

    def _generateListing(self, listing):
        for child in listing.getchildren():
            self._generateElement(child)

    def _generateList(self, l):
        for child in l.getchildren():
            self._generateElement(child)

    def _generateEM(self, em):
        self._writeCommand('.B %s' % em.text)

    def _generateOutput(self, output):
        self._generateCode(output)

    def _generateItem(self, item):
        self._writeCommand('.IP \\[bu] 2')
        for child in item.getchildren():
            self._generateElement(child)

    def _generateElement(self, ele):
        if ele.tag == SECTION:
            self._generateSection(ele)
        elif ele.tag == SYNOPSIS:
            self._generateSynopsis(ele)
        elif ele.tag == CODE:
            self._generateCode(ele)
        elif ele.tag == OUTPUT:
            self._generateOutput(ele)
        elif ele.tag == P:
            self._generateP(ele)
        elif ele.tag == EM:
            self._generateEM(ele)
        elif ele.tag == LISTING:
            self._generateListing(ele)
        elif ele.tag == ITEM:
            self._generateItem(ele)
        elif ele.tag == LIST:
            self._generateList(ele)
        elif ele.tag == TITLE:
            pass
        elif ele.tag == SCREEN:
            self._generateScreen(ele)
        elif ele.tag == LINK:
            self._generateLink(ele)
        elif ele.tag == NOTE:
            self._generateNote(ele)
        elif ele.tag == TABLE:
            self._generateTable(ele)
        elif ele.tag == TR:
            self._generateTr(ele)
        elif ele.tag == TD:
            self._generateTd(ele)
        elif ele.tag == INCLUDE:
            f = ele.attrib['href']
            f = os.path.join(self.relpath, f)
            d = codecs.open(f, 'r', encoding='utf-8').read()
            self._writeLine(d)
        else:
            print('unknown element type %s' % ele)

    def _generateTable(self, table):
        for child in table.getchildren():
            self._generateElement(child)

    def _generateTr(self, tr):
        self._writeCommand('.TP')
        self._writeCommand('.B')
        for child in tr.getchildren():
            self._generateElement(child)
        self._writeCommand('.LP')

    def _generateTd(self, td):
        for child in td.getchildren():
            self._generateElement(child)

    def _generateLink(self, link):
        text = link.text
        if text and '()' in text:
            text = text.replace('()', '(%s)' % self.section)
        if text:
            self._writeCommand('.B ' + text)

    def _generateSections(self):
        for section in self.sections:
            self._generateElement(section)

    def _generateFooter(self):
        self._write('\n.B')
        self._write('\n.SH COLOPHON')
        self._write('\nThis page is part of %s.' % GROUP)
        self._write('\nPlease report any bugs at %s.' % BUG_URL.replace('-','\\-'))

    def _generate(self):
        self.realname = self.outFile
        self.outFile = codecs.open(self.outFile + '.tmp', 'w', encoding='utf-8')
        self._generateHeader()
        self._generateSections()
        self._generateFooter()
        os.rename(self.outFile.name, self.realname)
        self.outFile.close()

    def convert(self):
        self._parse()
        self._extract()
        self._generate()

def main(filenames, section='3'):
    for inFile in filenames:
        dirName = os.path.dirname(inFile) + '/man/'
        baseName = os.path.basename(inFile)
        baseFile = os.path.splitext(baseName)[0]
        outFile = dirName + baseFile + '.' + section
        c = Convert(inFile, outFile, section)
        c.convert()

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print('usage: %s SECTION FILENAMES...' % sys.argv[0])
        sys.exit(1)
    section = sys.argv[1]
    main(sys.argv[2:], section)
    sys.exit(0)
