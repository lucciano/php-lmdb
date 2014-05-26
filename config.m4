dnl $Id$
dnl config.m4 for extension lmdb

PHP_ARG_WITH(lmdb, for lmdb support,
[  --with-lmdb[=Path]             Include lmdb support])

if test "$PHP_LMDB" != "no"; then

  # --with-lmdb -> check with-path
  SEARCH_PATH="/usr/local /usr"
  SEARCH_FOR="lmdb.h"
  SEARCH_LIB="liblmdb"

  dnl search lmdb
  AC_MSG_CHECKING([for lmdb location])
  for i in $PHP_LMDB $SEARCH_PATH ; do
    if test -r $i/$SEARCH_FOR; then
	  LMDB_INCLUDE_DIR=$i
	  AC_MSG_RESULT(lmdb headers found in $i)
    fi

    if test -r $i/$PHP_LIBDIR/$SEARCH_LIB.a || test -r $i/$PHP_LIBDIR/$SEARCH_LIB.$SHLIB_SUFFIX_NAME; then
	  LMDB_LIB_DIR=$i/$PHP_LIBDIR
	  AC_MSG_RESULT(lmdb lib found in $i/lib)
    fi

    if test -r $i/$SEARCH_LIB.a || test -r $i/$SEARCH_LIB.$SHLIB_SUFFIX_NAME; then
	  LMDB_LIB_DIR=$i
	  AC_MSG_RESULT(lmdb lib found in $i)
    fi
  done
  
  if test -z "$LMDB_INCLUDE_DIR" || test -z "$LMDB_LIB_DIR"; then
    AC_MSG_RESULT([lmdb not found])
    AC_MSG_ERROR([Please reinstall the lmdb distribution])
  fi

  # --with-lmdb -> add include path
  PHP_ADD_INCLUDE($LMDB_INCLUDE_DIR)

  # --with-lmdb -> check for lib and symbol presence
  LIBNAME=lmdb
  PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $LMDB_LIB_DIR, LMDB_SHARED_LIBADD)
  
  PHP_SUBST(LMDB_SHARED_LIBADD)

  PHP_NEW_EXTENSION(lmdb, lmdb.c, $ext_shared)
fi
