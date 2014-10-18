/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 *  dso.c -- DSO system function emulation for AIX
 *
 *  This is *only* intended for AIX < 4.3.
 */

/*
 *  Based on libdl (dlfcn.c/dlfcn.h) which is
 *  Copyright (c) 1992,1993,1995,1996,1997,1988
 *  Jens-Uwe Mager, Helios Software GmbH, Hannover, Germany.
 *
 *  Not derived from licensed software.
 *
 *  Permission is granted to freely use, copy, modify, and redistribute
 *  this software, provided that the author is not construed to be liable
 *  for any results of using the software, alterations are clearly marked
 *  as such, and this notice is not modified.
 *
 *  Changes marked with `--jwe' were made on April 7 1996 by
 *  John W. Eaton <jwe@bevo.che.wisc.edu> to support g++ 
 *
 *  Bundled, stripped and adjusted on April 1998 as one single source file 
 *  for inclusion into the Apache HTTP server by 
 *  Ralf S. Engelschall <rse@apache.org>
 *
 *  Added to APR by David Reid April 2000
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ldr.h>
#include <a.out.h>
#include "apr_arch_dso.h"
#include "apr_portable.h"

#if APR_HAS_DSO

#undef FREAD
#undef FWRITE
#include <ldfcn.h>

/*
 * AIX 4.3 does remove some useful definitions from ldfcn.h. Define
 * these here to compensate for that lossage.
 */
#ifndef BEGINNING
#define BEGINNING SEEK_SET
#endif
#ifndef FSEEK
#define FSEEK(ldptr,o,p)   fseek(IOPTR(ldptr),(p==BEGINNING)?(OFFSET(ldptr) +o):o,p)
#endif
#ifndef FREAD
#define FREAD(p,s,n,ldptr) fread(p,s,n,IOPTR(ldptr))
#endif

/*
 * Mode flags for the dlopen routine.
 */
#undef  RTLD_LAZY
#define RTLD_LAZY	1	/* lazy function call binding */
#undef  RTLD_NOW
#define RTLD_NOW	2	/* immediate function call binding */
#undef  RTLD_GLOBAL
#define RTLD_GLOBAL	0x100	/* allow symbols to be global */

/*
 * To be able to initialize, a library may provide a dl_info structure
 * that contains functions to be called to initialize and terminate.
 */
struct dl_info {
    void (*init) (void);
    void (*fini) (void);
};

/* APR functions...
 *
 * As the AIX functions have been declared in the header file we just
 * add the basic "wrappers" here.
 */

APR_DECLARE(apr_status_t) apr_os_dso_handle_put(apr_dso_handle_t **aprdso,
                                                apr_os_dso_handle_t osdso,
                                                apr_pool_t *pool)
{
    *aprdso = apr_pcalloc(pool, sizeof **aprdso);
    (*aprdso)->handle = osdso;
    (*aprdso)->pool = pool;
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_os_dso_handle_get(apr_os_dso_handle_t *osdso,
                                                apr_dso_handle_t *aprdso)
{
    *osdso = aprdso->handle;
    return APR_SUCCESS;
}

static apr_status_t dso_cleanup(void *thedso)
{
    apr_dso_handle_t *dso = thedso;

    if (dso->handle != NULL && dlclose(dso->handle) != 0)
        return APR_EINIT;
    dso->handle = NULL;

    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_dso_load(apr_dso_handle_t **res_handle, 
                                       const char *path, apr_pool_t *ctx)
{
    void *os_handle = dlopen((char *)path, RTLD_NOW | RTLD_GLOBAL);

    *res_handle = apr_pcalloc(ctx, sizeof(*res_handle));

    if(os_handle == NULL) {
        (*res_handle)->errormsg = dlerror();       
        return APR_EDSOOPEN;
    }

    (*res_handle)->handle = (void*)os_handle;
    (*res_handle)->pool = ctx;
    (*res_handle)->errormsg = NULL;

    apr_pool_cleanup_register(ctx, *res_handle, dso_cleanup, apr_pool_cleanup_null);

    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_dso_unload(apr_dso_handle_t *handle)
{
    return apr_pool_cleanup_run(handle->pool, handle, dso_cleanup);
}

APR_DECLARE(apr_status_t) apr_dso_sym(apr_dso_handle_sym_t *ressym, 
                                      apr_dso_handle_t *handle, 
                                      const char *symname)
{
    void *retval = dlsym(handle->handle, symname);

    if (retval == NULL) {
        handle->errormsg = dlerror();
        return APR_ESYMNOTFOUND;
    }

    *ressym = retval;
    return APR_SUCCESS;
}

APR_DECLARE(const char *) apr_dso_error(apr_dso_handle_t *dso, char *buffer, apr_size_t buflen)
{
    if (dso->errormsg) {
        apr_cpystrn(buffer, dso->errormsg, buflen);
        return dso->errormsg;
    }
    return "No Error";
}



/*
 * We simulate dlopen() et al. through a call to load. Because AIX has
 * no call to find an exported symbol we read the loader section of the
 * loaded module and build a list of exported symbols and their virtual
 * address.
 */

typedef struct {
    char *name;			/* the symbols's name */
    void *addr;			/* its relocated virtual address */
} Export, *ExportPtr;

/*
 * xlC uses the following structure to list its constructors and
 * destructors. This is gleaned from the output of munch.
 */
typedef struct {
    void (*init) (void);	/* call static constructors */
    void (*term) (void);	/* call static destructors */
} Cdtor, *CdtorPtr;

typedef void (*GccCDtorPtr) (void);

/*
 * The void * handle returned from dlopen is actually a ModulePtr.
 */
typedef struct Module {
    struct Module *next;
    char *name;			/* module name for refcounting */
    int refCnt;			/* the number of references */
    void *entry;		/* entry point from load */
    struct dl_info *info;	/* optional init/terminate functions */
    CdtorPtr cdtors;		/* optional C++ constructors */
    GccCDtorPtr gcc_ctor;	/* g++ constructors  --jwe */
    GccCDtorPtr gcc_dtor;	/* g++ destructors  --jwe */
    int nExports;		/* the number of exports found */
    ExportPtr exports;		/* the array of exports */
} Module, *ModulePtr;

/*
 * We keep a list of all loaded modules to be able to call the fini
 * handlers and destructors at atexit() time.
 */
static ModulePtr modList;

/*
 * The last error from one of the dl* routines is kept in static
 * variables here. Each error is returned only once to the caller.
 */
static char errbuf[BUFSIZ];
static int errvalid;

/*
 * The `fixed' gcc header files on AIX 3.2.5 provide a prototype for
 * strdup().  --jwe
 */
extern char *strdup(const char *);
static void caterr(char *);
static int readExports(ModulePtr);
static void terminate(void);
static void *findMain(void);

void *dlopen(const char *path, int mode)
{
    register ModulePtr mp;
    static void *mainModule;

    /*
     * Upon the first call register a terminate handler that will
     * close all libraries. Also get a reference to the main module
     * for use with loadbind.
     */
    if (!mainModule) {
	if ((mainModule = findMain()) == NULL)
	    return NULL;
	atexit(terminate);
    }
    /*
     * Scan the list of modules if we have the module already loaded.
     */
    for (mp = modList; mp; mp = mp->next)
	if (strcmp(mp->name, path) == 0) {
	    mp->refCnt++;
	    return mp;
	}
    if ((mp = (ModulePtr) calloc(1, sizeof(*mp))) == NULL) {
	errvalid++;
	strcpy(errbuf, "calloc: ");
	strcat(errbuf, strerror(errno));
	return NULL;
    }
    if ((mp->name = strdup(path)) == NULL) {
	errvalid++;
	strcpy(errbuf, "strdup: ");
	strcat(errbuf, strerror(errno));
	free(mp);
	return NULL;
    }
    /*
     * load should be declared load(const char *...). Thus we
     * cast the path to a normal char *. Ugly.
     */
    if ((mp->entry = (void *) loadAndInit((char *) path, L_NOAUTODEFER, NULL)) == NULL) {
	free(mp->name);
	free(mp);
	errvalid++;
	strcpy(errbuf, "dlopen: ");
	strcat(errbuf, path);
	strcat(errbuf, ": ");
	/*
	 * If AIX says the file is not executable, the error
	 * can be further described by querying the loader about
	 * the last error.
	 */
	if (errno == ENOEXEC) {
	    char *tmp[BUFSIZ / sizeof(char *)];
	    if (loadquery(L_GETMESSAGES, tmp, sizeof(tmp)) == -1)
		strcpy(errbuf, strerror(errno));
	    else {
		char **p;
		for (p = tmp; *p; p++)
		    caterr(*p);
	    }
	}
	else
	    strcat(errbuf, strerror(errno));
	return NULL;
    }
    mp->refCnt = 1;
    mp->next = modList;
    modList = mp;
    if (loadbind(0, mainModule, mp->entry) == -1) {
	dlclose(mp);
	errvalid++;
	strcpy(errbuf, "loadbind: ");
	strcat(errbuf, strerror(errno));
	return NULL;
    }
    /*
     * If the user wants global binding, loadbind against all other
     * loaded modules.
     */
    if (mode & RTLD_GLOBAL) {
	register ModulePtr mp1;
	for (mp1 = mp->next; mp1; mp1 = mp1->next)
	    if (loadbind(0, mp1->entry, mp->entry) == -1) {
		dlclose(mp);
		errvalid++;
		strcpy(errbuf, "loadbind: ");
		strcat(errbuf, strerror(errno));
		return NULL;
	    }
    }
    if (readExports(mp) == -1) {
	dlclose(mp);
	return NULL;
    }
    /*
     * If there is a dl_info structure, call the init function.
     */
    if (mp->info = (struct dl_info *) dlsym(mp, "dl_info")) {
	if (mp->info->init)
	    (*mp->info->init) ();
    }
    else
	errvalid = 0;
    /*
     * If the shared object was compiled using xlC we will need
     * to call static constructors (and later on dlclose destructors).
     */
    if (mp->cdtors = (CdtorPtr) dlsym(mp, "__cdtors")) {
	CdtorPtr cp = mp->cdtors;
	while (cp->init || cp->term) {
	    if (cp->init && cp->init != (void (*)(void)) 0xffffffff)
		(*cp->init) ();
	    cp++;
	}
	/*
	 * If the shared object was compiled using g++, we will need
	 * to call global constructors using the _GLOBAL__DI function,
	 * and later, global destructors using the _GLOBAL_DD
	 * funciton.  --jwe
	 */
    }
    else if (mp->gcc_ctor = (GccCDtorPtr) dlsym(mp, "_GLOBAL__DI")) {
	(*mp->gcc_ctor) ();
	mp->gcc_dtor = (GccCDtorPtr) dlsym(mp, "_GLOBAL__DD");
    }
    else
	errvalid = 0;
    return mp;
}

/*
 * Attempt to decipher an AIX loader error message and append it
 * to our static error message buffer.
 */
static void caterr(char *s)
{
    register char *p = s;

    while (*p >= '0' && *p <= '9')
	p++;
    switch (atoi(s)) {
    case L_ERROR_TOOMANY:
	strcat(errbuf, "to many errors");
	break;
    case L_ERROR_NOLIB:
	strcat(errbuf, "can't load library");
	strcat(errbuf, p);
	break;
    case L_ERROR_UNDEF:
	strcat(errbuf, "can't find symbol");
	strcat(errbuf, p);
	break;
    case L_ERROR_RLDBAD:
	strcat(errbuf, "bad RLD");
	strcat(errbuf, p);
	break;
    case L_ERROR_FORMAT:
	strcat(errbuf, "bad exec format in");
	strcat(errbuf, p);
	break;
    case L_ERROR_ERRNO:
	strcat(errbuf, strerror(atoi(++p)));
	break;
    default:
	strcat(errbuf, s);
	break;
    }
}

void *dlsym(void *handle, const char *symbol)
{
    register ModulePtr mp = (ModulePtr) handle;
    register ExportPtr ep;
    register int i;

    /*
     * Could speed up the search, but I assume that one assigns
     * the result to function pointers anyways.
     */
    for (ep = mp->exports, i = mp->nExports; i; i--, ep++)
	if (strcmp(ep->name, symbol) == 0)
	    return ep->addr;
    errvalid++;
    strcpy(errbuf, "dlsym: undefined symbol ");
    strcat(errbuf, symbol);
    return NULL;
}

const char *dlerror(void)
{
    if (errvalid) {
	errvalid = 0;
	return errbuf;
    }
    return NULL;
}

int dlclose(void *handle)
{
    register ModulePtr mp = (ModulePtr) handle;
    int result;
    register ModulePtr mp1;

    if (--mp->refCnt > 0)
	return 0;
    if (mp->info && mp->info->fini)
	(*mp->info->fini) ();
    if (mp->cdtors) {
	CdtorPtr cp = mp->cdtors;
	while (cp->init || cp->term) {
	    if (cp->term && cp->init != (void (*)(void)) 0xffffffff)
		(*cp->term) ();
	    cp++;
	}
	/*
	 * If the function to handle global destructors for g++
	 * exists, call it.  --jwe
	 */
    }
    else if (mp->gcc_dtor) {
	(*mp->gcc_dtor) ();
    }
    result = unload(mp->entry);
    if (result == -1) {
	errvalid++;
	strcpy(errbuf, strerror(errno));
    }
    if (mp->exports) {
	register ExportPtr ep;
	register int i;
	for (ep = mp->exports, i = mp->nExports; i; i--, ep++)
	    if (ep->name)
		free(ep->name);
	free(mp->exports);
    }
    if (mp == modList)
	modList = mp->next;
    else {
	for (mp1 = modList; mp1; mp1 = mp1->next)
	    if (mp1->next == mp) {
		mp1->next = mp->next;
		break;
	    }
    }
    free(mp->name);
    free(mp);
    return result;
}

static void terminate(void)
{
    while (modList)
	dlclose(modList);
}

/*
 * Build the export table from the XCOFF .loader section.
 */
static int readExports(ModulePtr mp)
{
    LDFILE *ldp = NULL;
    SCNHDR sh, shdata;
    LDHDR *lhp;
    char *ldbuf;
    LDSYM *ls;
    int i;
    ExportPtr ep;
    struct ld_info *lp;
    char *buf;
    int size = 4 * 1024;
    void *dataorg;

    /*
     * The module might be loaded due to the LIBPATH
     * environment variable. Search for the loaded
     * module using L_GETINFO.
     */
    if ((buf = malloc(size)) == NULL) {
	errvalid++;
	strcpy(errbuf, "readExports: ");
	strcat(errbuf, strerror(errno));
	return -1;
    }
    while ((i = loadquery(L_GETINFO, buf, size)) == -1 && errno == ENOMEM) {
	free(buf);
	size += 4 * 1024;
	if ((buf = malloc(size)) == NULL) {
	    errvalid++;
	    strcpy(errbuf, "readExports: ");
	    strcat(errbuf, strerror(errno));
	    return -1;
	}
    }
    if (i == -1) {
	errvalid++;
	strcpy(errbuf, "readExports: ");
	strcat(errbuf, strerror(errno));
	free(buf);
	return -1;
    }
    /*
     * Traverse the list of loaded modules. The entry point
     * returned by load() does actually point to the TOC
     * entry contained in the data segment.
     */
    lp = (struct ld_info *) buf;
    while (lp) {
	if ((unsigned long) mp->entry >= (unsigned long) lp->ldinfo_dataorg &&
	    (unsigned long) mp->entry < (unsigned long) lp->ldinfo_dataorg +
	    lp->ldinfo_datasize) {
	    dataorg = lp->ldinfo_dataorg;
	    ldp = ldopen(lp->ldinfo_filename, ldp);
	    break;
	}
	if (lp->ldinfo_next == 0)
	    lp = NULL;
	else
	    lp = (struct ld_info *) ((char *) lp + lp->ldinfo_next);
    }
    free(buf);
    if (!ldp) {
	errvalid++;
	strcpy(errbuf, "readExports: ");
	strcat(errbuf, strerror(errno));
	return -1;
    }
    if (TYPE(ldp) != U802TOCMAGIC) {
	errvalid++;
	strcpy(errbuf, "readExports: bad magic");
	while (ldclose(ldp) == FAILURE);
	return -1;
    }
    /*
     * Get the padding for the data section. This is needed for
     * AIX 4.1 compilers. This is used when building the final
     * function pointer to the exported symbol.
     */
    if (ldnshread(ldp, _DATA, &shdata) != SUCCESS) {
	errvalid++;
	strcpy(errbuf, "readExports: cannot read data section header");
	while (ldclose(ldp) == FAILURE);
	return -1;
    }
    if (ldnshread(ldp, _LOADER, &sh) != SUCCESS) {
	errvalid++;
	strcpy(errbuf, "readExports: cannot read loader section header");
	while (ldclose(ldp) == FAILURE);
	return -1;
    }
    /*
     * We read the complete loader section in one chunk, this makes
     * finding long symbol names residing in the string table easier.
     */
    if ((ldbuf = (char *) malloc(sh.s_size)) == NULL) {
	errvalid++;
	strcpy(errbuf, "readExports: ");
	strcat(errbuf, strerror(errno));
	while (ldclose(ldp) == FAILURE);
	return -1;
    }
    if (FSEEK(ldp, sh.s_scnptr, BEGINNING) != OKFSEEK) {
	errvalid++;
	strcpy(errbuf, "readExports: cannot seek to loader section");
	free(ldbuf);
	while (ldclose(ldp) == FAILURE);
	return -1;
    }
    if (FREAD(ldbuf, sh.s_size, 1, ldp) != 1) {
	errvalid++;
	strcpy(errbuf, "readExports: cannot read loader section");
	free(ldbuf);
	while (ldclose(ldp) == FAILURE);
	return -1;
    }
    lhp = (LDHDR *) ldbuf;
    ls = (LDSYM *) (ldbuf + LDHDRSZ);
    /*
     * Count the number of exports to include in our export table.
     */
    for (i = lhp->l_nsyms; i; i--, ls++) {
	if (!LDR_EXPORT(*ls))
	    continue;
	mp->nExports++;
    }
    if ((mp->exports = (ExportPtr) calloc(mp->nExports, sizeof(*mp->exports))) == NULL) {
	errvalid++;
	strcpy(errbuf, "readExports: ");
	strcat(errbuf, strerror(errno));
	free(ldbuf);
	while (ldclose(ldp) == FAILURE);
	return -1;
    }
    /*
     * Fill in the export table. All entries are relative to
     * the beginning of the data origin.
     */
    ep = mp->exports;
    ls = (LDSYM *) (ldbuf + LDHDRSZ);
    for (i = lhp->l_nsyms; i; i--, ls++) {
	char *symname;
	char tmpsym[SYMNMLEN + 1];
	if (!LDR_EXPORT(*ls))
	    continue;
	if (ls->l_zeroes == 0)
	    symname = ls->l_offset + lhp->l_stoff + ldbuf;
	else {
	    /*
	     * The l_name member is not zero terminated, we
	     * must copy the first SYMNMLEN chars and make
	     * sure we have a zero byte at the end.
	     */
	    strncpy(tmpsym, ls->l_name, SYMNMLEN);
	    tmpsym[SYMNMLEN] = '\0';
	    symname = tmpsym;
	}
	ep->name = strdup(symname);
	ep->addr = (void *) ((unsigned long) dataorg +
			     ls->l_value - shdata.s_vaddr);
	ep++;
    }
    free(ldbuf);
    while (ldclose(ldp) == FAILURE);
    return 0;
}

/*
 * Find the main modules data origin. This is used as export pointer
 * for loadbind() to be able to resolve references to the main part.
 */
static void *findMain(void)
{
    struct ld_info *lp;
    char *buf;
    int size = 4 * 1024;
    int i;
    void *ret;

    if ((buf = malloc(size)) == NULL) {
	errvalid++;
	strcpy(errbuf, "findMain: ");
	strcat(errbuf, strerror(errno));
	return NULL;
    }
    while ((i = loadquery(L_GETINFO, buf, size)) == -1 && errno == ENOMEM) {
	free(buf);
	size += 4 * 1024;
	if ((buf = malloc(size)) == NULL) {
	    errvalid++;
	    strcpy(errbuf, "findMain: ");
	    strcat(errbuf, strerror(errno));
	    return NULL;
	}
    }
    if (i == -1) {
	errvalid++;
	strcpy(errbuf, "findMain: ");
	strcat(errbuf, strerror(errno));
	free(buf);
	return NULL;
    }
    /*
     * The first entry is the main module. The data segment
     * starts with the TOC entries for all exports, so the
     * data segment origin works as argument for loadbind.
     */
    lp = (struct ld_info *) buf;
    ret = lp->ldinfo_dataorg;
    free(buf);
    return ret;
}

#endif
