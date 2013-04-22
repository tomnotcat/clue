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
#ifndef __CTK_BASE_DECORATOR_H__
#define __CTK_BASE_DECORATOR_H__

#include "ctktypes.h"
#include <gtk/gtk.h>

#ifdef G_OS_WIN32
#include <gdk/gdkwin32.h>
#endif

G_BEGIN_DECLS

#define CTK_TYPE_BASE_DECORATOR (ctk_base_decorator_get_type())
#define CTK_BASE_DECORATOR(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), CTK_TYPE_BASE_DECORATOR, CtkBaseDecorator))
#define CTK_IS_BASE_DECORATOR(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), CTK_TYPE_BASE_DECORATOR))
#define CTK_BASE_DECORATOR_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), CTK_TYPE_BASE_DECORATOR, CtkBaseDecoratorClass))
#define CTK_IS_BASE_DECORATOR_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), CTK_TYPE_BASE_DECORATOR))
#define CTK_BASE_DECORATOR_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), CTK_TYPE_BASE_DECORATOR, CtkBaseDecoratorClass))

typedef struct _CtkBaseDecoratorPrivate CtkBaseDecoratorPrivate;
typedef struct _CtkBaseDecoratorClass CtkBaseDecoratorClass;

#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))

struct _CtkBaseDecorator {
    GObject parent_instance;
    CtkBaseDecoratorPrivate *priv;
};

struct _CtkBaseDecoratorClass {
    GObjectClass parent_class;
    gboolean (*attach) (CtkBaseDecorator *self);
    void (*detach) (CtkBaseDecorator *self);
#ifdef G_OS_WIN32
    gboolean (*proc) (CtkBaseDecorator *self,
                      HWND hwnd,
                      UINT msg,
                      WPARAM wparam,
                      LPARAM lparam,
                      LRESULT *result);
#endif
};

GType ctk_base_decorator_get_type (void) G_GNUC_CONST;

CtkBaseDecorator* ctk_base_decorator_new (void);

gboolean ctk_base_decorator_attach (CtkBaseDecorator *self,
                                    GtkWidget *widget);

void ctk_base_decorator_detach (CtkBaseDecorator *self);

GtkWidget* ctk_base_decorator_get_window (CtkBaseDecorator *self);

void ctk_base_decorator_set_close_mode (CtkBaseDecorator *self,
                                        CtkCloseMode mode,
                                        guint timeout);

gboolean ctk_is_win7 (void);

#ifdef G_OS_WIN32
HWND _ctk_base_decorator_get_hwnd (CtkBaseDecorator *self);

LRESULT _ctk_base_decorator_call_old_proc (CtkBaseDecorator *self,
                                           HWND hwnd,
                                           UINT msg,
                                           WPARAM wparam,
                                           LPARAM lparam);
#endif

G_END_DECLS

#endif /* __CTK_BASE_DECORATOR_H__ */
