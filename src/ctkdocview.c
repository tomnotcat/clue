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

static void ctk_doc_view_class_init (CtkDocViewClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->set_property = ctk_doc_view_set_property;
    gobject_class->get_property = ctk_doc_view_get_property;
    gobject_class->finalize = ctk_doc_view_finalize;

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
