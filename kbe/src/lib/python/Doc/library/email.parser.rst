:mod:`email.parser`: Parsing email messages
-------------------------------------------

.. module:: email.parser
   :synopsis: Parse flat text email messages to produce a message object structure.


Message object structures can be created in one of two ways: they can be created
from whole cloth by instantiating :class:`~email.message.Message` objects and
stringing them together via :meth:`~email.message.Message.attach` and
:meth:`~email.message.Message.set_payload` calls, or they
can be created by parsing a flat text representation of the email message.

The :mod:`email` package provides a standard parser that understands most email
document structures, including MIME documents.  You can pass the parser a string
or a file object, and the parser will return to you the root
:class:`~email.message.Message` instance of the object structure.  For simple,
non-MIME messages the payload of this root object will likely be a string
containing the text of the message.  For MIME messages, the root object will
return ``True`` from its :meth:`~email.message.Message.is_multipart` method, and
the subparts can be accessed via the :meth:`~email.message.Message.get_payload`
and :meth:`~email.message.Message.walk` methods.

There are actually two parser interfaces available for use, the classic
:class:`Parser` API and the incremental :class:`FeedParser` API.  The classic
:class:`Parser` API is fine if you have the entire text of the message in memory
as a string, or if the entire message lives in a file on the file system.
:class:`FeedParser` is more appropriate for when you're reading the message from
a stream which might block waiting for more input (e.g. reading an email message
from a socket).  The :class:`FeedParser` can consume and parse the message
incrementally, and only returns the root object when you close the parser [#]_.

Note that the parser can be extended in limited ways, and of course you can
implement your own parser completely from scratch.  There is no magical
connection between the :mod:`email` package's bundled parser and the
:class:`~email.message.Message` class, so your custom parser can create message
object trees any way it finds necessary.


FeedParser API
^^^^^^^^^^^^^^

The :class:`FeedParser`, imported from the :mod:`email.feedparser` module,
provides an API that is conducive to incremental parsing of email messages, such
as would be necessary when reading the text of an email message from a source
that can block (e.g. a socket).  The :class:`FeedParser` can of course be used
to parse an email message fully contained in a string or a file, but the classic
:class:`Parser` API may be more convenient for such use cases.  The semantics
and results of the two parser APIs are identical.

The :class:`FeedParser`'s API is simple; you create an instance, feed it a bunch
of text until there's no more to feed it, then close the parser to retrieve the
root message object.  The :class:`FeedParser` is extremely accurate when parsing
standards-compliant messages, and it does a very good job of parsing
non-compliant messages, providing information about how a message was deemed
broken.  It will populate a message object's *defects* attribute with a list of
any problems it found in a message.  See the :mod:`email.errors` module for the
list of defects that it can find.

Here is the API for the :class:`FeedParser`:


.. class:: FeedParser(_factory=email.message.Message, *, policy=policy.compat32)

   Create a :class:`FeedParser` instance.  Optional *_factory* is a no-argument
   callable that will be called whenever a new message object is needed.  It
   defaults to the :class:`email.message.Message` class.

   If *policy* is specified (it must be an instance of a :mod:`~email.policy`
   class) use the rules it specifies to update the representation of the
   message.  If *policy* is not set, use the :class:`compat32
   <email.policy.Compat32>` policy, which maintains backward compatibility with
   the Python 3.2 version of the email package.  For more information see the
   :mod:`~email.policy` documentation.

   .. versionchanged:: 3.3 Added the *policy* keyword.

   .. method:: feed(data)

      Feed the :class:`FeedParser` some more data.  *data* should be a string
      containing one or more lines.  The lines can be partial and the
      :class:`FeedParser` will stitch such partial lines together properly.  The
      lines in the string can have any of the common three line endings,
      carriage return, newline, or carriage return and newline (they can even be
      mixed).

   .. method:: close()

      Closing a :class:`FeedParser` completes the parsing of all previously fed
      data, and returns the root message object.  It is undefined what happens
      if you feed more data to a closed :class:`FeedParser`.


.. class:: BytesFeedParser(_factory=email.message.Message)

   Works exactly like :class:`FeedParser` except that the input to the
   :meth:`~FeedParser.feed` method must be bytes and not string.

   .. versionadded:: 3.2


Parser class API
^^^^^^^^^^^^^^^^

The :class:`Parser` class, imported from the :mod:`email.parser` module,
provides an API that can be used to parse a message when the complete contents
of the message are available in a string or file.  The :mod:`email.parser`
module also provides header-only parsers, called :class:`HeaderParser` and
:class:`BytesHeaderParser`, which can be used if you're only interested in the
headers of the message.  :class:`HeaderParser` and :class:`BytesHeaderParser`
can be much faster in these situations, since they do not attempt to parse the
message body, instead setting the payload to the raw body as a string.  They
have the same API as the :class:`Parser` and :class:`BytesParser` classes.

.. versionadded:: 3.3
   The BytesHeaderParser class.


.. class:: Parser(_class=email.message.Message, *, policy=policy.compat32)

   The constructor for the :class:`Parser` class takes an optional argument
   *_class*.  This must be a callable factory (such as a function or a class), and
   it is used whenever a sub-message object needs to be created.  It defaults to
   :class:`~email.message.Message` (see :mod:`email.message`).  The factory will
   be called without arguments.

   If *policy* is specified (it must be an instance of a :mod:`~email.policy`
   class) use the rules it specifies to update the representation of the
   message.  If *policy* is not set, use the :class:`compat32
   <email.policy.Compat32>` policy, which maintains backward compatibility with
   the Python 3.2 version of the email package.  For more information see the
   :mod:`~email.policy` documentation.

   .. versionchanged:: 3.3
      Removed the *strict* argument that was deprecated in 2.4.  Added the
      *policy* keyword.

   The other public :class:`Parser` methods are:


   .. method:: parse(fp, headersonly=False)

      Read all the data from the file-like object *fp*, parse the resulting
      text, and return the root message object.  *fp* must support both the
      :meth:`~io.TextIOBase.readline` and the :meth:`~io.TextIOBase.read`
      methods on file-like objects.

      The text contained in *fp* must be formatted as a block of :rfc:`2822`
      style headers and header continuation lines, optionally preceded by a
      envelope header.  The header block is terminated either by the end of the
      data or by a blank line.  Following the header block is the body of the
      message (which may contain MIME-encoded subparts).

      Optional *headersonly* is a flag specifying whether to stop parsing after
      reading the headers or not.  The default is ``False``, meaning it parses
      the entire contents of the file.

   .. method:: parsestr(text, headersonly=False)

      Similar to the :meth:`parse` method, except it takes a string object
      instead of a file-like object.  Calling this method on a string is exactly
      equivalent to wrapping *text* in a :class:`~io.StringIO` instance first and
      calling :meth:`parse`.

      Optional *headersonly* is as with the :meth:`parse` method.


.. class:: BytesParser(_class=email.message.Message, *, policy=policy.compat32)

   This class is exactly parallel to :class:`Parser`, but handles bytes input.
   The *_class* and *strict* arguments are interpreted in the same way as for
   the :class:`Parser` constructor.

   If *policy* is specified (it must be an instance of a :mod:`~email.policy`
   class) use the rules it specifies to update the representation of the
   message.  If *policy* is not set, use the :class:`compat32
   <email.policy.Compat32>` policy, which maintains backward compatibility with
   the Python 3.2 version of the email package.  For more information see the
   :mod:`~email.policy` documentation.

   .. versionchanged:: 3.3
      Removed the *strict* argument.  Added the *policy* keyword.

   .. method:: parse(fp, headeronly=False)

      Read all the data from the binary file-like object *fp*, parse the
      resulting bytes, and return the message object.  *fp* must support
      both the :meth:`~io.IOBase.readline` and the :meth:`~io.IOBase.read`
      methods on file-like objects.

      The bytes contained in *fp* must be formatted as a block of :rfc:`2822`
      style headers and header continuation lines, optionally preceded by a
      envelope header.  The header block is terminated either by the end of the
      data or by a blank line.  Following the header block is the body of the
      message (which may contain MIME-encoded subparts, including subparts
      with a :mailheader:`Content-Transfer-Encoding` of ``8bit``.

      Optional *headersonly* is a flag specifying whether to stop parsing after
      reading the headers or not.  The default is ``False``, meaning it parses
      the entire contents of the file.

   .. method:: parsebytes(bytes, headersonly=False)

      Similar to the :meth:`parse` method, except it takes a byte string object
      instead of a file-like object.  Calling this method on a byte string is
      exactly equivalent to wrapping *text* in a :class:`~io.BytesIO` instance
      first and calling :meth:`parse`.

      Optional *headersonly* is as with the :meth:`parse` method.

   .. versionadded:: 3.2


Since creating a message object structure from a string or a file object is such
a common task, four functions are provided as a convenience.  They are available
in the top-level :mod:`email` package namespace.

.. currentmodule:: email

.. function:: message_from_string(s, _class=email.message.Message, *, \
                                  policy=policy.compat32)

   Return a message object structure from a string.  This is exactly equivalent to
   ``Parser().parsestr(s)``.  *_class* and *policy* are interpreted as
   with the :class:`~email.parser.Parser` class constructor.

   .. versionchanged:: 3.3
      Removed the *strict* argument.  Added the *policy* keyword.

.. function:: message_from_bytes(s, _class=email.message.Message, *, \
                                 policy=policy.compat32)

   Return a message object structure from a byte string.  This is exactly
   equivalent to ``BytesParser().parsebytes(s)``.  Optional *_class* and
   *strict* are interpreted as with the :class:`~email.parser.Parser` class
   constructor.

   .. versionadded:: 3.2
   .. versionchanged:: 3.3
      Removed the *strict* argument.  Added the *policy* keyword.

.. function:: message_from_file(fp, _class=email.message.Message, *, \
                                policy=policy.compat32)

   Return a message object structure tree from an open :term:`file object`.
   This is exactly equivalent to ``Parser().parse(fp)``.  *_class*
   and *policy* are interpreted as with the :class:`~email.parser.Parser` class
   constructor.

   .. versionchanged::
      Removed the *strict* argument.  Added the *policy* keyword.

.. function:: message_from_binary_file(fp, _class=email.message.Message, *, \
                                       policy=policy.compat32)

   Return a message object structure tree from an open binary :term:`file
   object`.  This is exactly equivalent to ``BytesParser().parse(fp)``.
   *_class* and *policy* are interpreted as with the
   :class:`~email.parser.Parser` class constructor.

   .. versionadded:: 3.2
   .. versionchanged:: 3.3
      Removed the *strict* argument.  Added the *policy* keyword.

Here's an example of how you might use this at an interactive Python prompt::

   >>> import email
   >>> msg = email.message_from_string(myString)  # doctest: +SKIP


Additional notes
^^^^^^^^^^^^^^^^

Here are some notes on the parsing semantics:

* Most non-\ :mimetype:`multipart` type messages are parsed as a single message
  object with a string payload.  These objects will return ``False`` for
  :meth:`~email.message.Message.is_multipart`.  Their
  :meth:`~email.message.Message.get_payload` method will return a string object.

* All :mimetype:`multipart` type messages will be parsed as a container message
  object with a list of sub-message objects for their payload.  The outer
  container message will return ``True`` for
  :meth:`~email.message.Message.is_multipart` and their
  :meth:`~email.message.Message.get_payload` method will return the list of
  :class:`~email.message.Message` subparts.

* Most messages with a content type of :mimetype:`message/\*` (e.g.
  :mimetype:`message/delivery-status` and :mimetype:`message/rfc822`) will also be
  parsed as container object containing a list payload of length 1.  Their
  :meth:`~email.message.Message.is_multipart` method will return ``True``.
  The single element in the list payload will be a sub-message object.

* Some non-standards compliant messages may not be internally consistent about
  their :mimetype:`multipart`\ -edness.  Such messages may have a
  :mailheader:`Content-Type` header of type :mimetype:`multipart`, but their
  :meth:`~email.message.Message.is_multipart` method may return ``False``.
  If such messages were parsed with the :class:`~email.parser.FeedParser`,
  they will have an instance of the
  :class:`~email.errors.MultipartInvariantViolationDefect` class in their
  *defects* attribute list.  See :mod:`email.errors` for details.

.. rubric:: Footnotes

.. [#] As of email package version 3.0, introduced in Python 2.4, the classic
   :class:`~email.parser.Parser` was re-implemented in terms of the
   :class:`~email.parser.FeedParser`, so the semantics and results are
   identical between the two parsers.

