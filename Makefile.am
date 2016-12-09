# -*- coding: utf-8 -*-
# ------------------------------------------------------------------------------
# Copyright © 2016, HST Project.
# Please see the COPYING file in this distribution for license details.
# ------------------------------------------------------------------------------

ACLOCAL_AMFLAGS = -I m4
AM_DEFAULT_SOURCE_EXT = .cc
check_LTLIBRARIES = libtests.la
check_PROGRAMS = \
	tests/test-harness
bin_PROGRAMS = hst
EXTRA_DIST = tap-driver.sh
TESTS = ${check_PROGRAMS}
LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL) $(top_srcdir)/tap-driver.sh
@VALGRIND_CHECK_RULES@

hst_SOURCES = \
	src/hst/hst.cc

libtests_la_SOURCES = \
	tests/test-cases.h \
	tests/test-cases.cc \
	tests/test-harness.cc.in

LDADD = libtests.la
tests_test_harness_LDFLAGS = -no-install

dist_doc_DATA = README
