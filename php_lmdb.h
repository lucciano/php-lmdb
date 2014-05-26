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

#ifndef PHP_LMDB_H
#define PHP_LMDB_H

#define PHP_LMDB_VERSION "0.1.0"

extern zend_module_entry lmdb_module_entry;
#define phpext_lmdb_ptr &lmdb_module_entry

#ifdef PHP_WIN32
#	define PHP_LMDB_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_LMDB_API __attribute__ ((visibility("default")))
#else
#	define PHP_LMDB_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(lmdb);
PHP_MSHUTDOWN_FUNCTION(lmdb);
PHP_RINIT_FUNCTION(lmdb);
PHP_RSHUTDOWN_FUNCTION(lmdb);
PHP_MINFO_FUNCTION(lmdb);

PHP_FUNCTION(mdb_version);
PHP_FUNCTION(mdb_strerror);

#ifdef ZTS
#define LDB_G(v) TSRMG(lmdb_globals_id, zend_lmdb_globals *, v)
#else
#define LDB_G(v) (lmdb_globals.v)
#endif

#endif	/* PHP_LMDB_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
