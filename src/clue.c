/* Clue - a full featured reading environment.
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
#include <gimo.h>
#include <gtk/gtk.h>
#include <libgda.h>
#include <locale.h>
#include <oren.h>

#ifdef G_OS_WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

static void _start_plugin (GimoContext *context,
                           gchar **starts)
{
    GimoPlugin *plugin;
    gchar **it = starts;

    while (*it) {
        if (!*(*it)) {
            ++it;
            continue;
        }

        plugin = gimo_context_query_plugin (context, *it);
        if (plugin) {
            if (!gimo_plugin_start (plugin, NULL)) {
                gchar *err_str = gimo_dup_error_string ();

                g_warning ("Start plugin error: %s: %s",
                           gimo_plugin_get_id (plugin), err_str);

                g_free (err_str);
            }
        }
        else {
            g_warning ("Plugin not exist: %s", *it);
        }

        ++it;
    }
}

static void _load_plugin (GimoContext *context,
                          const gchar *app_path,
                          const gchar *file_path,
                          gboolean start)
{
    GPtrArray *plugins = NULL;

    if (gimo_context_load_plugin (context, file_path, NULL, &plugins)) {
        gchar *start_file;

        if (start) {
            GimoPlugin *p;
            guint i;

            for (i = 0; i < plugins->len; ++i) {
                p = (GimoPlugin *) g_ptr_array_index (plugins, i);
                gimo_plugin_start (p, NULL);
            }
        }

        start_file = g_build_filename (app_path, file_path, "start", NULL);
        if (g_file_test (start_file, G_FILE_TEST_EXISTS)) {
            gchar *content = NULL;
            gchar **starts = NULL;
            gsize length = 0;
            GError *error = NULL;

            if (g_file_get_contents (start_file,
                                     &content,
                                     &length,
                                     &error))
            {
                starts = g_strsplit_set (content, "\r\n", 0);
                if (starts) {
                    _start_plugin (context, starts);
                    g_strfreev (starts);
                }

                g_free (content);
            }
            else {
                g_warning ("Read start error: %s: %s",
                           start_file, error->message);
                g_clear_error (&error);
            }
        }

        g_free (start_file);
        g_ptr_array_unref (plugins);
    }
    else {
        gchar *err_str = gimo_dup_error_string ();

        g_warning ("Load plugin error: %s: %s",
                   file_path, err_str);

        g_free (err_str);
    }
}

static gboolean _runnable_async_run (GimoRunnable *run)
{
    gimo_runnable_run (run);
    g_object_unref (run);
    return FALSE;
}

static void _context_async_run (GimoContext *context, GimoRunnable *run)
{
    g_idle_add ((GSourceFunc) _runnable_async_run, g_object_ref (run));
}

static gboolean _dispatch_queued_handle (OrenGPDispatch *dispatch)
{
    while (oren_gpdispatch_handle_queued (dispatch));
    g_object_unref (dispatch);
    return FALSE;
}

static void _dispatch_queued (OrenGPDispatch *dispatch)
{
    g_idle_add ((GSourceFunc) _dispatch_queued_handle, g_object_ref (dispatch));
}

static gboolean _clue_call_gc (gpointer x)
{
    GimoContext *context = (GimoContext *) x;

    gimo_context_call_gc (context, TRUE);

    return TRUE;
}

int main (int argc, char *argv[])
{
    static gchar **starts = NULL;
    static gchar **files = NULL;
    static gint silent = 0;

    static GOptionEntry entries[] = {
        { "start", 's', 0, G_OPTION_ARG_STRING_ARRAY, &starts, "Startup plugins", NULL },
        { "silent", 'l', 0, G_OPTION_ARG_INT, &silent, "Run silently", NULL },
        { G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY, &files, NULL, "FILE" },
        { NULL }
    };

    GError *error = NULL;
    GOptionContext *optctx;

    GimoContext *context = NULL;
    OrenGPDispatch *dispatch = NULL;
    GtkCssProvider *provider;
    GtkSettings *settings;
    gchar *utf8_path;
    gchar *app_path;
    gchar *abs_path;

    setlocale (LC_ALL, "zh_CN.UTF-8");

    utf8_path = g_locale_to_utf8 (argv[0], -1, NULL, NULL, NULL);
    app_path = g_path_get_dirname (utf8_path);
    g_free (utf8_path);

    /* Pixbuf module */
#ifdef G_OS_WIN32
    abs_path = g_build_filename (app_path, "gdk-pixbuf-2.0", "loaders.cache", NULL);
    g_setenv ("GDK_PIXBUF_MODULE_FILE", abs_path, 1);
    g_free (abs_path);

    abs_path = g_build_path (G_DIR_SEPARATOR_S, app_path, "gdk-pixbuf-2.0", NULL);
    g_setenv ("GDK_PIXBUF_MODULEDIR", abs_path, 1);
    g_free (abs_path);
#endif

    gtk_init (&argc, &argv);
    gda_init ();

    settings = gtk_settings_get_default ();
    g_object_set (settings, "gtk-show-input-method-menu", FALSE, NULL);
    g_object_set (settings, "gtk-show-unicode-menu", FALSE, NULL);

    /* Init SQLite provider */
    gda_config_get_provider_info ("SQLite");

    /* Load CSS style */
    provider = gtk_css_provider_new ();
    abs_path = g_build_filename (app_path, "style", "gtk.css", NULL);

#ifdef G_OS_WIN32
    if (gtk_css_provider_load_from_path (provider, abs_path, &error)) {
        GdkScreen *screen = gdk_screen_get_default ();

        gtk_style_context_add_provider_for_screen (
            screen, GTK_STYLE_PROVIDER (provider),
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }
    else {
        g_warning ("Load CSS error: %s: %s", abs_path, error->message);
        g_clear_error (&error);
    }
#endif

    g_free (abs_path);
    g_object_unref (provider);

    /* FIXME: use g_irepository_prepend_search_path to add typelib path. */
    abs_path = g_build_path (G_DIR_SEPARATOR_S, app_path, "typelib", NULL);
    g_setenv ("GI_TYPELIB_PATH", abs_path, 0);
    g_free (abs_path);

    abs_path = g_build_path (G_DIR_SEPARATOR_S, app_path, "gjs-1.0", NULL);
    g_setenv ("GJS_PATH", abs_path, 0);
    g_free (abs_path);

    abs_path = g_build_path (G_DIR_SEPARATOR_S, app_path, "glib-2.0", NULL);
    g_setenv ("GSETTINGS_SCHEMA_DIR", abs_path, 0);
    g_free (abs_path);

    if (!silent)
        gimo_trace_error (TRUE);

    optctx = g_option_context_new ("clue");
    g_option_context_add_main_entries (optctx, entries, "");

    if (!g_option_context_parse (optctx, &argc, &argv, &error)) {
        g_warning ("option parsing failed: %s", error->message);
        g_clear_error (&error);
    }

    g_option_context_free (optctx);

    context = gimo_context_new ();
    gimo_context_add_paths (context, app_path);

    gimo_bind_string (G_OBJECT (context), "app_path", app_path);

    abs_path = g_locale_from_utf8 (app_path, -1, NULL, NULL, NULL);
#ifdef G_OS_WIN32
    SetCurrentDirectory (abs_path);
#else
    chdir (app_path);
#endif
    g_free (abs_path);

    g_signal_connect (context,
                      "async-run",
                      G_CALLBACK (_context_async_run),
                      NULL);

    dispatch = oren_gpdispatch_new (TRUE);

    g_signal_connect (dispatch,
                      "queued",
                      G_CALLBACK (_dispatch_queued),
                      NULL);

    gimo_bind_object (G_OBJECT (context),
                      "dispatch",
                      G_OBJECT (dispatch));

    g_object_unref (dispatch);

    if (files) {
        gchar **it = files;

        while (*it) {
            _load_plugin (context, app_path, *it, !starts);
            ++it;
        }
    }
    else {
#ifdef COMMON_PLUGINS
        _load_plugin (context, "", COMMON_PLUGINS, FALSE);
#else
        _load_plugin (context, app_path, "common", FALSE);
#endif

#ifdef MAIN_PLUGINS
        _load_plugin (context, "", MAIN_PLUGINS, FALSE);
#else
        _load_plugin (context, app_path, "main", FALSE);
#endif
    }

    g_free (app_path);

    if (starts) {
        _start_plugin (context, starts);
        g_strfreev (starts);
    }

    if (files)
        g_strfreev (files);

    gimo_context_run_plugins (context);

    g_timeout_add (60000, _clue_call_gc, context);

    gtk_main ();

    gimo_context_destroy (context);
    g_object_unref (context);

    return 0;
}
