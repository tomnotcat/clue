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
#include "ctkdecoratorwidgetprivate.h"
#include "ctkmarshal.h"
#include <cairo-gobject.h>

G_DEFINE_TYPE (CtkDecoratorWidget, ctk_decorator_widget, G_TYPE_OBJECT)

enum {
    SIG_DRAW,
    SIG_BUTTONPRESS,
    SIG_BUTTONRELEASE,
    SIG_MOTIONNOTIFY,
    SIG_ENTERNOTIFY,
    SIG_LEAVENOTIFY,
    LAST_SIGNAL
};

enum {
    PROP_0,
    PROP_WINDOW
};

static guint widget_signals[LAST_SIGNAL] = { 0 };

static void ctk_decorator_widget_init (CtkDecoratorWidget *self)
{
    CtkDecoratorWidgetPrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              CTK_TYPE_DECORATOR_WIDGET,
                                              CtkDecoratorWidgetPrivate);
    priv = self->priv;

    priv->window = NULL;
    priv->context = NULL;
    priv->init_border.left = 0;
    priv->init_border.top = 0;
    priv->init_border.right = 0;
    priv->init_border.bottom = 0;
    priv->init_rect.left = 0;
    priv->init_rect.top = 0;
    priv->init_rect.right = 0;
    priv->init_rect.bottom = 0;
    priv->rect.left = 0;
    priv->rect.top = 0;
    priv->rect.right = 0;
    priv->rect.bottom = 0;
    priv->left = 0.0;
    priv->top = 0.0;
    priv->right = 0.0;
    priv->bottom = 0.0;
}

static void ctk_decorator_widget_set_property (GObject *object,
                                              guint prop_id,
                                              const GValue *value,
                                              GParamSpec *pspec)
{
    CtkDecoratorWidget *self = CTK_DECORATOR_WIDGET (object);
    CtkDecoratorWidgetPrivate *priv = self->priv;

    switch (prop_id) {
    case PROP_WINDOW:
        priv->window = g_value_get_object (value);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void ctk_decorator_widget_get_property (GObject *object,
                                              guint prop_id,
                                              GValue *value,
                                              GParamSpec *pspec)
{
    CtkDecoratorWidget *self = CTK_DECORATOR_WIDGET (object);
    CtkDecoratorWidgetPrivate *priv = self->priv;

    switch (prop_id) {
    case PROP_WINDOW:
        g_value_set_object (value, priv->window);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void ctk_decorator_widget_finalize (GObject *gobject)
{
    CtkDecoratorWidget *self = CTK_DECORATOR_WIDGET (gobject);
    CtkDecoratorWidgetPrivate *priv = self->priv;

    if (priv->context) {
        _gtk_style_context_set_widget (priv->context, NULL);
        g_object_unref (priv->context);
    }

    G_OBJECT_CLASS (ctk_decorator_widget_parent_class)->finalize (gobject);
}

/* We guard against the draw signal callbacks modifying the state of the
 * cairo context by surounding it with save/restore.
 * Maybe we should also cairo_new_path() just to be sure?
 */
static void ctk_decorator_widget_draw_marshaller (GClosure *closure,
                                                 GValue *return_value,
                                                 guint n_param_values,
                                                 const GValue *param_values,
                                                 gpointer invocation_hint,
                                                 gpointer marshal_data)
{
    cairo_t *cr = g_value_get_boxed (&param_values[1]);

    cairo_save (cr);

    g_cclosure_marshal_VOID__OBJECT (closure,
                                     return_value,
                                     n_param_values,
                                     param_values,
                                     invocation_hint,
                                     marshal_data);

    cairo_restore (cr);
}

static void ctk_decorator_widget_class_init (CtkDecoratorWidgetClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->set_property = ctk_decorator_widget_set_property;
    gobject_class->get_property = ctk_decorator_widget_get_property;
    gobject_class->finalize = ctk_decorator_widget_finalize;

    klass->draw = NULL;
    klass->button_press = NULL;
    klass->button_release = NULL;
    klass->motion_notify = NULL;
    klass->enter_notify = NULL;
    klass->leave_notify = NULL;

    g_type_class_add_private (gobject_class,
                              sizeof (CtkDecoratorWidgetPrivate));

    widget_signals[SIG_DRAW] =
        g_signal_new ("draw",
                      G_OBJECT_CLASS_TYPE (gobject_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (CtkDecoratorWidgetClass, draw),
                      NULL, NULL,
                      ctk_decorator_widget_draw_marshaller,
                      G_TYPE_NONE, 1,
                      CAIRO_GOBJECT_TYPE_CONTEXT);

    widget_signals[SIG_BUTTONPRESS] =
        g_signal_new ("button-press",
                      G_OBJECT_CLASS_TYPE (gobject_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (CtkDecoratorWidgetClass, button_press),
                      NULL, NULL,
                      ctk_marshal_VOID__INT_INT,
                      G_TYPE_NONE,
                      2,
                      G_TYPE_INT,
                      G_TYPE_INT);

    widget_signals[SIG_BUTTONRELEASE] =
        g_signal_new ("button-release",
                      G_OBJECT_CLASS_TYPE (gobject_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (CtkDecoratorWidgetClass, button_release),
                      NULL, NULL,
                      ctk_marshal_VOID__INT_INT,
                      G_TYPE_NONE,
                      2,
                      G_TYPE_INT,
                      G_TYPE_INT);

    widget_signals[SIG_MOTIONNOTIFY] =
        g_signal_new ("motion-notify",
                      G_OBJECT_CLASS_TYPE (gobject_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (CtkDecoratorWidgetClass, motion_notify),
                      NULL, NULL,
                      ctk_marshal_VOID__INT_INT,
                      G_TYPE_NONE,
                      2,
                      G_TYPE_INT,
                      G_TYPE_INT);

    widget_signals[SIG_ENTERNOTIFY] =
        g_signal_new ("enter-notify",
                      G_OBJECT_CLASS_TYPE (gobject_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (CtkDecoratorWidgetClass, enter_notify),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

    widget_signals[SIG_LEAVENOTIFY] =
        g_signal_new ("leave-notify",
                      G_OBJECT_CLASS_TYPE (gobject_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (CtkDecoratorWidgetClass, leave_notify),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

    g_object_class_install_property (
        gobject_class, PROP_WINDOW,
        g_param_spec_object ("window",
                             "Window",
                             "The host window",
                             GTK_TYPE_WIDGET,
                             G_PARAM_READABLE |
                             G_PARAM_WRITABLE |
                             G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_STATIC_STRINGS));
}

CtkDecoratorWidget* ctk_decorator_widget_new (GtkWidget *window)
{
    return g_object_new (CTK_TYPE_DECORATOR_WIDGET, 
                         "window", window, NULL);
}

/**
 * ctk_decorator_widget_get_style_context:
 * @widget: an #CtkDecoratorWidget
 *
 * Returns the style context associated to @widget.
 *
 * Returns: (transfer none): a #GtkStyleContext. This memory is owned by @widget and
 *          must not be freed.
 **/
GtkStyleContext* ctk_decorator_widget_get_style_context (CtkDecoratorWidget *widget)
{
    CtkDecoratorWidgetPrivate *priv;
    GdkScreen *screen;

    g_return_val_if_fail (CTK_IS_DECORATOR_WIDGET (widget), NULL);

    priv = widget->priv;
 
    if (priv->context)
        return priv->context;

    priv->context = gtk_style_context_new ();

    gtk_style_context_set_direction (priv->context, 
                                     gtk_widget_get_direction (priv->window));

    screen = gtk_widget_get_screen (priv->window);
    if (screen)
        gtk_style_context_set_screen (priv->context, screen);

    _gtk_style_context_set_widget (priv->context, priv->window);

    return priv->context;
}

/**
 * ctk_decorator_widget_get_allocation:
 * @self: an #CtkDecoratorWidget
 * @allocation: (out): a pointer to a #GtkAllocation to copy to
 *
 * Retrieves the widget's allocation.
 */
void ctk_decorator_widget_get_allocation (CtkDecoratorWidget *self,
                                         GtkAllocation *allocation)
{
    CtkDecoratorWidgetPrivate *priv;

    g_return_if_fail (CTK_IS_DECORATOR_WIDGET (self));

    priv = self->priv;

    allocation->x = priv->rect.left;
    allocation->y = priv->rect.top;
    allocation->width = priv->rect.right - priv->rect.left;
    allocation->height = priv->rect.bottom - priv->rect.top;
}

void _ctk_decorator_widget_draw (CtkDecoratorWidget *self,
                                cairo_t *cr)
{
    g_return_if_fail (CTK_IS_DECORATOR_WIDGET (self));

    g_signal_emit (self,
                   widget_signals[SIG_DRAW],
                   0,
                   cr);
}

void _ctk_decorator_widget_button_press (CtkDecoratorWidget *self,
                                        gint x, 
                                        gint y)
{
    g_return_if_fail (CTK_IS_DECORATOR_WIDGET (self));

    g_signal_emit (self,
                   widget_signals[SIG_BUTTONPRESS],
                   0,
                   x,
                   y);
}

void _ctk_decorator_widget_button_release (CtkDecoratorWidget *self,
                                          gint x, 
                                          gint y)
{
    g_return_if_fail (CTK_IS_DECORATOR_WIDGET (self));

    g_signal_emit (self,
                   widget_signals[SIG_BUTTONRELEASE],
                   0,
                   x,
                   y);
}

void _ctk_decorator_widget_motion_notify (CtkDecoratorWidget *self,
                                         gint x, 
                                         gint y)
{
    g_return_if_fail (CTK_IS_DECORATOR_WIDGET (self));

    g_signal_emit (self,
                   widget_signals[SIG_MOTIONNOTIFY],
                   0,
                   x,
                   y);
}

void _ctk_decorator_widget_enter_notify (CtkDecoratorWidget *self)
{
    g_return_if_fail (CTK_IS_DECORATOR_WIDGET (self));

    g_signal_emit (self,
                   widget_signals[SIG_ENTERNOTIFY],
                   0);
}

void _ctk_decorator_widget_leave_notify (CtkDecoratorWidget *self)
{
    g_return_if_fail (CTK_IS_DECORATOR_WIDGET (self));

    g_signal_emit (self,
                   widget_signals[SIG_LEAVENOTIFY],
                   0);
}
