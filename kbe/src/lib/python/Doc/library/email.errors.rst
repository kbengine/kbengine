:mod:`email.errors`: Exception and Defect classes
-------------------------------------------------

.. module:: email.errors
   :synopsis: The exception classes used by the email package.


The following exception classes are defined in the :mod:`email.errors` module:


.. exception:: MessageError()

   This is the base class for all exceptions that the :mod:`email` package can
   raise.  It is derived from the standard :exc:`Exception` class and defines no
   additional methods.


.. exception:: MessageParseError()

   This is the base class for exceptions raised by the :class:`~email.parser.Parser`
   class.  It is derived from :exc:`MessageError`.


.. exception:: HeaderParseError()

   Raised under some error conditions when parsing the :rfc:`2822` headers of a
   message, this class is derived from :exc:`MessageParseError`. It can be raised
   from the :meth:`Parser.parse <email.parser.Parser.parse>` or
   :meth:`Parser.parsestr <email.parser.Parser.parsestr>` methods.

   Situations where it can be raised include finding an envelope header after the
   first :rfc:`2822` header of the message, finding a continuation line before the
   first :rfc:`2822` header is found, or finding a line in the headers which is
   neither a header or a continuation line.


.. exception:: BoundaryError()

   Raised under some error conditions when parsing the :rfc:`2822` headers of a
   message, this class is derived from :exc:`MessageParseError`. It can be raised
   from the :meth:`Parser.parse <email.parser.Parser.parse>` or
   :meth:`Parser.parsestr <email.parser.Parser.parsestr>` methods.

   Situations where it can be raised include not being able to find the starting or
   terminating boundary in a :mimetype:`multipart/\*` message when strict parsing
   is used.


.. exception:: MultipartConversionError()

   Raised when a payload is added to a :class:`~email.message.Message` object
   using :meth:`add_payload`, but the payload is already a scalar and the
   message's :mailheader:`Content-Type` main type is not either
   :mimetype:`multipart` or missing.  :exc:`MultipartConversionError` multiply
   inherits from :exc:`MessageError` and the built-in :exc:`TypeError`.

   Since :meth:`Message.add_payload` is deprecated, this exception is rarely
   raised in practice.  However the exception may also be raised if the
   :meth:`~email.message.Message.attach`
   method is called on an instance of a class derived from
   :class:`~email.mime.nonmultipart.MIMENonMultipart` (e.g.
   :class:`~email.mime.image.MIMEImage`).

Here's the list of the defects that the :class:`~email.parser.FeedParser`
can find while parsing messages.  Note that the defects are added to the message
where the problem was found, so for example, if a message nested inside a
:mimetype:`multipart/alternative` had a malformed header, that nested message
object would have a defect, but the containing messages would not.

All defect classes are subclassed from :class:`email.errors.MessageDefect`, but
this class is *not* an exception!

* :class:`NoBoundaryInMultipartDefect` -- A message claimed to be a multipart,
  but had no :mimetype:`boundary` parameter.

* :class:`StartBoundaryNotFoundDefect` -- The start boundary claimed in the
  :mailheader:`Content-Type` header was never found.

* :class:`CloseBoundaryNotFoundDefect` -- A start boundary was found, but
  no corresponding close boundary was ever found.

  .. versionadded:: 3.3

* :class:`FirstHeaderLineIsContinuationDefect` -- The message had a continuation
  line as its first header line.

* :class:`MisplacedEnvelopeHeaderDefect` - A "Unix From" header was found in the
  middle of a header block.

* :class:`MissingHeaderBodySeparatorDefect` - A line was found while parsing
  headers that had no leading white space but contained no ':'.  Parsing
  continues assuming that the line represents the first line of the body.

  .. versionadded:: 3.3

* :class:`MalformedHeaderDefect` -- A header was found that was missing a colon,
  or was otherwise malformed.

  .. deprecated:: 3.3
     This defect has not been used for several Python versions.

* :class:`MultipartInvariantViolationDefect` -- A message claimed to be a
  :mimetype:`multipart`, but no subparts were found.  Note that when a message
  has this defect, its :meth:`~email.message.Message.is_multipart` method may
  return false even though its content type claims to be :mimetype:`multipart`.

* :class:`InvalidBase64PaddingDefect` -- When decoding a block of base64
  enocded bytes, the padding was not correct.  Enough padding is added to
  perform the decode, but the resulting decoded bytes may be invalid.

* :class:`InvalidBase64CharactersDefect` -- When decoding a block of base64
  enocded bytes, characters outside the base64 alphebet were encountered.
  The characters are ignored, but the resulting decoded bytes may be invalid.
