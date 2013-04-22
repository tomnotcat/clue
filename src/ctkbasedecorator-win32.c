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
#include "uubasedecorator.h"
#include "uumousemanager.h"

#define MOUSE_TRACK_MASK (CTK_MOUSE_TRACK_LBUTTONDOWN | \
                          CTK_MOUSE_TRACK_RBUTTONDOWN | \
                          CTK_MOUSE_TRACK_MBUTTONDOWN | \
                          CTK_MOUSE_TRACK_NCLBUTTONDOWN | \
                          CTK_MOUSE_TRACK_NCRBUTTONDOWN | \
                          CTK_MOUSE_TRACK_NCMBUTTONDOWN)

G_DEFINE_TYPE (CtkBaseDecorator, ctk_base_decorator, G_TYPE_OBJECT)

struct _CtkBaseDecoratorPrivate {
    GtkWidget *window;
    WNDPROC old_proc;
    HWND hwnd;
    UUCloseMode close_mode;
    guint close_time;
};

static GQuark decorator_quark;
#define DECORATOR_PROP_NAME "CTK_BASE_DECORATOR"

static void _ctk_base_decorator_destroy (gpointer x)
{
    ctk_base_decorator_detach (x);
    g_object_unref (x);
}

static LRESULT CALLBACK _ctk_base_decorator_proc (HWND hwnd,
                                                  UINT msg,
                                                  WPARAM wparam,
                                                  LPARAM lparam)
{
    CtkBaseDecorator *self;
    CtkBaseDecoratorPrivate *priv;

    self = (CtkBaseDecorator *) GetProp (hwnd , DECORATOR_PROP_NAME);
    if (!self)
        return DefWindowProc (hwnd, msg, wparam, lparam);

    priv = self->priv;

    if (priv->old_proc) {
        LRESULT lr = 0;

        if (CTK_BASE_DECORATOR_GET_CLASS (self)->proc &&
            CTK_BASE_DECORATOR_GET_CLASS (self)->proc (self, hwnd, msg, wparam, lparam, &lr))
        {
            return lr;
        }

        switch(msg) {
        case WM_CANCELMODE:
        case WM_ACTIVATEAPP:
            if (!wparam) {
                if (CTK_CLOSE_MODE_LOST_FOCUS == priv->close_mode) {
                    PostMessage (priv->hwnd, WM_CLOSE, 0, 0);
                }
            }
            break;

        case CTK_MOUSE_MSG_LBUTTONDOWN:
        case CTK_MOUSE_MSG_RBUTTONDOWN:
        case CTK_MOUSE_MSG_MBUTTONDOWN:
        case CTK_MOUSE_MSG_NCLBUTTONDOWN:
        case CTK_MOUSE_MSG_NCRBUTTONDOWN:
        case CTK_MOUSE_MSG_NCMBUTTONDOWN:
            if (CTK_CLOSE_MODE_LOST_FOCUS == priv->close_mode) {
                RECT rect;
                POINT pt;

                pt.x = GET_X_LPARAM(lparam);
                pt.y = GET_Y_LPARAM(lparam);

                GetWindowRect (priv->hwnd, &rect);
                if (!PtInRect (&rect, pt)) {
                    PostMessage (priv->hwnd, WM_CLOSE, 0, 0);
                }
            }
            return 0;

        default:
            break;
        }

        return CallWindowProc (priv->old_proc, hwnd, msg, wparam, lparam);
    }

    return DefWindowProc (hwnd, msg, wparam, lparam);
}

static void ctk_base_decorator_init (CtkBaseDecorator *self)
{
    CtkBaseDecoratorPrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              CTK_TYPE_BASE_DECORATOR,
                                              CtkBaseDecoratorPrivate);
    priv = self->priv;

    priv->window = NULL;
    priv->old_proc = NULL;
    priv->hwnd = NULL;
    priv->close_mode = CTK_CLOSE_MODE_NONE;
    priv->close_time = 0;
}

static void ctk_base_decorator_finalize (GObject *gobject)
{
    G_OBJECT_CLASS (ctk_base_decorator_parent_class)->finalize (gobject);
}

static void ctk_base_decorator_class_init (CtkBaseDecoratorClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = ctk_base_decorator_finalize;

    klass->attach = NULL;
    klass->detach = NULL;
    klass->proc = NULL;

    g_type_class_add_private (gobject_class,
                              sizeof (CtkBaseDecoratorPrivate));
}

CtkBaseDecorator* ctk_base_decorator_new (void)
{
    return g_object_new (CTK_TYPE_BASE_DECORATOR, NULL);
}

gboolean ctk_base_decorator_attach (CtkBaseDecorator *self,
                                    GtkWidget *widget)
{
    CtkBaseDecoratorPrivate *priv;
    GdkWindow *window;
    HWND hwnd;

    g_return_val_if_fail (CTK_IS_BASE_DECORATOR (self), FALSE);

    priv = self->priv;

    if (priv->window)
        return FALSE;

    gtk_widget_realize (widget);
    window = gtk_widget_get_window (widget);
    if (!window)
        return FALSE;

    hwnd = GDK_WINDOW_HWND (window);
    if (!IsWindow (hwnd))
        return FALSE;

    if (!decorator_quark)
        decorator_quark = g_quark_from_static_string ("_ctk_window_decorator_");

    priv->old_proc = (WNDPROC) GetWindowLong (hwnd, GWL_WNDPROC);
    if (!SetWindowLong (hwnd, GWL_WNDPROC, (LONG)(_ctk_base_decorator_proc))) {
        priv->old_proc = NULL;
        return FALSE;
    }

    SetProp (hwnd, DECORATOR_PROP_NAME, (HANDLE) self);

    g_object_set_qdata_full (G_OBJECT (widget),
                             decorator_quark,
                             g_object_ref (self),
                             _ctk_base_decorator_destroy);
    priv->window = widget;
    priv->hwnd = hwnd;

    switch (priv->close_mode) {
    case CTK_CLOSE_MODE_LOST_FOCUS:
        ctk_mouse_manager_add_track (priv->hwnd, MOUSE_TRACK_MASK);
        break;

    case CTK_CLOSE_MODE_MOUSE_LEAVE:
        ctk_mouse_manager_add_track (priv->hwnd, MOUSE_TRACK_MASK);
        break;

    default:
        break;
    }

    if (CTK_BASE_DECORATOR_GET_CLASS (self)->attach)
        return CTK_BASE_DECORATOR_GET_CLASS (self)->attach (self);

    return TRUE;
}

void ctk_base_decorator_detach (CtkBaseDecorator *self)
{
    CtkBaseDecoratorPrivate *priv;

    g_return_if_fail (CTK_IS_BASE_DECORATOR (self));

    priv = self->priv;

    if (priv->window) {
        gpointer data;

        if (CTK_BASE_DECORATOR_GET_CLASS (self)->detach)
            CTK_BASE_DECORATOR_GET_CLASS (self)->detach (self);

        data = g_object_get_qdata (G_OBJECT (priv->window), decorator_quark);
        if (data == (gpointer) self || NULL == data) {
            SetWindowLong (priv->hwnd, GWL_WNDPROC, (LONG) priv->old_proc);
            RemoveProp (priv->hwnd, DECORATOR_PROP_NAME);

            if (data) {
                g_object_set_qdata (G_OBJECT (priv->window),
                                    decorator_quark, NULL);
            }
        }

        priv->window = NULL;
        priv->old_proc = NULL;
        priv->hwnd = NULL;
    }
}

/**
 * ctk_base_decorator_get_window:
 * @self: an #CtkBaseDecorator
 *
 * Get the attached window.
 *
 * Return value: (transfer none): a #GtkWidget
 */
GtkWidget* ctk_base_decorator_get_window (CtkBaseDecorator *self)
{
    g_return_val_if_fail (CTK_IS_BASE_DECORATOR (self), NULL);

    return self->priv->window;
}

void ctk_base_decorator_set_close_mode (CtkBaseDecorator *self,
                                        CtkCloseMode mode,
                                        guint timeout)
{
    CtkBaseDecoratorPrivate *priv;

    g_return_if_fail (CTK_IS_BASE_DECORATOR (self));

    priv = self->priv;

    if (priv->hwnd) {
        switch (mode) {
        case CTK_CLOSE_MODE_LOST_FOCUS:
            ctk_mouse_manager_add_track (priv->hwnd, MOUSE_TRACK_MASK);
            break;

        case CTK_CLOSE_MODE_MOUSE_LEAVE:
            ctk_mouse_manager_add_track (priv->hwnd, MOUSE_TRACK_MASK);
            break;

        default:
            ctk_mouse_manager_remove_track (priv->hwnd, MOUSE_TRACK_MASK);
            break;
        }
    }

    priv->close_mode = mode;
    priv->close_time = timeout;

    if (priv->hwnd) {
    }
}

HWND _ctk_base_decorator_get_hwnd (CtkBaseDecorator *self)
{
    g_return_val_if_fail (CTK_IS_BASE_DECORATOR (self), NULL);

    return self->priv->hwnd;
}

LRESULT _ctk_base_decorator_call_old_proc (CtkBaseDecorator *self,
                                           HWND hwnd,
                                           UINT msg,
                                           WPARAM wparam,
                                           LPARAM lparam)
{
    g_return_val_if_fail (CTK_IS_BASE_DECORATOR (self), 0);

    return CallWindowProc (self->priv->old_proc, hwnd, msg, wparam, lparam);
}

gboolean ctk_is_win7 (void)
{
    OSVERSIONINFOEX osvi;

    osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFOEX);
    GetVersionEx ((OSVERSIONINFO*) &osvi);

    /* The version of win7 is NT6.1 */
    if (VER_PLATFORM_WIN32_NT == osvi.dwPlatformId &&
        osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1)
    {
        return TRUE;
    }

    return FALSE;
}
