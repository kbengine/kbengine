/* Path configuration like module_search_path (sys.path) */

#include "Python.h"
#include "osdefs.h"
#include "internal/pystate.h"
#include <wchar.h>

#ifdef __cplusplus
extern "C" {
#endif


_PyPathConfig _Py_path_config = _PyPathConfig_INIT;


void
_PyPathConfig_Clear(_PyPathConfig *config)
{
    /* _PyMem_SetDefaultAllocator() is needed to get a known memory allocator,
       since Py_SetPath(), Py_SetPythonHome() and Py_SetProgramName() can be
       called before Py_Initialize() which can changes the memory allocator. */
    PyMemAllocatorEx old_alloc;
    _PyMem_SetDefaultAllocator(PYMEM_DOMAIN_RAW, &old_alloc);

#define CLEAR(ATTR) \
    do { \
        PyMem_RawFree(ATTR); \
        ATTR = NULL; \
    } while (0)

    CLEAR(config->prefix);
    CLEAR(config->program_full_path);
#ifdef MS_WINDOWS
    CLEAR(config->dll_path);
#else
    CLEAR(config->exec_prefix);
#endif
    CLEAR(config->module_search_path);
    CLEAR(config->home);
    CLEAR(config->program_name);
#undef CLEAR

    PyMem_SetAllocator(PYMEM_DOMAIN_RAW, &old_alloc);
}


/* Initialize paths for Py_GetPath(), Py_GetPrefix(), Py_GetExecPrefix()
   and Py_GetProgramFullPath() */
_PyInitError
_PyPathConfig_Init(const _PyCoreConfig *core_config)
{
    if (_Py_path_config.module_search_path) {
        /* Already initialized */
        return _Py_INIT_OK();
    }

    _PyInitError err;
    _PyPathConfig new_config = _PyPathConfig_INIT;

    PyMemAllocatorEx old_alloc;
    _PyMem_SetDefaultAllocator(PYMEM_DOMAIN_RAW, &old_alloc);

    /* Calculate program_full_path, prefix, exec_prefix (Unix)
       or dll_path (Windows), and module_search_path */
    err = _PyPathConfig_Calculate(&new_config, core_config);
    if (_Py_INIT_FAILED(err)) {
        _PyPathConfig_Clear(&new_config);
        goto done;
    }

    /* Copy home and program_name from core_config */
    if (core_config->home != NULL) {
        new_config.home = _PyMem_RawWcsdup(core_config->home);
        if (new_config.home == NULL) {
            err = _Py_INIT_NO_MEMORY();
            goto done;
        }
    }
    else {
        new_config.home = NULL;
    }

    new_config.program_name = _PyMem_RawWcsdup(core_config->program_name);
    if (new_config.program_name == NULL) {
        err = _Py_INIT_NO_MEMORY();
        goto done;
    }

    _PyPathConfig_Clear(&_Py_path_config);
    _Py_path_config = new_config;

    err = _Py_INIT_OK();

done:
    PyMem_SetAllocator(PYMEM_DOMAIN_RAW, &old_alloc);
    return err;
}


static void
pathconfig_global_init(void)
{
    if (_Py_path_config.module_search_path) {
        /* Already initialized */
        return;
    }

    _PyInitError err;
    _PyCoreConfig config = _PyCoreConfig_INIT;

    err = _PyCoreConfig_Read(&config);
    if (_Py_INIT_FAILED(err)) {
        goto error;
    }

    err = _PyPathConfig_Init(&config);
    if (_Py_INIT_FAILED(err)) {
        goto error;
    }

    _PyCoreConfig_Clear(&config);
    return;

error:
    _PyCoreConfig_Clear(&config);
    _Py_FatalInitError(err);
}


/* External interface */

void
Py_SetPath(const wchar_t *path)
{
    if (path == NULL) {
        _PyPathConfig_Clear(&_Py_path_config);
        return;
    }

    PyMemAllocatorEx old_alloc;
    _PyMem_SetDefaultAllocator(PYMEM_DOMAIN_RAW, &old_alloc);

    _PyPathConfig new_config;
    new_config.program_full_path = _PyMem_RawWcsdup(Py_GetProgramName());
    new_config.prefix = _PyMem_RawWcsdup(L"");
#ifdef MS_WINDOWS
    new_config.dll_path = _PyMem_RawWcsdup(L"");
#else
    new_config.exec_prefix = _PyMem_RawWcsdup(L"");
#endif
    new_config.module_search_path = _PyMem_RawWcsdup(path);

    /* steal the home and program_name values (to leave them unchanged) */
    new_config.home = _Py_path_config.home;
    _Py_path_config.home = NULL;
    new_config.program_name = _Py_path_config.program_name;
    _Py_path_config.program_name = NULL;

    _PyPathConfig_Clear(&_Py_path_config);
    _Py_path_config = new_config;

    PyMem_SetAllocator(PYMEM_DOMAIN_RAW, &old_alloc);
}


void
Py_SetPythonHome(const wchar_t *home)
{
    if (home == NULL) {
        return;
    }

    PyMemAllocatorEx old_alloc;
    _PyMem_SetDefaultAllocator(PYMEM_DOMAIN_RAW, &old_alloc);

    PyMem_RawFree(_Py_path_config.home);
    _Py_path_config.home = _PyMem_RawWcsdup(home);

    PyMem_SetAllocator(PYMEM_DOMAIN_RAW, &old_alloc);

    if (_Py_path_config.home == NULL) {
        Py_FatalError("Py_SetPythonHome() failed: out of memory");
    }
}


void
Py_SetProgramName(const wchar_t *program_name)
{
    if (program_name == NULL || program_name[0] == L'\0') {
        return;
    }

    PyMemAllocatorEx old_alloc;
    _PyMem_SetDefaultAllocator(PYMEM_DOMAIN_RAW, &old_alloc);

    PyMem_RawFree(_Py_path_config.program_name);
    _Py_path_config.program_name = _PyMem_RawWcsdup(program_name);

    PyMem_SetAllocator(PYMEM_DOMAIN_RAW, &old_alloc);

    if (_Py_path_config.program_name == NULL) {
        Py_FatalError("Py_SetProgramName() failed: out of memory");
    }
}


void
_Py_SetProgramFullPath(const wchar_t *program_full_path)
{
    if (program_full_path == NULL || program_full_path[0] == L'\0') {
        return;
    }

    PyMemAllocatorEx old_alloc;
    _PyMem_SetDefaultAllocator(PYMEM_DOMAIN_RAW, &old_alloc);

    PyMem_RawFree(_Py_path_config.program_full_path);
    _Py_path_config.program_full_path = _PyMem_RawWcsdup(program_full_path);

    PyMem_SetAllocator(PYMEM_DOMAIN_RAW, &old_alloc);

    if (_Py_path_config.program_full_path == NULL) {
        Py_FatalError("Py_SetProgramFullPath() failed: out of memory");
    }
}


wchar_t *
Py_GetPath(void)
{
    pathconfig_global_init();
    return _Py_path_config.module_search_path;
}


wchar_t *
Py_GetPrefix(void)
{
    pathconfig_global_init();
    return _Py_path_config.prefix;
}


wchar_t *
Py_GetExecPrefix(void)
{
#ifdef MS_WINDOWS
    return Py_GetPrefix();
#else
    pathconfig_global_init();
    return _Py_path_config.exec_prefix;
#endif
}


wchar_t *
Py_GetProgramFullPath(void)
{
    pathconfig_global_init();
    return _Py_path_config.program_full_path;
}


wchar_t*
Py_GetPythonHome(void)
{
    pathconfig_global_init();
    return _Py_path_config.home;
}


wchar_t *
Py_GetProgramName(void)
{
    pathconfig_global_init();
    return _Py_path_config.program_name;
}

/* Compute argv[0] which will be prepended to sys.argv */
PyObject*
_PyPathConfig_ComputeArgv0(int argc, wchar_t **argv)
{
    wchar_t *argv0;
    wchar_t *p = NULL;
    Py_ssize_t n = 0;
    int have_script_arg = 0;
    int have_module_arg = 0;
#ifdef HAVE_READLINK
    wchar_t link[MAXPATHLEN+1];
    wchar_t argv0copy[2*MAXPATHLEN+1];
    int nr = 0;
#endif
#if defined(HAVE_REALPATH)
    wchar_t fullpath[MAXPATHLEN];
#elif defined(MS_WINDOWS)
    wchar_t fullpath[MAX_PATH];
#endif

    argv0 = argv[0];
    if (argc > 0 && argv0 != NULL) {
        have_module_arg = (wcscmp(argv0, L"-m") == 0);
        have_script_arg = !have_module_arg && (wcscmp(argv0, L"-c") != 0);
    }

    if (have_module_arg) {
        #if defined(HAVE_REALPATH) || defined(MS_WINDOWS)
            _Py_wgetcwd(fullpath, Py_ARRAY_LENGTH(fullpath));
            argv0 = fullpath;
            n = wcslen(argv0);
        #else
            argv0 = L".";
            n = 1;
        #endif
    }

#ifdef HAVE_READLINK
    if (have_script_arg)
        nr = _Py_wreadlink(argv0, link, MAXPATHLEN);
    if (nr > 0) {
        /* It's a symlink */
        link[nr] = '\0';
        if (link[0] == SEP)
            argv0 = link; /* Link to absolute path */
        else if (wcschr(link, SEP) == NULL)
            ; /* Link without path */
        else {
            /* Must join(dirname(argv0), link) */
            wchar_t *q = wcsrchr(argv0, SEP);
            if (q == NULL)
                argv0 = link; /* argv0 without path */
            else {
                /* Must make a copy, argv0copy has room for 2 * MAXPATHLEN */
                wcsncpy(argv0copy, argv0, MAXPATHLEN);
                q = wcsrchr(argv0copy, SEP);
                wcsncpy(q+1, link, MAXPATHLEN);
                q[MAXPATHLEN + 1] = L'\0';
                argv0 = argv0copy;
            }
        }
    }
#endif /* HAVE_READLINK */

#if SEP == '\\'
    /* Special case for Microsoft filename syntax */
    if (have_script_arg) {
        wchar_t *q;
#if defined(MS_WINDOWS)
        /* Replace the first element in argv with the full path. */
        wchar_t *ptemp;
        if (GetFullPathNameW(argv0,
                           Py_ARRAY_LENGTH(fullpath),
                           fullpath,
                           &ptemp)) {
            argv0 = fullpath;
        }
#endif
        p = wcsrchr(argv0, SEP);
        /* Test for alternate separator */
        q = wcsrchr(p ? p : argv0, '/');
        if (q != NULL)
            p = q;
        if (p != NULL) {
            n = p + 1 - argv0;
            if (n > 1 && p[-1] != ':')
                n--; /* Drop trailing separator */
        }
    }
#else /* All other filename syntaxes */
    if (have_script_arg) {
#if defined(HAVE_REALPATH)
        if (_Py_wrealpath(argv0, fullpath, Py_ARRAY_LENGTH(fullpath))) {
            argv0 = fullpath;
        }
#endif
        p = wcsrchr(argv0, SEP);
    }
    if (p != NULL) {
        n = p + 1 - argv0;
#if SEP == '/' /* Special case for Unix filename syntax */
        if (n > 1)
            n--; /* Drop trailing separator */
#endif /* Unix */
    }
#endif /* All others */

    return PyUnicode_FromWideChar(argv0, n);
}


/* Search for a prefix value in an environment file (pyvenv.cfg).
   If found, copy it into the provided buffer. */
int
_Py_FindEnvConfigValue(FILE *env_file, const wchar_t *key,
                       wchar_t *value, size_t value_size)
{
    int result = 0; /* meaning not found */
    char buffer[MAXPATHLEN*2+1];  /* allow extra for key, '=', etc. */

    fseek(env_file, 0, SEEK_SET);
    while (!feof(env_file)) {
        char * p = fgets(buffer, MAXPATHLEN*2, env_file);

        if (p == NULL) {
            break;
        }

        size_t n = strlen(p);
        if (p[n - 1] != '\n') {
            /* line has overflowed - bail */
            break;
        }
        if (p[0] == '#') {
            /* Comment - skip */
            continue;
        }

        wchar_t *tmpbuffer = _Py_DecodeUTF8_surrogateescape(buffer, n);
        if (tmpbuffer) {
            wchar_t * state;
            wchar_t * tok = wcstok(tmpbuffer, L" \t\r\n", &state);
            if ((tok != NULL) && !wcscmp(tok, key)) {
                tok = wcstok(NULL, L" \t", &state);
                if ((tok != NULL) && !wcscmp(tok, L"=")) {
                    tok = wcstok(NULL, L"\r\n", &state);
                    if (tok != NULL) {
                        wcsncpy(value, tok, MAXPATHLEN);
                        result = 1;
                        PyMem_RawFree(tmpbuffer);
                        break;
                    }
                }
            }
            PyMem_RawFree(tmpbuffer);
        }
    }
    return result;
}

#ifdef __cplusplus
}
#endif
