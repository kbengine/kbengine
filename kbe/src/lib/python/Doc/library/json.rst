:mod:`json` --- JSON encoder and decoder
========================================

.. module:: json
   :synopsis: Encode and decode the JSON format.
.. moduleauthor:: Bob Ippolito <bob@redivi.com>
.. sectionauthor:: Bob Ippolito <bob@redivi.com>

`JSON (JavaScript Object Notation) <http://json.org>`_, specified by
:rfc:`4627`, is a lightweight data interchange format based on a subset of
`JavaScript <http://en.wikipedia.org/wiki/JavaScript>`_ syntax (`ECMA-262 3rd
edition <http://www.ecma-international.org/publications/files/ECMA-ST-ARCH/ECMA-262,%203rd%20edition,%20December%201999.pdf>`_).

:mod:`json` exposes an API familiar to users of the standard library
:mod:`marshal` and :mod:`pickle` modules.

Encoding basic Python object hierarchies::

    >>> import json
    >>> json.dumps(['foo', {'bar': ('baz', None, 1.0, 2)}])
    '["foo", {"bar": ["baz", null, 1.0, 2]}]'
    >>> print(json.dumps("\"foo\bar"))
    "\"foo\bar"
    >>> print(json.dumps('\u1234'))
    "\u1234"
    >>> print(json.dumps('\\'))
    "\\"
    >>> print(json.dumps({"c": 0, "b": 0, "a": 0}, sort_keys=True))
    {"a": 0, "b": 0, "c": 0}
    >>> from io import StringIO
    >>> io = StringIO()
    >>> json.dump(['streaming API'], io)
    >>> io.getvalue()
    '["streaming API"]'

Compact encoding::

    >>> import json
    >>> json.dumps([1,2,3,{'4': 5, '6': 7}], separators=(',', ':'))
    '[1,2,3,{"4":5,"6":7}]'

Pretty printing::

    >>> import json
    >>> print(json.dumps({'4': 5, '6': 7}, sort_keys=True, indent=4))
    {
        "4": 5,
        "6": 7
    }

Decoding JSON::

    >>> import json
    >>> json.loads('["foo", {"bar":["baz", null, 1.0, 2]}]')
    ['foo', {'bar': ['baz', None, 1.0, 2]}]
    >>> json.loads('"\\"foo\\bar"')
    '"foo\x08ar'
    >>> from io import StringIO
    >>> io = StringIO('["streaming API"]')
    >>> json.load(io)
    ['streaming API']

Specializing JSON object decoding::

    >>> import json
    >>> def as_complex(dct):
    ...     if '__complex__' in dct:
    ...         return complex(dct['real'], dct['imag'])
    ...     return dct
    ...
    >>> json.loads('{"__complex__": true, "real": 1, "imag": 2}',
    ...     object_hook=as_complex)
    (1+2j)
    >>> import decimal
    >>> json.loads('1.1', parse_float=decimal.Decimal)
    Decimal('1.1')

Extending :class:`JSONEncoder`::

    >>> import json
    >>> class ComplexEncoder(json.JSONEncoder):
    ...     def default(self, obj):
    ...         if isinstance(obj, complex):
    ...             return [obj.real, obj.imag]
    ...         # Let the base class default method raise the TypeError
    ...         return json.JSONEncoder.default(self, obj)
    ...
    >>> json.dumps(2 + 1j, cls=ComplexEncoder)
    '[2.0, 1.0]'
    >>> ComplexEncoder().encode(2 + 1j)
    '[2.0, 1.0]'
    >>> list(ComplexEncoder().iterencode(2 + 1j))
    ['[2.0', ', 1.0', ']']


.. highlight:: bash

Using json.tool from the shell to validate and pretty-print::

    $ echo '{"json":"obj"}' | python -mjson.tool
    {
        "json": "obj"
    }
    $ echo '{1.2:3.4}' | python -mjson.tool
    Expecting property name enclosed in double quotes: line 1 column 2 (char 1)

.. highlight:: python3

.. note::

   JSON is a subset of `YAML <http://yaml.org/>`_ 1.2.  The JSON produced by
   this module's default settings (in particular, the default *separators*
   value) is also a subset of YAML 1.0 and 1.1.  This module can thus also be
   used as a YAML serializer.


Basic Usage
-----------

.. function:: dump(obj, fp, skipkeys=False, ensure_ascii=True, \
                   check_circular=True, allow_nan=True, cls=None, \
                   indent=None, separators=None, default=None, \
                   sort_keys=False, **kw)

   Serialize *obj* as a JSON formatted stream to *fp* (a ``.write()``-supporting
   :term:`file-like object`) using this :ref:`conversion table
   <py-to-json-table>`.

   If *skipkeys* is ``True`` (default: ``False``), then dict keys that are not
   of a basic type (:class:`str`, :class:`int`, :class:`float`, :class:`bool`,
   ``None``) will be skipped instead of raising a :exc:`TypeError`.

   The :mod:`json` module always produces :class:`str` objects, not
   :class:`bytes` objects. Therefore, ``fp.write()`` must support :class:`str`
   input.

   If *ensure_ascii* is ``True`` (the default), the output is guaranteed to
   have all incoming non-ASCII characters escaped.  If *ensure_ascii* is
   ``False``, these characters will be output as-is.

   If *check_circular* is ``False`` (default: ``True``), then the circular
   reference check for container types will be skipped and a circular reference
   will result in an :exc:`OverflowError` (or worse).

   If *allow_nan* is ``False`` (default: ``True``), then it will be a
   :exc:`ValueError` to serialize out of range :class:`float` values (``nan``,
   ``inf``, ``-inf``) in strict compliance of the JSON specification, instead of
   using the JavaScript equivalents (``NaN``, ``Infinity``, ``-Infinity``).

   If *indent* is a non-negative integer or string, then JSON array elements and
   object members will be pretty-printed with that indent level.  An indent level
   of 0, negative, or ``""`` will only insert newlines.  ``None`` (the default)
   selects the most compact representation. Using a positive integer indent
   indents that many spaces per level.  If *indent* is a string (such as ``"\t"``),
   that string is used to indent each level.

   .. versionchanged:: 3.2
      Allow strings for *indent* in addition to integers.

   If specified, *separators* should be an ``(item_separator, key_separator)``
   tuple.  The default is ``(', ', ': ')`` if *indent* is ``None`` and
   ``(',', ': ')`` otherwise.  To get the most compact JSON representation,
   you should specify ``(',', ':')`` to eliminate whitespace.

   .. versionchanged:: 3.4
      Use ``(',', ': ')`` as default if *indent* is not ``None``.

   *default(obj)* is a function that should return a serializable version of
   *obj* or raise :exc:`TypeError`.  The default simply raises :exc:`TypeError`.

   If *sort_keys* is ``True`` (default: ``False``), then the output of
   dictionaries will be sorted by key.

   To use a custom :class:`JSONEncoder` subclass (e.g. one that overrides the
   :meth:`default` method to serialize additional types), specify it with the
   *cls* kwarg; otherwise :class:`JSONEncoder` is used.


.. function:: dumps(obj, skipkeys=False, ensure_ascii=True, \
                    check_circular=True, allow_nan=True, cls=None, \
                    indent=None, separators=None, default=None, \
                    sort_keys=False, **kw)

   Serialize *obj* to a JSON formatted :class:`str` using this :ref:`conversion
   table <py-to-json-table>`.  The arguments have the same meaning as in
   :func:`dump`.

   .. note::

      Unlike :mod:`pickle` and :mod:`marshal`, JSON is not a framed protocol,
      so trying to serialize multiple objects with repeated calls to
      :func:`dump` using the same *fp* will result in an invalid JSON file.

   .. note::

      Keys in key/value pairs of JSON are always of the type :class:`str`. When
      a dictionary is converted into JSON, all the keys of the dictionary are
      coerced to strings. As a result of this, if a dictionary is converted
      into JSON and then back into a dictionary, the dictionary may not equal
      the original one. That is, ``loads(dumps(x)) != x`` if x has non-string
      keys.

.. function:: load(fp, cls=None, object_hook=None, parse_float=None, parse_int=None, parse_constant=None, object_pairs_hook=None, **kw)

   Deserialize *fp* (a ``.read()``-supporting :term:`file-like object`
   containing a JSON document) to a Python object using this :ref:`conversion
   table <json-to-py-table>`.

   *object_hook* is an optional function that will be called with the result of
   any object literal decoded (a :class:`dict`).  The return value of
   *object_hook* will be used instead of the :class:`dict`.  This feature can be used
   to implement custom decoders (e.g. `JSON-RPC <http://www.jsonrpc.org>`_
   class hinting).

   *object_pairs_hook* is an optional function that will be called with the
   result of any object literal decoded with an ordered list of pairs.  The
   return value of *object_pairs_hook* will be used instead of the
   :class:`dict`.  This feature can be used to implement custom decoders that
   rely on the order that the key and value pairs are decoded (for example,
   :func:`collections.OrderedDict` will remember the order of insertion). If
   *object_hook* is also defined, the *object_pairs_hook* takes priority.

   .. versionchanged:: 3.1
      Added support for *object_pairs_hook*.

   *parse_float*, if specified, will be called with the string of every JSON
   float to be decoded.  By default, this is equivalent to ``float(num_str)``.
   This can be used to use another datatype or parser for JSON floats
   (e.g. :class:`decimal.Decimal`).

   *parse_int*, if specified, will be called with the string of every JSON int
   to be decoded.  By default, this is equivalent to ``int(num_str)``.  This can
   be used to use another datatype or parser for JSON integers
   (e.g. :class:`float`).

   *parse_constant*, if specified, will be called with one of the following
   strings: ``'-Infinity'``, ``'Infinity'``, ``'NaN'``.
   This can be used to raise an exception if invalid JSON numbers
   are encountered.

   .. versionchanged:: 3.1
      *parse_constant* doesn't get called on 'null', 'true', 'false' anymore.

   To use a custom :class:`JSONDecoder` subclass, specify it with the ``cls``
   kwarg; otherwise :class:`JSONDecoder` is used.  Additional keyword arguments
   will be passed to the constructor of the class.

   If the data being deserialized is not a valid JSON document, a
   :exc:`ValueError` will be raised.

.. function:: loads(s, encoding=None, cls=None, object_hook=None, parse_float=None, parse_int=None, parse_constant=None, object_pairs_hook=None, **kw)

   Deserialize *s* (a :class:`str` instance containing a JSON document) to a
   Python object using this :ref:`conversion table <json-to-py-table>`.

   The other arguments have the same meaning as in :func:`load`, except
   *encoding* which is ignored and deprecated.

   If the data being deserialized is not a valid JSON document, a
   :exc:`ValueError` will be raised.

Encoders and Decoders
---------------------

.. class:: JSONDecoder(object_hook=None, parse_float=None, parse_int=None, parse_constant=None, strict=True, object_pairs_hook=None)

   Simple JSON decoder.

   Performs the following translations in decoding by default:

   .. _json-to-py-table:

   +---------------+-------------------+
   | JSON          | Python            |
   +===============+===================+
   | object        | dict              |
   +---------------+-------------------+
   | array         | list              |
   +---------------+-------------------+
   | string        | str               |
   +---------------+-------------------+
   | number (int)  | int               |
   +---------------+-------------------+
   | number (real) | float             |
   +---------------+-------------------+
   | true          | True              |
   +---------------+-------------------+
   | false         | False             |
   +---------------+-------------------+
   | null          | None              |
   +---------------+-------------------+

   It also understands ``NaN``, ``Infinity``, and ``-Infinity`` as their
   corresponding ``float`` values, which is outside the JSON spec.

   *object_hook*, if specified, will be called with the result of every JSON
   object decoded and its return value will be used in place of the given
   :class:`dict`.  This can be used to provide custom deserializations (e.g. to
   support JSON-RPC class hinting).

   *object_pairs_hook*, if specified will be called with the result of every
   JSON object decoded with an ordered list of pairs.  The return value of
   *object_pairs_hook* will be used instead of the :class:`dict`.  This
   feature can be used to implement custom decoders that rely on the order
   that the key and value pairs are decoded (for example,
   :func:`collections.OrderedDict` will remember the order of insertion). If
   *object_hook* is also defined, the *object_pairs_hook* takes priority.

   .. versionchanged:: 3.1
      Added support for *object_pairs_hook*.

   *parse_float*, if specified, will be called with the string of every JSON
   float to be decoded.  By default, this is equivalent to ``float(num_str)``.
   This can be used to use another datatype or parser for JSON floats
   (e.g. :class:`decimal.Decimal`).

   *parse_int*, if specified, will be called with the string of every JSON int
   to be decoded.  By default, this is equivalent to ``int(num_str)``.  This can
   be used to use another datatype or parser for JSON integers
   (e.g. :class:`float`).

   *parse_constant*, if specified, will be called with one of the following
   strings: ``'-Infinity'``, ``'Infinity'``, ``'NaN'``, ``'null'``, ``'true'``,
   ``'false'``.  This can be used to raise an exception if invalid JSON numbers
   are encountered.

   If *strict* is ``False`` (``True`` is the default), then control characters
   will be allowed inside strings.  Control characters in this context are
   those with character codes in the 0-31 range, including ``'\t'`` (tab),
   ``'\n'``, ``'\r'`` and ``'\0'``.

   If the data being deserialized is not a valid JSON document, a
   :exc:`ValueError` will be raised.

   .. method:: decode(s)

      Return the Python representation of *s* (a :class:`str` instance
      containing a JSON document)

   .. method:: raw_decode(s)

      Decode a JSON document from *s* (a :class:`str` beginning with a
      JSON document) and return a 2-tuple of the Python representation
      and the index in *s* where the document ended.

      This can be used to decode a JSON document from a string that may have
      extraneous data at the end.


.. class:: JSONEncoder(skipkeys=False, ensure_ascii=True, check_circular=True, allow_nan=True, sort_keys=False, indent=None, separators=None, default=None)

   Extensible JSON encoder for Python data structures.

   Supports the following objects and types by default:

   .. _py-to-json-table:

   +----------------------------------------+---------------+
   | Python                                 | JSON          |
   +========================================+===============+
   | dict                                   | object        |
   +----------------------------------------+---------------+
   | list, tuple                            | array         |
   +----------------------------------------+---------------+
   | str                                    | string        |
   +----------------------------------------+---------------+
   | int, float, int- & float-derived Enums | number        |
   +----------------------------------------+---------------+
   | True                                   | true          |
   +----------------------------------------+---------------+
   | False                                  | false         |
   +----------------------------------------+---------------+
   | None                                   | null          |
   +----------------------------------------+---------------+

   .. versionchanged:: 3.4
      Added support for int- and float-derived Enum classes.

   To extend this to recognize other objects, subclass and implement a
   :meth:`default` method with another method that returns a serializable object
   for ``o`` if possible, otherwise it should call the superclass implementation
   (to raise :exc:`TypeError`).

   If *skipkeys* is ``False`` (the default), then it is a :exc:`TypeError` to
   attempt encoding of keys that are not str, int, float or None.  If
   *skipkeys* is ``True``, such items are simply skipped.

   If *ensure_ascii* is ``True`` (the default), the output is guaranteed to
   have all incoming non-ASCII characters escaped.  If *ensure_ascii* is
   ``False``, these characters will be output as-is.

   If *check_circular* is ``True`` (the default), then lists, dicts, and custom
   encoded objects will be checked for circular references during encoding to
   prevent an infinite recursion (which would cause an :exc:`OverflowError`).
   Otherwise, no such check takes place.

   If *allow_nan* is ``True`` (the default), then ``NaN``, ``Infinity``, and
   ``-Infinity`` will be encoded as such.  This behavior is not JSON
   specification compliant, but is consistent with most JavaScript based
   encoders and decoders.  Otherwise, it will be a :exc:`ValueError` to encode
   such floats.

   If *sort_keys* is ``True`` (default ``False``), then the output of dictionaries
   will be sorted by key; this is useful for regression tests to ensure that
   JSON serializations can be compared on a day-to-day basis.

   If *indent* is a non-negative integer or string, then JSON array elements and
   object members will be pretty-printed with that indent level.  An indent level
   of 0, negative, or ``""`` will only insert newlines.  ``None`` (the default)
   selects the most compact representation. Using a positive integer indent
   indents that many spaces per level.  If *indent* is a string (such as ``"\t"``),
   that string is used to indent each level.

   .. versionchanged:: 3.2
      Allow strings for *indent* in addition to integers.

   If specified, *separators* should be an ``(item_separator, key_separator)``
   tuple.  The default is ``(', ', ': ')`` if *indent* is ``None`` and
   ``(',', ': ')`` otherwise.  To get the most compact JSON representation,
   you should specify ``(',', ':')`` to eliminate whitespace.

   .. versionchanged:: 3.4
      Use ``(',', ': ')`` as default if *indent* is not ``None``.

   If specified, *default* is a function that gets called for objects that can't
   otherwise be serialized.  It should return a JSON encodable version of the
   object or raise a :exc:`TypeError`.


   .. method:: default(o)

      Implement this method in a subclass such that it returns a serializable
      object for *o*, or calls the base implementation (to raise a
      :exc:`TypeError`).

      For example, to support arbitrary iterators, you could implement default
      like this::

         def default(self, o):
            try:
                iterable = iter(o)
            except TypeError:
                pass
            else:
                return list(iterable)
            # Let the base class default method raise the TypeError
            return json.JSONEncoder.default(self, o)


   .. method:: encode(o)

      Return a JSON string representation of a Python data structure, *o*.  For
      example::

        >>> json.JSONEncoder().encode({"foo": ["bar", "baz"]})
        '{"foo": ["bar", "baz"]}'


   .. method:: iterencode(o)

      Encode the given object, *o*, and yield each string representation as
      available.  For example::

            for chunk in json.JSONEncoder().iterencode(bigobject):
                mysocket.write(chunk)


Standard Compliance
-------------------

The JSON format is specified by :rfc:`4627`.  This section details this
module's level of compliance with the RFC.  For simplicity,
:class:`JSONEncoder` and :class:`JSONDecoder` subclasses, and parameters other
than those explicitly mentioned, are not considered.

This module does not comply with the RFC in a strict fashion, implementing some
extensions that are valid JavaScript but not valid JSON.  In particular:

- Top-level non-object, non-array values are accepted and output;
- Infinite and NaN number values are accepted and output;
- Repeated names within an object are accepted, and only the value of the last
  name-value pair is used.

Since the RFC permits RFC-compliant parsers to accept input texts that are not
RFC-compliant, this module's deserializer is technically RFC-compliant under
default settings.

Character Encodings
^^^^^^^^^^^^^^^^^^^

The RFC recommends that JSON be represented using either UTF-8, UTF-16, or
UTF-32, with UTF-8 being the default.

As permitted, though not required, by the RFC, this module's serializer sets
*ensure_ascii=True* by default, thus escaping the output so that the resulting
strings only contain ASCII characters.

Other than the *ensure_ascii* parameter, this module is defined strictly in
terms of conversion between Python objects and
:class:`Unicode strings <str>`, and thus does not otherwise address the issue
of character encodings.


Top-level Non-Object, Non-Array Values
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The RFC specifies that the top-level value of a JSON text must be either a
JSON object or array (Python :class:`dict` or :class:`list`).  This module's
deserializer also accepts input texts consisting solely of a
JSON null, boolean, number, or string value::

   >>> just_a_json_string = '"spam and eggs"'  # Not by itself a valid JSON text
   >>> json.loads(just_a_json_string)
   'spam and eggs'

This module itself does not include a way to request that such input texts be
regarded as illegal.  Likewise, this module's serializer also accepts single
Python :data:`None`, :class:`bool`, numeric, and :class:`str`
values as input and will generate output texts consisting solely of a top-level
JSON null, boolean, number, or string value without raising an exception::

   >>> neither_a_list_nor_a_dict = "spam and eggs"
   >>> json.dumps(neither_a_list_nor_a_dict)  # The result is not a valid JSON text
   '"spam and eggs"'

This module's serializer does not itself include a way to enforce the
aforementioned constraint.


Infinite and NaN Number Values
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The RFC does not permit the representation of infinite or NaN number values.
Despite that, by default, this module accepts and outputs ``Infinity``,
``-Infinity``, and ``NaN`` as if they were valid JSON number literal values::

   >>> # Neither of these calls raises an exception, but the results are not valid JSON
   >>> json.dumps(float('-inf'))
   '-Infinity'
   >>> json.dumps(float('nan'))
   'NaN'
   >>> # Same when deserializing
   >>> json.loads('-Infinity')
   -inf
   >>> json.loads('NaN')
   nan

In the serializer, the *allow_nan* parameter can be used to alter this
behavior.  In the deserializer, the *parse_constant* parameter can be used to
alter this behavior.


Repeated Names Within an Object
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The RFC specifies that the names within a JSON object should be unique, but
does not specify how repeated names in JSON objects should be handled.  By
default, this module does not raise an exception; instead, it ignores all but
the last name-value pair for a given name::

   >>> weird_json = '{"x": 1, "x": 2, "x": 3}'
   >>> json.loads(weird_json)
   {'x': 3}

The *object_pairs_hook* parameter can be used to alter this behavior.
