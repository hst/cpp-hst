# -*- coding: utf-8 -*-
# ------------------------------------------------------------------------------
# Copyright © 2016-2017, HST Project.
# Please see the COPYING file in this distribution for license details.
# ------------------------------------------------------------------------------

ACLOCAL_AMFLAGS = -I m4
AM_CPPFLAGS = -I. -I$(top_srcdir)/src
AM_DEFAULT_SOURCE_EXT = .cc
noinst_LTLIBRARIES = libhst.la
bin_PROGRAMS = hst
EXTRA_DIST = tap-driver.sh
TESTS = ${check_PROGRAMS}
LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL) $(top_srcdir)/tap-driver.sh
@VALGRIND_CHECK_RULES@

libhst_la_SOURCES = \
	src/hst/csp0.h \
	src/hst/csp0.cc \
	src/hst/environment.h \
	src/hst/environment.cc \
	src/hst/event.h \
	src/hst/event.cc \
	src/hst/external-choice.cc \
	src/hst/hash.h \
	src/hst/interleave.cc \
	src/hst/internal-choice.cc \
	src/hst/prefix.cc \
	src/hst/process.h \
	src/hst/process.cc \
	src/hst/sequential-composition.cc

hst_SOURCES = \
	src/hst/hst/hst.cc
hst_LDADD = libhst.la

# Keep this sorted generally in order of dependency, so that more basic features
# are checked before larger features that build on them.
check_PROGRAMS = \
	tests/test-harness \
	tests/test-events \
	tests/test-csp0 \
	tests/test-operators

check_LTLIBRARIES = libtests.la
libtests_la_SOURCES = \
	tests/test-cases.h \
	tests/test-cases.cc \
	tests/test-harness.cc.in

LDADD = libhst.la libtests.la
tests_test_csp0_LDFLAGS = -no-install
tests_test_events_LDFLAGS = -no-install
tests_test_harness_LDFLAGS = -no-install
tests_test_operators_LDFLAGS = -no-install

dist_doc_DATA = README.md
