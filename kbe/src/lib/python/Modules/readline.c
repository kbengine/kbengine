/* This module makes GNU readline available to Python.  It has ideas
 * contributed by Lee Busby, LLNL, and William Magro, Cornell Theory
 * Center.  The completer interface was inspired by Lele Gaifax.  More
 * recently, it was largely rewritten by Guido van Rossum.
 */

/* Standard definitions */
#include "Python.h"
#include <stddef.h>
#include <setjmp.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>

#if defined(HAVE_SETLOCALE)
/* GNU readline() mistakenly sets the LC_CTYPE locale.
 * This is evil.  Only the user or the app's main() should do this!
 * We must save and restore the locale around the rl_initialize() call.
 */
#define SAVE_LOCALE
#include <locale.h>
#endif

#ifdef SAVE_LOCALE
#  define RESTORE_LOCALE(sl) { setlocale(LC_CTYPE, sl); free(sl); }
#else
#  define RESTORE_LOCALE(sl)
#endif

/* GNU readline definitions */
#undef HAVE_CONFIG_H /* Else readline/chardefs.h includes strings.h */
#include <readline/readline.h>
#include <readline/history.h>

#ifdef HAVE_RL_COMPLETION_MATCHES
#define completion_matches(x, y) \
    rl_completion_matches((x), ((rl_compentry_func_t *)(y)))
#else
#if defined(_RL_FUNCTION_TYPEDEF)
extern char **completion_matches(char *, rl_compentry_func_t *);
#else

#if !defined(__APPLE__)
extern char **completion_matches(char *, CPFunction *);
#endif
#endif
#endif

#ifdef __APPLE__
/*
 * It is possible to link the readline module to the readline
 * emulation library of editline/libedit.
 *
 * On OSX this emulation library is not 100% API compatible
 * with the "real" readline and cannot be detected at compile-time,
 * hence we use a runtime check to detect if we're using libedit
 *
 * Currently there is one known API incompatibility:
 * - 'get_history' has a 1-based index with GNU readline, and a 0-based
 *   index with older versions of libedit's emulation.
 * - Note that replace_history and remove_history use a 0-based index
 *   with both implementations.
 */
static int using_libedit_emulation = 0;
static const char libedit_version_tag[] = "EditLine wrapper";

static int libedit_history_start = 0;
#endif /* __APPLE__ */

#ifdef HAVE_RL_COMPLETION_DISPLAY_MATCHES_HOOK
static void
on_completion_display_matches_hook(char **matches,
                                   int num_matches, int max_length);
#endif

/* Memory allocated for rl_completer_word_break_characters
   (see issue #17289 for the motivation). */
static char *completer_word_break_characters;

typedef struct {
  PyObject *completion_display_matches_hook;
  PyObject *startup_hook;
  PyObject *pre_input_hook;
  PyObject *completer;
  PyObject *begidx;
  PyObject *endidx;
} readlinestate;


#define readline_state(o) ((readlinestate *)PyModule_GetState(o))

static int
readline_clear(PyObject *m)
{
   readlinestate *state = readline_state(m);
   Py_CLEAR(state->completion_display_matches_hook);
   Py_CLEAR(state->startup_hook);
   Py_CLEAR(state->pre_input_hook);
   Py_CLEAR(state->completer);
   Py_CLEAR(state->begidx);
   Py_CLEAR(state->endidx);
   return 0;
}

static int
readline_traverse(PyObject *m, visitproc visit, void *arg)
{
    readlinestate *state = readline_state(m);
    Py_VISIT(state->completion_display_matches_hook);
    Py_VISIT(state->startup_hook);
    Py_VISIT(state->pre_input_hook);
    Py_VISIT(state->completer);
    Py_VISIT(state->begidx);
    Py_VISIT(state->endidx);
    return 0;
}

static void
readline_free(void *m)
{
    readline_clear((PyObject *)m);
}

static PyModuleDef readlinemodule;

#define readlinestate_global ((readlinestate *)PyModule_GetState(PyState_FindModule(&readlinemodule)))


/* Exported function to send one line to readline's init file parser */

static PyObject *
parse_and_bind(PyObject *self, PyObject *args)
{
    char *s, *copy;
    if (!PyArg_ParseTuple(args, "s:parse_and_bind", &s))
        return NULL;
    /* Make a copy -- rl_parse_and_bind() modifies its argument */
    /* Bernard Herzog */
    copy = PyMem_Malloc(1 + strlen(s));
    if (copy == NULL)
        return PyErr_NoMemory();
    strcpy(copy, s);
    rl_parse_and_bind(copy);
    PyMem_Free(copy); /* Free the copy */
    Py_RETURN_NONE;
}

PyDoc_STRVAR(doc_parse_and_bind,
"parse_and_bind(string) -> None\n\
Parse and execute single line of a readline init file.");


/* Exported function to parse a readline init file */

static PyObject *
read_init_file(PyObject *self, PyObject *args)
{
    PyObject *filename_obj = Py_None, *filename_bytes;
    if (!PyArg_ParseTuple(args, "|O:read_init_file", &filename_obj))
        return NULL;
    if (filename_obj != Py_None) {
        if (!PyUnicode_FSConverter(filename_obj, &filename_bytes))
            return NULL;
        errno = rl_read_init_file(PyBytes_AsString(filename_bytes));
        Py_DECREF(filename_bytes);
    } else
        errno = rl_read_init_file(NULL);
    if (errno)
        return PyErr_SetFromErrno(PyExc_IOError);
    Py_RETURN_NONE;
}

PyDoc_STRVAR(doc_read_init_file,
"read_init_file([filename]) -> None\n\
Parse a readline initialization file.\n\
The default filename is the last filename used.");


/* Exported function to load a readline history file */

static PyObject *
read_history_file(PyObject *self, PyObject *args)
{
    PyObject *filename_obj = Py_None, *filename_bytes;
    if (!PyArg_ParseTuple(args, "|O:read_history_file", &filename_obj))
        return NULL;
    if (filename_obj != Py_None) {
        if (!PyUnicode_FSConverter(filename_obj, &filename_bytes))
            return NULL;
        errno = read_history(PyBytes_AsString(filename_bytes));
        Py_DECREF(filename_bytes);
    } else
        errno = read_history(NULL);
    if (errno)
        return PyErr_SetFromErrno(PyExc_IOError);
    Py_RETURN_NONE;
}

static int _history_length = -1; /* do not truncate history by default */
PyDoc_STRVAR(doc_read_history_file,
"read_history_file([filename]) -> None\n\
Load a readline history file.\n\
The default filename is ~/.history.");


/* Exported function to save a readline history file */

static PyObject *
write_history_file(PyObject *self, PyObject *args)
{
    PyObject *filename_obj = Py_None, *filename_bytes;
    char *filename;
    int err;
    if (!PyArg_ParseTuple(args, "|O:write_history_file", &filename_obj))
        return NULL;
    if (filename_obj != Py_None) {
        if (!PyUnicode_FSConverter(filename_obj, &filename_bytes))
            return NULL;
        filename = PyBytes_AsString(filename_bytes);
    } else {
        filename_bytes = NULL;
        filename = NULL;
    }
    errno = err = write_history(filename);
    if (!err && _history_length >= 0)
        history_truncate_file(filename, _history_length);
    Py_XDECREF(filename_bytes);
    errno = err;
    if (errno)
        return PyErr_SetFromErrno(PyExc_IOError);
    Py_RETURN_NONE;
}

PyDoc_STRVAR(doc_write_history_file,
"write_history_file([filename]) -> None\n\
Save a readline history file.\n\
The default filename is ~/.history.");


/* Set history length */

static PyObject*
set_history_length(PyObject *self, PyObject *args)
{
    int length = _history_length;
    if (!PyArg_ParseTuple(args, "i:set_history_length", &length))
        return NULL;
    _history_length = length;
    Py_RETURN_NONE;
}

PyDoc_STRVAR(set_history_length_doc,
"set_history_length(length) -> None\n\
set the maximal number of items which will be written to\n\
the history file. A negative length is used to inhibit\n\
history truncation.");


/* Get history length */

static PyObject*
get_history_length(PyObject *self, PyObject *noarg)
{
    return PyLong_FromLong(_history_length);
}

PyDoc_STRVAR(get_history_length_doc,
"get_history_length() -> int\n\
return the maximum number of items that will be written to\n\
the history file.");


/* Generic hook function setter */

static PyObject *
set_hook(const char *funcname, PyObject **hook_var, PyObject *args)
{
    PyObject *function = Py_None;
    char buf[80];
    PyOS_snprintf(buf, sizeof(buf), "|O:set_%.50s", funcname);
    if (!PyArg_ParseTuple(args, buf, &function))
        return NULL;
    if (function == Py_None) {
        Py_CLEAR(*hook_var);
    }
    else if (PyCallable_Check(function)) {
        PyObject *tmp = *hook_var;
        Py_INCREF(function);
        *hook_var = function;
        Py_XDECREF(tmp);
    }
    else {
        PyErr_Format(PyExc_TypeError,
                     "set_%.50s(func): argument not callable",
                     funcname);
        return NULL;
    }
    Py_RETURN_NONE;
}


/* Exported functions to specify hook functions in Python */


#ifdef HAVE_RL_PRE_INPUT_HOOK

#endif

static PyObject *
set_completion_display_matches_hook(PyObject *self, PyObject *args)
{
    PyObject *result = set_hook("completion_display_matches_hook",
                    &readlinestate_global->completion_display_matches_hook, args);
#ifdef HAVE_RL_COMPLETION_DISPLAY_MATCHES_HOOK
    /* We cannot set this hook globally, since it replaces the
       default completion display. */
    rl_completion_display_matches_hook =
        readlinestate_global->completion_display_matches_hook ?
#if defined(_RL_FUNCTION_TYPEDEF)
        (rl_compdisp_func_t *)on_completion_display_matches_hook : 0;
#else
        (VFunction *)on_completion_display_matches_hook : 0;
#endif
#endif
    return result;

}

PyDoc_STRVAR(doc_set_completion_display_matches_hook,
"set_completion_display_matches_hook([function]) -> None\n\
Set or remove the completion display function.\n\
The function is called as\n\
  function(substitution, [matches], longest_match_length)\n\
once each time matches need to be displayed.");

static PyObject *
set_startup_hook(PyObject *self, PyObject *args)
{
    return set_hook("startup_hook", &readlinestate_global->startup_hook, args);
}

PyDoc_STRVAR(doc_set_startup_hook,
"set_startup_hook([function]) -> None\n\
Set or remove the startup_hook function.\n\
The function is called with no arguments just\n\
before readline prints the first prompt.");


#ifdef HAVE_RL_PRE_INPUT_HOOK

/* Set pre-input hook */

static PyObject *
set_pre_input_hook(PyObject *self, PyObject *args)
{
    return set_hook("pre_input_hook", &readlinestate_global->pre_input_hook, args);
}

PyDoc_STRVAR(doc_set_pre_input_hook,
"set_pre_input_hook([function]) -> None\n\
Set or remove the pre_input_hook function.\n\
The function is called with no arguments after the first prompt\n\
has been printed and just before readline starts reading input\n\
characters.");

#endif


/* Exported function to specify a word completer in Python */







/* Get the completion type for the scope of the tab-completion */
static PyObject *
get_completion_type(PyObject *self, PyObject *noarg)
{
  return PyLong_FromLong(rl_completion_type);
}

PyDoc_STRVAR(doc_get_completion_type,
"get_completion_type() -> int\n\
Get the type of completion being attempted.");


/* Get the beginning index for the scope of the tab-completion */

static PyObject *
get_begidx(PyObject *self, PyObject *noarg)
{
    Py_INCREF(readlinestate_global->begidx);
    return readlinestate_global->begidx;
}

PyDoc_STRVAR(doc_get_begidx,
"get_begidx() -> int\n\
get the beginning index of the readline tab-completion scope");


/* Get the ending index for the scope of the tab-completion */

static PyObject *
get_endidx(PyObject *self, PyObject *noarg)
{
    Py_INCREF(readlinestate_global->endidx);
    return readlinestate_global->endidx;
}

PyDoc_STRVAR(doc_get_endidx,
"get_endidx() -> int\n\
get the ending index of the readline tab-completion scope");


/* Set the tab-completion word-delimiters that readline uses */

static PyObject *
set_completer_delims(PyObject *self, PyObject *args)
{
    char *break_chars;

    if (!PyArg_ParseTuple(args, "s:set_completer_delims", &break_chars)) {
        return NULL;
    }
    /* Keep a reference to the allocated memory in the module state in case
       some other module modifies rl_completer_word_break_characters
       (see issue #17289). */
    free(completer_word_break_characters);
    completer_word_break_characters = strdup(break_chars);
    if (completer_word_break_characters) {
        rl_completer_word_break_characters = completer_word_break_characters;
        Py_RETURN_NONE;
    }
    else
        return PyErr_NoMemory();
}

PyDoc_STRVAR(doc_set_completer_delims,
"set_completer_delims(string) -> None\n\
set the readline word delimiters for tab-completion");

/* _py_free_history_entry: Utility function to free a history entry. */

#if defined(RL_READLINE_VERSION) && RL_READLINE_VERSION >= 0x0500

/* Readline version >= 5.0 introduced a timestamp field into the history entry
   structure; this needs to be freed to avoid a memory leak.  This version of
   readline also introduced the handy 'free_history_entry' function, which
   takes care of the timestamp. */

static void
_py_free_history_entry(HIST_ENTRY *entry)
{
    histdata_t data = free_history_entry(entry);
    free(data);
}

#else

/* No free_history_entry function;  free everything manually. */

static void
_py_free_history_entry(HIST_ENTRY *entry)
{
    if (entry->line)
        free((void *)entry->line);
    if (entry->data)
        free(entry->data);
    free(entry);
}

#endif

static PyObject *
py_remove_history(PyObject *self, PyObject *args)
{
    int entry_number;
    HIST_ENTRY *entry;

    if (!PyArg_ParseTuple(args, "i:remove_history", &entry_number))
        return NULL;
    if (entry_number < 0) {
        PyErr_SetString(PyExc_ValueError,
                        "History index cannot be negative");
        return NULL;
    }
    entry = remove_history(entry_number);
    if (!entry) {
        PyErr_Format(PyExc_ValueError,
                     "No history item at position %d",
                      entry_number);
        return NULL;
    }
    /* free memory allocated for the history entry */
    _py_free_history_entry(entry);
    Py_RETURN_NONE;
}

PyDoc_STRVAR(doc_remove_history,
"remove_history_item(pos) -> None\n\
remove history item given by its position");

static PyObject *
py_replace_history(PyObject *self, PyObject *args)
{
    int entry_number;
    char *line;
    HIST_ENTRY *old_entry;

    if (!PyArg_ParseTuple(args, "is:replace_history", &entry_number,
                          &line)) {
        return NULL;
    }
    if (entry_number < 0) {
        PyErr_SetString(PyExc_ValueError,
                        "History index cannot be negative");
        return NULL;
    }
    old_entry = replace_history_entry(entry_number, line, (void *)NULL);
    if (!old_entry) {
        PyErr_Format(PyExc_ValueError,
                     "No history item at position %d",
                     entry_number);
        return NULL;
    }
    /* free memory allocated for the old history entry */
    _py_free_history_entry(old_entry);
    Py_RETURN_NONE;
}

PyDoc_STRVAR(doc_replace_history,
"replace_history_item(pos, line) -> None\n\
replaces history item given by its position with contents of line");

/* Add a line to the history buffer */

static PyObject *
py_add_history(PyObject *self, PyObject *args)
{
    char *line;

    if(!PyArg_ParseTuple(args, "s:add_history", &line)) {
        return NULL;
    }
    add_history(line);
    Py_RETURN_NONE;
}

PyDoc_STRVAR(doc_add_history,
"add_history(string) -> None\n\
add a line to the history buffer");


/* Get the tab-completion word-delimiters that readline uses */

static PyObject *
get_completer_delims(PyObject *self, PyObject *noarg)
{
    return PyUnicode_FromString(rl_completer_word_break_characters);
}

PyDoc_STRVAR(doc_get_completer_delims,
"get_completer_delims() -> string\n\
get the readline word delimiters for tab-completion");


/* Set the completer function */

static PyObject *
set_completer(PyObject *self, PyObject *args)
{
    return set_hook("completer", &readlinestate_global->completer, args);
}

PyDoc_STRVAR(doc_set_completer,
"set_completer([function]) -> None\n\
Set or remove the completer function.\n\
The function is called as function(text, state),\n\
for state in 0, 1, 2, ..., until it returns a non-string.\n\
It should return the next possible completion starting with 'text'.");


static PyObject *
get_completer(PyObject *self, PyObject *noargs)
{
    if (readlinestate_global->completer == NULL) {
        Py_RETURN_NONE;
    }
    Py_INCREF(readlinestate_global->completer);
    return readlinestate_global->completer;
}

PyDoc_STRVAR(doc_get_completer,
"get_completer() -> function\n\
\n\
Returns current completer function.");

/* Private function to get current length of history.  XXX It may be
 * possible to replace this with a direct use of history_length instead,
 * but it's not clear whether BSD's libedit keeps history_length up to date.
 * See issue #8065.*/

static int
_py_get_history_length(void)
{
    HISTORY_STATE *hist_st = history_get_history_state();
    int length = hist_st->length;
    /* the history docs don't say so, but the address of hist_st changes each
       time history_get_history_state is called which makes me think it's
       freshly malloc'd memory...  on the other hand, the address of the last
       line stays the same as long as history isn't extended, so it appears to
       be malloc'd but managed by the history package... */
    free(hist_st);
    return length;
}

/* Exported function to get any element of history */

static PyObject *
get_history_item(PyObject *self, PyObject *args)
{
    int idx = 0;
    HIST_ENTRY *hist_ent;

    if (!PyArg_ParseTuple(args, "i:index", &idx))
        return NULL;
#ifdef  __APPLE__
    if (using_libedit_emulation) {
        /* Older versions of libedit's readline emulation
         * use 0-based indexes, while readline and newer
         * versions of libedit use 1-based indexes.
         */
        int length = _py_get_history_length();

        idx = idx - 1 + libedit_history_start;

        /*
         * Apple's readline emulation crashes when
         * the index is out of range, therefore
         * test for that and fail gracefully.
         */
        if (idx < (0 + libedit_history_start)
                || idx >= (length + libedit_history_start)) {
            Py_RETURN_NONE;
        }
    }
#endif /* __APPLE__ */
    if ((hist_ent = history_get(idx)))
        return PyUnicode_FromString(hist_ent->line);
    else {
        Py_RETURN_NONE;
    }
}

PyDoc_STRVAR(doc_get_history_item,
"get_history_item() -> string\n\
return the current contents of history item at index.");


/* Exported function to get current length of history */

static PyObject *
get_current_history_length(PyObject *self, PyObject *noarg)
{
    return PyLong_FromLong((long)_py_get_history_length());
}

PyDoc_STRVAR(doc_get_current_history_length,
"get_current_history_length() -> integer\n\
return the current (not the maximum) length of history.");


/* Exported function to read the current line buffer */

static PyObject *
get_line_buffer(PyObject *self, PyObject *noarg)
{
    return PyUnicode_FromString(rl_line_buffer);
}

PyDoc_STRVAR(doc_get_line_buffer,
"get_line_buffer() -> string\n\
return the current contents of the line buffer.");


#ifdef HAVE_RL_COMPLETION_APPEND_CHARACTER

/* Exported function to clear the current history */

static PyObject *
py_clear_history(PyObject *self, PyObject *noarg)
{
    clear_history();
    Py_RETURN_NONE;
}

PyDoc_STRVAR(doc_clear_history,
"clear_history() -> None\n\
Clear the current readline history.");
#endif


/* Exported function to insert text into the line buffer */

static PyObject *
insert_text(PyObject *self, PyObject *args)
{
    char *s;
    if (!PyArg_ParseTuple(args, "s:insert_text", &s))
        return NULL;
    rl_insert_text(s);
    Py_RETURN_NONE;
}

PyDoc_STRVAR(doc_insert_text,
"insert_text(string) -> None\n\
Insert text into the command line.");


/* Redisplay the line buffer */

static PyObject *
redisplay(PyObject *self, PyObject *noarg)
{
    rl_redisplay();
    Py_RETURN_NONE;
}

PyDoc_STRVAR(doc_redisplay,
"redisplay() -> None\n\
Change what's displayed on the screen to reflect the current\n\
contents of the line buffer.");


/* Table of functions exported by the module */

static struct PyMethodDef readline_methods[] =
{
    {"parse_and_bind", parse_and_bind, METH_VARARGS, doc_parse_and_bind},
    {"get_line_buffer", get_line_buffer, METH_NOARGS, doc_get_line_buffer},
    {"insert_text", insert_text, METH_VARARGS, doc_insert_text},
    {"redisplay", redisplay, METH_NOARGS, doc_redisplay},
    {"read_init_file", read_init_file, METH_VARARGS, doc_read_init_file},
    {"read_history_file", read_history_file,
     METH_VARARGS, doc_read_history_file},
    {"write_history_file", write_history_file,
     METH_VARARGS, doc_write_history_file},
    {"get_history_item", get_history_item,
     METH_VARARGS, doc_get_history_item},
    {"get_current_history_length", (PyCFunction)get_current_history_length,
     METH_NOARGS, doc_get_current_history_length},
    {"set_history_length", set_history_length,
     METH_VARARGS, set_history_length_doc},
    {"get_history_length", get_history_length,
     METH_NOARGS, get_history_length_doc},
    {"set_completer", set_completer, METH_VARARGS, doc_set_completer},
    {"get_completer", get_completer, METH_NOARGS, doc_get_completer},
    {"get_completion_type", get_completion_type,
     METH_NOARGS, doc_get_completion_type},
    {"get_begidx", get_begidx, METH_NOARGS, doc_get_begidx},
    {"get_endidx", get_endidx, METH_NOARGS, doc_get_endidx},

    {"set_completer_delims", set_completer_delims,
     METH_VARARGS, doc_set_completer_delims},
    {"add_history", py_add_history, METH_VARARGS, doc_add_history},
    {"remove_history_item", py_remove_history, METH_VARARGS, doc_remove_history},
    {"replace_history_item", py_replace_history, METH_VARARGS, doc_replace_history},
    {"get_completer_delims", get_completer_delims,
     METH_NOARGS, doc_get_completer_delims},

    {"set_completion_display_matches_hook", set_completion_display_matches_hook,
     METH_VARARGS, doc_set_completion_display_matches_hook},
    {"set_startup_hook", set_startup_hook,
     METH_VARARGS, doc_set_startup_hook},
#ifdef HAVE_RL_PRE_INPUT_HOOK
    {"set_pre_input_hook", set_pre_input_hook,
     METH_VARARGS, doc_set_pre_input_hook},
#endif
#ifdef HAVE_RL_COMPLETION_APPEND_CHARACTER
    {"clear_history", py_clear_history, METH_NOARGS, doc_clear_history},
#endif
    {0, 0}
};


/* C function to call the Python hooks. */

static int
on_hook(PyObject *func)
{
    int result = 0;
    if (func != NULL) {
        PyObject *r;
        r = PyObject_CallFunction(func, NULL);
        if (r == NULL)
            goto error;
        if (r == Py_None)
            result = 0;
        else {
            result = PyLong_AsLong(r);
            if (result == -1 && PyErr_Occurred())
                goto error;
        }
        Py_DECREF(r);
        goto done;
      error:
        PyErr_Clear();
        Py_XDECREF(r);
      done:
        return result;
    }
    return result;
}

static int
#if defined(_RL_FUNCTION_TYPEDEF)
on_startup_hook(void)
#else
on_startup_hook()
#endif
{
    int r;
#ifdef WITH_THREAD
    PyGILState_STATE gilstate = PyGILState_Ensure();
#endif
    r = on_hook(readlinestate_global->startup_hook);
#ifdef WITH_THREAD
    PyGILState_Release(gilstate);
#endif
    return r;
}

#ifdef HAVE_RL_PRE_INPUT_HOOK
static int
#if defined(_RL_FUNCTION_TYPEDEF)
on_pre_input_hook(void)
#else
on_pre_input_hook()
#endif
{
    int r;
#ifdef WITH_THREAD
    PyGILState_STATE gilstate = PyGILState_Ensure();
#endif
    r = on_hook(readlinestate_global->pre_input_hook);
#ifdef WITH_THREAD
    PyGILState_Release(gilstate);
#endif
    return r;
}
#endif


/* C function to call the Python completion_display_matches */

#ifdef HAVE_RL_COMPLETION_DISPLAY_MATCHES_HOOK
static void
on_completion_display_matches_hook(char **matches,
                                   int num_matches, int max_length)
{
    int i;
    PyObject *m=NULL, *s=NULL, *r=NULL;
#ifdef WITH_THREAD
    PyGILState_STATE gilstate = PyGILState_Ensure();
#endif
    m = PyList_New(num_matches);
    if (m == NULL)
        goto error;
    for (i = 0; i < num_matches; i++) {
        s = PyUnicode_FromString(matches[i+1]);
        if (s == NULL)
            goto error;
        if (PyList_SetItem(m, i, s) == -1)
            goto error;
    }
    r = PyObject_CallFunction(readlinestate_global->completion_display_matches_hook,
                              "sOi", matches[0], m, max_length);

    Py_DECREF(m); m=NULL;

    if (r == NULL ||
        (r != Py_None && PyLong_AsLong(r) == -1 && PyErr_Occurred())) {
        goto error;
    }
    Py_CLEAR(r);

    if (0) {
    error:
        PyErr_Clear();
        Py_XDECREF(m);
        Py_XDECREF(r);
    }
#ifdef WITH_THREAD
    PyGILState_Release(gilstate);
#endif
}

#endif

/* C function to call the Python completer. */

static char *
on_completion(const char *text, int state)
{
    char *result = NULL;
    if (readlinestate_global->completer != NULL) {
        PyObject *r;
#ifdef WITH_THREAD
        PyGILState_STATE gilstate = PyGILState_Ensure();
#endif
        rl_attempted_completion_over = 1;
        r = PyObject_CallFunction(readlinestate_global->completer, "si", text, state);
        if (r == NULL)
            goto error;
        if (r == Py_None) {
            result = NULL;
        }
        else {
            char *s = _PyUnicode_AsString(r);
            if (s == NULL)
                goto error;
            result = strdup(s);
        }
        Py_DECREF(r);
        goto done;
      error:
        PyErr_Clear();
        Py_XDECREF(r);
      done:
#ifdef WITH_THREAD
        PyGILState_Release(gilstate);
#endif
        return result;
    }
    return result;
}


/* A more flexible constructor that saves the "begidx" and "endidx"
 * before calling the normal completer */

static char **
flex_complete(const char *text, int start, int end)
{
    char **result;
#ifdef WITH_THREAD
    PyGILState_STATE gilstate = PyGILState_Ensure();
#endif
#ifdef HAVE_RL_COMPLETION_APPEND_CHARACTER
    rl_completion_append_character ='\0';
#endif
#ifdef HAVE_RL_COMPLETION_SUPPRESS_APPEND
    rl_completion_suppress_append = 0;
#endif
    Py_XDECREF(readlinestate_global->begidx);
    Py_XDECREF(readlinestate_global->endidx);
    readlinestate_global->begidx = PyLong_FromLong((long) start);
    readlinestate_global->endidx = PyLong_FromLong((long) end);
    result = completion_matches(text, *on_completion);
#ifdef WITH_THREAD
    PyGILState_Release(gilstate);
#endif
    return result;
}


/* Helper to initialize GNU readline properly. */

static void
setup_readline(readlinestate *mod_state)
{
#ifdef SAVE_LOCALE
    char *saved_locale = strdup(setlocale(LC_CTYPE, NULL));
    if (!saved_locale)
        Py_FatalError("not enough memory to save locale");
#endif

#ifdef __APPLE__
    /* the libedit readline emulation resets key bindings etc
     * when calling rl_initialize.  So call it upfront
     */
    if (using_libedit_emulation)
        rl_initialize();

    /* Detect if libedit's readline emulation uses 0-based
     * indexing or 1-based indexing.
     */
    add_history("1");
    if (history_get(1) == NULL) {
        libedit_history_start = 0;
    } else {
        libedit_history_start = 1;
    }
    clear_history();
#endif /* __APPLE__ */

    using_history();

    rl_readline_name = "python";
    /* Force rebind of TAB to insert-tab */
    rl_bind_key('\t', rl_insert);
    /* Bind both ESC-TAB and ESC-ESC to the completion function */
    rl_bind_key_in_map ('\t', rl_complete, emacs_meta_keymap);
    rl_bind_key_in_map ('\033', rl_complete, emacs_meta_keymap);
    /* Set our hook functions */
    rl_startup_hook = on_startup_hook;
#ifdef HAVE_RL_PRE_INPUT_HOOK
    rl_pre_input_hook = on_pre_input_hook;
#endif
    /* Set our completion function */
    rl_attempted_completion_function = flex_complete;
    /* Set Python word break characters */
    completer_word_break_characters =
        rl_completer_word_break_characters =
        strdup(" \t\n`~!@#$%^&*()-=+[{]}\\|;:'\",<>/?");
        /* All nonalphanums except '.' */

    mod_state->begidx = PyLong_FromLong(0L);
    mod_state->endidx = PyLong_FromLong(0L);
    /* Initialize (allows .inputrc to override)
     *
     * XXX: A bug in the readline-2.2 library causes a memory leak
     * inside this function.  Nothing we can do about it.
     */
#ifdef __APPLE__
    if (using_libedit_emulation)
        rl_read_init_file(NULL);
    else
#endif /* __APPLE__ */
        rl_initialize();

    RESTORE_LOCALE(saved_locale)
}

/* Wrapper around GNU readline that handles signals differently. */


#if defined(HAVE_RL_CALLBACK) && defined(HAVE_SELECT)

static  char *completed_input_string;
static void
rlhandler(char *text)
{
    completed_input_string = text;
    rl_callback_handler_remove();
}

extern PyThreadState* _PyOS_ReadlineTState;

static char *
readline_until_enter_or_signal(const char *prompt, int *signal)
{
    char * not_done_reading = "";
    fd_set selectset;

    *signal = 0;
#ifdef HAVE_RL_CATCH_SIGNAL
    rl_catch_signals = 0;
#endif

    rl_callback_handler_install (prompt, rlhandler);
    FD_ZERO(&selectset);

    completed_input_string = not_done_reading;

    while (completed_input_string == not_done_reading) {
        int has_input = 0, err = 0;

        while (!has_input)
        {               struct timeval timeout = {0, 100000}; /* 0.1 seconds */

            /* [Bug #1552726] Only limit the pause if an input hook has been
               defined.  */
            struct timeval *timeoutp = NULL;
            if (PyOS_InputHook)
                timeoutp = &timeout;
            FD_SET(fileno(rl_instream), &selectset);
            /* select resets selectset if no input was available */
            has_input = select(fileno(rl_instream) + 1, &selectset,
                               NULL, NULL, timeoutp);
            err = errno;
            if(PyOS_InputHook) PyOS_InputHook();
        }

        if (has_input > 0) {
            rl_callback_read_char();
        }
        else if (err == EINTR) {
            int s;
#ifdef WITH_THREAD
            PyEval_RestoreThread(_PyOS_ReadlineTState);
#endif
            s = PyErr_CheckSignals();
#ifdef WITH_THREAD
            PyEval_SaveThread();
#endif
            if (s < 0) {
                rl_free_line_state();
                rl_cleanup_after_signal();
                rl_callback_handler_remove();
                *signal = 1;
                completed_input_string = NULL;
            }
        }
    }

    return completed_input_string;
}


#else

/* Interrupt handler */

static jmp_buf jbuf;

/* ARGSUSED */
static void
onintr(int sig)
{
    longjmp(jbuf, 1);
}


static char *
readline_until_enter_or_signal(const char *prompt, int *signal)
{
    PyOS_sighandler_t old_inthandler;
    char *p;

    *signal = 0;

    old_inthandler = PyOS_setsig(SIGINT, onintr);
    if (setjmp(jbuf)) {
#ifdef HAVE_SIGRELSE
        /* This seems necessary on SunOS 4.1 (Rasmus Hahn) */
        sigrelse(SIGINT);
#endif
        PyOS_setsig(SIGINT, old_inthandler);
        *signal = 1;
        return NULL;
    }
    rl_event_hook = PyOS_InputHook;
    p = readline(prompt);
    PyOS_setsig(SIGINT, old_inthandler);

    return p;
}
#endif /*defined(HAVE_RL_CALLBACK) && defined(HAVE_SELECT) */


static char *
call_readline(FILE *sys_stdin, FILE *sys_stdout, const char *prompt)
{
    size_t n;
    char *p, *q;
    int signal;

#ifdef SAVE_LOCALE
    char *saved_locale = strdup(setlocale(LC_CTYPE, NULL));
    if (!saved_locale)
        Py_FatalError("not enough memory to save locale");
    setlocale(LC_CTYPE, "");
#endif

    if (sys_stdin != rl_instream || sys_stdout != rl_outstream) {
        rl_instream = sys_stdin;
        rl_outstream = sys_stdout;
#ifdef HAVE_RL_COMPLETION_APPEND_CHARACTER
        rl_prep_terminal (1);
#endif
    }

    p = readline_until_enter_or_signal(prompt, &signal);

    /* we got an interrupt signal */
    if (signal) {
        RESTORE_LOCALE(saved_locale)
        return NULL;
    }

    /* We got an EOF, return a empty string. */
    if (p == NULL) {
        p = PyMem_RawMalloc(1);
        if (p != NULL)
            *p = '\0';
        RESTORE_LOCALE(saved_locale)
        return p;
    }

    /* we have a valid line */
    n = strlen(p);
    if (n > 0) {
        const char *line;
        int length = _py_get_history_length();
        if (length > 0)
#ifdef __APPLE__
            if (using_libedit_emulation) {
                /* handle older 0-based or newer 1-based indexing */
                line = (const char *)history_get(length + libedit_history_start - 1)->line;
            } else
#endif /* __APPLE__ */
            line = (const char *)history_get(length)->line;
        else
            line = "";
        if (strcmp(p, line))
            add_history(p);
    }
    /* Copy the malloc'ed buffer into a PyMem_Malloc'ed one and
       release the original. */
    q = p;
    p = PyMem_RawMalloc(n+2);
    if (p != NULL) {
        strncpy(p, q, n);
        p[n] = '\n';
        p[n+1] = '\0';
    }
    free(q);
    RESTORE_LOCALE(saved_locale)
    return p;
}


/* Initialize the module */

PyDoc_STRVAR(doc_module,
"Importing this module enables command line editing using GNU readline.");

#ifdef __APPLE__
PyDoc_STRVAR(doc_module_le,
"Importing this module enables command line editing using libedit readline.");
#endif /* __APPLE__ */

static struct PyModuleDef readlinemodule = {
    PyModuleDef_HEAD_INIT,
    "readline",
    doc_module,
    sizeof(readlinestate),
    readline_methods,
    NULL,
    readline_traverse,
    readline_clear,
    readline_free
};


PyMODINIT_FUNC
PyInit_readline(void)
{
    PyObject *m;
    readlinestate *mod_state;

#ifdef __APPLE__
    if (strncmp(rl_library_version, libedit_version_tag, strlen(libedit_version_tag)) == 0) {
        using_libedit_emulation = 1;
    }

    if (using_libedit_emulation)
        readlinemodule.m_doc = doc_module_le;

#endif /* __APPLE__ */

    m = PyModule_Create(&readlinemodule);

    if (m == NULL)
        return NULL;

    mod_state = (readlinestate *) PyModule_GetState(m);
    PyOS_ReadlineFunctionPointer = call_readline;
    setup_readline(mod_state);
    return m;
}
