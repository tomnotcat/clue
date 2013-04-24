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
#include "ctkcommondialog.h"

/**
 * ctk_open_single_file:
 * @parent: (allow-none): a #GtkWidget
 * @name: (allow-none): the initial file name
 * @dir: (allow-none): the initial directory
 * @title: (allow-none): the dialog title
 * @filters: (allow-none) (array zero-terminated=1):
 *     a %NULL-terminated array of strings holding filters
 *
 * Display a file choose dialog to choose a single file.
 *
 * Return value: (transfer full): the selected file name or %NULL
 **/
gchar* ctk_open_single_file (GtkWidget *parent,
                             const gchar *name,
                             const gchar *dir,
                             const gchar *title,
                             gchar **filters)
{
    GtkWidget *dialog;
    GtkFileChooser *chooser;
    gchar *filename = NULL;

    dialog = gtk_file_chooser_dialog_new (title,
                                          GTK_WINDOW (parent),
                                          GTK_FILE_CHOOSER_ACTION_OPEN,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                          NULL);
    chooser = GTK_FILE_CHOOSER (dialog);

    if (name)
        gtk_file_chooser_select_filename (chooser, name);

    if (dir)
        gtk_file_chooser_set_current_folder (chooser, dir);

    if (filters) {
        GtkFileFilter *filter;
        gchar **it = filters;
        gchar **patterns;

        while (*it) {
            filter = gtk_file_filter_new ();
            gtk_file_filter_set_name (filter, *it);
            ++it;

            if (NULL == *it) {
                g_object_unref (filter);
                break;
            }

            patterns = g_strsplit_set (*it, ";", 0);
            if (patterns) {
                gchar **pattern = patterns;

                while (*pattern) {
                    gtk_file_filter_add_pattern (filter, *pattern);
                    ++pattern;
                }

                g_strfreev (patterns);
            }

            gtk_file_chooser_add_filter (chooser, filter);
            it++;
        }
    }

    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

    gtk_widget_destroy (dialog);
    return filename;
}
