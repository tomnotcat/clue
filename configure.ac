#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])

m4_define([ctk_major_version], [1])
m4_define([ctk_minor_version], [0])
m4_define([ctk_micro_version], [0])
m4_define([ctk_interface_age], [0])
m4_define([ctk_binary_age], [0])
m4_define([ctk_version], [ctk_major_version.ctk_minor_version.ctk_micro_version])

# libtool version related macros
m4_define([ctk_lt_release], [ctk_major_version.ctk_minor_version])
m4_define([ctk_lt_current],
        [m4_eval(100 * ctk_minor_version + ctk_micro_version - ctk_interface_age)])
m4_define([ctk_lt_revision], [ctk_interface_age])
m4_define([ctk_lt_age], [m4_eval(ctk_binary_age - ctk_interface_age)])
m4_define([ctk_lt_current_minus_age], [m4_eval(ctk_lt_current - ctk_lt_age)])

AC_INIT([clue], [ctk_version], [tomnotcat@163.com])
AC_CONFIG_SRCDIR([src/clue.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([-Wall -Werror])

CTK_MAJOR_VERSION=ctk_major_version
CTK_MINOR_VERSION=ctk_minor_version
CTK_MICRO_VERSION=ctk_micro_version
CTK_INTERFACE_AGE=ctk_interface_age
CTK_BINARY_AGE=ctk_binary_age
CTK_VERSION=ctk_version
AC_SUBST(CTK_MAJOR_VERSION)
AC_SUBST(CTK_MINOR_VERSION)
AC_SUBST(CTK_MICRO_VERSION)
AC_SUBST(CTK_INTERFACE_AGE)
AC_SUBST(CTK_BINARY_AGE)
AC_SUBST(CTK_VERSION)

# libtool versioning
LT_RELEASE=ctk_lt_release
LT_CURRENT=ctk_lt_current
LT_REVISION=ctk_lt_revision
LT_AGE=ctk_lt_age
LT_CURRENT_MINUS_AGE=ctk_lt_current_minus_age
LT_VERSION_INFO=${LT_CURRENT}:${LT_REVISION}:${LT_AGE}
AC_SUBST(LT_RELEASE)
AC_SUBST(LT_CURRENT)
AC_SUBST(LT_REVISION)
AC_SUBST(LT_AGE)
AC_SUBST(LT_CURRENT_MINUS_AGE)
AC_SUBST(LT_VERSION_INFO)

dnl Initialize libtool
LT_PREREQ([2.2])
LT_INIT([disable-static])

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_INSTALL
AM_PROG_CC_C_O

dnl give error and exit if we don't have pkgconfig
AC_CHECK_PROG(HAVE_PKGCONFIG, pkg-config, [ ],
    [AC_MSG_ERROR([You need to have pkg-config installed!])])

# Checks for libraries.

#
# Checks for GLib
#
GLIB_REQUIRED_VERSION=2.32.0
AC_SUBST(GLIB_REQUIRED_VERSION)

GLIB_MODULES="glib-2.0 >= $GLIB_REQUIRED_VERSION gobject-2.0 gthread-2.0 gmodule-no-export-2.0"

PKG_CHECK_MODULES(GLIB, $GLIB_MODULES, :,
    AC_MSG_ERROR([
*** Glib $GLIB_REQUIRED_VERSION or better is required. The latest version of
*** Glib is always available from ftp://ftp.gtk.org/.]))

# Setup GLIB_MKENUMS to use glib-mkenums even if GLib is uninstalled.
GLIB_MKENUMS=`$PKG_CONFIG --variable=glib_mkenums glib-2.0`
AC_SUBST(GLIB_MKENUMS)

# Add in gthread-2.0 to CFLAGS but not to LIBS so we get any
# necesary defines for thread-safety.
GLIB_CFLAGS=`$PKG_CONFIG --cflags $GLIB_MODULES gthread-2.0`

#
# Checks for GObject Introspection
#
GOBJECT_INTROSPECTION_CHECK([1.34.2])

#
# Checks for GTK
#
PKG_CHECK_MODULES(GTK, [gtk+-3.0 >= 3.6 gmodule-2.0], have_gtk=yes, have_gtk=no)

if test "x${have_gtk}" = "xno"; then
   AC_MSG_ERROR([gtk+-3.0 required but not found])
fi

#
# Checks for libgda
#
PKG_CHECK_MODULES(LIBGDA, [libgda-5.0], have_gda=yes, have_gda=no)

if test "x${have_gtk}" = "xno"; then
   AC_MSG_ERROR([libgda-5.0 required but not found])
fi

#
# Check pygobject
#
PKG_CHECK_MODULES(PYGOBJECT, [pygobject-3.0 python-2.7],
                             have_pygobject=yes, have_pygobject=no)

if test "x${have_pygobject}" = "xno"; then
   AC_MSG_ERROR([pygobject-3.0 required but not found])
fi

#
# Check Gjs
#
PKG_CHECK_MODULES(GJS, [gjs-1.0 mozjs185], have_gjs=yes, have_gjs=no)

if test "x${have_gjs}" = "xno"; then
   AC_MSG_ERROR([Gjs-1.0 required but not found])
fi

#
# Check Gimo
#
PKG_CHECK_MODULES(GIMO, [gimo-1.0], have_gimo=yes, have_gimo=no)

if test "x${have_gimo}" = "xno"; then
   AC_MSG_ERROR([Gimo-1.0 required but not found])
fi

GIMO_PLUGIN_DIR=$(pkg-config gimo-1.0 --variable=plugindir)
AC_SUBST([GIMO_PLUGIN_DIR])

#
# Check Oren
#
PKG_CHECK_MODULES(OREN, [oren-1.0], have_oren=yes, have_oren=no)

if test "x${have_oren}" = "xno"; then
   AC_MSG_ERROR([Oren-1.0 required but not found])
fi

# Checks for header files.
AC_CHECK_HEADERS([ \
   assert.h ctype.h dlfunc.h errno.h float.h limits.h locale.h \
   math.h pthread.h stdarg.h stddef.h stdint.h stdio.h stdlib.h \
   string.h sys/time.h time.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STRTOD
AC_CHECK_FUNCS([memmove memset select setlocale \
                        strchr strspn strtol strtoul])

# Configure options: --enable-debug[=no].
AC_ARG_ENABLE([debug],
        [AS_HELP_STRING([--enable-debug],
                [enable debug code (default is no)])],
        [debug=yes], [debug=no])

AM_CONDITIONAL([DEBUG], [test x$debug = xyes])

if test "x${debug}" != "xno"; then
   AC_DEFINE([CTK_DEBUG], [1], [Debugging mode])
   AC_CHECK_PROG([VALGRIND], [valgrind], [valgrind])
   CFLAGS="${CFLAGS} -g3 -gdwarf-2 -O0"
fi

RUN_ENVIRONMENT="GI_TYPELIB_PATH=\$(top_builddir)/src:/usr/local/lib/girepository-1.0 \
   LD_LIBRARY_PATH=\$(top_builddir)/src/.libs:/usr/local/lib"

TESTS_ENVIRONMENT="${RUN_ENVIRONMENT} CTK_TEST=TRUE"

if test -n "${VALGRIND}"; then
   TESTS_ENVIRONMENT+=" VALGRIND_TEST=TRUE \
   G_SLICE=always-malloc G_DEBUG=gc-friendly libtool --mode=execute \
   ${VALGRIND} -q --tool=memcheck --leak-check=full --num-callers=40 \
   --show-possibly-lost=yes --malloc-fill=4 --free-fill=5 \
   --suppressions=\$(top_srcdir)/tests/clue.supp --error-exitcode=1"
fi

AC_SUBST([RUN_ENVIRONMENT])
AC_SUBST([TESTS_ENVIRONMENT])

STRICT_CFLAGS="-pedantic -Wstrict-prototypes"
AC_SUBST([STRICT_CFLAGS])

CFLAGS="${CFLAGS} -Wall -Werror -Wno-long-long"
CXXFLAGS="${CXXFLAGS} -pedantic -Wall -Werror -Wno-long-long"

CTK_CFLAGS="-I\$(top_srcdir)/src"
CTK_LIBS="-L\$(top_builddir)/src -lctk-1.0"
AC_SUBST([CTK_CFLAGS])
AC_SUBST([CTK_LIBS])

CLUE_MAIN_PLUGINS="\$(abs_top_srcdir)/plugins/main"
AC_SUBST([CLUE_MAIN_PLUGINS])

AC_CONFIG_FILES([Makefile
        src/Makefile
        tests/Makefile
        win32/Makefile])

AC_OUTPUT

echo \
"-------------------------------------------------
${PACKAGE_NAME} ${CTK_VERSION}
Prefix: '${prefix}'.
Compiler: '${CC} ${CFLAGS} ${CPPFLAGS}'
Libraries: '${LIBS}'
Debug: '${debug}'
TESTS_ENVIRONMENT: '${TESTS_ENVIRONMENT}'
-------------------------------------------------"
