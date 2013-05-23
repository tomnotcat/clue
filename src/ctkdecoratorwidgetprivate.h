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
#ifndef __CTK_DECORATOR_WIDGET_PRIVATE_H__
#define __CTK_DECORATOR_WIDGET_PRIVATE_H__

#include "ctkdecoratorwidget.h"
#include <Windows.h>

G_BEGIN_DECLS

struct _CtkDecoratorWidgetPrivate {
    GtkWidget *window;
    GtkStyleContext *context;
    RECT init_border;
    RECT init_rect;
    RECT rect;
    gfloat left, top, right, bottom;
};

void _ctk_decorator_widget_draw (CtkDecoratorWidget *self,
                                cairo_t *cr);

void _ctk_decorator_widget_button_press (CtkDecoratorWidget *self,
                                        gint x, 
                                        gint y);

void _ctk_decorator_widget_button_release (CtkDecoratorWidget *self,
                                          gint x, 
                                          gint y);

void _ctk_decorator_widget_motion_notify (CtkDecoratorWidget *self,
                                         gint x, 
                                         gint y);

void _ctk_decorator_widget_enter_notify (CtkDecoratorWidget *self);

void _ctk_decorator_widget_leave_notify (CtkDecoratorWidget *self);

G_END_DECLS

#endif /* __CTK_DECORATOR_WIDGET_PRIVATE_H__ */
