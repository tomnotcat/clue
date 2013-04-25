all: all-am
	@if test -n "${TEST}"; then \
		$(TESTS_ENVIRONMENT) ${TEST}; \
	fi

AM_CPPFLAGS = ${STRICT_CFLAGS} ${CTK_CFLAGS} ${GLIB_CFLAGS} \
	-DTEST_TOP_SRCDIR="\"${abs_top_srcdir}/tests/\""
AM_LDFLAGS = ${CTK_LIBS} ${GLIB_LIBS}

check_PROGRAMS =
check_SCRIPTS =

# Test mime types.
check_PROGRAMS += test-mime
test_mime_SOURCES = test-mime.c
check_SCRIPTS += test-mime.py

TESTS = ${check_PROGRAMS} ${check_SCRIPTS}

EXTRA_DIST = ${check_SCRIPTS} clue.supp test.pdf test.txt