ctk_common_headers = ctkenums.h ctktypes.h ctkfileutils.h \
	ctkcommondialog.h ctkdocmodel.h ctkdocview.h ctkdocument.h \
	ctkbasedecorator.h ctkwindowdecorator.h
ctk_common_sources = ctkenums.c ctkfileutils.c ctkdocmodel.c \
	ctkdocview.c ctkdocument.c ctkbasedecorator-x11.c \
	ctkwindowdecorator-x11.c ctkcommondialog-x11.c

ctk_sources = ${ctk_common_headers} ${ctk_common_sources}

# Libraries
lib_LTLIBRARIES = libctk-1.0.la
libctk_1_0_la_CFLAGS = ${STRICT_CFLAGS} ${GLIB_CFLAGS} \
	${GTK_CFLAGS} ${GIMO_CFLAGS}
libctk_1_0_la_LDFLAGS = -version-info ${LT_VERSION_INFO} \
	${GLIB_LIBS} ${GTK_LIBS} ${GIMO_LIBS}
libctk_1_0_la_SOURCES = ${ctk_sources}

ctkincludedir=$(includedir)/clue-1.0
ctkinclude_HEADERS = ${ctk_common_headers}

# Programs
bin_PROGRAMS = clue

clue_SOURCES = clue.c
clue_CFLAGS = -DCOMMON_PLUGINS=\"@GIMO_PLUGIN_DIR@\" \
	-DMAIN_PLUGINS=\"@CLUE_MAIN_PLUGINS@\" \
	-DBACKEND_PLUGINS=\"@CLUE_BACKEND_PLUGINS@\" \
	${GLIB_CFLAGS} ${GTK_CFLAGS} ${CTK_CFLAGS} \
	${LIBGDA_CFLAGS} ${GIMO_CFLAGS} ${OREN_CFLAGS}
clue_LDFLAGS = ${GLIB_LIBS} ${GTK_LIBS} \
	${CTK_LIBS} ${LIBGDA_LIBS} ${GIMO_LIBS} ${OREN_LIBS}

# GObject Introspection
CLEANFILES =

-include $(INTROSPECTION_MAKEFILE)

if HAVE_INTROSPECTION

INTROSPECTION_SCANNER_ARGS = --add-include-path=$(srcdir) \
	--warn-all --warn-error
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir)
INTROSPECTION_GIRS = Ctk-1.0.gir

Ctk-1.0.gir: $(INTROSPECTION_SCANNER) Makefile

Ctk_1_0_gir_INCLUDES = GObject-2.0 Gtk-3.0 Gimo-1.0
Ctk_1_0_gir_CFLAGS = $(INCLUDES) -I${srcdir}/
Ctk_1_0_gir_LIBS = libctk-1.0.la
Ctk_1_0_gir_FILES = ${ctk_common_headers} ${ctk_common_sources}
Ctk_1_0_gir_EXPORT_PACKAGES = ctk-1.0

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(gir_DATA) $(typelib_DATA)

endif # HAVE_INTROSPECTION

SUBDIRS = . backend
EXTRA_DIST = ctkbasedecorator-win32.c ctkwindowdecorator-win32.c