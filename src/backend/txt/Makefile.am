pluginlib_LTLIBRARIES = clue-txt-1.0.la
pluginlibdir = ${libdir}/clue-plugins-1.0
pluginlib_DATA = clue-txt-1.0.xml

clue_txt_1_0_la_SOURCES = txtdocument.h txtdocument.c
clue_txt_1_0_la_CFLAGS = ${GLIB_CFLAGS} ${GIMO_CFLAGS} ${CTK_CFLAGS}
clue_txt_1_0_la_LDFLAGS = ${GLIB_LIBS} ${GIMO_LIBS} ${CTK_LIBS} \
	-module -avoid-version -shared

EXTRA_DIST = clue-txt-1.0.xml