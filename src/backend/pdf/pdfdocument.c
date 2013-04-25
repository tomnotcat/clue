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
#include "pdfdocument.h"

G_DEFINE_TYPE (PdfDocument, pdf_document, CTK_TYPE_DOCUMENT)

struct _PdfDocumentPrivate {
    gpointer n;
};

static gboolean _pdf_document_load (CtkDocument *self,
                                    const gchar *uri,
                                    GError **error)
{
    return TRUE;
}

static void pdf_document_init (PdfDocument *self)
{
    PdfDocumentPrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              PDF_TYPE_DOCUMENT,
                                              PdfDocumentPrivate);
    priv = self->priv;

    priv->n = NULL;
}

static void pdf_document_finalize (GObject *gobject)
{
    PdfDocument *self = PDF_DOCUMENT (gobject);
    PdfDocumentPrivate *priv = self->priv;

    g_free (priv->n);

    G_OBJECT_CLASS (pdf_document_parent_class)->finalize (gobject);
}

static void pdf_document_class_init (PdfDocumentClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    CtkDocumentClass *doc_class = CTK_DOCUMENT_CLASS (klass);

    gobject_class->finalize = pdf_document_finalize;

    doc_class->load = _pdf_document_load;

    g_type_class_add_private (gobject_class,
                              sizeof (PdfDocumentPrivate));
}

PdfDocument* pdf_document_new (void)
{
    return g_object_new (PDF_TYPE_DOCUMENT, NULL);
}

PdfDocument* clue_new_document (void)
{
    return pdf_document_new ();
}
