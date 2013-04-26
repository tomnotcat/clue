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

G_DEFINE_TYPE (CtkDocModel, ctk_doc_model, G_TYPE_OBJECT)

struct _CtkDocModelPrivate {
    gpointer n;
};

static void ctk_doc_model_init (CtkDocModel *self)
{
    CtkDocModelPrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              CTK_TYPE_DOC_MODEL,
                                              CtkDocModelPrivate);
    priv = self->priv;

    priv->n = NULL;
}

static void ctk_doc_model_finalize (GObject *gobject)
{
    CtkDocModel *self = CTK_DOC_MODEL (gobject);
    CtkDocModelPrivate *priv = self->priv;

    g_free (priv->n);

    G_OBJECT_CLASS (ctk_doc_model_parent_class)->finalize (gobject);
}

static void ctk_doc_model_class_init (CtkDocModelClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = ctk_doc_model_finalize;

    g_type_class_add_private (gobject_class,
                              sizeof (CtkDocModelPrivate));
}

CtkDocModel* ctk_doc_model_new (void)
{
    return g_object_new (CTK_TYPE_DOC_MODEL, NULL);
}

void ctk_doc_model_set_document (CtkDocModel *self,
                                 CtkDocument *doc)
{
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
    return NULL;
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
