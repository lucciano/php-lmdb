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
ZEND_BEGIN_ARG_INFO(void_arg, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(int_arg, 0)
ZEND_ARG_INFO(0, err)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(env_open_arg, 0)
ZEND_ARG_INFO(0, path)
ZEND_ARG_INFO(0, flags)
ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(env_set_mapsize_arg, 0)
ZEND_ARG_INFO(0, size)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(env_set_maxdbs_arg, 0)
ZEND_ARG_INFO(0, size)
ZEND_END_ARG_INFO()

/* {{{ lmdb_functions[]
 */
const zend_function_entry lmdb_functions[] = {
	PHP_FE(mdb_version, void_arg)
	PHP_FE(mdb_strerror, int_arg)
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

/* {{{ proto array mdb_version()
   Return the mdb library version information. */
PHP_FUNCTION(mdb_version) {

    array_init(return_value);


	int major;
	int minor;
	int patch;
	char* mdb_version_str = mdb_version(&major, &minor, &patch);
    add_assoc_string_ex(return_value, "version", sizeof("version"), mdb_version_str,0);
    add_assoc_long(return_value, "major", major);
    add_assoc_long(return_value, "minor", minor);
    add_assoc_long(return_value, "patch", patch);

    RETVAL_ZVAL(return_value,1,0);
}

/* {{{ proto string mdb_strerror(int err)
   Return a string describing a given error code. */
PHP_FUNCTION(mdb_strerror) {
	int err_code;
	char *error_str;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
                        &err_code) == FAILURE) {
                return;
        }
	error_str = mdb_strerror(err_code);
	RETURN_STRING(error_str,1);
}

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
/* {{{ proto lmDB\\Env::__construct()
   Instantiates a LmDB\\Env object*/
PHP_METHOD(lmdb_env, __construct)
{
	lmdb_env_object * intern = (lmdb_env_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
	int rtr = mdb_env_create	(& (intern->env));
	//@TODO: throw exception if rtr != 0;
}

/* {{{ proto lmDB\\Env::open($path, [$flags=0 [, $mode=0666]])
   Open a data base */
PHP_METHOD(lmdb_env, open)
{
	char * path;
	int path_len;
	long p_flags=0, p_mode=0;
	unsigned int flags = 0; 
	mdb_mode_t mode = 0;
	int rtr ;

	lmdb_env_object * intern = (lmdb_env_object *)zend_object_store_get_object(getThis() TSRMLS_CC);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|ll",
                        &path, &path_len, &p_flags, &p_mode) == FAILURE) {
                return;
        }
	flags = (unsigned int)p_flags; 
	if(p_mode != 0){
		mode = (mdb_mode_t)  p_mode;
	}else{
		mode = 0666;
	}

	rtr = mdb_env_open(intern->env, path, flags, mode);
	//TODO: throw exception instace of returning error code.
        RETURN_LONG(rtr);
}

/* {{{ proto lmDB\\Env::set_mapsize(size_t)
  Set the size of the memory map to use for this environment.*/
PHP_METHOD(lmdb_env, set_mapsize)
{
	size_t size = 0;
	int rtr;
	lmdb_env_object * intern = (lmdb_env_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &size) == FAILURE) {
                return;
        }
	rtr = mdb_env_set_mapsize(intern->env, size);
        RETURN_LONG(rtr);
}

/* {{{ proto lmDB\\Env::set_maxdbs(int)
  Set the maximum number of named databases for the environment.*/
PHP_METHOD(lmdb_env, set_maxdbs)
{
	int dbs = 0;
	int rtr;
	lmdb_env_object * intern = (lmdb_env_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &dbs) == FAILURE) {
                return;
        }
	rtr = mdb_env_set_mapsize(intern->env, dbs);
        RETURN_LONG(rtr);
}

zend_class_entry *php_lmdb_env_class_entry;
zend_class_entry *php_lmdb_val_class_entry;

static zend_function_entry php_lmdb_env_class_methods[] = {
        PHP_ME(lmdb_env, __construct, void_arg, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
        PHP_ME(lmdb_env, open, env_open_arg, ZEND_ACC_PUBLIC )
        PHP_ME(lmdb_env, set_mapsize, env_set_mapsize_arg, ZEND_ACC_PUBLIC )
        PHP_ME(lmdb_env, set_maxdbs, env_set_maxdbs_arg, ZEND_ACC_PUBLIC )
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

        /* Register constants */
	REGISTER_LONG_CONSTANT("MDB_FIXEDMAP", MDB_FIXEDMAP, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MDB_NOSUBDIR", MDB_NOSUBDIR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MDB_RDONLY", MDB_RDONLY, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MDB_WRITEMAP", MDB_WRITEMAP, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MDB_NOMETASYNC", MDB_NOMETASYNC, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MDB_NOSYNC", MDB_NOSYNC, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MDB_MAPASYNC", MDB_MAPASYNC, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MDB_NOTLS", MDB_NOTLS, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MDB_NOLOCK", MDB_NOLOCK, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MDB_NORDAHEAD", MDB_NORDAHEAD, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MDB_NOMEMINIT", MDB_NOMEMINIT, CONST_CS | CONST_PERSISTENT);

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
