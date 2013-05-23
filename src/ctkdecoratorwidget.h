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
#ifndef __CTK_DECORATOR_WIDGET_H__
#define __CTK_DECORATOR_WIDGET_H__

#include "ctktypes.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define CTK_TYPE_DECORATOR_WIDGET (ctk_decorator_widget_get_type())
#define CTK_DECORATOR_WIDGET(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), CTK_TYPE_DECORATOR_WIDGET, CtkDecoratorWidget))
#define CTK_IS_DECORATOR_WIDGET(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), CTK_TYPE_DECORATOR_WIDGET))
#define CTK_DECORATOR_WIDGET_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), CTK_TYPE_DECORATOR_WIDGET, CtkDecoratorWidgetClass))
#define CTK_IS_DECORATOR_WIDGET_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), CTK_TYPE_DECORATOR_WIDGET))
#define CTK_DECORATOR_WIDGET_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), CTK_TYPE_DECORATOR_WIDGET, CtkDecoratorWidgetClass))

typedef struct _CtkDecoratorWidgetPrivate CtkDecoratorWidgetPrivate;
typedef struct _CtkDecoratorWidgetClass CtkDecoratorWidgetClass;

struct _CtkDecoratorWidget {
    GObject parent_instance;
    CtkDecoratorWidgetPrivate *priv;
};

struct _CtkDecoratorWidgetClass {
    GObjectClass parent_class;
    void (*draw) (CtkDecoratorWidget *self, cairo_t *cr);
    void (*button_press) (CtkDecoratorWidget *self, gint x, gint y);
    void (*button_release) (CtkDecoratorWidget *self, gint x, gint y);
    void (*motion_notify) (CtkDecoratorWidget *self, gint x, gint y);
    void (*enter_notify) (CtkDecoratorWidget *self);
    void (*leave_notify) (CtkDecoratorWidget *self);
};

GType ctk_decorator_widget_get_type (void) G_GNUC_CONST;

CtkDecoratorWidget* ctk_decorator_widget_new (GtkWidget *window);

GtkStyleContext* ctk_decorator_widget_get_style_context (CtkDecoratorWidget *widget);

void ctk_decorator_widget_get_allocation (CtkDecoratorWidget *self,
                                         GtkAllocation *allocation);

G_END_DECLS

#endif /* __CTK_DECORATOR_WIDGET_H__ */
