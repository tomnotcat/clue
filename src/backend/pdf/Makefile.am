pluginlib_LTLIBRARIES = clue-pdf-1.0.la
pluginlibdir = ${libdir}/clue-plugins-1.0
pluginlib_DATA = clue-pdf-1.0.xml

clue_pdf_1_0_la_SOURCES = pdfpage.h pdfpage.c \
	pdfdocument.h pdfdocument.c
clue_pdf_1_0_la_CFLAGS = ${GLIB_CFLAGS} ${GIMO_CFLAGS} \
	${CAIRO_CFLAGS} ${CTK_CFLAGS} ${MUPDF_CFLAGS}
clue_pdf_1_0_la_LDFLAGS = ${GLIB_LIBS} ${GIMO_LIBS} ${CTK_LIBS} \
	${CAIRO_LIBS} ${MUPDF_LIBS} -module -avoid-version -shared

EXTRA_DIST = clue-pdf-1.0.xml