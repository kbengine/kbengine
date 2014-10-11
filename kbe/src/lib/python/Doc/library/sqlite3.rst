:mod:`sqlite3` --- DB-API 2.0 interface for SQLite databases
============================================================

.. module:: sqlite3
   :synopsis: A DB-API 2.0 implementation using SQLite 3.x.
.. sectionauthor:: Gerhard Häring <gh@ghaering.de>


SQLite is a C library that provides a lightweight disk-based database that
doesn't require a separate server process and allows accessing the database
using a nonstandard variant of the SQL query language. Some applications can use
SQLite for internal data storage.  It's also possible to prototype an
application using SQLite and then port the code to a larger database such as
PostgreSQL or Oracle.

The sqlite3 module was written by Gerhard Häring.  It provides a SQL interface
compliant with the DB-API 2.0 specification described by :pep:`249`.

To use the module, you must first create a :class:`Connection` object that
represents the database.  Here the data will be stored in the
:file:`example.db` file::

   import sqlite3
   conn = sqlite3.connect('example.db')

You can also supply the special name ``:memory:`` to create a database in RAM.

Once you have a :class:`Connection`, you can create a :class:`Cursor`  object
and call its :meth:`~Cursor.execute` method to perform SQL commands::

   c = conn.cursor()

   # Create table
   c.execute('''CREATE TABLE stocks
                (date text, trans text, symbol text, qty real, price real)''')

   # Insert a row of data
   c.execute("INSERT INTO stocks VALUES ('2006-01-05','BUY','RHAT',100,35.14)")

   # Save (commit) the changes
   conn.commit()

   # We can also close the connection if we are done with it.
   # Just be sure any changes have been committed or they will be lost.
   conn.close()

The data you've saved is persistent and is available in subsequent sessions::

   import sqlite3
   conn = sqlite3.connect('example.db')
   c = conn.cursor()

Usually your SQL operations will need to use values from Python variables.  You
shouldn't assemble your query using Python's string operations because doing so
is insecure; it makes your program vulnerable to an SQL injection attack
(see http://xkcd.com/327/ for humorous example of what can go wrong).

Instead, use the DB-API's parameter substitution.  Put ``?`` as a placeholder
wherever you want to use a value, and then provide a tuple of values as the
second argument to the cursor's :meth:`~Cursor.execute` method.  (Other database
modules may use a different placeholder, such as ``%s`` or ``:1``.) For
example::

   # Never do this -- insecure!
   symbol = 'RHAT'
   c.execute("SELECT * FROM stocks WHERE symbol = '%s'" % symbol)

   # Do this instead
   t = ('RHAT',)
   c.execute('SELECT * FROM stocks WHERE symbol=?', t)
   print(c.fetchone())

   # Larger example that inserts many records at a time
   purchases = [('2006-03-28', 'BUY', 'IBM', 1000, 45.00),
                ('2006-04-05', 'BUY', 'MSFT', 1000, 72.00),
                ('2006-04-06', 'SELL', 'IBM', 500, 53.00),
               ]
   c.executemany('INSERT INTO stocks VALUES (?,?,?,?,?)', purchases)

To retrieve data after executing a SELECT statement, you can either treat the
cursor as an :term:`iterator`, call the cursor's :meth:`~Cursor.fetchone` method to
retrieve a single matching row, or call :meth:`~Cursor.fetchall` to get a list of the
matching rows.

This example uses the iterator form::

   >>> for row in c.execute('SELECT * FROM stocks ORDER BY price'):
           print(row)

   ('2006-01-05', 'BUY', 'RHAT', 100, 35.14)
   ('2006-03-28', 'BUY', 'IBM', 1000, 45.0)
   ('2006-04-06', 'SELL', 'IBM', 500, 53.0)
   ('2006-04-05', 'BUY', 'MSFT', 1000, 72.0)


.. seealso::

   https://github.com/ghaering/pysqlite
      The pysqlite web page -- sqlite3 is developed externally under the name
      "pysqlite".

   http://www.sqlite.org
      The SQLite web page; the documentation describes the syntax and the
      available data types for the supported SQL dialect.

   http://www.w3schools.com/sql/
      Tutorial, reference and examples for learning SQL syntax.

   :pep:`249` - Database API Specification 2.0
      PEP written by Marc-André Lemburg.


.. _sqlite3-module-contents:

Module functions and constants
------------------------------


.. data:: version

   The version number of this module, as a string. This is not the version of
   the SQLite library.


.. data:: version_info

   The version number of this module, as a tuple of integers. This is not the
   version of the SQLite library.


.. data:: sqlite_version

   The version number of the run-time SQLite library, as a string.


.. data:: sqlite_version_info

   The version number of the run-time SQLite library, as a tuple of integers.


.. data:: PARSE_DECLTYPES

   This constant is meant to be used with the *detect_types* parameter of the
   :func:`connect` function.

   Setting it makes the :mod:`sqlite3` module parse the declared type for each
   column it returns.  It will parse out the first word of the declared type,
   i. e.  for "integer primary key", it will parse out "integer", or for
   "number(10)" it will parse out "number". Then for that column, it will look
   into the converters dictionary and use the converter function registered for
   that type there.


.. data:: PARSE_COLNAMES

   This constant is meant to be used with the *detect_types* parameter of the
   :func:`connect` function.

   Setting this makes the SQLite interface parse the column name for each column it
   returns.  It will look for a string formed [mytype] in there, and then decide
   that 'mytype' is the type of the column. It will try to find an entry of
   'mytype' in the converters dictionary and then use the converter function found
   there to return the value. The column name found in :attr:`Cursor.description`
   is only the first word of the column name, i.  e. if you use something like
   ``'as "x [datetime]"'`` in your SQL, then we will parse out everything until the
   first blank for the column name: the column name would simply be "x".


.. function:: connect(database[, timeout, detect_types, isolation_level, check_same_thread, factory, cached_statements, uri])

   Opens a connection to the SQLite database file *database*. You can use
   ``":memory:"`` to open a database connection to a database that resides in RAM
   instead of on disk.

   When a database is accessed by multiple connections, and one of the processes
   modifies the database, the SQLite database is locked until that transaction is
   committed. The *timeout* parameter specifies how long the connection should wait
   for the lock to go away until raising an exception. The default for the timeout
   parameter is 5.0 (five seconds).

   For the *isolation_level* parameter, please see the
   :attr:`Connection.isolation_level` property of :class:`Connection` objects.

   SQLite natively supports only the types TEXT, INTEGER, REAL, BLOB and NULL. If
   you want to use other types you must add support for them yourself. The
   *detect_types* parameter and the using custom **converters** registered with the
   module-level :func:`register_converter` function allow you to easily do that.

   *detect_types* defaults to 0 (i. e. off, no type detection), you can set it to
   any combination of :const:`PARSE_DECLTYPES` and :const:`PARSE_COLNAMES` to turn
   type detection on.

   By default, the :mod:`sqlite3` module uses its :class:`Connection` class for the
   connect call.  You can, however, subclass the :class:`Connection` class and make
   :func:`connect` use your class instead by providing your class for the *factory*
   parameter.

   Consult the section :ref:`sqlite3-types` of this manual for details.

   The :mod:`sqlite3` module internally uses a statement cache to avoid SQL parsing
   overhead. If you want to explicitly set the number of statements that are cached
   for the connection, you can set the *cached_statements* parameter. The currently
   implemented default is to cache 100 statements.

   If *uri* is true, *database* is interpreted as a URI. This allows you
   to specify options. For example, to open a database in read-only mode
   you can use::

       db = sqlite3.connect('file:path/to/database?mode=ro', uri=True)

   More information about this feature, including a list of recognized options, can
   be found in the `SQLite URI documentation <http://www.sqlite.org/uri.html>`_.

   .. versionchanged:: 3.4
      Added the *uri* parameter.


.. function:: register_converter(typename, callable)

   Registers a callable to convert a bytestring from the database into a custom
   Python type. The callable will be invoked for all database values that are of
   the type *typename*. Confer the parameter *detect_types* of the :func:`connect`
   function for how the type detection works. Note that the case of *typename* and
   the name of the type in your query must match!


.. function:: register_adapter(type, callable)

   Registers a callable to convert the custom Python type *type* into one of
   SQLite's supported types. The callable *callable* accepts as single parameter
   the Python value, and must return a value of the following types: int,
   float, str or bytes.


.. function:: complete_statement(sql)

   Returns :const:`True` if the string *sql* contains one or more complete SQL
   statements terminated by semicolons. It does not verify that the SQL is
   syntactically correct, only that there are no unclosed string literals and the
   statement is terminated by a semicolon.

   This can be used to build a shell for SQLite, as in the following example:


   .. literalinclude:: ../includes/sqlite3/complete_statement.py


.. function:: enable_callback_tracebacks(flag)

   By default you will not get any tracebacks in user-defined functions,
   aggregates, converters, authorizer callbacks etc. If you want to debug them,
   you can call this function with *flag* set to ``True``. Afterwards, you will
   get tracebacks from callbacks on ``sys.stderr``. Use :const:`False` to
   disable the feature again.


.. _sqlite3-connection-objects:

Connection Objects
------------------

.. class:: Connection

   A SQLite database connection has the following attributes and methods:

   .. attribute:: isolation_level

      Get or set the current isolation level. :const:`None` for autocommit mode or
      one of "DEFERRED", "IMMEDIATE" or "EXCLUSIVE". See section
      :ref:`sqlite3-controlling-transactions` for a more detailed explanation.

   .. attribute:: in_transaction

      :const:`True` if a transaction is active (there are uncommitted changes),
      :const:`False` otherwise.  Read-only attribute.

      .. versionadded:: 3.2

   .. method:: cursor([cursorClass])

      The cursor method accepts a single optional parameter *cursorClass*. If
      supplied, this must be a custom cursor class that extends
      :class:`sqlite3.Cursor`.

   .. method:: commit()

      This method commits the current transaction. If you don't call this method,
      anything you did since the last call to ``commit()`` is not visible from
      other database connections. If you wonder why you don't see the data you've
      written to the database, please check you didn't forget to call this method.

   .. method:: rollback()

      This method rolls back any changes to the database since the last call to
      :meth:`commit`.

   .. method:: close()

      This closes the database connection. Note that this does not automatically
      call :meth:`commit`. If you just close your database connection without
      calling :meth:`commit` first, your changes will be lost!

   .. method:: execute(sql, [parameters])

      This is a nonstandard shortcut that creates an intermediate cursor object by
      calling the cursor method, then calls the cursor's :meth:`execute
      <Cursor.execute>` method with the parameters given.


   .. method:: executemany(sql, [parameters])

      This is a nonstandard shortcut that creates an intermediate cursor object by
      calling the cursor method, then calls the cursor's :meth:`executemany
      <Cursor.executemany>` method with the parameters given.

   .. method:: executescript(sql_script)

      This is a nonstandard shortcut that creates an intermediate cursor object by
      calling the cursor method, then calls the cursor's :meth:`executescript
      <Cursor.executescript>` method with the parameters given.


   .. method:: create_function(name, num_params, func)

      Creates a user-defined function that you can later use from within SQL
      statements under the function name *name*. *num_params* is the number of
      parameters the function accepts, and *func* is a Python callable that is called
      as the SQL function.

      The function can return any of the types supported by SQLite: bytes, str, int,
      float and None.

      Example:

      .. literalinclude:: ../includes/sqlite3/md5func.py


   .. method:: create_aggregate(name, num_params, aggregate_class)

      Creates a user-defined aggregate function.

      The aggregate class must implement a ``step`` method, which accepts the number
      of parameters *num_params*, and a ``finalize`` method which will return the
      final result of the aggregate.

      The ``finalize`` method can return any of the types supported by SQLite:
      bytes, str, int, float and None.

      Example:

      .. literalinclude:: ../includes/sqlite3/mysumaggr.py


   .. method:: create_collation(name, callable)

      Creates a collation with the specified *name* and *callable*. The callable will
      be passed two string arguments. It should return -1 if the first is ordered
      lower than the second, 0 if they are ordered equal and 1 if the first is ordered
      higher than the second.  Note that this controls sorting (ORDER BY in SQL) so
      your comparisons don't affect other SQL operations.

      Note that the callable will get its parameters as Python bytestrings, which will
      normally be encoded in UTF-8.

      The following example shows a custom collation that sorts "the wrong way":

      .. literalinclude:: ../includes/sqlite3/collation_reverse.py

      To remove a collation, call ``create_collation`` with None as callable::

         con.create_collation("reverse", None)


   .. method:: interrupt()

      You can call this method from a different thread to abort any queries that might
      be executing on the connection. The query will then abort and the caller will
      get an exception.


   .. method:: set_authorizer(authorizer_callback)

      This routine registers a callback. The callback is invoked for each attempt to
      access a column of a table in the database. The callback should return
      :const:`SQLITE_OK` if access is allowed, :const:`SQLITE_DENY` if the entire SQL
      statement should be aborted with an error and :const:`SQLITE_IGNORE` if the
      column should be treated as a NULL value. These constants are available in the
      :mod:`sqlite3` module.

      The first argument to the callback signifies what kind of operation is to be
      authorized. The second and third argument will be arguments or :const:`None`
      depending on the first argument. The 4th argument is the name of the database
      ("main", "temp", etc.) if applicable. The 5th argument is the name of the
      inner-most trigger or view that is responsible for the access attempt or
      :const:`None` if this access attempt is directly from input SQL code.

      Please consult the SQLite documentation about the possible values for the first
      argument and the meaning of the second and third argument depending on the first
      one. All necessary constants are available in the :mod:`sqlite3` module.


   .. method:: set_progress_handler(handler, n)

      This routine registers a callback. The callback is invoked for every *n*
      instructions of the SQLite virtual machine. This is useful if you want to
      get called from SQLite during long-running operations, for example to update
      a GUI.

      If you want to clear any previously installed progress handler, call the
      method with :const:`None` for *handler*.


   .. method:: set_trace_callback(trace_callback)

      Registers *trace_callback* to be called for each SQL statement that is
      actually executed by the SQLite backend.

      The only argument passed to the callback is the statement (as string) that
      is being executed. The return value of the callback is ignored. Note that
      the backend does not only run statements passed to the :meth:`Cursor.execute`
      methods.  Other sources include the transaction management of the Python
      module and the execution of triggers defined in the current database.

      Passing :const:`None` as *trace_callback* will disable the trace callback.

      .. versionadded:: 3.3


   .. method:: enable_load_extension(enabled)

      This routine allows/disallows the SQLite engine to load SQLite extensions
      from shared libraries.  SQLite extensions can define new functions,
      aggregates or whole new virtual table implementations.  One well-known
      extension is the fulltext-search extension distributed with SQLite.

      Loadable extensions are disabled by default. See [#f1]_.

      .. versionadded:: 3.2

      .. literalinclude:: ../includes/sqlite3/load_extension.py

   .. method:: load_extension(path)

      This routine loads a SQLite extension from a shared library.  You have to
      enable extension loading with :meth:`enable_load_extension` before you can
      use this routine.

      Loadable extensions are disabled by default. See [#f1]_.

      .. versionadded:: 3.2

   .. attribute:: row_factory

      You can change this attribute to a callable that accepts the cursor and the
      original row as a tuple and will return the real result row.  This way, you can
      implement more advanced ways of returning results, such  as returning an object
      that can also access columns by name.

      Example:

      .. literalinclude:: ../includes/sqlite3/row_factory.py

      If returning a tuple doesn't suffice and you want name-based access to
      columns, you should consider setting :attr:`row_factory` to the
      highly-optimized :class:`sqlite3.Row` type. :class:`Row` provides both
      index-based and case-insensitive name-based access to columns with almost no
      memory overhead. It will probably be better than your own custom
      dictionary-based approach or even a db_row based solution.

      .. XXX what's a db_row-based solution?


   .. attribute:: text_factory

      Using this attribute you can control what objects are returned for the ``TEXT``
      data type. By default, this attribute is set to :class:`str` and the
      :mod:`sqlite3` module will return Unicode objects for ``TEXT``. If you want to
      return bytestrings instead, you can set it to :class:`bytes`.

      For efficiency reasons, there's also a way to return :class:`str` objects
      only for non-ASCII data, and :class:`bytes` otherwise. To activate it, set
      this attribute to :const:`sqlite3.OptimizedUnicode`.

      You can also set it to any other callable that accepts a single bytestring
      parameter and returns the resulting object.

      See the following example code for illustration:

      .. literalinclude:: ../includes/sqlite3/text_factory.py


   .. attribute:: total_changes

      Returns the total number of database rows that have been modified, inserted, or
      deleted since the database connection was opened.


   .. attribute:: iterdump

      Returns an iterator to dump the database in an SQL text format.  Useful when
      saving an in-memory database for later restoration.  This function provides
      the same capabilities as the :kbd:`.dump` command in the :program:`sqlite3`
      shell.

      Example::

         # Convert file existing_db.db to SQL dump file dump.sql
         import sqlite3, os

         con = sqlite3.connect('existing_db.db')
         with open('dump.sql', 'w') as f:
             for line in con.iterdump():
                 f.write('%s\n' % line)


.. _sqlite3-cursor-objects:

Cursor Objects
--------------

.. class:: Cursor

   A :class:`Cursor` instance has the following attributes and methods.

   .. method:: execute(sql, [parameters])

      Executes an SQL statement. The SQL statement may be parameterized (i. e.
      placeholders instead of SQL literals). The :mod:`sqlite3` module supports two
      kinds of placeholders: question marks (qmark style) and named placeholders
      (named style).

      Here's an example of both styles:

      .. literalinclude:: ../includes/sqlite3/execute_1.py

      :meth:`execute` will only execute a single SQL statement. If you try to execute
      more than one statement with it, it will raise a Warning. Use
      :meth:`executescript` if you want to execute multiple SQL statements with one
      call.


   .. method:: executemany(sql, seq_of_parameters)

      Executes an SQL command against all parameter sequences or mappings found in
      the sequence *sql*.  The :mod:`sqlite3` module also allows using an
      :term:`iterator` yielding parameters instead of a sequence.

      .. literalinclude:: ../includes/sqlite3/executemany_1.py

      Here's a shorter example using a :term:`generator`:

      .. literalinclude:: ../includes/sqlite3/executemany_2.py


   .. method:: executescript(sql_script)

      This is a nonstandard convenience method for executing multiple SQL statements
      at once. It issues a ``COMMIT`` statement first, then executes the SQL script it
      gets as a parameter.

      *sql_script* can be an instance of :class:`str` or :class:`bytes`.

      Example:

      .. literalinclude:: ../includes/sqlite3/executescript.py


   .. method:: fetchone()

      Fetches the next row of a query result set, returning a single sequence,
      or :const:`None` when no more data is available.


   .. method:: fetchmany(size=cursor.arraysize)

      Fetches the next set of rows of a query result, returning a list.  An empty
      list is returned when no more rows are available.

      The number of rows to fetch per call is specified by the *size* parameter.
      If it is not given, the cursor's arraysize determines the number of rows
      to be fetched. The method should try to fetch as many rows as indicated by
      the size parameter. If this is not possible due to the specified number of
      rows not being available, fewer rows may be returned.

      Note there are performance considerations involved with the *size* parameter.
      For optimal performance, it is usually best to use the arraysize attribute.
      If the *size* parameter is used, then it is best for it to retain the same
      value from one :meth:`fetchmany` call to the next.

   .. method:: fetchall()

      Fetches all (remaining) rows of a query result, returning a list.  Note that
      the cursor's arraysize attribute can affect the performance of this operation.
      An empty list is returned when no rows are available.


   .. attribute:: rowcount

      Although the :class:`Cursor` class of the :mod:`sqlite3` module implements this
      attribute, the database engine's own support for the determination of "rows
      affected"/"rows selected" is quirky.

      For :meth:`executemany` statements, the number of modifications are summed up
      into :attr:`rowcount`.

      As required by the Python DB API Spec, the :attr:`rowcount` attribute "is -1 in
      case no ``executeXX()`` has been performed on the cursor or the rowcount of the
      last operation is not determinable by the interface". This includes ``SELECT``
      statements because we cannot determine the number of rows a query produced
      until all rows were fetched.

      With SQLite versions before 3.6.5, :attr:`rowcount` is set to 0 if
      you make a ``DELETE FROM table`` without any condition.

   .. attribute:: lastrowid

      This read-only attribute provides the rowid of the last modified row. It is
      only set if you issued a ``INSERT`` statement using the :meth:`execute`
      method. For operations other than ``INSERT`` or when :meth:`executemany` is
      called, :attr:`lastrowid` is set to :const:`None`.

   .. attribute:: description

      This read-only attribute provides the column names of the last query. To
      remain compatible with the Python DB API, it returns a 7-tuple for each
      column where the last six items of each tuple are :const:`None`.

      It is set for ``SELECT`` statements without any matching rows as well.

.. _sqlite3-row-objects:

Row Objects
-----------

.. class:: Row

   A :class:`Row` instance serves as a highly optimized
   :attr:`~Connection.row_factory` for :class:`Connection` objects.
   It tries to mimic a tuple in most of its features.

   It supports mapping access by column name and index, iteration,
   representation, equality testing and :func:`len`.

   If two :class:`Row` objects have exactly the same columns and their
   members are equal, they compare equal.

   .. method:: keys

      This method returns a tuple of column names. Immediately after a query,
      it is the first member of each tuple in :attr:`Cursor.description`.

Let's assume we initialize a table as in the example given above::

   conn = sqlite3.connect(":memory:")
   c = conn.cursor()
   c.execute('''create table stocks
   (date text, trans text, symbol text,
    qty real, price real)''')
   c.execute("""insert into stocks
             values ('2006-01-05','BUY','RHAT',100,35.14)""")
   conn.commit()
   c.close()

Now we plug :class:`Row` in::

   >>> conn.row_factory = sqlite3.Row
   >>> c = conn.cursor()
   >>> c.execute('select * from stocks')
   <sqlite3.Cursor object at 0x7f4e7dd8fa80>
   >>> r = c.fetchone()
   >>> type(r)
   <class 'sqlite3.Row'>
   >>> tuple(r)
   ('2006-01-05', 'BUY', 'RHAT', 100.0, 35.14)
   >>> len(r)
   5
   >>> r[2]
   'RHAT'
   >>> r.keys()
   ['date', 'trans', 'symbol', 'qty', 'price']
   >>> r['qty']
   100.0
   >>> for member in r:
   ...     print(member)
   ...
   2006-01-05
   BUY
   RHAT
   100.0
   35.14


.. _sqlite3-types:

SQLite and Python types
-----------------------


Introduction
^^^^^^^^^^^^

SQLite natively supports the following types: ``NULL``, ``INTEGER``,
``REAL``, ``TEXT``, ``BLOB``.

The following Python types can thus be sent to SQLite without any problem:

+-------------------------------+-------------+
| Python type                   | SQLite type |
+===============================+=============+
| :const:`None`                 | ``NULL``    |
+-------------------------------+-------------+
| :class:`int`                  | ``INTEGER`` |
+-------------------------------+-------------+
| :class:`float`                | ``REAL``    |
+-------------------------------+-------------+
| :class:`str`                  | ``TEXT``    |
+-------------------------------+-------------+
| :class:`bytes`                | ``BLOB``    |
+-------------------------------+-------------+


This is how SQLite types are converted to Python types by default:

+-------------+----------------------------------------------+
| SQLite type | Python type                                  |
+=============+==============================================+
| ``NULL``    | :const:`None`                                |
+-------------+----------------------------------------------+
| ``INTEGER`` | :class:`int`                                 |
+-------------+----------------------------------------------+
| ``REAL``    | :class:`float`                               |
+-------------+----------------------------------------------+
| ``TEXT``    | depends on :attr:`~Connection.text_factory`, |
|             | :class:`str` by default                      |
+-------------+----------------------------------------------+
| ``BLOB``    | :class:`bytes`                               |
+-------------+----------------------------------------------+

The type system of the :mod:`sqlite3` module is extensible in two ways: you can
store additional Python types in a SQLite database via object adaptation, and
you can let the :mod:`sqlite3` module convert SQLite types to different Python
types via converters.


Using adapters to store additional Python types in SQLite databases
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

As described before, SQLite supports only a limited set of types natively. To
use other Python types with SQLite, you must **adapt** them to one of the
sqlite3 module's supported types for SQLite: one of NoneType, int, float,
str, bytes.

There are two ways to enable the :mod:`sqlite3` module to adapt a custom Python
type to one of the supported ones.


Letting your object adapt itself
""""""""""""""""""""""""""""""""

This is a good approach if you write the class yourself. Let's suppose you have
a class like this::

   class Point:
       def __init__(self, x, y):
           self.x, self.y = x, y

Now you want to store the point in a single SQLite column.  First you'll have to
choose one of the supported types first to be used for representing the point.
Let's just use str and separate the coordinates using a semicolon. Then you need
to give your class a method ``__conform__(self, protocol)`` which must return
the converted value. The parameter *protocol* will be :class:`PrepareProtocol`.

.. literalinclude:: ../includes/sqlite3/adapter_point_1.py


Registering an adapter callable
"""""""""""""""""""""""""""""""

The other possibility is to create a function that converts the type to the
string representation and register the function with :meth:`register_adapter`.

.. literalinclude:: ../includes/sqlite3/adapter_point_2.py

The :mod:`sqlite3` module has two default adapters for Python's built-in
:class:`datetime.date` and :class:`datetime.datetime` types.  Now let's suppose
we want to store :class:`datetime.datetime` objects not in ISO representation,
but as a Unix timestamp.

.. literalinclude:: ../includes/sqlite3/adapter_datetime.py


Converting SQLite values to custom Python types
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Writing an adapter lets you send custom Python types to SQLite. But to make it
really useful we need to make the Python to SQLite to Python roundtrip work.

Enter converters.

Let's go back to the :class:`Point` class. We stored the x and y coordinates
separated via semicolons as strings in SQLite.

First, we'll define a converter function that accepts the string as a parameter
and constructs a :class:`Point` object from it.

.. note::

   Converter functions **always** get called with a :class:`bytes` object, no
   matter under which data type you sent the value to SQLite.

::

   def convert_point(s):
       x, y = map(float, s.split(b";"))
       return Point(x, y)

Now you need to make the :mod:`sqlite3` module know that what you select from
the database is actually a point. There are two ways of doing this:

* Implicitly via the declared type

* Explicitly via the column name

Both ways are described in section :ref:`sqlite3-module-contents`, in the entries
for the constants :const:`PARSE_DECLTYPES` and :const:`PARSE_COLNAMES`.

The following example illustrates both approaches.

.. literalinclude:: ../includes/sqlite3/converter_point.py


Default adapters and converters
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

There are default adapters for the date and datetime types in the datetime
module. They will be sent as ISO dates/ISO timestamps to SQLite.

The default converters are registered under the name "date" for
:class:`datetime.date` and under the name "timestamp" for
:class:`datetime.datetime`.

This way, you can use date/timestamps from Python without any additional
fiddling in most cases. The format of the adapters is also compatible with the
experimental SQLite date/time functions.

The following example demonstrates this.

.. literalinclude:: ../includes/sqlite3/pysqlite_datetime.py

If a timestamp stored in SQLite has a fractional part longer than 6
numbers, its value will be truncated to microsecond precision by the
timestamp converter.


.. _sqlite3-controlling-transactions:

Controlling Transactions
------------------------

By default, the :mod:`sqlite3` module opens transactions implicitly before a
Data Modification Language (DML)  statement (i.e.
``INSERT``/``UPDATE``/``DELETE``/``REPLACE``), and commits transactions
implicitly before a non-DML, non-query statement (i. e.
anything other than ``SELECT`` or the aforementioned).

So if you are within a transaction and issue a command like ``CREATE TABLE
...``, ``VACUUM``, ``PRAGMA``, the :mod:`sqlite3` module will commit implicitly
before executing that command. There are two reasons for doing that. The first
is that some of these commands don't work within transactions. The other reason
is that sqlite3 needs to keep track of the transaction state (if a transaction
is active or not).  The current transaction state is exposed through the
:attr:`Connection.in_transaction` attribute of the connection object.

You can control which kind of ``BEGIN`` statements sqlite3 implicitly executes
(or none at all) via the *isolation_level* parameter to the :func:`connect`
call, or via the :attr:`isolation_level` property of connections.

If you want **autocommit mode**, then set :attr:`isolation_level` to None.

Otherwise leave it at its default, which will result in a plain "BEGIN"
statement, or set it to one of SQLite's supported isolation levels: "DEFERRED",
"IMMEDIATE" or "EXCLUSIVE".



Using :mod:`sqlite3` efficiently
--------------------------------


Using shortcut methods
^^^^^^^^^^^^^^^^^^^^^^

Using the nonstandard :meth:`execute`, :meth:`executemany` and
:meth:`executescript` methods of the :class:`Connection` object, your code can
be written more concisely because you don't have to create the (often
superfluous) :class:`Cursor` objects explicitly. Instead, the :class:`Cursor`
objects are created implicitly and these shortcut methods return the cursor
objects. This way, you can execute a ``SELECT`` statement and iterate over it
directly using only a single call on the :class:`Connection` object.

.. literalinclude:: ../includes/sqlite3/shortcut_methods.py


Accessing columns by name instead of by index
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

One useful feature of the :mod:`sqlite3` module is the built-in
:class:`sqlite3.Row` class designed to be used as a row factory.

Rows wrapped with this class can be accessed both by index (like tuples) and
case-insensitively by name:

.. literalinclude:: ../includes/sqlite3/rowclass.py


Using the connection as a context manager
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Connection objects can be used as context managers
that automatically commit or rollback transactions.  In the event of an
exception, the transaction is rolled back; otherwise, the transaction is
committed:

.. literalinclude:: ../includes/sqlite3/ctx_manager.py


Common issues
-------------

Multithreading
^^^^^^^^^^^^^^

Older SQLite versions had issues with sharing connections between threads.
That's why the Python module disallows sharing connections and cursors between
threads. If you still try to do so, you will get an exception at runtime.

The only exception is calling the :meth:`~Connection.interrupt` method, which
only makes sense to call from a different thread.

.. rubric:: Footnotes

.. [#f1] The sqlite3 module is not built with loadable extension support by
   default, because some platforms (notably Mac OS X) have SQLite
   libraries which are compiled without this feature. To get loadable
   extension support, you must pass --enable-loadable-sqlite-extensions to
   configure.
