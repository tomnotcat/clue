/* Ctk - The Clue Toolkit
 *
 * Copyright (C) 2013 Tom Wong
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "config.h"
#include "ctkdocument.h"
#include "ctkdocview.h"
#include <gtk/gtk.h>

static void test_docview_common (const gchar *backend,
                                 const gchar *file)
{
    CtkDocument* (*newdoc) (void) = NULL;
    GModule *module;
    GtkWidget *window;
    CtkDocument *doc;
    CtkDocView *view;

    module = g_module_open (backend, G_MODULE_BIND_LAZY);
    g_module_symbol (module, "clue_new_document", (gpointer *)&newdoc);

    doc = newdoc ();
    g_assert (ctk_document_load_from_file (doc, file, NULL));

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

    g_signal_connect (window,
                      "destroy",
                      G_CALLBACK (gtk_main_quit),
                      NULL);

    view = ctk_doc_view_new ();
    ctk_doc_view_set_document (view, doc);
    ctk_doc_view_set_document (view, NULL);
    ctk_doc_view_set_document (view, doc);
    g_object_unref (doc);

    gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (view));

    gtk_widget_set_size_request (window, 400, 400);
    gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
    gtk_widget_show_all (window);

    gtk_main ();
}

int main (int argc, char *argv[])
{
    gtk_init (&argc, &argv);

    test_docview_common (TEST_PDF_BACKEND, TEST_PDF_FILE);
#if 0
    test_docview_common (TEST_TXT_BACKEND, TEST_TXT_FILE);
#endif

    return 0;
}
