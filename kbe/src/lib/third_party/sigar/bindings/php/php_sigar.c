/*
 * Copyright (c) 2007 Hyperic, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_sigar.h"
#include "sigar.h"
#include "sigar_fileinfo.h"
#include "sigar_format.h"

#define PHP_SIGAR_RETURN_STRING(s) \
    RETURN_STRING(s, 1)

static char *php_sigar_net_address_to_string(sigar_net_address_t address)
{
    char addr_str[SIGAR_INET6_ADDRSTRLEN];
    sigar_net_address_to_string(NULL, &address, addr_str);
    return estrdup(addr_str);
}

#define PHP_SIGAR_RETURN_NETADDR(a) \
    RETURN_STRING(php_sigar_net_address_to_string(a), 0)

#define zSIGAR_PARSE_PID \
    zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &pid)

#define zSIGAR_PARSE_NAME \
    zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_len)

#define zSIGAR_OBJ \
    (((php_sigar_obj_t *)zend_object_store_get_object(getThis() TSRMLS_CC))->ptr)

#define zSIGAR \
    sigar_t *sigar = (sigar_t *)zSIGAR_OBJ

typedef struct {
    zend_object std;
    void *ptr;
} php_sigar_obj_t;

static void php_sigar_obj_dtor(void *object TSRMLS_DC)
{
    php_sigar_obj_t *intern = (php_sigar_obj_t *)object;

    zend_object_std_dtor(&intern->std TSRMLS_CC);
    if (intern->ptr) {
        efree(intern->ptr);
    }
    efree(intern);
}

static zend_object_value php_sigar_ctor(zend_objects_free_object_storage_t dtor,
                                        zend_object_handlers *handlers,
                                        php_sigar_obj_t **obj,
                                        zend_class_entry *class_type)
{
    php_sigar_obj_t *intern = emalloc(sizeof(*intern));
    zend_object_value retval;

    memset(intern, 0, sizeof(*intern));
    if (obj) {
        *obj = intern;
    }

    zend_object_std_init(&intern->std, class_type TSRMLS_CC);

    retval.handle = zend_objects_store_put(intern,
                                           NULL,
                                           dtor,
                                           NULL TSRMLS_CC);

    retval.handlers = handlers;

    return retval;
}

static void php_sigar_close(void *object TSRMLS_DC)
{
    php_sigar_obj_t *intern = (php_sigar_obj_t *)object;

    if (!intern) {
        return;
    }

    if (intern->ptr) {
        sigar_close((sigar_t *)intern->ptr);
        intern->ptr = NULL;
    }

    php_sigar_obj_dtor(object);
}

static zend_object_handlers php_sigar_object_handlers;

static zend_object_value php_sigar_new(zend_class_entry *class_type TSRMLS_DC)
{
    php_sigar_obj_t *intern;

    zend_object_value retval =
        php_sigar_ctor(php_sigar_close,
                       &php_sigar_object_handlers,
                       &intern,
                       class_type);

    sigar_open(&((sigar_t *)intern->ptr));

    return retval;
}

static php_sigar_obj_t *php_sigar_obj_new(char *name, zval *object TSRMLS_DC)
{
    zend_class_entry **ce;
    zend_lookup_class(name, strlen(name), &ce TSRMLS_CC);
    Z_TYPE_P(object) = IS_OBJECT;
    object_init_ex(object, *ce);
    object->refcount = 1;
    object->is_ref = 0;
    return (php_sigar_obj_t *)zend_object_store_get_object(object TSRMLS_CC);
}

#define PHP_SIGAR_INIT_HANDLERS(handlers) \
    memcpy(&handlers, zend_get_std_object_handlers(), sizeof(handlers)); \
    handlers.clone_obj = NULL

#include "php_sigar_generated.c"

static zend_function_entry php_sigar_class_functions[] = {
    PHP_SIGAR_FUNCTIONS
    {NULL, NULL, NULL}
};

static PHP_MINIT_FUNCTION(sigar)
{
    zend_class_entry ce;

    PHP_SIGAR_INIT_HANDLERS(php_sigar_object_handlers);
    INIT_CLASS_ENTRY(ce, "Sigar", php_sigar_class_functions);
    ce.create_object = php_sigar_new;
    zend_register_internal_class(&ce TSRMLS_CC);

    PHP_SIGAR_INIT;

    return SUCCESS;
}

zend_function_entry php_sigar_functions[] = {
    {NULL, NULL, NULL}
};

zend_module_entry sigar_module_entry = {
    STANDARD_MODULE_HEADER,
    "sigar",
    php_sigar_functions,
    PHP_MINIT(sigar),
    NULL,
    NULL,
    NULL,
    NULL,
    "0.1",
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_SIGAR
ZEND_GET_MODULE(sigar)
#endif
