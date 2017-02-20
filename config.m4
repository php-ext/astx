dnl $Id$
dnl config.m4 for extension astx

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary.


dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(astx, for astx support,
dnl [  --with-astx             Include astx support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(astx, whether to enable astx support,
[  --enable-astx           Enable astx support])


if test "$PHP_ASTX" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-astx -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/astx.h"  # you most likely want to change this
  dnl if test -r $PHP_ASTX/$SEARCH_FOR; then # path given as parameter
  dnl   ASTX_DIR=$PHP_ASTX
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for astx files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       ASTX_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$ASTX_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the astx distribution])
  dnl fi

  dnl # --with-astx -> add include path
  dnl PHP_ADD_INCLUDE($ASTX_DIR/include)

  dnl # --with-astx -> check for lib and symbol presence
  dnl LIBNAME=astx # you may want to change this
  dnl LIBSYMBOL=astx # you most likely want to change this

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $ASTX_DIR/$PHP_LIBDIR, ASTX_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_ASTXLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong astx lib version or lib not found])
  dnl ],[
  dnl   -L$ASTX/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(ASTX_SHARED_LIBADD)

  PHP_NEW_EXTENSION(astx, astx.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi