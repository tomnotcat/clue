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
#include "pdfpage.h"
#include "mupdf-internal.h"

G_DEFINE_TYPE (PdfPage, pdf_page, CTK_TYPE_DOC_PAGE)

struct _PdfPagePrivate {
    pdf_document *doc;
    pdf_page *page;
};

static void _pdf_page_get_size (CtkDocPage *page,
                                gdouble *width,
                                gdouble *height)
{
    PdfPage *self = PDF_PAGE (page);
    PdfPagePrivate *priv = self->priv;
    fz_rect bbox = fz_empty_rect;

    if (priv->page)
        pdf_bound_page (priv->doc, priv->page, &bbox);

    if (width)
        *width = bbox.x1;

    if (height)
        *height = bbox.y1;
}

static void _pdf_page_render (CtkDocPage *self, cairo_t *cr)
{
}

static void _pdf_page_close (CtkDocPage *page)
{
    PdfPage *self = PDF_PAGE (page);
    PdfPagePrivate *priv = self->priv;

    if (priv->page) {
        pdf_free_page (priv->doc, priv->page);
        priv->doc = NULL;
        priv->page = NULL;
    }

    CTK_DOC_PAGE_CLASS (pdf_page_parent_class)->close (page);
}

static void pdf_page_init (PdfPage *self)
{
    PdfPagePrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              PDF_TYPE_PAGE,
                                              PdfPagePrivate);
    priv = self->priv;

    priv->doc = NULL;
    priv->page = NULL;
}

static void pdf_page_constructed (GObject *gobject)
{
    PdfPage *self = PDF_PAGE (gobject);
    PdfPagePrivate *priv = self->priv;
    CtkDocument *doc;
    gint index;

    doc = ctk_doc_page_get_document (CTK_DOC_PAGE (self));
    index = ctk_doc_page_get_index (CTK_DOC_PAGE (self));

    g_object_get (doc, "pdf-document", &priv->doc, NULL);
    priv->page = pdf_load_page (priv->doc, index);

    g_assert (priv->page);
}

static void pdf_page_finalize (GObject *gobject)
{
    PdfPage *self = PDF_PAGE (gobject);
    PdfPagePrivate *priv = self->priv;

    g_assert (!priv->doc && !priv->page);

    G_OBJECT_CLASS (pdf_page_parent_class)->finalize (gobject);
}

static void pdf_page_class_init (PdfPageClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    CtkDocPageClass *page_class = CTK_DOC_PAGE_CLASS (klass);

    gobject_class->constructed = pdf_page_constructed;
    gobject_class->finalize = pdf_page_finalize;

    page_class->get_size = _pdf_page_get_size;
    page_class->render = _pdf_page_render;
    page_class->close = _pdf_page_close;

    g_type_class_add_private (gobject_class,
                              sizeof (PdfPagePrivate));
}

CtkDocPage* pdf_page_new (CtkDocument *doc, gint index)
{
    return g_object_new (PDF_TYPE_PAGE,
                         "document", doc,
                         "index", index, NULL);
}
