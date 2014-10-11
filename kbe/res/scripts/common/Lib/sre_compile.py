#
# Secret Labs' Regular Expression Engine
#
# convert template to internal format
#
# Copyright (c) 1997-2001 by Secret Labs AB.  All rights reserved.
#
# See the sre.py file for information on usage and redistribution.
#

"""Internal support module for sre"""

import _sre
import sre_parse
from sre_constants import *
from _sre import MAXREPEAT

assert _sre.MAGIC == MAGIC, "SRE module mismatch"

if _sre.CODESIZE == 2:
    MAXCODE = 65535
else:
    MAXCODE = 0xFFFFFFFF

def _identityfunction(x):
    return x

_LITERAL_CODES = set([LITERAL, NOT_LITERAL])
_REPEATING_CODES = set([REPEAT, MIN_REPEAT, MAX_REPEAT])
_SUCCESS_CODES = set([SUCCESS, FAILURE])
_ASSERT_CODES = set([ASSERT, ASSERT_NOT])

def _compile(code, pattern, flags):
    # internal: compile a (sub)pattern
    emit = code.append
    _len = len
    LITERAL_CODES = _LITERAL_CODES
    REPEATING_CODES = _REPEATING_CODES
    SUCCESS_CODES = _SUCCESS_CODES
    ASSERT_CODES = _ASSERT_CODES
    for op, av in pattern:
        if op in LITERAL_CODES:
            if flags & SRE_FLAG_IGNORECASE:
                emit(OPCODES[OP_IGNORE[op]])
                emit(_sre.getlower(av, flags))
            else:
                emit(OPCODES[op])
                emit(av)
        elif op is IN:
            if flags & SRE_FLAG_IGNORECASE:
                emit(OPCODES[OP_IGNORE[op]])
                def fixup(literal, flags=flags):
                    return _sre.getlower(literal, flags)
            else:
                emit(OPCODES[op])
                fixup = _identityfunction
            skip = _len(code); emit(0)
            _compile_charset(av, flags, code, fixup)
            code[skip] = _len(code) - skip
        elif op is ANY:
            if flags & SRE_FLAG_DOTALL:
                emit(OPCODES[ANY_ALL])
            else:
                emit(OPCODES[ANY])
        elif op in REPEATING_CODES:
            if flags & SRE_FLAG_TEMPLATE:
                raise error("internal: unsupported template operator")
            elif _simple(av) and op is not REPEAT:
                if op is MAX_REPEAT:
                    emit(OPCODES[REPEAT_ONE])
                else:
                    emit(OPCODES[MIN_REPEAT_ONE])
                skip = _len(code); emit(0)
                emit(av[0])
                emit(av[1])
                _compile(code, av[2], flags)
                emit(OPCODES[SUCCESS])
                code[skip] = _len(code) - skip
            else:
                emit(OPCODES[REPEAT])
                skip = _len(code); emit(0)
                emit(av[0])
                emit(av[1])
                _compile(code, av[2], flags)
                code[skip] = _len(code) - skip
                if op is MAX_REPEAT:
                    emit(OPCODES[MAX_UNTIL])
                else:
                    emit(OPCODES[MIN_UNTIL])
        elif op is SUBPATTERN:
            if av[0]:
                emit(OPCODES[MARK])
                emit((av[0]-1)*2)
            # _compile_info(code, av[1], flags)
            _compile(code, av[1], flags)
            if av[0]:
                emit(OPCODES[MARK])
                emit((av[0]-1)*2+1)
        elif op in SUCCESS_CODES:
            emit(OPCODES[op])
        elif op in ASSERT_CODES:
            emit(OPCODES[op])
            skip = _len(code); emit(0)
            if av[0] >= 0:
                emit(0) # look ahead
            else:
                lo, hi = av[1].getwidth()
                if lo != hi:
                    raise error("look-behind requires fixed-width pattern")
                emit(lo) # look behind
            _compile(code, av[1], flags)
            emit(OPCODES[SUCCESS])
            code[skip] = _len(code) - skip
        elif op is CALL:
            emit(OPCODES[op])
            skip = _len(code); emit(0)
            _compile(code, av, flags)
            emit(OPCODES[SUCCESS])
            code[skip] = _len(code) - skip
        elif op is AT:
            emit(OPCODES[op])
            if flags & SRE_FLAG_MULTILINE:
                av = AT_MULTILINE.get(av, av)
            if flags & SRE_FLAG_LOCALE:
                av = AT_LOCALE.get(av, av)
            elif flags & SRE_FLAG_UNICODE:
                av = AT_UNICODE.get(av, av)
            emit(ATCODES[av])
        elif op is BRANCH:
            emit(OPCODES[op])
            tail = []
            tailappend = tail.append
            for av in av[1]:
                skip = _len(code); emit(0)
                # _compile_info(code, av, flags)
                _compile(code, av, flags)
                emit(OPCODES[JUMP])
                tailappend(_len(code)); emit(0)
                code[skip] = _len(code) - skip
            emit(0) # end of branch
            for tail in tail:
                code[tail] = _len(code) - tail
        elif op is CATEGORY:
            emit(OPCODES[op])
            if flags & SRE_FLAG_LOCALE:
                av = CH_LOCALE[av]
            elif flags & SRE_FLAG_UNICODE:
                av = CH_UNICODE[av]
            emit(CHCODES[av])
        elif op is GROUPREF:
            if flags & SRE_FLAG_IGNORECASE:
                emit(OPCODES[OP_IGNORE[op]])
            else:
                emit(OPCODES[op])
            emit(av-1)
        elif op is GROUPREF_EXISTS:
            emit(OPCODES[op])
            emit(av[0]-1)
            skipyes = _len(code); emit(0)
            _compile(code, av[1], flags)
            if av[2]:
                emit(OPCODES[JUMP])
                skipno = _len(code); emit(0)
                code[skipyes] = _len(code) - skipyes + 1
                _compile(code, av[2], flags)
                code[skipno] = _len(code) - skipno
            else:
                code[skipyes] = _len(code) - skipyes + 1
        else:
            raise ValueError("unsupported operand type", op)

def _compile_charset(charset, flags, code, fixup=None):
    # compile charset subprogram
    emit = code.append
    if fixup is None:
        fixup = _identityfunction
    for op, av in _optimize_charset(charset, fixup):
        emit(OPCODES[op])
        if op is NEGATE:
            pass
        elif op is LITERAL:
            emit(fixup(av))
        elif op is RANGE:
            emit(fixup(av[0]))
            emit(fixup(av[1]))
        elif op is CHARSET:
            code.extend(av)
        elif op is BIGCHARSET:
            code.extend(av)
        elif op is CATEGORY:
            if flags & SRE_FLAG_LOCALE:
                emit(CHCODES[CH_LOCALE[av]])
            elif flags & SRE_FLAG_UNICODE:
                emit(CHCODES[CH_UNICODE[av]])
            else:
                emit(CHCODES[av])
        else:
            raise error("internal: unsupported set operator")
    emit(OPCODES[FAILURE])

def _optimize_charset(charset, fixup):
    # internal: optimize character set
    out = []
    tail = []
    charmap = bytearray(256)
    for op, av in charset:
        while True:
            try:
                if op is LITERAL:
                    charmap[fixup(av)] = 1
                elif op is RANGE:
                    for i in range(fixup(av[0]), fixup(av[1])+1):
                        charmap[i] = 1
                elif op is NEGATE:
                    out.append((op, av))
                else:
                    tail.append((op, av))
            except IndexError:
                if len(charmap) == 256:
                    # character set contains non-UCS1 character codes
                    charmap += b'\0' * 0xff00
                    continue
                # character set contains non-BMP character codes
                tail.append((op, av))
            break

    # compress character map
    runs = []
    q = 0
    while True:
        p = charmap.find(1, q)
        if p < 0:
            break
        if len(runs) >= 2:
            runs = None
            break
        q = charmap.find(0, p)
        if q < 0:
            runs.append((p, len(charmap)))
            break
        runs.append((p, q))
    if runs is not None:
        # use literal/range
        for p, q in runs:
            if q - p == 1:
                out.append((LITERAL, p))
            else:
                out.append((RANGE, (p, q - 1)))
        out += tail
        if len(out) < len(charset):
            return out
        return charset

    # use bitmap
    if len(charmap) == 256:
        data = _mk_bitmap(charmap)
        out.append((CHARSET, data))
        out += tail
        return out

    # To represent a big charset, first a bitmap of all characters in the
    # set is constructed. Then, this bitmap is sliced into chunks of 256
    # characters, duplicate chunks are eliminated, and each chunk is
    # given a number. In the compiled expression, the charset is
    # represented by a 32-bit word sequence, consisting of one word for
    # the number of different chunks, a sequence of 256 bytes (64 words)
    # of chunk numbers indexed by their original chunk position, and a
    # sequence of 256-bit chunks (8 words each).

    # Compression is normally good: in a typical charset, large ranges of
    # Unicode will be either completely excluded (e.g. if only cyrillic
    # letters are to be matched), or completely included (e.g. if large
    # subranges of Kanji match). These ranges will be represented by
    # chunks of all one-bits or all zero-bits.

    # Matching can be also done efficiently: the more significant byte of
    # the Unicode character is an index into the chunk number, and the
    # less significant byte is a bit index in the chunk (just like the
    # CHARSET matching).

    charmap = bytes(charmap) # should be hashable
    comps = {}
    mapping = bytearray(256)
    block = 0
    data = bytearray()
    for i in range(0, 65536, 256):
        chunk = charmap[i: i + 256]
        if chunk in comps:
            mapping[i // 256] = comps[chunk]
        else:
            mapping[i // 256] = comps[chunk] = block
            block += 1
            data += chunk
    data = _mk_bitmap(data)
    data[0:0] = [block] + _bytes_to_codes(mapping)
    out.append((BIGCHARSET, data))
    out += tail
    return out

_CODEBITS = _sre.CODESIZE * 8
_BITS_TRANS = b'0' + b'1' * 255
def _mk_bitmap(bits, _CODEBITS=_CODEBITS, _int=int):
    s = bits.translate(_BITS_TRANS)[::-1]
    return [_int(s[i - _CODEBITS: i], 2)
            for i in range(len(s), 0, -_CODEBITS)]

def _bytes_to_codes(b):
    # Convert block indices to word array
    import array
    a = array.array('I', b)
    assert a.itemsize == _sre.CODESIZE
    assert len(a) * a.itemsize == len(b)
    return a.tolist()

def _simple(av):
    # check if av is a "simple" operator
    lo, hi = av[2].getwidth()
    return lo == hi == 1 and av[2][0][0] != SUBPATTERN

def _generate_overlap_table(prefix):
    """
    Generate an overlap table for the following prefix.
    An overlap table is a table of the same size as the prefix which
    informs about the potential self-overlap for each index in the prefix:
    - if overlap[i] == 0, prefix[i:] can't overlap prefix[0:...]
    - if overlap[i] == k with 0 < k <= i, prefix[i-k+1:i+1] overlaps with
      prefix[0:k]
    """
    table = [0] * len(prefix)
    for i in range(1, len(prefix)):
        idx = table[i - 1]
        while prefix[i] != prefix[idx]:
            if idx == 0:
                table[i] = 0
                break
            idx = table[idx - 1]
        else:
            table[i] = idx + 1
    return table

def _compile_info(code, pattern, flags):
    # internal: compile an info block.  in the current version,
    # this contains min/max pattern width, and an optional literal
    # prefix or a character map
    lo, hi = pattern.getwidth()
    if lo == 0:
        return # not worth it
    # look for a literal prefix
    prefix = []
    prefixappend = prefix.append
    prefix_skip = 0
    charset = [] # not used
    charsetappend = charset.append
    if not (flags & SRE_FLAG_IGNORECASE):
        # look for literal prefix
        for op, av in pattern.data:
            if op is LITERAL:
                if len(prefix) == prefix_skip:
                    prefix_skip = prefix_skip + 1
                prefixappend(av)
            elif op is SUBPATTERN and len(av[1]) == 1:
                op, av = av[1][0]
                if op is LITERAL:
                    prefixappend(av)
                else:
                    break
            else:
                break
        # if no prefix, look for charset prefix
        if not prefix and pattern.data:
            op, av = pattern.data[0]
            if op is SUBPATTERN and av[1]:
                op, av = av[1][0]
                if op is LITERAL:
                    charsetappend((op, av))
                elif op is BRANCH:
                    c = []
                    cappend = c.append
                    for p in av[1]:
                        if not p:
                            break
                        op, av = p[0]
                        if op is LITERAL:
                            cappend((op, av))
                        else:
                            break
                    else:
                        charset = c
            elif op is BRANCH:
                c = []
                cappend = c.append
                for p in av[1]:
                    if not p:
                        break
                    op, av = p[0]
                    if op is LITERAL:
                        cappend((op, av))
                    else:
                        break
                else:
                    charset = c
            elif op is IN:
                charset = av
##     if prefix:
##         print "*** PREFIX", prefix, prefix_skip
##     if charset:
##         print "*** CHARSET", charset
    # add an info block
    emit = code.append
    emit(OPCODES[INFO])
    skip = len(code); emit(0)
    # literal flag
    mask = 0
    if prefix:
        mask = SRE_INFO_PREFIX
        if len(prefix) == prefix_skip == len(pattern.data):
            mask = mask + SRE_INFO_LITERAL
    elif charset:
        mask = mask + SRE_INFO_CHARSET
    emit(mask)
    # pattern length
    if lo < MAXCODE:
        emit(lo)
    else:
        emit(MAXCODE)
        prefix = prefix[:MAXCODE]
    if hi < MAXCODE:
        emit(hi)
    else:
        emit(0)
    # add literal prefix
    if prefix:
        emit(len(prefix)) # length
        emit(prefix_skip) # skip
        code.extend(prefix)
        # generate overlap table
        code.extend(_generate_overlap_table(prefix))
    elif charset:
        _compile_charset(charset, flags, code)
    code[skip] = len(code) - skip

def isstring(obj):
    return isinstance(obj, (str, bytes))

def _code(p, flags):

    flags = p.pattern.flags | flags
    code = []

    # compile info block
    _compile_info(code, p, flags)

    # compile the pattern
    _compile(code, p.data, flags)

    code.append(OPCODES[SUCCESS])

    return code

def compile(p, flags=0):
    # internal: convert pattern list to internal format

    if isstring(p):
        pattern = p
        p = sre_parse.parse(p, flags)
    else:
        pattern = None

    code = _code(p, flags)

    # print code

    # XXX: <fl> get rid of this limitation!
    if p.pattern.groups > 100:
        raise AssertionError(
            "sorry, but this version only supports 100 named groups"
            )

    # map in either direction
    groupindex = p.pattern.groupdict
    indexgroup = [None] * p.pattern.groups
    for k, i in groupindex.items():
        indexgroup[i] = k

    return _sre.compile(
        pattern, flags | p.pattern.flags, code,
        p.pattern.groups-1,
        groupindex, indexgroup
        )
