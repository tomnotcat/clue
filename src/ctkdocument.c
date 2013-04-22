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
#include "ctkdocument.h"

G_DEFINE_ABSTRACT_TYPE (CtkDocument, ctk_document, G_TYPE_OBJECT)

struct _CtkDocumentPrivate {
    gpointer n;
};

static void ctk_document_init (CtkDocument *self)
{
    CtkDocumentPrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              CTK_TYPE_DOCUMENT,
                                              CtkDocumentPrivate);
    priv = self->priv;

    priv->n = NULL;
}

static void ctk_document_finalize (GObject *gobject)
{
    CtkDocument *self = CTK_DOCUMENT (gobject);
    CtkDocumentPrivate *priv = self->priv;

    g_free (priv->n);

    G_OBJECT_CLASS (ctk_document_parent_class)->finalize (gobject);
}

static void ctk_document_class_init (CtkDocumentClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = ctk_document_finalize;

    g_type_class_add_private (gobject_class,
                              sizeof (CtkDocumentPrivate));
}

CtkDocument* ctk_document_new (void)
{
    return g_object_new (CTK_TYPE_DOCUMENT, NULL);
}
