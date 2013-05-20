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
#include "ctkdocpage.h"
#include "ctkdocument.h"

G_DEFINE_ABSTRACT_TYPE (CtkDocPage, ctk_doc_page, G_TYPE_OBJECT)

enum {
    PROP_0,
    PROP_DOCUMENT,
    PROP_INDEX
};

struct _CtkDocPagePrivate {
    CtkDocument *document;
    gint index;
};

static void _doc_page_close (CtkDocPage *self)
{
    self->priv->document = NULL;
}

static void ctk_doc_page_init (CtkDocPage *self)
{
    CtkDocPagePrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              CTK_TYPE_DOC_PAGE,
                                              CtkDocPagePrivate);
    priv = self->priv;

    priv->document = NULL;
    priv->index = 0;
}

static void ctk_doc_page_set_property (GObject *object,
                                       guint prop_id,
                                       const GValue *value,
                                       GParamSpec *pspec)
{
    CtkDocPage *self = CTK_DOC_PAGE (object);
    CtkDocPagePrivate *priv = self->priv;

    switch (prop_id) {
    case PROP_DOCUMENT:
        priv->document = g_value_get_object (value);
        break;

    case PROP_INDEX:
        priv->index = g_value_get_int (value);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void ctk_doc_page_get_property (GObject *object,
                                       guint prop_id,
                                       GValue *value,
                                       GParamSpec *pspec)
{
    CtkDocPage *self = CTK_DOC_PAGE (object);
    CtkDocPagePrivate *priv = self->priv;

    switch (prop_id) {
    case PROP_DOCUMENT:
        g_value_set_object (value, priv->document);
        break;

    case PROP_INDEX:
        g_value_set_int (value, priv->index);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void ctk_doc_page_finalize (GObject *gobject)
{
    CtkDocPage *self = CTK_DOC_PAGE (gobject);
    CtkDocPagePrivate *priv = self->priv;

    g_assert (NULL == priv->document);

    G_OBJECT_CLASS (ctk_doc_page_parent_class)->finalize (gobject);
}

static void ctk_doc_page_class_init (CtkDocPageClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->set_property = ctk_doc_page_set_property;
    gobject_class->get_property = ctk_doc_page_get_property;
    gobject_class->finalize = ctk_doc_page_finalize;

    klass->get_size = NULL;
    klass->render = NULL;
    klass->close = _doc_page_close;

    g_type_class_add_private (gobject_class,
                              sizeof (CtkDocPagePrivate));

    g_object_class_install_property (
        gobject_class, PROP_DOCUMENT,
        g_param_spec_object ("document",
                             "Document",
                             "The document of this page",
                             CTK_TYPE_DOCUMENT,
                             G_PARAM_READABLE |
                             G_PARAM_WRITABLE |
                             G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_INDEX,
        g_param_spec_int ("index",
                          "Page index",
                          "The index of this page",
                          0,
                          G_MAXINT,
                          0,
                          G_PARAM_READABLE |
                          G_PARAM_WRITABLE |
                          G_PARAM_CONSTRUCT_ONLY |
                          G_PARAM_STATIC_STRINGS));
}

/**
 * ctk_doc_page_get_size:
 * @self: a #CtkDocPage
 * @width: (out) (allow-none): location to store the page width
 * @height: (out) (allow-none): location to store the page height
 *
 * Get the page size.
 */
void ctk_doc_page_get_size (CtkDocPage *self,
                            gdouble *width,
                            gdouble *height)
{
    CTK_DOC_PAGE_GET_CLASS (self)->get_size (self, width, height);
}

void ctk_doc_page_render (CtkDocPage *self,
                          cairo_surface_t *surface,
                          const cairo_matrix_t *ctm,
                          const cairo_rectangle_int_t *area)
{
    CTK_DOC_PAGE_GET_CLASS (self)->render (self, surface, ctm, area);
}

/**
 * ctk_doc_page_get_document:
 * @self: a #CtkDocPage
 *
 * Get the document of the page.
 *
 * Returns: (allow-none) (transfer none): a #CtkDocument
 */
CtkDocument* ctk_doc_page_get_document (CtkDocPage *self)
{
    g_return_val_if_fail (CTK_IS_DOC_PAGE (self), NULL);

    return self->priv->document;
}

gint ctk_doc_page_get_index (CtkDocPage *self)
{
    g_return_val_if_fail (CTK_IS_DOC_PAGE (self), -1);

    return self->priv->index;
}
