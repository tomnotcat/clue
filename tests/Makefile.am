all: all-am
	@if test -n "${TEST}"; then \
		$(TESTS_ENVIRONMENT) ${TEST}; \
	fi

AM_CPPFLAGS = ${STRICT_CFLAGS} ${CTK_CFLAGS} ${GLIB_CFLAGS} \
	${GTK_CFLAGS} -DTEST_TOP_SRCDIR="\"${abs_top_srcdir}/tests/\""
AM_LDFLAGS = ${CTK_LIBS} ${GLIB_LIBS} ${GTK_LIBS}

check_PROGRAMS =
check_SCRIPTS =

# Test mime types.
check_PROGRAMS += test-mime
test_mime_SOURCES = test-mime.c
check_SCRIPTS += test-mime.py

check_PROGRAMS += test-docview
test_docview_SOURCES = test-docview.c
test_docview_CFLAGS = ${AM_CPPFLAGS} \
	-DTEST_PDF_BACKEND="\"${abs_top_srcdir}/src/backend/pdf/.libs/clue-pdf-1.0.so\"" \
	-DTEST_PDF_FILE="\"${abs_top_srcdir}/tests/test.pdf\"" \
	-DTEST_TXT_BACKEND="\"${abs_top_srcdir}/src/backend/txt/.libs/clue-txt-1.0.so\"" \
	-DTEST_TXT_FILE="\"${abs_top_srcdir}/tests/test.txt\""

TESTS = ${check_PROGRAMS} ${check_SCRIPTS}

EXTRA_DIST = ${check_SCRIPTS} clue.supp test.pdf test.txt