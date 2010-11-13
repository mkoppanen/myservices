#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(myservices, 0.0.1, mikko.koppanen@gmail.com)
AM_INIT_AUTOMAKE([dist-bzip2 foreign])

AC_PROG_LIBTOOL
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_SRCDIR([src/main.c])
AC_PROG_CC_C99
AM_PROG_CC_C_O

AC_HEADER_STDBOOL
AC_HEADER_STDC

AC_ARG_WITH(zookeeper,
    [  --with-zookeeper=[PATH]        path to zookeeper],
    [
        if test "$withval" != "yes"; then
            ZOOKEEPER_PREFIX="$withval"
            CFLAGS="$CFLAGS -I$ZOOKEEPER_PREFIX/include/c-client-src"
            CPPFLAGS="$CPPFLAGS -I$ZOOKEEPER_PREFIX/include/c-client-src"
        fi
    ],
    [])
    
if test -z $ZOOKEEPER_PREFIX; then
    dnl Try default path 
    ZOOKEEPER_PREFIX="/usr"
    CFLAGS="$CFLAGS -I$ZOOKEEPER_PREFIX/include/c-client-src"
    CPPFLAGS="$CPPFLAGS -I$ZOOKEEPER_PREFIX/include/c-client-src"
fi

AC_CHECK_HEADERS([zookeeper.h], [have_zookeeper_h="yes"], [have_zookeeper_h="no"])  
    
if test "x$have_zookeeper_h" != "xyes"; then
  AC_MSG_ERROR([Could not find zookeeper in $ZOOKEEPER_PREFIX/include/c-client-src])
fi

AC_CHECK_LIB(zookeeper_mt, zookeeper_init, ,AC_MSG_ERROR([zookeeper libraries not found.]))
AC_SUBST(ZOOKEEPER_LIBS)

# Checks for standard c lib
AC_FUNC_MALLOC
AC_CHECK_FUNCS([strlen])

AC_OUTPUT

