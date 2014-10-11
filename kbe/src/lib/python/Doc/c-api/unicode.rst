.. highlightlang:: c

.. _unicodeobjects:

Unicode Objects and Codecs
--------------------------

.. sectionauthor:: Marc-André Lemburg <mal@lemburg.com>
.. sectionauthor:: Georg Brandl <georg@python.org>

Unicode Objects
^^^^^^^^^^^^^^^

Since the implementation of :pep:`393` in Python 3.3, Unicode objects internally
use a variety of representations, in order to allow handling the complete range
of Unicode characters while staying memory efficient.  There are special cases
for strings where all code points are below 128, 256, or 65536; otherwise, code
points must be below 1114112 (which is the full Unicode range).

:c:type:`Py_UNICODE*` and UTF-8 representations are created on demand and cached
in the Unicode object.  The :c:type:`Py_UNICODE*` representation is deprecated
and inefficient; it should be avoided in performance- or memory-sensitive
situations.

Due to the transition between the old APIs and the new APIs, unicode objects
can internally be in two states depending on how they were created:

* "canonical" unicode objects are all objects created by a non-deprecated
  unicode API.  They use the most efficient representation allowed by the
  implementation.

* "legacy" unicode objects have been created through one of the deprecated
  APIs (typically :c:func:`PyUnicode_FromUnicode`) and only bear the
  :c:type:`Py_UNICODE*` representation; you will have to call
  :c:func:`PyUnicode_READY` on them before calling any other API.


Unicode Type
""""""""""""

These are the basic Unicode object types used for the Unicode implementation in
Python:

.. c:type:: Py_UCS4
            Py_UCS2
            Py_UCS1

   These types are typedefs for unsigned integer types wide enough to contain
   characters of 32 bits, 16 bits and 8 bits, respectively.  When dealing with
   single Unicode characters, use :c:type:`Py_UCS4`.

   .. versionadded:: 3.3


.. c:type:: Py_UNICODE

   This is a typedef of :c:type:`wchar_t`, which is a 16-bit type or 32-bit type
   depending on the platform.

   .. versionchanged:: 3.3
      In previous versions, this was a 16-bit type or a 32-bit type depending on
      whether you selected a "narrow" or "wide" Unicode version of Python at
      build time.


.. c:type:: PyASCIIObject
            PyCompactUnicodeObject
            PyUnicodeObject

   These subtypes of :c:type:`PyObject` represent a Python Unicode object.  In
   almost all cases, they shouldn't be used directly, since all API functions
   that deal with Unicode objects take and return :c:type:`PyObject` pointers.

   .. versionadded:: 3.3


.. c:var:: PyTypeObject PyUnicode_Type

   This instance of :c:type:`PyTypeObject` represents the Python Unicode type.  It
   is exposed to Python code as ``str``.


The following APIs are really C macros and can be used to do fast checks and to
access internal read-only data of Unicode objects:

.. c:function:: int PyUnicode_Check(PyObject *o)

   Return true if the object *o* is a Unicode object or an instance of a Unicode
   subtype.


.. c:function:: int PyUnicode_CheckExact(PyObject *o)

   Return true if the object *o* is a Unicode object, but not an instance of a
   subtype.


.. c:function:: int PyUnicode_READY(PyObject *o)

   Ensure the string object *o* is in the "canonical" representation.  This is
   required before using any of the access macros described below.

   .. XXX expand on when it is not required

   Returns 0 on success and -1 with an exception set on failure, which in
   particular happens if memory allocation fails.

   .. versionadded:: 3.3


.. c:function:: Py_ssize_t PyUnicode_GET_LENGTH(PyObject *o)

   Return the length of the Unicode string, in code points.  *o* has to be a
   Unicode object in the "canonical" representation (not checked).

   .. versionadded:: 3.3


.. c:function:: Py_UCS1* PyUnicode_1BYTE_DATA(PyObject *o)
                Py_UCS2* PyUnicode_2BYTE_DATA(PyObject *o)
                Py_UCS4* PyUnicode_4BYTE_DATA(PyObject *o)

   Return a pointer to the canonical representation cast to UCS1, UCS2 or UCS4
   integer types for direct character access.  No checks are performed if the
   canonical representation has the correct character size; use
   :c:func:`PyUnicode_KIND` to select the right macro.  Make sure
   :c:func:`PyUnicode_READY` has been called before accessing this.

   .. versionadded:: 3.3


.. c:macro:: PyUnicode_WCHAR_KIND
             PyUnicode_1BYTE_KIND
             PyUnicode_2BYTE_KIND
             PyUnicode_4BYTE_KIND

   Return values of the :c:func:`PyUnicode_KIND` macro.

   .. versionadded:: 3.3


.. c:function:: int PyUnicode_KIND(PyObject *o)

   Return one of the PyUnicode kind constants (see above) that indicate how many
   bytes per character this Unicode object uses to store its data.  *o* has to
   be a Unicode object in the "canonical" representation (not checked).

   .. XXX document "0" return value?

   .. versionadded:: 3.3


.. c:function:: void* PyUnicode_DATA(PyObject *o)

   Return a void pointer to the raw unicode buffer.  *o* has to be a Unicode
   object in the "canonical" representation (not checked).

   .. versionadded:: 3.3


.. c:function:: void PyUnicode_WRITE(int kind, void *data, Py_ssize_t index, \
                                     Py_UCS4 value)

   Write into a canonical representation *data* (as obtained with
   :c:func:`PyUnicode_DATA`).  This macro does not do any sanity checks and is
   intended for usage in loops.  The caller should cache the *kind* value and
   *data* pointer as obtained from other macro calls.  *index* is the index in
   the string (starts at 0) and *value* is the new code point value which should
   be written to that location.

   .. versionadded:: 3.3


.. c:function:: Py_UCS4 PyUnicode_READ(int kind, void *data, Py_ssize_t index)

   Read a code point from a canonical representation *data* (as obtained with
   :c:func:`PyUnicode_DATA`).  No checks or ready calls are performed.

   .. versionadded:: 3.3


.. c:function:: Py_UCS4 PyUnicode_READ_CHAR(PyObject *o, Py_ssize_t index)

   Read a character from a Unicode object *o*, which must be in the "canonical"
   representation.  This is less efficient than :c:func:`PyUnicode_READ` if you
   do multiple consecutive reads.

   .. versionadded:: 3.3


.. c:function:: PyUnicode_MAX_CHAR_VALUE(PyObject *o)

   Return the maximum code point that is suitable for creating another string
   based on *o*, which must be in the "canonical" representation.  This is
   always an approximation but more efficient than iterating over the string.

   .. versionadded:: 3.3


.. c:function:: int PyUnicode_ClearFreeList()

   Clear the free list. Return the total number of freed items.


.. c:function:: Py_ssize_t PyUnicode_GET_SIZE(PyObject *o)

   Return the size of the deprecated :c:type:`Py_UNICODE` representation, in
   code units (this includes surrogate pairs as 2 units).  *o* has to be a
   Unicode object (not checked).

   .. deprecated-removed:: 3.3 4.0
      Part of the old-style Unicode API, please migrate to using
      :c:func:`PyUnicode_GET_LENGTH`.


.. c:function:: Py_ssize_t PyUnicode_GET_DATA_SIZE(PyObject *o)

   Return the size of the deprecated :c:type:`Py_UNICODE` representation in
   bytes.  *o* has to be a Unicode object (not checked).

   .. deprecated-removed:: 3.3 4.0
      Part of the old-style Unicode API, please migrate to using
      :c:func:`PyUnicode_GET_LENGTH`.


.. c:function:: Py_UNICODE* PyUnicode_AS_UNICODE(PyObject *o)
                const char* PyUnicode_AS_DATA(PyObject *o)

   Return a pointer to a :c:type:`Py_UNICODE` representation of the object.  The
   ``AS_DATA`` form casts the pointer to :c:type:`const char *`.  *o* has to be
   a Unicode object (not checked).

   .. versionchanged:: 3.3
      This macro is now inefficient -- because in many cases the
      :c:type:`Py_UNICODE` representation does not exist and needs to be created
      -- and can fail (return *NULL* with an exception set).  Try to port the
      code to use the new :c:func:`PyUnicode_nBYTE_DATA` macros or use
      :c:func:`PyUnicode_WRITE` or :c:func:`PyUnicode_READ`.

   .. deprecated-removed:: 3.3 4.0
      Part of the old-style Unicode API, please migrate to using the
      :c:func:`PyUnicode_nBYTE_DATA` family of macros.


Unicode Character Properties
""""""""""""""""""""""""""""

Unicode provides many different character properties. The most often needed ones
are available through these macros which are mapped to C functions depending on
the Python configuration.


.. c:function:: int Py_UNICODE_ISSPACE(Py_UNICODE ch)

   Return 1 or 0 depending on whether *ch* is a whitespace character.


.. c:function:: int Py_UNICODE_ISLOWER(Py_UNICODE ch)

   Return 1 or 0 depending on whether *ch* is a lowercase character.


.. c:function:: int Py_UNICODE_ISUPPER(Py_UNICODE ch)

   Return 1 or 0 depending on whether *ch* is an uppercase character.


.. c:function:: int Py_UNICODE_ISTITLE(Py_UNICODE ch)

   Return 1 or 0 depending on whether *ch* is a titlecase character.


.. c:function:: int Py_UNICODE_ISLINEBREAK(Py_UNICODE ch)

   Return 1 or 0 depending on whether *ch* is a linebreak character.


.. c:function:: int Py_UNICODE_ISDECIMAL(Py_UNICODE ch)

   Return 1 or 0 depending on whether *ch* is a decimal character.


.. c:function:: int Py_UNICODE_ISDIGIT(Py_UNICODE ch)

   Return 1 or 0 depending on whether *ch* is a digit character.


.. c:function:: int Py_UNICODE_ISNUMERIC(Py_UNICODE ch)

   Return 1 or 0 depending on whether *ch* is a numeric character.


.. c:function:: int Py_UNICODE_ISALPHA(Py_UNICODE ch)

   Return 1 or 0 depending on whether *ch* is an alphabetic character.


.. c:function:: int Py_UNICODE_ISALNUM(Py_UNICODE ch)

   Return 1 or 0 depending on whether *ch* is an alphanumeric character.


.. c:function:: int Py_UNICODE_ISPRINTABLE(Py_UNICODE ch)

   Return 1 or 0 depending on whether *ch* is a printable character.
   Nonprintable characters are those characters defined in the Unicode character
   database as "Other" or "Separator", excepting the ASCII space (0x20) which is
   considered printable.  (Note that printable characters in this context are
   those which should not be escaped when :func:`repr` is invoked on a string.
   It has no bearing on the handling of strings written to :data:`sys.stdout` or
   :data:`sys.stderr`.)


These APIs can be used for fast direct character conversions:


.. c:function:: Py_UNICODE Py_UNICODE_TOLOWER(Py_UNICODE ch)

   Return the character *ch* converted to lower case.

   .. deprecated:: 3.3
      This function uses simple case mappings.


.. c:function:: Py_UNICODE Py_UNICODE_TOUPPER(Py_UNICODE ch)

   Return the character *ch* converted to upper case.

   .. deprecated:: 3.3
      This function uses simple case mappings.


.. c:function:: Py_UNICODE Py_UNICODE_TOTITLE(Py_UNICODE ch)

   Return the character *ch* converted to title case.

   .. deprecated:: 3.3
      This function uses simple case mappings.


.. c:function:: int Py_UNICODE_TODECIMAL(Py_UNICODE ch)

   Return the character *ch* converted to a decimal positive integer.  Return
   ``-1`` if this is not possible.  This macro does not raise exceptions.


.. c:function:: int Py_UNICODE_TODIGIT(Py_UNICODE ch)

   Return the character *ch* converted to a single digit integer. Return ``-1`` if
   this is not possible.  This macro does not raise exceptions.


.. c:function:: double Py_UNICODE_TONUMERIC(Py_UNICODE ch)

   Return the character *ch* converted to a double. Return ``-1.0`` if this is not
   possible.  This macro does not raise exceptions.


These APIs can be used to work with surrogates:

.. c:macro:: Py_UNICODE_IS_SURROGATE(ch)

   Check if *ch* is a surrogate (``0xD800 <= ch <= 0xDFFF``).

.. c:macro:: Py_UNICODE_IS_HIGH_SURROGATE(ch)

   Check if *ch* is an high surrogate (``0xD800 <= ch <= 0xDBFF``).

.. c:macro:: Py_UNICODE_IS_LOW_SURROGATE(ch)

   Check if *ch* is a low surrogate (``0xDC00 <= ch <= 0xDFFF``).

.. c:macro:: Py_UNICODE_JOIN_SURROGATES(high, low)

   Join two surrogate characters and return a single Py_UCS4 value.
   *high* and *low* are respectively the leading and trailing surrogates in a
   surrogate pair.


Creating and accessing Unicode strings
""""""""""""""""""""""""""""""""""""""

To create Unicode objects and access their basic sequence properties, use these
APIs:

.. c:function:: PyObject* PyUnicode_New(Py_ssize_t size, Py_UCS4 maxchar)

   Create a new Unicode object.  *maxchar* should be the true maximum code point
   to be placed in the string.  As an approximation, it can be rounded up to the
   nearest value in the sequence 127, 255, 65535, 1114111.

   This is the recommended way to allocate a new Unicode object.  Objects
   created using this function are not resizable.

   .. versionadded:: 3.3


.. c:function:: PyObject* PyUnicode_FromKindAndData(int kind, const void *buffer, \
                                                    Py_ssize_t size)

   Create a new Unicode object with the given *kind* (possible values are
   :c:macro:`PyUnicode_1BYTE_KIND` etc., as returned by
   :c:func:`PyUnicode_KIND`).  The *buffer* must point to an array of *size*
   units of 1, 2 or 4 bytes per character, as given by the kind.

   .. versionadded:: 3.3


.. c:function:: PyObject* PyUnicode_FromStringAndSize(const char *u, Py_ssize_t size)

   Create a Unicode object from the char buffer *u*.  The bytes will be
   interpreted as being UTF-8 encoded.  The buffer is copied into the new
   object. If the buffer is not *NULL*, the return value might be a shared
   object, i.e. modification of the data is not allowed.

   If *u* is *NULL*, this function behaves like :c:func:`PyUnicode_FromUnicode`
   with the buffer set to *NULL*.  This usage is deprecated in favor of
   :c:func:`PyUnicode_New`.


.. c:function:: PyObject *PyUnicode_FromString(const char *u)

   Create a Unicode object from an UTF-8 encoded null-terminated char buffer
   *u*.


.. c:function:: PyObject* PyUnicode_FromFormat(const char *format, ...)

   Take a C :c:func:`printf`\ -style *format* string and a variable number of
   arguments, calculate the size of the resulting Python unicode string and return
   a string with the values formatted into it.  The variable arguments must be C
   types and must correspond exactly to the format characters in the *format*
   ASCII-encoded string. The following format characters are allowed:

   .. % This should be exactly the same as the table in PyErr_Format.
   .. % The descriptions for %zd and %zu are wrong, but the truth is complicated
   .. % because not all compilers support the %z width modifier -- we fake it
   .. % when necessary via interpolating PY_FORMAT_SIZE_T.
   .. % Similar comments apply to the %ll width modifier and
   .. % PY_FORMAT_LONG_LONG.

   .. tabularcolumns:: |l|l|L|

   +-------------------+---------------------+--------------------------------+
   | Format Characters | Type                | Comment                        |
   +===================+=====================+================================+
   | :attr:`%%`        | *n/a*               | The literal % character.       |
   +-------------------+---------------------+--------------------------------+
   | :attr:`%c`        | int                 | A single character,            |
   |                   |                     | represented as an C int.       |
   +-------------------+---------------------+--------------------------------+
   | :attr:`%d`        | int                 | Exactly equivalent to          |
   |                   |                     | ``printf("%d")``.              |
   +-------------------+---------------------+--------------------------------+
   | :attr:`%u`        | unsigned int        | Exactly equivalent to          |
   |                   |                     | ``printf("%u")``.              |
   +-------------------+---------------------+--------------------------------+
   | :attr:`%ld`       | long                | Exactly equivalent to          |
   |                   |                     | ``printf("%ld")``.             |
   +-------------------+---------------------+--------------------------------+
   | :attr:`%li`       | long                | Exactly equivalent to          |
   |                   |                     | ``printf("%li")``.             |
   +-------------------+---------------------+--------------------------------+
   | :attr:`%lu`       | unsigned long       | Exactly equivalent to          |
   |                   |                     | ``printf("%lu")``.             |
   +-------------------+---------------------+--------------------------------+
   | :attr:`%lld`      | long long           | Exactly equivalent to          |
   |                   |                     | ``printf("%lld")``.            |
   +-------------------+---------------------+--------------------------------+
   | :attr:`%lli`      | long long           | Exactly equivalent to          |
   |                   |                     | ``printf("%lli")``.            |
   +-------------------+---------------------+--------------------------------+
   | :attr:`%llu`      | unsigned long long  | Exactly equivalent to          |
   |                   |                     | ``printf("%llu")``.            |
   +-------------------+---------------------+--------------------------------+
   | :attr:`%zd`       | Py_ssize_t          | Exactly equivalent to          |
   |                   |                     | ``printf("%zd")``.             |
   +-------------------+---------------------+--------------------------------+
   | :attr:`%zi`       | Py_ssize_t          | Exactly equivalent to          |
   |                   |                     | ``printf("%zi")``.             |
   +-------------------+---------------------+--------------------------------+
   | :attr:`%zu`       | size_t              | Exactly equivalent to          |
   |                   |                     | ``printf("%zu")``.             |
   +-------------------+---------------------+--------------------------------+
   | :attr:`%i`        | int                 | Exactly equivalent to          |
   |                   |                     | ``printf("%i")``.              |
   +-------------------+---------------------+--------------------------------+
   | :attr:`%x`        | int                 | Exactly equivalent to          |
   |                   |                     | ``printf("%x")``.              |
   +-------------------+---------------------+--------------------------------+
   | :attr:`%s`        | char\*              | A null-terminated C character  |
   |                   |                     | array.                         |
   +-------------------+---------------------+--------------------------------+
   | :attr:`%p`        | void\*              | The hex representation of a C  |
   |                   |                     | pointer. Mostly equivalent to  |
   |                   |                     | ``printf("%p")`` except that   |
   |                   |                     | it is guaranteed to start with |
   |                   |                     | the literal ``0x`` regardless  |
   |                   |                     | of what the platform's         |
   |                   |                     | ``printf`` yields.             |
   +-------------------+---------------------+--------------------------------+
   | :attr:`%A`        | PyObject\*          | The result of calling          |
   |                   |                     | :func:`ascii`.                 |
   +-------------------+---------------------+--------------------------------+
   | :attr:`%U`        | PyObject\*          | A unicode object.              |
   +-------------------+---------------------+--------------------------------+
   | :attr:`%V`        | PyObject\*, char \* | A unicode object (which may be |
   |                   |                     | *NULL*) and a null-terminated  |
   |                   |                     | C character array as a second  |
   |                   |                     | parameter (which will be used, |
   |                   |                     | if the first parameter is      |
   |                   |                     | *NULL*).                       |
   +-------------------+---------------------+--------------------------------+
   | :attr:`%S`        | PyObject\*          | The result of calling          |
   |                   |                     | :c:func:`PyObject_Str`.        |
   +-------------------+---------------------+--------------------------------+
   | :attr:`%R`        | PyObject\*          | The result of calling          |
   |                   |                     | :c:func:`PyObject_Repr`.       |
   +-------------------+---------------------+--------------------------------+

   An unrecognized format character causes all the rest of the format string to be
   copied as-is to the result string, and any extra arguments discarded.

   .. note::

      The `"%lld"` and `"%llu"` format specifiers are only available
      when :const:`HAVE_LONG_LONG` is defined.

   .. note::
      The width formatter unit is number of characters rather than bytes.
      The precision formatter unit is number of bytes for ``"%s"`` and
      ``"%V"`` (if the ``PyObject*`` argument is NULL), and a number of
      characters for ``"%A"``, ``"%U"``, ``"%S"``, ``"%R"`` and ``"%V"``
      (if the ``PyObject*`` argument is not NULL).

   .. versionchanged:: 3.2
      Support for ``"%lld"`` and ``"%llu"`` added.

   .. versionchanged:: 3.3
      Support for ``"%li"``, ``"%lli"`` and ``"%zi"`` added.

   .. versionchanged:: 3.4
      Support width and precision formatter for ``"%s"``, ``"%A"``, ``"%U"``,
      ``"%V"``, ``"%S"``, ``"%R"`` added.


.. c:function:: PyObject* PyUnicode_FromFormatV(const char *format, va_list vargs)

   Identical to :c:func:`PyUnicode_FromFormat` except that it takes exactly two
   arguments.


.. c:function:: PyObject* PyUnicode_FromEncodedObject(PyObject *obj, \
                               const char *encoding, const char *errors)

   Coerce an encoded object *obj* to an Unicode object and return a reference with
   incremented refcount.

   :class:`bytes`, :class:`bytearray` and other char buffer compatible objects
   are decoded according to the given *encoding* and using the error handling
   defined by *errors*. Both can be *NULL* to have the interface use the default
   values (see the next section for details).

   All other objects, including Unicode objects, cause a :exc:`TypeError` to be
   set.

   The API returns *NULL* if there was an error.  The caller is responsible for
   decref'ing the returned objects.


.. c:function:: Py_ssize_t PyUnicode_GetLength(PyObject *unicode)

   Return the length of the Unicode object, in code points.

   .. versionadded:: 3.3


.. c:function:: int PyUnicode_CopyCharacters(PyObject *to, Py_ssize_t to_start, \
                    PyObject *from, Py_ssize_t from_start, Py_ssize_t how_many)

   Copy characters from one Unicode object into another.  This function performs
   character conversion when necessary and falls back to :c:func:`memcpy` if
   possible.  Returns ``-1`` and sets an exception on error, otherwise returns
   ``0``.

   .. versionadded:: 3.3


.. c:function:: Py_ssize_t PyUnicode_Fill(PyObject *unicode, Py_ssize_t start, \
                        Py_ssize_t length, Py_UCS4 fill_char)

   Fill a string with a character: write *fill_char* into
   ``unicode[start:start+length]``.

   Fail if *fill_char* is bigger than the string maximum character, or if the
   string has more than 1 reference.

   Return the number of written character, or return ``-1`` and raise an
   exception on error.

   .. versionadded:: 3.3


.. c:function:: int PyUnicode_WriteChar(PyObject *unicode, Py_ssize_t index, \
                                        Py_UCS4 character)

   Write a character to a string.  The string must have been created through
   :c:func:`PyUnicode_New`.  Since Unicode strings are supposed to be immutable,
   the string must not be shared, or have been hashed yet.

   This function checks that *unicode* is a Unicode object, that the index is
   not out of bounds, and that the object can be modified safely (i.e. that it
   its reference count is one), in contrast to the macro version
   :c:func:`PyUnicode_WRITE_CHAR`.

   .. versionadded:: 3.3


.. c:function:: Py_UCS4 PyUnicode_ReadChar(PyObject *unicode, Py_ssize_t index)

   Read a character from a string.  This function checks that *unicode* is a
   Unicode object and the index is not out of bounds, in contrast to the macro
   version :c:func:`PyUnicode_READ_CHAR`.

   .. versionadded:: 3.3


.. c:function:: PyObject* PyUnicode_Substring(PyObject *str, Py_ssize_t start, \
                                              Py_ssize_t end)

   Return a substring of *str*, from character index *start* (included) to
   character index *end* (excluded).  Negative indices are not supported.

   .. versionadded:: 3.3


.. c:function:: Py_UCS4* PyUnicode_AsUCS4(PyObject *u, Py_UCS4 *buffer, \
                                          Py_ssize_t buflen, int copy_null)

   Copy the string *u* into a UCS4 buffer, including a null character, if
   *copy_null* is set.  Returns *NULL* and sets an exception on error (in
   particular, a :exc:`ValueError` if *buflen* is smaller than the length of
   *u*).  *buffer* is returned on success.

   .. versionadded:: 3.3


.. c:function:: Py_UCS4* PyUnicode_AsUCS4Copy(PyObject *u)

   Copy the string *u* into a new UCS4 buffer that is allocated using
   :c:func:`PyMem_Malloc`.  If this fails, *NULL* is returned with a
   :exc:`MemoryError` set.

   .. versionadded:: 3.3


Deprecated Py_UNICODE APIs
""""""""""""""""""""""""""

.. deprecated-removed:: 3.3 4.0

These API functions are deprecated with the implementation of :pep:`393`.
Extension modules can continue using them, as they will not be removed in Python
3.x, but need to be aware that their use can now cause performance and memory hits.


.. c:function:: PyObject* PyUnicode_FromUnicode(const Py_UNICODE *u, Py_ssize_t size)

   Create a Unicode object from the Py_UNICODE buffer *u* of the given size. *u*
   may be *NULL* which causes the contents to be undefined. It is the user's
   responsibility to fill in the needed data.  The buffer is copied into the new
   object.

   If the buffer is not *NULL*, the return value might be a shared object.
   Therefore, modification of the resulting Unicode object is only allowed when
   *u* is *NULL*.

   If the buffer is *NULL*, :c:func:`PyUnicode_READY` must be called once the
   string content has been filled before using any of the access macros such as
   :c:func:`PyUnicode_KIND`.

   Please migrate to using :c:func:`PyUnicode_FromKindAndData` or
   :c:func:`PyUnicode_New`.


.. c:function:: Py_UNICODE* PyUnicode_AsUnicode(PyObject *unicode)

   Return a read-only pointer to the Unicode object's internal
   :c:type:`Py_UNICODE` buffer, or *NULL* on error. This will create the
   :c:type:`Py_UNICODE*` representation of the object if it is not yet
   available. Note that the resulting :c:type:`Py_UNICODE` string may contain
   embedded null characters, which would cause the string to be truncated when
   used in most C functions.

   Please migrate to using :c:func:`PyUnicode_AsUCS4`,
   :c:func:`PyUnicode_Substring`, :c:func:`PyUnicode_ReadChar` or similar new
   APIs.


.. c:function:: PyObject* PyUnicode_TransformDecimalToASCII(Py_UNICODE *s, Py_ssize_t size)

   Create a Unicode object by replacing all decimal digits in
   :c:type:`Py_UNICODE` buffer of the given *size* by ASCII digits 0--9
   according to their decimal value.  Return *NULL* if an exception occurs.


.. c:function:: Py_UNICODE* PyUnicode_AsUnicodeAndSize(PyObject *unicode, Py_ssize_t *size)

   Like :c:func:`PyUnicode_AsUnicode`, but also saves the :c:func:`Py_UNICODE`
   array length in *size*. Note that the resulting :c:type:`Py_UNICODE*` string
   may contain embedded null characters, which would cause the string to be
   truncated when used in most C functions.

   .. versionadded:: 3.3


.. c:function:: Py_UNICODE* PyUnicode_AsUnicodeCopy(PyObject *unicode)

   Create a copy of a Unicode string ending with a nul character. Return *NULL*
   and raise a :exc:`MemoryError` exception on memory allocation failure,
   otherwise return a new allocated buffer (use :c:func:`PyMem_Free` to free
   the buffer). Note that the resulting :c:type:`Py_UNICODE*` string may
   contain embedded null characters, which would cause the string to be
   truncated when used in most C functions.

   .. versionadded:: 3.2

   Please migrate to using :c:func:`PyUnicode_AsUCS4Copy` or similar new APIs.


.. c:function:: Py_ssize_t PyUnicode_GetSize(PyObject *unicode)

   Return the size of the deprecated :c:type:`Py_UNICODE` representation, in
   code units (this includes surrogate pairs as 2 units).

   Please migrate to using :c:func:`PyUnicode_GetLength`.


.. c:function:: PyObject* PyUnicode_FromObject(PyObject *obj)

   Shortcut for ``PyUnicode_FromEncodedObject(obj, NULL, "strict")`` which is used
   throughout the interpreter whenever coercion to Unicode is needed.


Locale Encoding
"""""""""""""""

The current locale encoding can be used to decode text from the operating
system.

.. c:function:: PyObject* PyUnicode_DecodeLocaleAndSize(const char *str, \
                                                        Py_ssize_t len, \
                                                        const char *errors)

   Decode a string from the current locale encoding. The supported
   error handlers are ``"strict"`` and ``"surrogateescape"``
   (:pep:`383`). The decoder uses ``"strict"`` error handler if
   *errors* is ``NULL``.  *str* must end with a null character but
   cannot contain embedded null characters.

   .. seealso::

      Use :c:func:`PyUnicode_DecodeFSDefaultAndSize` to decode a string from
      :c:data:`Py_FileSystemDefaultEncoding` (the locale encoding read at
      Python startup).

   .. versionadded:: 3.3


.. c:function:: PyObject* PyUnicode_DecodeLocale(const char *str, const char *errors)

   Similar to :c:func:`PyUnicode_DecodeLocaleAndSize`, but compute the string
   length using :c:func:`strlen`.

   .. versionadded:: 3.3


.. c:function:: PyObject* PyUnicode_EncodeLocale(PyObject *unicode, const char *errors)

   Encode a Unicode object to the current locale encoding. The
   supported error handlers are ``"strict"`` and ``"surrogateescape"``
   (:pep:`383`). The encoder uses ``"strict"`` error handler if
   *errors* is ``NULL``. Return a :class:`bytes` object. *str* cannot
   contain embedded null characters.

   .. seealso::

      Use :c:func:`PyUnicode_EncodeFSDefault` to encode a string to
      :c:data:`Py_FileSystemDefaultEncoding` (the locale encoding read at
      Python startup).

   .. versionadded:: 3.3


File System Encoding
""""""""""""""""""""

To encode and decode file names and other environment strings,
:c:data:`Py_FileSystemEncoding` should be used as the encoding, and
``"surrogateescape"`` should be used as the error handler (:pep:`383`). To
encode file names during argument parsing, the ``"O&"`` converter should be
used, passing :c:func:`PyUnicode_FSConverter` as the conversion function:

.. c:function:: int PyUnicode_FSConverter(PyObject* obj, void* result)

   ParseTuple converter: encode :class:`str` objects to :class:`bytes` using
   :c:func:`PyUnicode_EncodeFSDefault`; :class:`bytes` objects are output as-is.
   *result* must be a :c:type:`PyBytesObject*` which must be released when it is
   no longer used.

   .. versionadded:: 3.1


To decode file names during argument parsing, the ``"O&"`` converter should be
used, passing :c:func:`PyUnicode_FSDecoder` as the conversion function:

.. c:function:: int PyUnicode_FSDecoder(PyObject* obj, void* result)

   ParseTuple converter: decode :class:`bytes` objects to :class:`str` using
   :c:func:`PyUnicode_DecodeFSDefaultAndSize`; :class:`str` objects are output
   as-is. *result* must be a :c:type:`PyUnicodeObject*` which must be released
   when it is no longer used.

   .. versionadded:: 3.2


.. c:function:: PyObject* PyUnicode_DecodeFSDefaultAndSize(const char *s, Py_ssize_t size)

   Decode a string using :c:data:`Py_FileSystemDefaultEncoding` and the
   ``"surrogateescape"`` error handler, or ``"strict"`` on Windows.

   If :c:data:`Py_FileSystemDefaultEncoding` is not set, fall back to the
   locale encoding.

   .. seealso::

      :c:data:`Py_FileSystemDefaultEncoding` is initialized at startup from the
      locale encoding and cannot be modified later. If you need to decode a
      string from the current locale encoding, use
      :c:func:`PyUnicode_DecodeLocaleAndSize`.

   .. versionchanged:: 3.2
      Use ``"strict"`` error handler on Windows.


.. c:function:: PyObject* PyUnicode_DecodeFSDefault(const char *s)

   Decode a null-terminated string using :c:data:`Py_FileSystemDefaultEncoding`
   and the ``"surrogateescape"`` error handler, or ``"strict"`` on Windows.

   If :c:data:`Py_FileSystemDefaultEncoding` is not set, fall back to the
   locale encoding.

   Use :c:func:`PyUnicode_DecodeFSDefaultAndSize` if you know the string length.

   .. versionchanged:: 3.2
      Use ``"strict"`` error handler on Windows.


.. c:function:: PyObject* PyUnicode_EncodeFSDefault(PyObject *unicode)

   Encode a Unicode object to :c:data:`Py_FileSystemDefaultEncoding` with the
   ``"surrogateescape"`` error handler, or ``"strict"`` on Windows, and return
   :class:`bytes`. Note that the resulting :class:`bytes` object may contain
   null bytes.

   If :c:data:`Py_FileSystemDefaultEncoding` is not set, fall back to the
   locale encoding.

   .. seealso::

      :c:data:`Py_FileSystemDefaultEncoding` is initialized at startup from the
      locale encoding and cannot be modified later. If you need to encode a
      string to the current locale encoding, use
      :c:func:`PyUnicode_EncodeLocale`.

   .. versionadded:: 3.2


wchar_t Support
"""""""""""""""

:c:type:`wchar_t` support for platforms which support it:

.. c:function:: PyObject* PyUnicode_FromWideChar(const wchar_t *w, Py_ssize_t size)

   Create a Unicode object from the :c:type:`wchar_t` buffer *w* of the given *size*.
   Passing -1 as the *size* indicates that the function must itself compute the length,
   using wcslen.
   Return *NULL* on failure.


.. c:function:: Py_ssize_t PyUnicode_AsWideChar(PyUnicodeObject *unicode, wchar_t *w, Py_ssize_t size)

   Copy the Unicode object contents into the :c:type:`wchar_t` buffer *w*.  At most
   *size* :c:type:`wchar_t` characters are copied (excluding a possibly trailing
   0-termination character).  Return the number of :c:type:`wchar_t` characters
   copied or -1 in case of an error.  Note that the resulting :c:type:`wchar_t*`
   string may or may not be 0-terminated.  It is the responsibility of the caller
   to make sure that the :c:type:`wchar_t*` string is 0-terminated in case this is
   required by the application. Also, note that the :c:type:`wchar_t*` string
   might contain null characters, which would cause the string to be truncated
   when used with most C functions.


.. c:function:: wchar_t* PyUnicode_AsWideCharString(PyObject *unicode, Py_ssize_t *size)

   Convert the Unicode object to a wide character string. The output string
   always ends with a nul character. If *size* is not *NULL*, write the number
   of wide characters (excluding the trailing 0-termination character) into
   *\*size*.

   Returns a buffer allocated by :c:func:`PyMem_Alloc` (use
   :c:func:`PyMem_Free` to free it) on success. On error, returns *NULL*,
   *\*size* is undefined and raises a :exc:`MemoryError`. Note that the
   resulting :c:type:`wchar_t` string might contain null characters, which
   would cause the string to be truncated when used with most C functions.

   .. versionadded:: 3.2


UCS4 Support
""""""""""""

.. versionadded:: 3.3

.. XXX are these meant to be public?

.. c:function:: size_t Py_UCS4_strlen(const Py_UCS4 *u)
                Py_UCS4* Py_UCS4_strcpy(Py_UCS4 *s1, const Py_UCS4 *s2)
                Py_UCS4* Py_UCS4_strncpy(Py_UCS4 *s1, const Py_UCS4 *s2, size_t n)
                Py_UCS4* Py_UCS4_strcat(Py_UCS4 *s1, const Py_UCS4 *s2)
                int Py_UCS4_strcmp(const Py_UCS4 *s1, const Py_UCS4 *s2)
                int Py_UCS4_strncmp(const Py_UCS4 *s1, const Py_UCS4 *s2, size_t n)
                Py_UCS4* Py_UCS4_strchr(const Py_UCS4 *s, Py_UCS4 c)
                Py_UCS4* Py_UCS4_strrchr(const Py_UCS4 *s, Py_UCS4 c)

   These utility functions work on strings of :c:type:`Py_UCS4` characters and
   otherwise behave like the C standard library functions with the same name.


.. _builtincodecs:

Built-in Codecs
^^^^^^^^^^^^^^^

Python provides a set of built-in codecs which are written in C for speed. All of
these codecs are directly usable via the following functions.

Many of the following APIs take two arguments encoding and errors, and they
have the same semantics as the ones of the built-in :func:`str` string object
constructor.

Setting encoding to *NULL* causes the default encoding to be used
which is ASCII.  The file system calls should use
:c:func:`PyUnicode_FSConverter` for encoding file names. This uses the
variable :c:data:`Py_FileSystemDefaultEncoding` internally. This
variable should be treated as read-only: on some systems, it will be a
pointer to a static string, on others, it will change at run-time
(such as when the application invokes setlocale).

Error handling is set by errors which may also be set to *NULL* meaning to use
the default handling defined for the codec.  Default error handling for all
built-in codecs is "strict" (:exc:`ValueError` is raised).

The codecs all use a similar interface.  Only deviation from the following
generic ones are documented for simplicity.


Generic Codecs
""""""""""""""

These are the generic codec APIs:


.. c:function:: PyObject* PyUnicode_Decode(const char *s, Py_ssize_t size, \
                              const char *encoding, const char *errors)

   Create a Unicode object by decoding *size* bytes of the encoded string *s*.
   *encoding* and *errors* have the same meaning as the parameters of the same name
   in the :func:`str` built-in function.  The codec to be used is looked up
   using the Python codec registry.  Return *NULL* if an exception was raised by
   the codec.


.. c:function:: PyObject* PyUnicode_AsEncodedString(PyObject *unicode, \
                              const char *encoding, const char *errors)

   Encode a Unicode object and return the result as Python bytes object.
   *encoding* and *errors* have the same meaning as the parameters of the same
   name in the Unicode :meth:`~str.encode` method. The codec to be used is looked up
   using the Python codec registry. Return *NULL* if an exception was raised by
   the codec.


.. c:function:: PyObject* PyUnicode_Encode(const Py_UNICODE *s, Py_ssize_t size, \
                              const char *encoding, const char *errors)

   Encode the :c:type:`Py_UNICODE` buffer *s* of the given *size* and return a Python
   bytes object.  *encoding* and *errors* have the same meaning as the
   parameters of the same name in the Unicode :meth:`~str.encode` method.  The codec
   to be used is looked up using the Python codec registry.  Return *NULL* if an
   exception was raised by the codec.

   .. deprecated-removed:: 3.3 4.0
      Part of the old-style :c:type:`Py_UNICODE` API; please migrate to using
      :c:func:`PyUnicode_AsEncodedString`.


UTF-8 Codecs
""""""""""""

These are the UTF-8 codec APIs:


.. c:function:: PyObject* PyUnicode_DecodeUTF8(const char *s, Py_ssize_t size, const char *errors)

   Create a Unicode object by decoding *size* bytes of the UTF-8 encoded string
   *s*. Return *NULL* if an exception was raised by the codec.


.. c:function:: PyObject* PyUnicode_DecodeUTF8Stateful(const char *s, Py_ssize_t size, \
                              const char *errors, Py_ssize_t *consumed)

   If *consumed* is *NULL*, behave like :c:func:`PyUnicode_DecodeUTF8`. If
   *consumed* is not *NULL*, trailing incomplete UTF-8 byte sequences will not be
   treated as an error. Those bytes will not be decoded and the number of bytes
   that have been decoded will be stored in *consumed*.


.. c:function:: PyObject* PyUnicode_AsUTF8String(PyObject *unicode)

   Encode a Unicode object using UTF-8 and return the result as Python bytes
   object.  Error handling is "strict".  Return *NULL* if an exception was
   raised by the codec.


.. c:function:: char* PyUnicode_AsUTF8AndSize(PyObject *unicode, Py_ssize_t *size)

   Return a pointer to the default encoding (UTF-8) of the Unicode object, and
   store the size of the encoded representation (in bytes) in *size*.  *size*
   can be *NULL*, in this case no size will be stored.

   In the case of an error, *NULL* is returned with an exception set and no
   *size* is stored.

   This caches the UTF-8 representation of the string in the Unicode object, and
   subsequent calls will return a pointer to the same buffer.  The caller is not
   responsible for deallocating the buffer.

   .. versionadded:: 3.3


.. c:function:: char* PyUnicode_AsUTF8(PyObject *unicode)

   As :c:func:`PyUnicode_AsUTF8AndSize`, but does not store the size.

   .. versionadded:: 3.3


.. c:function:: PyObject* PyUnicode_EncodeUTF8(const Py_UNICODE *s, Py_ssize_t size, const char *errors)

   Encode the :c:type:`Py_UNICODE` buffer *s* of the given *size* using UTF-8 and
   return a Python bytes object.  Return *NULL* if an exception was raised by
   the codec.

   .. deprecated-removed:: 3.3 4.0
      Part of the old-style :c:type:`Py_UNICODE` API; please migrate to using
      :c:func:`PyUnicode_AsUTF8String` or :c:func:`PyUnicode_AsUTF8AndSize`.


UTF-32 Codecs
"""""""""""""

These are the UTF-32 codec APIs:


.. c:function:: PyObject* PyUnicode_DecodeUTF32(const char *s, Py_ssize_t size, \
                              const char *errors, int *byteorder)

   Decode *size* bytes from a UTF-32 encoded buffer string and return the
   corresponding Unicode object.  *errors* (if non-*NULL*) defines the error
   handling. It defaults to "strict".

   If *byteorder* is non-*NULL*, the decoder starts decoding using the given byte
   order::

      *byteorder == -1: little endian
      *byteorder == 0:  native order
      *byteorder == 1:  big endian

   If ``*byteorder`` is zero, and the first four bytes of the input data are a
   byte order mark (BOM), the decoder switches to this byte order and the BOM is
   not copied into the resulting Unicode string.  If ``*byteorder`` is ``-1`` or
   ``1``, any byte order mark is copied to the output.

   After completion, *\*byteorder* is set to the current byte order at the end
   of input data.

   If *byteorder* is *NULL*, the codec starts in native order mode.

   Return *NULL* if an exception was raised by the codec.


.. c:function:: PyObject* PyUnicode_DecodeUTF32Stateful(const char *s, Py_ssize_t size, \
                              const char *errors, int *byteorder, Py_ssize_t *consumed)

   If *consumed* is *NULL*, behave like :c:func:`PyUnicode_DecodeUTF32`. If
   *consumed* is not *NULL*, :c:func:`PyUnicode_DecodeUTF32Stateful` will not treat
   trailing incomplete UTF-32 byte sequences (such as a number of bytes not divisible
   by four) as an error. Those bytes will not be decoded and the number of bytes
   that have been decoded will be stored in *consumed*.


.. c:function:: PyObject* PyUnicode_AsUTF32String(PyObject *unicode)

   Return a Python byte string using the UTF-32 encoding in native byte
   order. The string always starts with a BOM mark.  Error handling is "strict".
   Return *NULL* if an exception was raised by the codec.


.. c:function:: PyObject* PyUnicode_EncodeUTF32(const Py_UNICODE *s, Py_ssize_t size, \
                              const char *errors, int byteorder)

   Return a Python bytes object holding the UTF-32 encoded value of the Unicode
   data in *s*.  Output is written according to the following byte order::

      byteorder == -1: little endian
      byteorder == 0:  native byte order (writes a BOM mark)
      byteorder == 1:  big endian

   If byteorder is ``0``, the output string will always start with the Unicode BOM
   mark (U+FEFF). In the other two modes, no BOM mark is prepended.

   If *Py_UNICODE_WIDE* is not defined, surrogate pairs will be output
   as a single codepoint.

   Return *NULL* if an exception was raised by the codec.

   .. deprecated-removed:: 3.3 4.0
      Part of the old-style :c:type:`Py_UNICODE` API; please migrate to using
      :c:func:`PyUnicode_AsUTF32String`.


UTF-16 Codecs
"""""""""""""

These are the UTF-16 codec APIs:


.. c:function:: PyObject* PyUnicode_DecodeUTF16(const char *s, Py_ssize_t size, \
                              const char *errors, int *byteorder)

   Decode *size* bytes from a UTF-16 encoded buffer string and return the
   corresponding Unicode object.  *errors* (if non-*NULL*) defines the error
   handling. It defaults to "strict".

   If *byteorder* is non-*NULL*, the decoder starts decoding using the given byte
   order::

      *byteorder == -1: little endian
      *byteorder == 0:  native order
      *byteorder == 1:  big endian

   If ``*byteorder`` is zero, and the first two bytes of the input data are a
   byte order mark (BOM), the decoder switches to this byte order and the BOM is
   not copied into the resulting Unicode string.  If ``*byteorder`` is ``-1`` or
   ``1``, any byte order mark is copied to the output (where it will result in
   either a ``\ufeff`` or a ``\ufffe`` character).

   After completion, *\*byteorder* is set to the current byte order at the end
   of input data.

   If *byteorder* is *NULL*, the codec starts in native order mode.

   Return *NULL* if an exception was raised by the codec.


.. c:function:: PyObject* PyUnicode_DecodeUTF16Stateful(const char *s, Py_ssize_t size, \
                              const char *errors, int *byteorder, Py_ssize_t *consumed)

   If *consumed* is *NULL*, behave like :c:func:`PyUnicode_DecodeUTF16`. If
   *consumed* is not *NULL*, :c:func:`PyUnicode_DecodeUTF16Stateful` will not treat
   trailing incomplete UTF-16 byte sequences (such as an odd number of bytes or a
   split surrogate pair) as an error. Those bytes will not be decoded and the
   number of bytes that have been decoded will be stored in *consumed*.


.. c:function:: PyObject* PyUnicode_AsUTF16String(PyObject *unicode)

   Return a Python byte string using the UTF-16 encoding in native byte
   order. The string always starts with a BOM mark.  Error handling is "strict".
   Return *NULL* if an exception was raised by the codec.


.. c:function:: PyObject* PyUnicode_EncodeUTF16(const Py_UNICODE *s, Py_ssize_t size, \
                              const char *errors, int byteorder)

   Return a Python bytes object holding the UTF-16 encoded value of the Unicode
   data in *s*.  Output is written according to the following byte order::

      byteorder == -1: little endian
      byteorder == 0:  native byte order (writes a BOM mark)
      byteorder == 1:  big endian

   If byteorder is ``0``, the output string will always start with the Unicode BOM
   mark (U+FEFF). In the other two modes, no BOM mark is prepended.

   If *Py_UNICODE_WIDE* is defined, a single :c:type:`Py_UNICODE` value may get
   represented as a surrogate pair. If it is not defined, each :c:type:`Py_UNICODE`
   values is interpreted as an UCS-2 character.

   Return *NULL* if an exception was raised by the codec.

   .. deprecated-removed:: 3.3 4.0
      Part of the old-style :c:type:`Py_UNICODE` API; please migrate to using
      :c:func:`PyUnicode_AsUTF16String`.


UTF-7 Codecs
""""""""""""

These are the UTF-7 codec APIs:


.. c:function:: PyObject* PyUnicode_DecodeUTF7(const char *s, Py_ssize_t size, const char *errors)

   Create a Unicode object by decoding *size* bytes of the UTF-7 encoded string
   *s*.  Return *NULL* if an exception was raised by the codec.


.. c:function:: PyObject* PyUnicode_DecodeUTF7Stateful(const char *s, Py_ssize_t size, \
                              const char *errors, Py_ssize_t *consumed)

   If *consumed* is *NULL*, behave like :c:func:`PyUnicode_DecodeUTF7`.  If
   *consumed* is not *NULL*, trailing incomplete UTF-7 base-64 sections will not
   be treated as an error.  Those bytes will not be decoded and the number of
   bytes that have been decoded will be stored in *consumed*.


.. c:function:: PyObject* PyUnicode_EncodeUTF7(const Py_UNICODE *s, Py_ssize_t size, \
                              int base64SetO, int base64WhiteSpace, const char *errors)

   Encode the :c:type:`Py_UNICODE` buffer of the given size using UTF-7 and
   return a Python bytes object.  Return *NULL* if an exception was raised by
   the codec.

   If *base64SetO* is nonzero, "Set O" (punctuation that has no otherwise
   special meaning) will be encoded in base-64.  If *base64WhiteSpace* is
   nonzero, whitespace will be encoded in base-64.  Both are set to zero for the
   Python "utf-7" codec.

   .. deprecated-removed:: 3.3 4.0
      Part of the old-style :c:type:`Py_UNICODE` API.

   .. XXX replace with what?


Unicode-Escape Codecs
"""""""""""""""""""""

These are the "Unicode Escape" codec APIs:


.. c:function:: PyObject* PyUnicode_DecodeUnicodeEscape(const char *s, \
                              Py_ssize_t size, const char *errors)

   Create a Unicode object by decoding *size* bytes of the Unicode-Escape encoded
   string *s*.  Return *NULL* if an exception was raised by the codec.


.. c:function:: PyObject* PyUnicode_AsUnicodeEscapeString(PyObject *unicode)

   Encode a Unicode object using Unicode-Escape and return the result as Python
   string object.  Error handling is "strict". Return *NULL* if an exception was
   raised by the codec.


.. c:function:: PyObject* PyUnicode_EncodeUnicodeEscape(const Py_UNICODE *s, Py_ssize_t size)

   Encode the :c:type:`Py_UNICODE` buffer of the given *size* using Unicode-Escape and
   return a Python string object.  Return *NULL* if an exception was raised by the
   codec.

   .. deprecated-removed:: 3.3 4.0
      Part of the old-style :c:type:`Py_UNICODE` API; please migrate to using
      :c:func:`PyUnicode_AsUnicodeEscapeString`.


Raw-Unicode-Escape Codecs
"""""""""""""""""""""""""

These are the "Raw Unicode Escape" codec APIs:


.. c:function:: PyObject* PyUnicode_DecodeRawUnicodeEscape(const char *s, \
                              Py_ssize_t size, const char *errors)

   Create a Unicode object by decoding *size* bytes of the Raw-Unicode-Escape
   encoded string *s*.  Return *NULL* if an exception was raised by the codec.


.. c:function:: PyObject* PyUnicode_AsRawUnicodeEscapeString(PyObject *unicode)

   Encode a Unicode object using Raw-Unicode-Escape and return the result as
   Python string object. Error handling is "strict". Return *NULL* if an exception
   was raised by the codec.


.. c:function:: PyObject* PyUnicode_EncodeRawUnicodeEscape(const Py_UNICODE *s, \
                              Py_ssize_t size, const char *errors)

   Encode the :c:type:`Py_UNICODE` buffer of the given *size* using Raw-Unicode-Escape
   and return a Python string object.  Return *NULL* if an exception was raised by
   the codec.

   .. deprecated-removed:: 3.3 4.0
      Part of the old-style :c:type:`Py_UNICODE` API; please migrate to using
      :c:func:`PyUnicode_AsRawUnicodeEscapeString`.


Latin-1 Codecs
""""""""""""""

These are the Latin-1 codec APIs: Latin-1 corresponds to the first 256 Unicode
ordinals and only these are accepted by the codecs during encoding.


.. c:function:: PyObject* PyUnicode_DecodeLatin1(const char *s, Py_ssize_t size, const char *errors)

   Create a Unicode object by decoding *size* bytes of the Latin-1 encoded string
   *s*.  Return *NULL* if an exception was raised by the codec.


.. c:function:: PyObject* PyUnicode_AsLatin1String(PyObject *unicode)

   Encode a Unicode object using Latin-1 and return the result as Python bytes
   object.  Error handling is "strict".  Return *NULL* if an exception was
   raised by the codec.


.. c:function:: PyObject* PyUnicode_EncodeLatin1(const Py_UNICODE *s, Py_ssize_t size, const char *errors)

   Encode the :c:type:`Py_UNICODE` buffer of the given *size* using Latin-1 and
   return a Python bytes object.  Return *NULL* if an exception was raised by
   the codec.

   .. deprecated-removed:: 3.3 4.0
      Part of the old-style :c:type:`Py_UNICODE` API; please migrate to using
      :c:func:`PyUnicode_AsLatin1String`.


ASCII Codecs
""""""""""""

These are the ASCII codec APIs.  Only 7-bit ASCII data is accepted. All other
codes generate errors.


.. c:function:: PyObject* PyUnicode_DecodeASCII(const char *s, Py_ssize_t size, const char *errors)

   Create a Unicode object by decoding *size* bytes of the ASCII encoded string
   *s*.  Return *NULL* if an exception was raised by the codec.


.. c:function:: PyObject* PyUnicode_AsASCIIString(PyObject *unicode)

   Encode a Unicode object using ASCII and return the result as Python bytes
   object.  Error handling is "strict".  Return *NULL* if an exception was
   raised by the codec.


.. c:function:: PyObject* PyUnicode_EncodeASCII(const Py_UNICODE *s, Py_ssize_t size, const char *errors)

   Encode the :c:type:`Py_UNICODE` buffer of the given *size* using ASCII and
   return a Python bytes object.  Return *NULL* if an exception was raised by
   the codec.

   .. deprecated-removed:: 3.3 4.0
      Part of the old-style :c:type:`Py_UNICODE` API; please migrate to using
      :c:func:`PyUnicode_AsASCIIString`.


Character Map Codecs
""""""""""""""""""""

This codec is special in that it can be used to implement many different codecs
(and this is in fact what was done to obtain most of the standard codecs
included in the :mod:`encodings` package). The codec uses mapping to encode and
decode characters.

Decoding mappings must map single string characters to single Unicode
characters, integers (which are then interpreted as Unicode ordinals) or None
(meaning "undefined mapping" and causing an error).

Encoding mappings must map single Unicode characters to single string
characters, integers (which are then interpreted as Latin-1 ordinals) or None
(meaning "undefined mapping" and causing an error).

The mapping objects provided must only support the __getitem__ mapping
interface.

If a character lookup fails with a LookupError, the character is copied as-is
meaning that its ordinal value will be interpreted as Unicode or Latin-1 ordinal
resp. Because of this, mappings only need to contain those mappings which map
characters to different code points.

These are the mapping codec APIs:

.. c:function:: PyObject* PyUnicode_DecodeCharmap(const char *s, Py_ssize_t size, \
                              PyObject *mapping, const char *errors)

   Create a Unicode object by decoding *size* bytes of the encoded string *s* using
   the given *mapping* object.  Return *NULL* if an exception was raised by the
   codec. If *mapping* is *NULL* latin-1 decoding will be done. Else it can be a
   dictionary mapping byte or a unicode string, which is treated as a lookup table.
   Byte values greater that the length of the string and U+FFFE "characters" are
   treated as "undefined mapping".


.. c:function:: PyObject* PyUnicode_AsCharmapString(PyObject *unicode, PyObject *mapping)

   Encode a Unicode object using the given *mapping* object and return the result
   as Python string object.  Error handling is "strict".  Return *NULL* if an
   exception was raised by the codec.

The following codec API is special in that maps Unicode to Unicode.


.. c:function:: PyObject* PyUnicode_TranslateCharmap(const Py_UNICODE *s, Py_ssize_t size, \
                              PyObject *table, const char *errors)

   Translate a :c:type:`Py_UNICODE` buffer of the given *size* by applying a
   character mapping *table* to it and return the resulting Unicode object.  Return
   *NULL* when an exception was raised by the codec.

   The *mapping* table must map Unicode ordinal integers to Unicode ordinal
   integers or None (causing deletion of the character).

   Mapping tables need only provide the :meth:`__getitem__` interface; dictionaries
   and sequences work well.  Unmapped character ordinals (ones which cause a
   :exc:`LookupError`) are left untouched and are copied as-is.

   .. deprecated-removed:: 3.3 4.0
      Part of the old-style :c:type:`Py_UNICODE` API.

   .. XXX replace with what?


.. c:function:: PyObject* PyUnicode_EncodeCharmap(const Py_UNICODE *s, Py_ssize_t size, \
                              PyObject *mapping, const char *errors)

   Encode the :c:type:`Py_UNICODE` buffer of the given *size* using the given
   *mapping* object and return a Python string object. Return *NULL* if an
   exception was raised by the codec.

   .. deprecated-removed:: 3.3 4.0
      Part of the old-style :c:type:`Py_UNICODE` API; please migrate to using
      :c:func:`PyUnicode_AsCharmapString`.


MBCS codecs for Windows
"""""""""""""""""""""""

These are the MBCS codec APIs. They are currently only available on Windows and
use the Win32 MBCS converters to implement the conversions.  Note that MBCS (or
DBCS) is a class of encodings, not just one.  The target encoding is defined by
the user settings on the machine running the codec.

.. c:function:: PyObject* PyUnicode_DecodeMBCS(const char *s, Py_ssize_t size, const char *errors)

   Create a Unicode object by decoding *size* bytes of the MBCS encoded string *s*.
   Return *NULL* if an exception was raised by the codec.


.. c:function:: PyObject* PyUnicode_DecodeMBCSStateful(const char *s, int size, \
                              const char *errors, int *consumed)

   If *consumed* is *NULL*, behave like :c:func:`PyUnicode_DecodeMBCS`. If
   *consumed* is not *NULL*, :c:func:`PyUnicode_DecodeMBCSStateful` will not decode
   trailing lead byte and the number of bytes that have been decoded will be stored
   in *consumed*.


.. c:function:: PyObject* PyUnicode_AsMBCSString(PyObject *unicode)

   Encode a Unicode object using MBCS and return the result as Python bytes
   object.  Error handling is "strict".  Return *NULL* if an exception was
   raised by the codec.


.. c:function:: PyObject* PyUnicode_EncodeCodePage(int code_page, PyObject *unicode, const char *errors)

   Encode the Unicode object using the specified code page and return a Python
   bytes object.  Return *NULL* if an exception was raised by the codec. Use
   :c:data:`CP_ACP` code page to get the MBCS encoder.

   .. versionadded:: 3.3


.. c:function:: PyObject* PyUnicode_EncodeMBCS(const Py_UNICODE *s, Py_ssize_t size, const char *errors)

   Encode the :c:type:`Py_UNICODE` buffer of the given *size* using MBCS and return
   a Python bytes object.  Return *NULL* if an exception was raised by the
   codec.

   .. deprecated-removed:: 3.3 4.0
      Part of the old-style :c:type:`Py_UNICODE` API; please migrate to using
      :c:func:`PyUnicode_AsMBCSString` or :c:func:`PyUnicode_EncodeCodePage`.


Methods & Slots
"""""""""""""""


.. _unicodemethodsandslots:

Methods and Slot Functions
^^^^^^^^^^^^^^^^^^^^^^^^^^

The following APIs are capable of handling Unicode objects and strings on input
(we refer to them as strings in the descriptions) and return Unicode objects or
integers as appropriate.

They all return *NULL* or ``-1`` if an exception occurs.


.. c:function:: PyObject* PyUnicode_Concat(PyObject *left, PyObject *right)

   Concat two strings giving a new Unicode string.


.. c:function:: PyObject* PyUnicode_Split(PyObject *s, PyObject *sep, Py_ssize_t maxsplit)

   Split a string giving a list of Unicode strings.  If *sep* is *NULL*, splitting
   will be done at all whitespace substrings.  Otherwise, splits occur at the given
   separator.  At most *maxsplit* splits will be done.  If negative, no limit is
   set.  Separators are not included in the resulting list.


.. c:function:: PyObject* PyUnicode_Splitlines(PyObject *s, int keepend)

   Split a Unicode string at line breaks, returning a list of Unicode strings.
   CRLF is considered to be one line break.  If *keepend* is 0, the Line break
   characters are not included in the resulting strings.


.. c:function:: PyObject* PyUnicode_Translate(PyObject *str, PyObject *table, \
                              const char *errors)

   Translate a string by applying a character mapping table to it and return the
   resulting Unicode object.

   The mapping table must map Unicode ordinal integers to Unicode ordinal integers
   or None (causing deletion of the character).

   Mapping tables need only provide the :meth:`__getitem__` interface; dictionaries
   and sequences work well.  Unmapped character ordinals (ones which cause a
   :exc:`LookupError`) are left untouched and are copied as-is.

   *errors* has the usual meaning for codecs. It may be *NULL* which indicates to
   use the default error handling.


.. c:function:: PyObject* PyUnicode_Join(PyObject *separator, PyObject *seq)

   Join a sequence of strings using the given *separator* and return the resulting
   Unicode string.


.. c:function:: int PyUnicode_Tailmatch(PyObject *str, PyObject *substr, \
                        Py_ssize_t start, Py_ssize_t end, int direction)

   Return 1 if *substr* matches ``str[start:end]`` at the given tail end
   (*direction* == -1 means to do a prefix match, *direction* == 1 a suffix match),
   0 otherwise. Return ``-1`` if an error occurred.


.. c:function:: Py_ssize_t PyUnicode_Find(PyObject *str, PyObject *substr, \
                               Py_ssize_t start, Py_ssize_t end, int direction)

   Return the first position of *substr* in ``str[start:end]`` using the given
   *direction* (*direction* == 1 means to do a forward search, *direction* == -1 a
   backward search).  The return value is the index of the first match; a value of
   ``-1`` indicates that no match was found, and ``-2`` indicates that an error
   occurred and an exception has been set.


.. c:function:: Py_ssize_t PyUnicode_FindChar(PyObject *str, Py_UCS4 ch, \
                               Py_ssize_t start, Py_ssize_t end, int direction)

   Return the first position of the character *ch* in ``str[start:end]`` using
   the given *direction* (*direction* == 1 means to do a forward search,
   *direction* == -1 a backward search).  The return value is the index of the
   first match; a value of ``-1`` indicates that no match was found, and ``-2``
   indicates that an error occurred and an exception has been set.

   .. versionadded:: 3.3


.. c:function:: Py_ssize_t PyUnicode_Count(PyObject *str, PyObject *substr, \
                               Py_ssize_t start, Py_ssize_t end)

   Return the number of non-overlapping occurrences of *substr* in
   ``str[start:end]``.  Return ``-1`` if an error occurred.


.. c:function:: PyObject* PyUnicode_Replace(PyObject *str, PyObject *substr, \
                              PyObject *replstr, Py_ssize_t maxcount)

   Replace at most *maxcount* occurrences of *substr* in *str* with *replstr* and
   return the resulting Unicode object. *maxcount* == -1 means replace all
   occurrences.


.. c:function:: int PyUnicode_Compare(PyObject *left, PyObject *right)

   Compare two strings and return -1, 0, 1 for less than, equal, and greater than,
   respectively.


.. c:function:: int PyUnicode_CompareWithASCIIString(PyObject *uni, char *string)

   Compare a unicode object, *uni*, with *string* and return -1, 0, 1 for less
   than, equal, and greater than, respectively. It is best to pass only
   ASCII-encoded strings, but the function interprets the input string as
   ISO-8859-1 if it contains non-ASCII characters".


.. c:function:: PyObject* PyUnicode_RichCompare(PyObject *left,  PyObject *right,  int op)

   Rich compare two unicode strings and return one of the following:

   * ``NULL`` in case an exception was raised
   * :const:`Py_True` or :const:`Py_False` for successful comparisons
   * :const:`Py_NotImplemented` in case the type combination is unknown

   Note that :const:`Py_EQ` and :const:`Py_NE` comparisons can cause a
   :exc:`UnicodeWarning` in case the conversion of the arguments to Unicode fails
   with a :exc:`UnicodeDecodeError`.

   Possible values for *op* are :const:`Py_GT`, :const:`Py_GE`, :const:`Py_EQ`,
   :const:`Py_NE`, :const:`Py_LT`, and :const:`Py_LE`.


.. c:function:: PyObject* PyUnicode_Format(PyObject *format, PyObject *args)

   Return a new string object from *format* and *args*; this is analogous to
   ``format % args``.  The *args* argument must be a tuple.


.. c:function:: int PyUnicode_Contains(PyObject *container, PyObject *element)

   Check whether *element* is contained in *container* and return true or false
   accordingly.

   *element* has to coerce to a one element Unicode string. ``-1`` is returned
   if there was an error.


.. c:function:: void PyUnicode_InternInPlace(PyObject **string)

   Intern the argument *\*string* in place.  The argument must be the address of a
   pointer variable pointing to a Python unicode string object.  If there is an
   existing interned string that is the same as *\*string*, it sets *\*string* to
   it (decrementing the reference count of the old string object and incrementing
   the reference count of the interned string object), otherwise it leaves
   *\*string* alone and interns it (incrementing its reference count).
   (Clarification: even though there is a lot of talk about reference counts, think
   of this function as reference-count-neutral; you own the object after the call
   if and only if you owned it before the call.)


.. c:function:: PyObject* PyUnicode_InternFromString(const char *v)

   A combination of :c:func:`PyUnicode_FromString` and
   :c:func:`PyUnicode_InternInPlace`, returning either a new unicode string
   object that has been interned, or a new ("owned") reference to an earlier
   interned string object with the same value.
