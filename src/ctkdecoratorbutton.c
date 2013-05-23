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
#include "ctkdecoratorbutton.h"

G_DEFINE_TYPE (CtkDecoratorButton, ctk_decorator_button, CTK_TYPE_DECORATOR_WIDGET)

enum {
    SIG_CLICKED,
    LAST_SIGNAL
};

struct _CtkDecoratorButtonPrivate {
    gboolean pressed;
};

static guint button_signals[LAST_SIGNAL] = { 0 };

static void _ctk_decorator_button_draw (CtkDecoratorWidget *widget,
                                       cairo_t *cr)
{
    GtkAllocation allocation;
    GtkStyleContext *context;
    GtkStateFlags state;
    gint x, y, width, height;

    ctk_decorator_widget_get_allocation (widget, &allocation);
    context = ctk_decorator_widget_get_style_context (widget);
    state = gtk_style_context_get_state (context);

    x = 0;
    y = 0;
    width = allocation.width;
    height = allocation.height;

    gtk_render_background (context, cr, x, y, width, height);
    gtk_render_frame (context, cr, x, y, width, height);
}

static void _ctk_decorator_button_button_press (CtkDecoratorWidget *widget,
                                               gint x,
                                               gint y)
{
    CtkDecoratorButton *self = CTK_DECORATOR_BUTTON (widget);
    CtkDecoratorButtonPrivate *priv = self->priv;
    GtkStyleContext *context;

    context = ctk_decorator_widget_get_style_context (widget);

    priv->pressed = TRUE;
    gtk_style_context_set_state (context, GTK_STATE_FLAG_ACTIVE);
    gtk_style_context_invalidate (context);
}

static void _ctk_decorator_button_button_release (CtkDecoratorWidget *widget,
                                                 gint x,
                                                 gint y)
{
    CtkDecoratorButton *self = CTK_DECORATOR_BUTTON (widget);
    CtkDecoratorButtonPrivate *priv = self->priv;
    GtkStyleContext *context;
    GtkAllocation allocation;

    context = ctk_decorator_widget_get_style_context (widget);

    priv->pressed = FALSE;

    ctk_decorator_widget_get_allocation (widget, &allocation);
    if (x >= 0 && x < allocation.width && y >= 0 && y < allocation.height) {
        gtk_style_context_set_state (context, GTK_STATE_FLAG_PRELIGHT);

        g_signal_emit (self,
                       button_signals[SIG_CLICKED],
                       0);
    }
    else {
        gtk_style_context_set_state (context, GTK_STATE_FLAG_NORMAL);
    }

    gtk_style_context_invalidate (context);
}

static void _ctk_decorator_button_enter_notify (CtkDecoratorWidget *widget)
{
    CtkDecoratorButton *self = CTK_DECORATOR_BUTTON (widget);
    CtkDecoratorButtonPrivate *priv = self->priv;
    GtkStyleContext *context;

    context = ctk_decorator_widget_get_style_context (widget);

    if (priv->pressed)
        gtk_style_context_set_state (context, GTK_STATE_FLAG_ACTIVE);
    else
        gtk_style_context_set_state (context, GTK_STATE_FLAG_PRELIGHT);

    gtk_style_context_invalidate (context);
}

static void _ctk_decorator_button_leave_notify (CtkDecoratorWidget *widget)
{
    GtkStyleContext *context;

    context = ctk_decorator_widget_get_style_context (widget);

    gtk_style_context_set_state (context, GTK_STATE_FLAG_NORMAL);
    gtk_style_context_invalidate (context);
}

static void ctk_decorator_button_init (CtkDecoratorButton *self)
{
    CtkDecoratorButtonPrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              CTK_TYPE_DECORATOR_BUTTON,
                                              CtkDecoratorButtonPrivate);
    priv = self->priv;

    priv->pressed = FALSE;
}

static void ctk_decorator_button_finalize (GObject *gobject)
{
    G_OBJECT_CLASS (ctk_decorator_button_parent_class)->finalize (gobject);
}

static void ctk_decorator_button_class_init (CtkDecoratorButtonClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    CtkDecoratorWidgetClass *widget_class = CTK_DECORATOR_WIDGET_CLASS (klass);

    g_type_class_add_private (gobject_class,
                              sizeof (CtkDecoratorButtonPrivate));

    gobject_class->finalize = ctk_decorator_button_finalize;
    widget_class->draw = _ctk_decorator_button_draw;
    widget_class->button_press = _ctk_decorator_button_button_press;
    widget_class->button_release = _ctk_decorator_button_button_release;
    widget_class->enter_notify = _ctk_decorator_button_enter_notify;
    widget_class->leave_notify = _ctk_decorator_button_leave_notify;

    klass->clicked = NULL;

    button_signals[SIG_CLICKED] =
        g_signal_new ("clicked",
                      G_OBJECT_CLASS_TYPE (gobject_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (CtkDecoratorButtonClass, clicked),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);
}

CtkDecoratorWidget* ctk_decorator_button_new (GtkWidget *window)
{
    return g_object_new (CTK_TYPE_DECORATOR_BUTTON, 
                         "window", window, NULL);
}
