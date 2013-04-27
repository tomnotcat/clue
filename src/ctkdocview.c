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
#include "ctkdocview.h"

enum {
    PROP_0,
    PROP_HADJUSTMENT,
    PROP_VADJUSTMENT,
    PROP_HSCROLL_POLICY,
    PROP_VSCROLL_POLICY
};

G_DEFINE_TYPE_WITH_CODE (CtkDocView, ctk_doc_view, GTK_TYPE_CONTAINER,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_SCROLLABLE, NULL))

struct _CtkDocViewPrivate {
    gpointer n;
};

static void ctk_doc_view_init (CtkDocView *self)
{
    CtkDocViewPrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              CTK_TYPE_DOC_VIEW,
                                              CtkDocViewPrivate);
    priv = self->priv;

    gtk_widget_set_has_window (GTK_WIDGET (self), TRUE);
    gtk_widget_set_can_focus (GTK_WIDGET (self), TRUE);
    gtk_widget_set_redraw_on_allocate (GTK_WIDGET (self), FALSE);
    gtk_container_set_resize_mode (GTK_CONTAINER (self), GTK_RESIZE_QUEUE);

    gtk_widget_set_events (GTK_WIDGET (self),
                           GDK_EXPOSURE_MASK |
                           GDK_BUTTON_PRESS_MASK |
                           GDK_BUTTON_RELEASE_MASK |
                           GDK_SCROLL_MASK |
                           GDK_KEY_PRESS_MASK |
                           GDK_POINTER_MOTION_MASK |
                           GDK_POINTER_MOTION_HINT_MASK |
                           GDK_ENTER_NOTIFY_MASK |
                           GDK_LEAVE_NOTIFY_MASK);
    priv->n = NULL;
}

static void ctk_doc_view_set_property (GObject *object,
                                       guint prop_id,
                                       const GValue *value,
                                       GParamSpec *pspec)
{
    CtkDocView *self = CTK_DOC_VIEW (object);
    CtkDocViewPrivate *priv = self->priv;

    (void) priv;
    switch (prop_id) {
    case PROP_HADJUSTMENT:
        break;

    case PROP_VADJUSTMENT:
        break;

    case PROP_HSCROLL_POLICY:
        break;

    case PROP_VSCROLL_POLICY:
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void ctk_doc_view_get_property (GObject *object,
                                       guint prop_id,
                                       GValue *value,
                                       GParamSpec *pspec)
{
    CtkDocView *self = CTK_DOC_VIEW (object);
    CtkDocViewPrivate *priv = self->priv;

    (void) priv;
    switch (prop_id) {
    case PROP_HADJUSTMENT:
        break;

    case PROP_VADJUSTMENT:
        break;

    case PROP_HSCROLL_POLICY:
        break;

    case PROP_VSCROLL_POLICY:
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void ctk_doc_view_finalize (GObject *gobject)
{
    CtkDocView *self = CTK_DOC_VIEW (gobject);
    CtkDocViewPrivate *priv = self->priv;

    g_free (priv->n);

    G_OBJECT_CLASS (ctk_doc_view_parent_class)->finalize (gobject);
}

static void ctk_doc_view_size_request (GtkWidget *widget,
                                       GtkRequisition *requisition)
{
    requisition->width = 0;
    requisition->height = 0;
}

static void ctk_doc_view_get_preferred_width (GtkWidget *widget,
                                              gint *minimum,
                                              gint *natural)
{
    GtkRequisition requisition;
    g_print (">>>> get_preferred_width\n");

    ctk_doc_view_size_request (widget, &requisition);

    *minimum = *natural = requisition.width;
}

static void ctk_doc_view_get_preferred_height (GtkWidget *widget,
                                               gint *minimum,
                                               gint *natural)
{
    GtkRequisition requisition;
    g_print (">>>> get_preferred_height\n");

    ctk_doc_view_size_request (widget, &requisition);

    *minimum = *natural = requisition.height;
}

static void ctk_doc_view_size_allocate (GtkWidget *widget,
                                        GtkAllocation  *allocation)
{
    g_print (">>>> size_allocate: %d, %d, %d, %d\n",
             allocation->x, allocation->y,
             allocation->width, allocation->height);

    gtk_widget_set_allocation (widget, allocation);

    if (gtk_widget_get_realized (widget)) {
        gdk_window_move_resize (gtk_widget_get_window (widget),
                                allocation->x,
                                allocation->y,
                                allocation->width,
                                allocation->height);
    }
}

static void ctk_doc_view_realize (GtkWidget *widget)
{
    GtkAllocation allocation;
    GdkWindow *window;
    GdkWindowAttr attributes;
    gint attributes_mask;

    gtk_widget_set_realized (widget, TRUE);

    gtk_widget_get_allocation (widget, &allocation);

    attributes.window_type = GDK_WINDOW_CHILD;
    attributes.x = allocation.x;
    attributes.y = allocation.y;
    attributes.width = allocation.width;
    attributes.height = allocation.height;
    attributes.wclass = GDK_INPUT_OUTPUT;
    attributes.visual = gtk_widget_get_visual (widget);
    attributes.event_mask = gtk_widget_get_events (widget);

    attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL;

    window = gdk_window_new (gtk_widget_get_parent_window (widget),
                             &attributes,
                             attributes_mask);
    gtk_widget_set_window (widget, window);
    gdk_window_set_user_data (window, widget);

    gtk_style_context_set_background (gtk_widget_get_style_context (widget),
                                      window);
}

static void ctk_doc_view_paint (GtkWidget *widget,
                                cairo_t *cr)
{
    g_print (">>>> paint\n");
    cairo_rectangle (cr, 0, 0, 100, 100);
    cairo_fill (cr);
}

static void ctk_doc_view_draw_focus (GtkWidget *widget,
                                     cairo_t *cr)
{
    gboolean interior_focus;

    /* We clear the focus if we are in interior focus mode. */
    gtk_widget_style_get (widget,
                          "interior-focus", &interior_focus,
                          NULL);

    if (gtk_widget_has_visible_focus (widget) && !interior_focus) {
        GtkStyleContext *context;

        context = gtk_widget_get_style_context (widget);

        gtk_render_focus (context, cr, 0, 0,
                          gtk_widget_get_allocated_width (widget),
                          gtk_widget_get_allocated_height (widget));
    }
}

static gboolean ctk_doc_view_draw (GtkWidget *widget,
                                   cairo_t *cr)
{
    GdkWindow *window;

    if (gtk_cairo_should_draw_window (cr, gtk_widget_get_window (widget)))
        ctk_doc_view_draw_focus (widget, cr);

    window = gtk_widget_get_window (widget);
    if (gtk_cairo_should_draw_window (cr, window)) {
        cairo_save (cr);
        gtk_cairo_transform_to_window (cr, widget, window);
        ctk_doc_view_paint (widget, cr);
        cairo_restore (cr);
    }

    return FALSE;
}

static void ctk_doc_view_destroy (GtkWidget *widget)
{
    g_print (">>>>: destroy\n");
    GTK_WIDGET_CLASS (ctk_doc_view_parent_class)->destroy (widget);
}

static void ctk_doc_view_class_init (CtkDocViewClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    gobject_class->set_property = ctk_doc_view_set_property;
    gobject_class->get_property = ctk_doc_view_get_property;
    gobject_class->finalize = ctk_doc_view_finalize;

    widget_class->realize = ctk_doc_view_realize;
    widget_class->draw = ctk_doc_view_draw;
    widget_class->destroy = ctk_doc_view_destroy;
    widget_class->get_preferred_width = ctk_doc_view_get_preferred_width;
    widget_class->get_preferred_height = ctk_doc_view_get_preferred_height;
    widget_class->size_allocate = ctk_doc_view_size_allocate;

    /* GtkScrollable interface */
    g_object_class_override_property (gobject_class, PROP_HADJUSTMENT, "hadjustment");
    g_object_class_override_property (gobject_class, PROP_VADJUSTMENT, "vadjustment");
    g_object_class_override_property (gobject_class, PROP_HSCROLL_POLICY, "hscroll-policy");
    g_object_class_override_property (gobject_class, PROP_VSCROLL_POLICY, "vscroll-policy");

    g_type_class_add_private (gobject_class,
                              sizeof (CtkDocViewPrivate));
}

CtkDocView* ctk_doc_view_new (void)
{
    return g_object_new (CTK_TYPE_DOC_VIEW, NULL);
}

void ctk_doc_view_set_model (CtkDocView *self,
                             CtkDocModel *model)
{
}
