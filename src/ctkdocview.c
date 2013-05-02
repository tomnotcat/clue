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
#include "ctkdocmodel.h"
#include "ctkdocpage.h"
#include "ctkdocument.h"
#include <math.h>

#define CTK_DOC_MODEL_GET_DOC(model) \
    (model ? ctk_doc_model_get_document (model) : NULL)

enum {
    PROP_0,
    PROP_HADJUSTMENT,
    PROP_VADJUSTMENT,
    PROP_HSCROLL_POLICY,
    PROP_VSCROLL_POLICY
};

G_DEFINE_TYPE_WITH_CODE (CtkDocView, ctk_doc_view, GTK_TYPE_CONTAINER,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_SCROLLABLE, NULL))

typedef enum {
    SCROLL_TO_KEEP_POSITION,
    SCROLL_TO_PAGE_POSITION,
    SCROLL_TO_CENTER,
    SCROLL_TO_FIND_LOCATION
} PendingScroll;

struct _CtkDocViewPrivate {
    CtkDocModel *model;
    GtkRequisition requisition;

    /* Scrolling */
    GtkAdjustment *hadjustment;
    GtkAdjustment *vadjustment;
    PendingScroll pending_scroll;

    /* GtkScrollablePolicy needs to be checked when
     * driving the scrollable adjustment values */
    guint hscroll_policy : 1;
    guint vscroll_policy : 1;
};

static void on_adjustment_value_changed (GtkAdjustment *adjustment,
                                         CtkDocView *view)
{
    g_print ("))))))))))))))))))))))\n");
}

static void ctk_doc_view_set_adjustment_values (CtkDocView *self,
                                                GtkOrientation orientation)
{
    CtkDocViewPrivate *priv = self->priv;
    GtkWidget *widget = GTK_WIDGET (self);
    GtkAdjustment *adjustment;
    GtkAllocation allocation;
    int req_size;
    int alloc_size;
    gdouble page_size;
    gdouble value;
    gdouble upper;
    double factor;
    gint new_value;

    gtk_widget_get_allocation (widget, &allocation);

    if (orientation == GTK_ORIENTATION_HORIZONTAL)  {
        req_size = priv->requisition.width;
        alloc_size = allocation.width;
        adjustment = priv->hadjustment;
    }
    else {
        req_size = priv->requisition.height;
        alloc_size = allocation.height;
        adjustment = priv->vadjustment;
    }

    if (!adjustment)
        return;

    factor = 1.0;
    value = gtk_adjustment_get_value (adjustment);
    upper = gtk_adjustment_get_upper (adjustment);
    page_size = gtk_adjustment_get_page_size (adjustment);

    switch (priv->pending_scroll) {
    case SCROLL_TO_KEEP_POSITION:
    case SCROLL_TO_FIND_LOCATION:
        factor = value / upper;
        break;

    case SCROLL_TO_PAGE_POSITION:
        break;

    case SCROLL_TO_CENTER:
        factor = (value + page_size * 0.5) / upper;
        break;
    }

    upper = MAX (alloc_size, req_size);
    page_size = alloc_size;

    gtk_adjustment_set_page_size (adjustment, page_size);
    gtk_adjustment_set_step_increment (adjustment, alloc_size * 0.1);
    gtk_adjustment_set_page_increment (adjustment, alloc_size * 0.9);
    gtk_adjustment_set_lower (adjustment, 0);
    gtk_adjustment_set_upper (adjustment, upper);

    /*
     * We add 0.5 to the values before to average out our rounding errors.
     */
    switch (priv->pending_scroll) {
    case SCROLL_TO_KEEP_POSITION:
    case SCROLL_TO_FIND_LOCATION:
        new_value = CLAMP (upper * factor + 0.5, 0, upper - page_size);
        gtk_adjustment_set_value (adjustment, (int) new_value);
        break;

    case SCROLL_TO_PAGE_POSITION:
        /* TODO
        ev_view_scroll_to_page_position (view, orientation);
        */
        break;

    case SCROLL_TO_CENTER:
        new_value = CLAMP (upper * factor - page_size * 0.5 + 0.5,
                           0, upper - page_size);
        gtk_adjustment_set_value (adjustment, (int) new_value);
        break;
    }

    gtk_adjustment_changed (adjustment);
}

static void ctk_doc_view_set_scroll_adjustment (CtkDocView *self,
                                                GtkOrientation orientation,
                                                GtkAdjustment *adjustment)
{
    CtkDocViewPrivate *priv = self->priv;
    GtkAdjustment **to_set;
    const gchar *prop_name;

    if (orientation == GTK_ORIENTATION_HORIZONTAL) {
        to_set = &priv->hadjustment;
        prop_name = "hadjustment";
    }
    else {
        to_set = &priv->vadjustment;
        prop_name = "vadjustment";
    }

    if (adjustment && adjustment == *to_set)
        return;

    if (*to_set) {
        g_signal_handlers_disconnect_by_func (*to_set,
                                              on_adjustment_value_changed,
                                              self);
        g_object_unref (*to_set);
    }

    if (!adjustment)
        adjustment = gtk_adjustment_new (0.0, 0.0, 0.0, 0.0, 0.0, 0.0);

    g_signal_connect (adjustment,
                      "value-changed",
                      G_CALLBACK (on_adjustment_value_changed),
                      self);

    *to_set = g_object_ref_sink (adjustment);
    ctk_doc_view_set_adjustment_values (self, orientation);

    g_object_notify (G_OBJECT (self), prop_name);
}

static void ctk_doc_view_document_changed (CtkDocModel *model,
                                           GParamSpec *pspec,
                                           CtkDocView *view)
{
    CtkDocViewPrivate *priv = view->priv;
    CtkDocument *doc = CTK_DOC_MODEL_GET_DOC (model);
    gdouble width = 0;
    gdouble height = 0;

    if (doc) {
        CtkDocPage *page;
        gint i, page_count;
        gdouble page_width, page_height;

        page_count = ctk_document_count_pages (doc);
        for (i = 0; i < page_count; ++i) {
            page = ctk_document_get_page (doc, i);
            ctk_doc_page_get_size (page, &page_width, &page_height);

            if (page_width > width)
                width = page_width;

            height += page_height;
        }
    }

    priv->requisition.width = ceil (width);
    priv->requisition.height = ceil (height);

    gtk_widget_queue_resize (GTK_WIDGET (view));
}

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
    priv->model = NULL;
    priv->requisition.width = 0;
    priv->requisition.height = 0;
    priv->hadjustment = NULL;
    priv->vadjustment = NULL;
    priv->pending_scroll = SCROLL_TO_KEEP_POSITION;
    priv->hscroll_policy = 0;
    priv->vscroll_policy = 0;
}

static void ctk_doc_view_set_property (GObject *object,
                                       guint prop_id,
                                       const GValue *value,
                                       GParamSpec *pspec)
{
    CtkDocView *self = CTK_DOC_VIEW (object);
    CtkDocViewPrivate *priv = self->priv;

    switch (prop_id) {
    case PROP_HADJUSTMENT:
        ctk_doc_view_set_scroll_adjustment (self,
                                            GTK_ORIENTATION_HORIZONTAL,
                                            g_value_get_object (value));
        break;

    case PROP_VADJUSTMENT:
        ctk_doc_view_set_scroll_adjustment (self,
                                            GTK_ORIENTATION_VERTICAL,
                                            g_value_get_object (value));
        break;

    case PROP_HSCROLL_POLICY:
        priv->hscroll_policy = g_value_get_enum (value);
        gtk_widget_queue_resize (GTK_WIDGET (self));
        break;

    case PROP_VSCROLL_POLICY:
        priv->vscroll_policy = g_value_get_enum (value);
        gtk_widget_queue_resize (GTK_WIDGET (self));
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

    switch (prop_id) {
    case PROP_HADJUSTMENT:
        g_value_set_object (value, priv->hadjustment);
        break;

    case PROP_VADJUSTMENT:
        g_value_set_object (value, priv->vadjustment);
        break;

    case PROP_HSCROLL_POLICY:
        g_value_set_enum (value, priv->hscroll_policy);
        break;

    case PROP_VSCROLL_POLICY:
        g_value_set_enum (value, priv->vscroll_policy);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void ctk_doc_view_dispose (GObject *gobject)
{
    CtkDocView *self = CTK_DOC_VIEW (gobject);

    gtk_scrollable_set_hadjustment (GTK_SCROLLABLE (self), NULL);
    gtk_scrollable_set_vadjustment (GTK_SCROLLABLE (self), NULL);

    G_OBJECT_CLASS (ctk_doc_view_parent_class)->dispose (gobject);
}

static void ctk_doc_view_finalize (GObject *gobject)
{
    CtkDocView *self = CTK_DOC_VIEW (gobject);
    CtkDocViewPrivate *priv = self->priv;

    if (priv->model)
        g_object_unref (priv->model);

    G_OBJECT_CLASS (ctk_doc_view_parent_class)->finalize (gobject);
}

static void ctk_doc_view_size_request (GtkWidget *widget,
                                       GtkRequisition *requisition)
{
    CtkDocView *self = CTK_DOC_VIEW (widget);
    CtkDocViewPrivate *priv = self->priv;

    *requisition = priv->requisition;
}

static void ctk_doc_view_get_preferred_width (GtkWidget *widget,
                                              gint *minimum,
                                              gint *natural)
{
    GtkRequisition requisition;

    ctk_doc_view_size_request (widget, &requisition);

    *minimum = *natural = requisition.width;
}

static void ctk_doc_view_get_preferred_height (GtkWidget *widget,
                                               gint *minimum,
                                               gint *natural)
{
    GtkRequisition requisition;

    ctk_doc_view_size_request (widget, &requisition);

    *minimum = *natural = requisition.height;
}

static void ctk_doc_view_size_allocate (GtkWidget *widget,
                                        GtkAllocation  *allocation)
{
    CtkDocView *self = CTK_DOC_VIEW (widget);

    gtk_widget_set_allocation (widget, allocation);

    if (gtk_widget_get_realized (widget)) {
        gdk_window_move_resize (gtk_widget_get_window (widget),
                                allocation->x,
                                allocation->y,
                                allocation->width,
                                allocation->height);
    }

    ctk_doc_view_set_adjustment_values (self, GTK_ORIENTATION_HORIZONTAL);
    ctk_doc_view_set_adjustment_values (self, GTK_ORIENTATION_VERTICAL);
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
    GTK_WIDGET_CLASS (ctk_doc_view_parent_class)->destroy (widget);
}

static void ctk_doc_view_class_init (CtkDocViewClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    gobject_class->set_property = ctk_doc_view_set_property;
    gobject_class->get_property = ctk_doc_view_get_property;
    gobject_class->dispose = ctk_doc_view_dispose;
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
    CtkDocViewPrivate *priv;

    g_return_if_fail (CTK_IS_DOC_VIEW (self));

    priv = self->priv;

    if (priv->model) {
        g_signal_handlers_disconnect_by_func (priv->model,
                                              ctk_doc_view_document_changed,
                                              self);
        g_object_unref (priv->model);
    }

    priv->model = model ? g_object_ref (model) : NULL;

    ctk_doc_view_document_changed (priv->model, NULL, self);

    if (priv->model) {
        g_signal_connect (priv->model,
                          "notify::document",
                          G_CALLBACK (ctk_doc_view_document_changed),
                          self);
    }
}
