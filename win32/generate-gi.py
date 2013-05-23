#!/usr/bin/env python
import sys, os

name = "g-ir-scanner.py"
path = os.environ['Path']
scanner = ""

for it in path.split (';'):
    if os.path.exists (it + os.path.sep + name):
        scanner = it + os.path.sep + name
        break

srcs = '''../../src/ctkbasedecorator.h ../../src/ctkcommondialog.h \
../../src/ctkdecoratorbutton.h ../../src/ctkdecoratorwidget.h \
../../src/ctkdecoratorwidgetprivate.h ../../src/ctkdocmodel.h \
../../src/ctkdocpage.h ../../src/ctkdocrendercache.h \
../../src/ctkdocrendertask.h ../../src/ctkdocument.h \
../../src/ctkdocview.h ../../src/ctkenums.h \
../../src/ctkfileutils.h ../../src/ctkmarshal.h \
../../src/ctkmousemanager.h ../../src/ctktypes.h \
../../src/ctkwindowdecorator.h ../../src/ctkbasedecorator-win32.c \
../../src/ctkcommondialog-win32.c ../../src/ctkdecoratorbutton.c \
../../src/ctkdecoratorwidget.c ../../src/ctkdocmodel.c \
../../src/ctkdocpage.c ../../src/ctkdocrendercache.c \
../../src/ctkdocrendertask.c ../../src/ctkdocument.c \
../../src/ctkdocview.c ../../src/ctkenums.c \
../../src/ctkfileutils.c ../../src/ctkmarshal.c \
../../src/ctkmousemanager.c ../../src/ctkwindowdecorator-win32.c'''

os.system ("cd " + sys.argv[1] + " && python.exe \"" + scanner + "\" --add-include-path=. --warn-all --namespace=Ctk --symbol-prefix=ctk \
 --nsversion=1.0 --no-libtool --include=GObject-2.0 --include=GdkPixbuf-2.0 --include=Gtk-3.0 --include=Gda-5.0 --include=Gimo-1.0 --include=Oren-1.0 \
 --pkg-export=Ctk-1.0 --library=" + sys.argv[2] + " -I../ctk --output Ctk-1.0.gir " + srcs +
" && g-ir-compiler --includedir=. Ctk-1.0.gir -o Ctk-1.0.typelib")
