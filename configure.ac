# -*- coding: utf-8 -*-
# ------------------------------------------------------------------------------
# Copyright © 2016, HST Project.
# Please see the COPYING file in this distribution for license details.
# ------------------------------------------------------------------------------

AC_INIT([hst], [0.0.0], [info@hst-project.net])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/hst/hst.cc])
AM_INIT_AUTOMAKE([foreign no-dist-gzip dist-xz subdir-objects])
AM_MAINTAINER_MODE([enable])  # Allow packagers to disable if they want
AM_SILENT_RULES([yes])
LT_INIT

# Turn on fatal warnings by default; you can override this by setting CPPFLAGS
# to something else when running configure.
: ${CPPFLAGS="-Wall -Werror"}

AC_PROG_CXX

AC_OUTPUT([Makefile])

cat <<EOF

------------------ Summary ------------------
 $PACKAGE_NAME version $PACKAGE_VERSION
  Prefix.........: $prefix
  C++ Compiler...: $CXX $CXXFLAGS $CPPFLAGS
  Linker.........: $LD $LDFLAGS $LIBS
---------------------------------------------

Check the above options and compile with:
 ${MAKE-make}

EOF
