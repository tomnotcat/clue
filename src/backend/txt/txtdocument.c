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
#include "txtdocument.h"

G_DEFINE_TYPE (TxtDocument, txt_document, CTK_TYPE_DOCUMENT)

struct _TxtDocumentPrivate {
    gpointer n;
};

static void txt_document_init (TxtDocument *self)
{
    TxtDocumentPrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              TXT_TYPE_DOCUMENT,
                                              TxtDocumentPrivate);
    priv = self->priv;

    priv->n = NULL;
}

static void txt_document_finalize (GObject *gobject)
{
    TxtDocument *self = TXT_DOCUMENT (gobject);
    TxtDocumentPrivate *priv = self->priv;

    g_free (priv->n);

    G_OBJECT_CLASS (txt_document_parent_class)->finalize (gobject);
}

static void txt_document_class_init (TxtDocumentClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = txt_document_finalize;

    g_type_class_add_private (gobject_class,
                              sizeof (TxtDocumentPrivate));
}

TxtDocument* txt_document_new (void)
{
    return g_object_new (TXT_TYPE_DOCUMENT, NULL);
}
