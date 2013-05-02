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
#include "ctkdocmodel.h"
#include "ctkdocument.h"

G_DEFINE_TYPE (CtkDocModel, ctk_doc_model, G_TYPE_OBJECT)

enum {
    PROP_0,
    PROP_DOCUMENT
};

struct _CtkDocModelPrivate {
    CtkDocument *doc;
};

static void ctk_doc_model_init (CtkDocModel *self)
{
    CtkDocModelPrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              CTK_TYPE_DOC_MODEL,
                                              CtkDocModelPrivate);
    priv = self->priv;

    priv->doc = NULL;
}

static void ctk_doc_model_set_property (GObject *object,
                                        guint prop_id,
                                        const GValue *value,
                                        GParamSpec *pspec)
{
    CtkDocModel *self = CTK_DOC_MODEL (object);

    switch (prop_id) {
    case PROP_DOCUMENT:
        ctk_doc_model_set_document (
            self, g_value_get_object (value));
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void ctk_doc_model_get_property (GObject *object,
                                        guint prop_id,
                                        GValue *value,
                                        GParamSpec *pspec)
{
    CtkDocModel *self = CTK_DOC_MODEL (object);
    CtkDocModelPrivate *priv = self->priv;

    switch (prop_id) {
    case PROP_DOCUMENT:
        g_value_set_object (value, priv->doc);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void ctk_doc_model_finalize (GObject *gobject)
{
    CtkDocModel *self = CTK_DOC_MODEL (gobject);
    CtkDocModelPrivate *priv = self->priv;

    if (priv->doc)
        g_object_unref (priv->doc);

    G_OBJECT_CLASS (ctk_doc_model_parent_class)->finalize (gobject);
}

static void ctk_doc_model_class_init (CtkDocModelClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->set_property = ctk_doc_model_set_property;
    gobject_class->get_property = ctk_doc_model_get_property;
    gobject_class->finalize = ctk_doc_model_finalize;

    g_type_class_add_private (gobject_class,
                              sizeof (CtkDocModelPrivate));

    g_object_class_install_property (
        gobject_class, PROP_DOCUMENT,
        g_param_spec_object ("document",
                             "Document",
                             "The current document",
                             CTK_TYPE_DOCUMENT,
                             G_PARAM_READWRITE |
                             G_PARAM_STATIC_STRINGS));
}

CtkDocModel* ctk_doc_model_new (void)
{
    return g_object_new (CTK_TYPE_DOC_MODEL, NULL);
}

void ctk_doc_model_set_document (CtkDocModel *self,
                                 CtkDocument *doc)
{
    CtkDocModelPrivate *priv;

    g_return_if_fail (CTK_IS_DOC_MODEL (self));

    priv = self->priv;

    if (priv->doc)
        g_object_unref (priv->doc);

    priv->doc = doc ? g_object_ref (doc) : NULL;

    g_object_notify (G_OBJECT (self), "document");
}

/**
 * ctk_doc_model_get_document:
 * @self: a #CtkDocModel
 *
 * Returns the #CtkDocument referenced by the model.
 *
 * Returns: (transfer none): a #CtkDocument
 */
CtkDocument* ctk_doc_model_get_document (CtkDocModel *self)
{
    g_return_val_if_fail (CTK_IS_DOC_MODEL (self), NULL);

    return self->priv->doc;
}

void ctk_doc_model_set_page (CtkDocModel *self,
                             gint page)
{
}

gint ctk_doc_model_get_page (CtkDocModel *self)
{
    return 0;
}

void ctk_doc_model_set_scale (CtkDocModel *self,
                              gdouble scale)
{
}

gdouble ctk_doc_model_get_scale (CtkDocModel *self)
{
    return 0.0;
}
