.. highlightlang:: c

.. _moduleobjects:

Module Objects
--------------

.. index:: object: module

There are only a few functions special to module objects.


.. c:var:: PyTypeObject PyModule_Type

   .. index:: single: ModuleType (in module types)

   This instance of :c:type:`PyTypeObject` represents the Python module type.  This
   is exposed to Python programs as ``types.ModuleType``.


.. c:function:: int PyModule_Check(PyObject *p)

   Return true if *p* is a module object, or a subtype of a module object.


.. c:function:: int PyModule_CheckExact(PyObject *p)

   Return true if *p* is a module object, but not a subtype of
   :c:data:`PyModule_Type`.


.. c:function:: PyObject* PyModule_NewObject(PyObject *name)

   .. index::
      single: __name__ (module attribute)
      single: __doc__ (module attribute)
      single: __file__ (module attribute)
      single: __package__ (module attribute)
      single: __loader__ (module attribute)

   Return a new module object with the :attr:`__name__` attribute set to *name*.
   The module's :attr:`__name__`, :attr:`__doc__`, :attr:`__package__`, and
   :attr:`__loader__` attributes are filled in (all but :attr:`__name__` are set
   to ``None``); the caller is responsible for providing a :attr:`__file__`
   attribute.

   .. versionadded:: 3.3

   .. versionchanged:: 3.4
      :attr:`__package__` and :attr:`__loader__` are set to ``None``.


.. c:function:: PyObject* PyModule_New(const char *name)

   Similar to :c:func:`PyImport_NewObject`, but the name is an UTF-8 encoded
   string instead of a Unicode object.


.. c:function:: PyObject* PyModule_GetDict(PyObject *module)

   .. index:: single: __dict__ (module attribute)

   Return the dictionary object that implements *module*'s namespace; this object
   is the same as the :attr:`__dict__` attribute of the module object.  This
   function never fails.  It is recommended extensions use other
   :c:func:`PyModule_\*` and :c:func:`PyObject_\*` functions rather than directly
   manipulate a module's :attr:`__dict__`.


.. c:function:: PyObject* PyModule_GetNameObject(PyObject *module)

   .. index::
      single: __name__ (module attribute)
      single: SystemError (built-in exception)

   Return *module*'s :attr:`__name__` value.  If the module does not provide one,
   or if it is not a string, :exc:`SystemError` is raised and *NULL* is returned.

   .. versionadded:: 3.3


.. c:function:: char* PyModule_GetName(PyObject *module)

   Similar to :c:func:`PyModule_GetNameObject` but return the name encoded to
   ``'utf-8'``.


.. c:function:: PyObject* PyModule_GetFilenameObject(PyObject *module)

   .. index::
      single: __file__ (module attribute)
      single: SystemError (built-in exception)

   Return the name of the file from which *module* was loaded using *module*'s
   :attr:`__file__` attribute.  If this is not defined, or if it is not a
   unicode string, raise :exc:`SystemError` and return *NULL*; otherwise return
   a reference to a Unicode object.

   .. versionadded:: 3.2


.. c:function:: char* PyModule_GetFilename(PyObject *module)

   Similar to :c:func:`PyModule_GetFilenameObject` but return the filename
   encoded to 'utf-8'.

   .. deprecated:: 3.2
      :c:func:`PyModule_GetFilename` raises :c:type:`UnicodeEncodeError` on
      unencodable filenames, use :c:func:`PyModule_GetFilenameObject` instead.


.. c:function:: void* PyModule_GetState(PyObject *module)

   Return the "state" of the module, that is, a pointer to the block of memory
   allocated at module creation time, or *NULL*.  See
   :c:member:`PyModuleDef.m_size`.


.. c:function:: PyModuleDef* PyModule_GetDef(PyObject *module)

   Return a pointer to the :c:type:`PyModuleDef` struct from which the module was
   created, or *NULL* if the module wasn't created with
   :c:func:`PyModule_Create`.

.. c:function:: PyObject* PyState_FindModule(PyModuleDef *def)

   Returns the module object that was created from *def* for the current interpreter.
   This method requires that the module object has been attached to the interpreter state with
   :c:func:`PyState_AddModule` beforehand. In case the corresponding module object is not
   found or has not been attached to the interpreter state yet, it returns NULL.

.. c:function:: int PyState_AddModule(PyObject *module, PyModuleDef *def)

   Attaches the module object passed to the function to the interpreter state. This allows
   the module object to be accessible via
   :c:func:`PyState_FindModule`.

   .. versionadded:: 3.3

.. c:function:: int PyState_RemoveModule(PyModuleDef *def)

   Removes the module object created from *def* from the interpreter state.

   .. versionadded:: 3.3

Initializing C modules
^^^^^^^^^^^^^^^^^^^^^^

These functions are usually used in the module initialization function.

.. c:function:: PyObject* PyModule_Create(PyModuleDef *module)

   Create a new module object, given the definition in *module*.  This behaves
   like :c:func:`PyModule_Create2` with *module_api_version* set to
   :const:`PYTHON_API_VERSION`.


.. c:function:: PyObject* PyModule_Create2(PyModuleDef *module, int module_api_version)

   Create a new module object, given the definition in *module*, assuming the
   API version *module_api_version*.  If that version does not match the version
   of the running interpreter, a :exc:`RuntimeWarning` is emitted.

   .. note::

      Most uses of this function should be using :c:func:`PyModule_Create`
      instead; only use this if you are sure you need it.


.. c:type:: PyModuleDef

   This struct holds all information that is needed to create a module object.
   There is usually only one static variable of that type for each module, which
   is statically initialized and then passed to :c:func:`PyModule_Create` in the
   module initialization function.

   .. c:member:: PyModuleDef_Base m_base

      Always initialize this member to :const:`PyModuleDef_HEAD_INIT`.

   .. c:member:: char* m_name

      Name for the new module.

   .. c:member:: char* m_doc

      Docstring for the module; usually a docstring variable created with
      :c:func:`PyDoc_STRVAR` is used.

   .. c:member:: Py_ssize_t m_size

      Some modules allow re-initialization (calling their ``PyInit_*`` function
      more than once). These modules should keep their state in a per-module
      memory area that can be retrieved with :c:func:`PyModule_GetState`.

      This memory should be used, rather than static globals, to hold per-module
      state, since it is then safe for use in multiple sub-interpreters.  It is
      freed when the module object is deallocated, after the :c:member:`m_free`
      function has been called, if present.

      Setting ``m_size`` to ``-1`` means that the module can not be
      re-initialized because it has global state. Setting it to a non-negative
      value means that the module can be re-initialized and specifies the
      additional amount of memory it requires for its state.

      See :PEP:`3121` for more details.

   .. c:member:: PyMethodDef* m_methods

      A pointer to a table of module-level functions, described by
      :c:type:`PyMethodDef` values.  Can be *NULL* if no functions are present.

   .. c:member:: inquiry m_reload

      Currently unused, should be *NULL*.

   .. c:member:: traverseproc m_traverse

      A traversal function to call during GC traversal of the module object, or
      *NULL* if not needed.

   .. c:member:: inquiry m_clear

      A clear function to call during GC clearing of the module object, or
      *NULL* if not needed.

   .. c:member:: freefunc m_free

      A function to call during deallocation of the module object, or *NULL* if
      not needed.


.. c:function:: int PyModule_AddObject(PyObject *module, const char *name, PyObject *value)

   Add an object to *module* as *name*.  This is a convenience function which can
   be used from the module's initialization function.  This steals a reference to
   *value*.  Return ``-1`` on error, ``0`` on success.


.. c:function:: int PyModule_AddIntConstant(PyObject *module, const char *name, long value)

   Add an integer constant to *module* as *name*.  This convenience function can be
   used from the module's initialization function. Return ``-1`` on error, ``0`` on
   success.


.. c:function:: int PyModule_AddStringConstant(PyObject *module, const char *name, const char *value)

   Add a string constant to *module* as *name*.  This convenience function can be
   used from the module's initialization function.  The string *value* must be
   null-terminated.  Return ``-1`` on error, ``0`` on success.


.. c:function:: int PyModule_AddIntMacro(PyObject *module, macro)

   Add an int constant to *module*. The name and the value are taken from
   *macro*. For example ``PyModule_AddIntMacro(module, AF_INET)`` adds the int
   constant *AF_INET* with the value of *AF_INET* to *module*.
   Return ``-1`` on error, ``0`` on success.


.. c:function:: int PyModule_AddStringMacro(PyObject *module, macro)

   Add a string constant to *module*.
