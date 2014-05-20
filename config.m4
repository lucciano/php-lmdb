dnl $Id$
dnl config.m4 for extension lmdb

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(lmdb, for lmdb support,
dnl Make sure that the comment is aligned:
dnl [  --with-lmdb             Include lmdb support])

dnl Otherwise use enable:

dnl PHP_ARG_ENABLE(lmdb, whether to enable lmdb support,
dnl Make sure that the comment is aligned:
dnl [  --enable-lmdb           Enable lmdb support])

if test "$PHP_LMDB" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-lmdb -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/lmdb.h"  # you most likely want to change this
  dnl if test -r $PHP_LMDB/$SEARCH_FOR; then # path given as parameter
  dnl   LMDB_DIR=$PHP_LMDB
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for lmdb files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       LMDB_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$LMDB_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the lmdb distribution])
  dnl fi

  dnl # --with-lmdb -> add include path
  dnl PHP_ADD_INCLUDE($LMDB_DIR/include)

  dnl # --with-lmdb -> check for lib and symbol presence
  dnl LIBNAME=lmdb # you may want to change this
  dnl LIBSYMBOL=lmdb # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $LMDB_DIR/lib, LMDB_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_LMDBLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong lmdb lib version or lib not found])
  dnl ],[
  dnl   -L$LMDB_DIR/lib -lm
  dnl ])
  dnl
  dnl PHP_SUBST(LMDB_SHARED_LIBADD)

  PHP_NEW_EXTENSION(lmdb, lmdb.c, $ext_shared)
fi
