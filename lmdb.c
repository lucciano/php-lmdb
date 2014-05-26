/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2012 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Luciano Andrade <andrade.luciano@gmail.com>                  |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "Zend/zend_exceptions.h"
#include "Zend/zend_interfaces.h"
#include "php_lmdb.h"

#include <lmdb.h>

#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION < 4)
# define LMDB_SAFE_MODE_CHECK(file) || (PG(safe_mode) && !php_checkuid((file), "rb+", CHECKUID_CHECK_MODE_PARAM))
#else
# define LMDB_SAFE_MODE_CHECK(file)
#endif

#define LMDB_CHECK_OPEN_BASEDIR(file) \
	if (php_check_open_basedir((file) TSRMLS_CC) LMDB_SAFE_MODE_CHECK((file))){ \
		RETURN_FALSE; \
	}

#ifndef PHP_FE_END
# define PHP_FE_END { NULL, NULL, NULL, 0, 0 }
#endif

#if ZEND_MODULE_API_NO < 20090626
# define Z_ADDREF_P(arg) ZVAL_ADDREF(arg)
# define Z_ADDREF_PP(arg) ZVAL_ADDREF(*(arg))
# define Z_DELREF_P(arg) ZVAL_DELREF(arg)
# define Z_DELREF_PP(arg) ZVAL_DELREF(*(arg))
#endif

#ifndef zend_parse_parameters_none
# define zend_parse_parameters_none() zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "")
#endif

#if ZEND_MODULE_API_NO >= 20100525
#define init_properties(intern) object_properties_init(&intern->std, class_type)
#else
#define init_properties(intern) do { \
	zval *tmp; \
	zend_hash_copy(intern->std.properties, \
    &class_type->default_properties, (copy_ctor_func_t) zval_add_ref,  \
    (void *) &tmp, sizeof(zval *)); \
} while(0)
#endif

#define php_lmdb_obj_new(obj, class_type)					\
  zend_object_value retval;										\
  obj *intern;													\
																\
  intern = (obj *)emalloc(sizeof(obj));               			\
  memset(intern, 0, sizeof(obj));                          		\
                                                                \
  zend_object_std_init(&intern->std, class_type TSRMLS_CC);     \
  init_properties(intern);										\
                                                                \
  retval.handle = zend_objects_store_put(intern,				\
     (zend_objects_store_dtor_t) zend_objects_destroy_object,	\
     php_##obj##_free, NULL TSRMLS_CC);							\
  retval.handlers = &lmdb_default_handlers;					\
                                                                \
  return retval;

/* arg info */
ZEND_BEGIN_ARG_INFO_EX(arginfo_lmdb_void, 0, 0, 0)
ZEND_END_ARG_INFO()



/* {{{ lmdb_functions[]
 */
const zend_function_entry lmdb_functions[] = {
	PHP_FE_END	/* Must be the last line in lmdb_functions[] */
};
/* }}} */

/* {{{ lmdb_module_entry
 */
zend_module_entry lmdb_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"lmdb",
	lmdb_functions,
	PHP_MINIT(lmdb),
	PHP_MSHUTDOWN(lmdb),
	NULL,
	NULL,
	PHP_MINFO(lmdb),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_LMDB_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_LMDB
ZEND_GET_MODULE(lmdb)
#endif

/* Objects */
typedef struct {
	zend_object std;
	MDB_env *env;
} lmdb_env_object;

typedef struct {
	zend_object std;
	MDB_val *val;
} lmdb_val_object;


/* Handlers */
static zend_object_handlers lmdb_default_handlers;
static zend_object_handlers lmdb_object_handlers;
static zend_object_handlers lmdb_iterator_object_handlers;

/* Class entries */
/* {{{ proto LevelDB LevelDB::__construct(string $name [, array $options [, array $readoptions [, array $writeoptions]]])
   Instantiates a LevelDB object and opens the give database */
PHP_METHOD(lmdb_env, __construct)
{
	lmdb_env_object * intern = (lmdb_env_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
	mdb_env_create	(& (intern->env));
}

zend_class_entry *php_lmdb_env_class_entry;
zend_class_entry *php_lmdb_val_class_entry;

static zend_function_entry php_lmdb_env_class_methods[] = {
        PHP_ME(lmdb_env, __construct, arginfo_lmdb_void , ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_FE_END
};

static zend_function_entry php_lmdb_val_class_methods[] = {
	PHP_FE_END
};

void php_lmdb_val_object_free(void *object TSRMLS_DC)
{
	zend_objects_free_object_storage((zend_object *)object TSRMLS_CC);
}

static zend_object_value php_lmdb_val_object_new(zend_class_entry *class_type TSRMLS_DC)
{
	php_lmdb_obj_new(lmdb_val_object, class_type);
}

void php_lmdb_env_object_free(void *object TSRMLS_DC)
{
	zend_objects_free_object_storage((zend_object *)object TSRMLS_CC);
}

static zend_object_value php_lmdb_env_object_new(zend_class_entry *class_type TSRMLS_DC)
{
	php_lmdb_obj_new(lmdb_env_object, class_type);
}

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(lmdb)
{
	zend_class_entry ce;
	zend_class_entry *exception_ce = zend_exception_get_default(TSRMLS_C);

	memcpy(&lmdb_default_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	memcpy(&lmdb_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	memcpy(&lmdb_iterator_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

	/* Register Lmdb Env Class */
	INIT_CLASS_ENTRY(ce, "Lmdb\\Env", php_lmdb_env_class_methods);
	ce.create_object = php_lmdb_env_object_new;
	php_lmdb_env_class_entry = zend_register_internal_class(&ce TSRMLS_CC);

	/* Register Lmdb val Class */
	INIT_CLASS_ENTRY(ce, "Lmdb\\Val", php_lmdb_val_class_methods);
	ce.create_object = php_lmdb_val_object_new;
	php_lmdb_val_class_entry = zend_register_internal_class(&ce TSRMLS_CC);

	zend_declare_property_long(php_lmdb_val_class_entry, "mv_size", strlen("mv_size"), 0, ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_null(php_lmdb_val_class_entry, "mv_data", strlen("mv_data"),  ZEND_ACC_PUBLIC TSRMLS_CC);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(lmdb)
{
       return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(lmdb)
{
	char tmp[32];
	snprintf(tmp, 32, "%d.%d.%d",MDB_VERSION_MAJOR, MDB_VERSION_MINOR, MDB_VERSION_PATCH);

	php_info_print_table_start();
	php_info_print_table_header(2, "lmdb support", "enabled");
	php_info_print_table_row(2, "lmdb extension version", PHP_LMDB_VERSION);
	php_info_print_table_row(2, "lmdb library version", tmp);
	php_info_print_table_end();
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */