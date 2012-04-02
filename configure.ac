
AC_INIT(wordpath, 0.1, [gbenison@gmail.com])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([src/dictionary.c])

AC_PROG_CC
AM_INIT_AUTOMAKE
LT_INIT
AC_PROG_INSTALL

PKG_CHECK_MODULES(GLIB,   glib-2.0)
PKG_CHECK_MODULES(IGRAPH, igraph)
PKG_CHECK_MODULES(GUILE,  guile-2.0)

AC_CONFIG_FILES([Makefile
                 src/Makefile])

AC_OUTPUT


