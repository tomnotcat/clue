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
#include <gdk/gdkwin32.h>
#include <string.h>

struct _OpenFileParam {
    GtkWidget *parent;
    const gchar *name;
    const gchar *dir;
    const gchar *title;
    gchar **filters;
    gchar *result;
};

static GMutex *_dialog_mutex;
static gboolean _dialog_finished;
static GMainLoop *_dialog_loop;
static HWND _dialog_parent;
static HWND _dialog_handle;

static UINT_PTR CALLBACK _open_file_hookproc (HWND hdlg,
                                              UINT uMsg,
                                              WPARAM wParam,
                                              LPARAM lParam)
{
    switch (uMsg) {
    case WM_INITDIALOG:
        {
            HWND hparent = hdlg;

            while (GetParent (hparent) != _dialog_parent)
                hparent = GetParent (hparent);

            g_mutex_lock (_dialog_mutex);
            _dialog_handle = hparent;
            g_mutex_unlock (_dialog_mutex);
        }
        break;

    case WM_CLOSE:
        break;

    default:
        break;
    }

    return 0;
}

static OPENFILENAME* _create_ofn (GtkWidget *parent,
                                  const gchar *lpszInitSel,
                                  const gchar *lpszInitDir,
                                  const gchar *lpszTitle,
                                  gchar **lpszFilters,
                                  gint nFileMode)
{
    const DWORD maxNameLen = 1023;
    const DWORD maxMultiLen = 65535;
    DWORD maxLen = (2 == nFileMode) ? maxMultiLen : maxNameLen;
    OPENFILENAME* ofn;
    gchar *tInitSel = NULL;
    gchar *tFilters = NULL;
    gchar *tInitDir = NULL;
    gchar *tTitle = NULL;

    if (lpszInitSel) {
        gchar *str = g_locale_from_utf8 (lpszInitSel, -1, NULL, NULL, NULL);

        if (strlen (str) < maxLen + 1) {
            tInitSel = g_malloc (maxLen + 1);
            strcpy (tInitSel, str);
        }
        else {
            tInitSel = str;
            str = NULL;
        }

        g_free (str);
    }
    else {
        tInitSel = g_malloc (maxLen + 1);
        tInitSel[0] = 0;
    }

    ofn = g_malloc (sizeof (OPENFILENAME));
    memset (ofn, 0, sizeof (OPENFILENAME));

    if (lpszFilters) {
        gchar **it = lpszFilters;
        gsize buf_len = 0;
        gsize cur_len = 0;
        gsize len = 0;

        while (*it) {
            gchar *str = g_locale_from_utf8 (*it, -1, NULL, NULL, NULL);
            len = strlen (str) + 1;

            if (buf_len - cur_len < len) {
                buf_len += len + 1;
                tFilters = g_realloc (tFilters, buf_len);
            }

            memcpy (tFilters + cur_len, str, len);
            cur_len += len;
            ++it;
            g_free (str);
        }

        if (cur_len)
            tFilters[cur_len] = '\0';
    }

    if (parent) {
        GdkWindow *window = gtk_widget_get_window (parent);
        _dialog_parent = GDK_WINDOW_HWND (window);
    }

    if (lpszInitDir)
        tInitDir = g_locale_from_utf8 (lpszInitDir, -1, NULL, NULL, NULL);

    if (lpszTitle)
        tTitle = g_locale_from_utf8 (lpszTitle, -1, NULL, NULL, NULL);

    ofn->lStructSize = sizeof (OPENFILENAME);
    ofn->hwndOwner = _dialog_parent;
    ofn->lpstrFilter = tFilters;
    ofn->lpstrFile = tInitSel;
    ofn->nMaxFile = maxLen;
    ofn->lpstrInitialDir = tInitDir;
    ofn->lpstrTitle = tTitle;
    ofn->Flags = (OFN_NOCHANGEDIR | OFN_HIDEREADONLY | OFN_EXPLORER |
                  OFN_ENABLEHOOK | OFN_ENABLESIZING);
    ofn->lpfnHook = _open_file_hookproc;
    ofn->lCustData = 0;

    if (nFileMode) {
        ofn->Flags |= (OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST);

        if (2 == nFileMode)
            ofn->Flags |= (OFN_ALLOWMULTISELECT);
    }
    else {
        ofn->Flags |= OFN_OVERWRITEPROMPT;
    }

    return ofn;
}

static void _destroy_ofn (OPENFILENAME *ofn)
{
    g_free ((gchar *) ofn->lpstrFilter);
    g_free ((gchar *) ofn->lpstrInitialDir);
    g_free ((gchar *) ofn->lpstrTitle);
    g_free ((gchar *) ofn->lpstrFile);
    g_free (ofn);
}

static gpointer _open_file_thread_func (gpointer data)
{
    struct _OpenFileParam *param = data;
    int index = 0;
    int sel_index = 0;
    OPENFILENAME *ofn;

    ofn = _create_ofn (param->parent,
                       param->name,
                       param->dir,
                       param->title,
                       param->filters,
                       1);
    if (NULL == ofn)
        return NULL;

    if (index)
        ofn->nFilterIndex = index + 1;

    if (GetOpenFileName (ofn)) {
        sel_index = ofn->nFilterIndex;
        param->result = g_locale_to_utf8 (ofn->lpstrFile, -1, NULL, NULL, NULL);
    }

    _destroy_ofn (ofn);

    g_mutex_lock (_dialog_mutex);
    _dialog_finished = TRUE;
    g_mutex_unlock (_dialog_mutex);

    return NULL;
}

static void _show_window_modal_dialog (GThreadFunc func, gpointer p)
{
    GThread *thread;

    if (_dialog_mutex)
        return;

    _dialog_mutex = g_malloc (sizeof (GMutex));
    g_mutex_init (_dialog_mutex);
    _dialog_finished = FALSE;
    _dialog_parent = NULL;
    _dialog_handle = NULL;

    _dialog_loop = g_main_loop_new (g_main_context_default (), FALSE);

    // thread
    thread = g_thread_new ("modal-dialog", func, p);

    while (1) {
        g_main_context_iteration (g_main_context_default (), FALSE);

        if (g_mutex_trylock (_dialog_mutex)) {
            const gboolean finished = _dialog_finished;
            HWND hpnt = _dialog_parent;
            HWND hdlg = _dialog_handle;
            g_mutex_unlock (_dialog_mutex);

            if (finished)
                break;

            if (hpnt && !IsWindow (hpnt)) {
                if (IsWindow (hdlg))
                    PostMessage (hdlg, WM_CLOSE, 0, 0);
            }
        }

        g_usleep (10000);
    }

    g_thread_join (thread);

    g_mutex_clear (_dialog_mutex);
    g_free (_dialog_mutex);
    _dialog_mutex = NULL;
}

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
    struct _OpenFileParam param;

    param.parent = parent;
    param.name = name;
    param.dir = dir;
    param.title = title;
    param.filters = filters;
    param.result = NULL;

    _show_window_modal_dialog (_open_file_thread_func, &param);

    return param.result;
}
