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
#include "pdfdocument.h"

G_DEFINE_TYPE (PdfPage, pdf_page, CTK_TYPE_DOC_PAGE)

struct _PdfPagePrivate {
    pdf_document *doc;
    pdf_page *page;
    fz_display_list *page_list;
};

static void _pdf_page_lock_ctx (PdfPage *self)
{
    CtkDocument *doc = ctk_doc_page_get_document (CTK_DOC_PAGE (self));
    pdf_document_lock (PDF_DOCUMENT (doc));
}

static void _pdf_page_unlock_ctx (PdfPage *self)
{
    CtkDocument *doc = ctk_doc_page_get_document (CTK_DOC_PAGE (self));
    pdf_document_unlock (PDF_DOCUMENT (doc));
}

static fz_display_list* _pdf_page_get_page_list (PdfPage *self)
{
    PdfPagePrivate *priv = self->priv;

    if (priv->page_list)
        return priv->page_list;

    _pdf_page_lock_ctx (self);

    if (NULL == priv->page_list) {
        fz_device *mdev;
        fz_cookie cookie = { 0 };

        priv->page_list = fz_new_display_list (priv->doc->ctx);
        mdev = fz_new_list_device (priv->doc->ctx, priv->page_list);
        pdf_run_page_contents (priv->doc, priv->page, mdev, &fz_identity, &cookie);
        fz_free_device (mdev);
    }

    _pdf_page_unlock_ctx (self);

    return priv->page_list;
}

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

static void _pdf_page_render (CtkDocPage *page,
                              cairo_surface_t *surface,
                              const cairo_matrix_t *_ctm,
                              const cairo_rectangle_int_t *area)
{
    PdfPage *self = PDF_PAGE (page);
    PdfPagePrivate *priv = self->priv;
    fz_context *ctx;
    fz_colorspace *colorspace;
    fz_pixmap *pixmap;
    fz_device *idev;
    fz_display_list *list;
    gint width, height;
    fz_matrix ctm;
    fz_cookie cookie = { 0 };

#ifdef _WIN32
    colorspace = fz_device_bgr;
#else
    colorspace = fz_device_rgb;
#endif

    /* TODO: implement a cairo device for mupdf ? */
    if (cairo_surface_get_type (surface) == CAIRO_SURFACE_TYPE_IMAGE) {
        g_assert (cairo_image_surface_get_format (surface) == CAIRO_FORMAT_ARGB32);
    }
    else {
        g_assert (0);
    }

    /* Draw page */
    _pdf_page_lock_ctx (self);
    ctx = fz_clone_context (priv->doc->ctx);
    _pdf_page_unlock_ctx (self);

    cairo_surface_flush (surface);

    width = cairo_image_surface_get_width (surface);
    height = cairo_image_surface_get_height (surface);

    ctm.a = _ctm->xx;
    ctm.b = _ctm->yx;
    ctm.c = _ctm->xy;
    ctm.d = _ctm->yy;
    ctm.e = _ctm->x0;
    ctm.f = _ctm->y0;

    pixmap = fz_new_pixmap_with_data (ctx, colorspace, width, height,
                                      cairo_image_surface_get_data (surface));
    idev = fz_new_draw_device (ctx, pixmap);

    list = _pdf_page_get_page_list (self);
    fz_run_display_list (list, idev, &ctm, &fz_infinite_rect, &cookie);

    cairo_surface_mark_dirty (surface);

    fz_free_device (idev);
    fz_drop_pixmap (ctx, pixmap);
    fz_free_context (ctx);
}

static void _pdf_page_close (CtkDocPage *page)
{
    PdfPage *self = PDF_PAGE (page);
    PdfPagePrivate *priv = self->priv;

    if (priv->page_list) {
        fz_free_display_list (priv->doc->ctx, priv->page_list);
        priv->page_list = NULL;
    }

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
    priv->page_list = NULL;
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
