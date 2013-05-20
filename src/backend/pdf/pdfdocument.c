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
#include "mupdf-internal.h"
#include "pdfpage.h"

G_DEFINE_TYPE (PdfDocument, pdf_document, CTK_TYPE_DOCUMENT)

enum {
    PROP_0,
    PROP_PDFCONTEXT,
    PROP_PDFDOCUMENT
};

struct _PdfDocumentPrivate {
    fz_context *ctx;
    pdf_document *doc;
    fz_locks_context locks;
    GMutex mutex[FZ_LOCK_MAX];
    GMutex ctx_mutex;
};

static int _pdf_file_read (fz_stream *stm, unsigned char *buf, int len)
{
    return g_input_stream_read (stm->state, buf, len, NULL, NULL);
}

static void _pdf_file_seek (fz_stream *stm, int offset, int whence)
{
    GSeekType type;

    switch (whence) {
    case SEEK_SET:
        type = G_SEEK_SET;
        break;

    case SEEK_CUR:
        type = G_SEEK_CUR;
        break;

    case SEEK_END:
        type = G_SEEK_END;
        break;

    default:
        return;
    }

    if (!g_seekable_seek (G_SEEKABLE (stm->state), offset, type, NULL, NULL))
        return;

    stm->pos = g_seekable_tell (G_SEEKABLE (stm->state));
    stm->rp = stm->bp;
    stm->wp = stm->bp;
}

static void _pdf_file_close (fz_context *ctx, void *state)
{
    g_object_unref (state);
}

static void _pdf_document_lock_mutex (void *user, int lock)
{
    GMutex *mutex = (GMutex *) user;

    g_mutex_lock (&mutex[lock]);
}

static void _pdf_document_unlock_mutex (void *user, int lock)
{
    GMutex *mutex = (GMutex *) user;

    g_mutex_unlock (&mutex[lock]);
}

static gboolean _pdf_document_load (CtkDocument *doc,
                                    GInputStream *stream,
                                    GError **error)
{
    PdfDocument *self = PDF_DOCUMENT (doc);
    PdfDocumentPrivate *priv = self->priv;
    fz_context *ctx = NULL;
    pdf_document *pdf = NULL;
    fz_stream *file = NULL;
    gboolean result = FALSE;

    g_assert (NULL == priv->ctx && NULL == priv->doc);

    ctx = fz_new_context (NULL, &priv->locks, FZ_STORE_UNLIMITED);
    if (!ctx) {
        g_set_error (error,
                     CTK_DOCUMENT_ERROR,
                     CTK_DOCUMENT_ERROR_INVALID,
                     "Create context failed");
        goto out;
    }

    file = fz_new_stream (ctx,
                          g_object_ref (stream),
                          _pdf_file_read,
                          _pdf_file_close);
    file->seek = _pdf_file_seek;

    pdf = pdf_open_document_with_stream (ctx, file);
    if (!pdf) {
        g_set_error (error,
                     CTK_DOCUMENT_ERROR,
                     CTK_DOCUMENT_ERROR_INVALID,
                     "Open document failed");
        goto out;
    }

    g_assert (!pdf_needs_password (pdf));

    priv->ctx = ctx;
    priv->doc = pdf;
    ctx = NULL;
    pdf = NULL;
    result = TRUE;

out:
    if (file)
        fz_close (file);

    if (pdf)
        pdf_close_document (pdf);

    if (ctx)
        fz_free_context (ctx);

    return result;
}

static void _pdf_document_close (CtkDocument *doc)
{
    PdfDocument *self = PDF_DOCUMENT (doc);
    PdfDocumentPrivate *priv = self->priv;

    if (priv->doc) {
        pdf_close_document (priv->doc);
        priv->doc = NULL;
    }

    if (priv->ctx) {
        fz_free_context (priv->ctx);
        priv->ctx = NULL;
    }
}

static gint _pdf_document_count_pages (CtkDocument *doc)
{
    PdfDocument *self = PDF_DOCUMENT (doc);
    PdfDocumentPrivate *priv = self->priv;

    return pdf_count_pages (priv->doc);
}

static void pdf_document_init (PdfDocument *self)
{
    PdfDocumentPrivate *priv;
    gint i;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              PDF_TYPE_DOCUMENT,
                                              PdfDocumentPrivate);
    priv = self->priv;

    priv->ctx = NULL;
    priv->doc = NULL;
    priv->locks.user = priv->mutex;
    priv->locks.lock = _pdf_document_lock_mutex;
    priv->locks.unlock = _pdf_document_unlock_mutex;

    for (i = 0; i < FZ_LOCK_MAX; ++i)
        g_mutex_init (&priv->mutex[i]);

    g_mutex_init (&priv->ctx_mutex);
}

static void pdf_document_get_property (GObject *object,
                                       guint prop_id,
                                       GValue *value,
                                       GParamSpec *pspec)
{
    PdfDocument *self = PDF_DOCUMENT (object);
    PdfDocumentPrivate *priv = self->priv;

    switch (prop_id) {
    case PROP_PDFCONTEXT:
        g_value_set_pointer (value, priv->ctx);
        break;

    case PROP_PDFDOCUMENT:
        g_value_set_pointer (value, priv->doc);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void pdf_document_finalize (GObject *gobject)
{
    PdfDocument *self = PDF_DOCUMENT (gobject);
    PdfDocumentPrivate *priv = self->priv;
    gint i;

    ctk_document_close (CTK_DOCUMENT (gobject));

    for (i = 0; i < FZ_LOCK_MAX; ++i)
        g_mutex_clear (&priv->mutex[i]);

    g_mutex_clear (&priv->ctx_mutex);

    G_OBJECT_CLASS (pdf_document_parent_class)->finalize (gobject);
}

static void pdf_document_class_init (PdfDocumentClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    CtkDocumentClass *doc_class = CTK_DOCUMENT_CLASS (klass);

    gobject_class->get_property = pdf_document_get_property;
    gobject_class->finalize = pdf_document_finalize;

    doc_class->load = _pdf_document_load;
    doc_class->close = _pdf_document_close;
    doc_class->count_pages = _pdf_document_count_pages;
    doc_class->get_page = pdf_page_new;

    g_type_class_add_private (gobject_class,
                              sizeof (PdfDocumentPrivate));

    g_object_class_install_property (
        gobject_class, PROP_PDFCONTEXT,
        g_param_spec_pointer ("pdf-context",
                              "PDF Context",
                              "The context of pdf lib",
                              G_PARAM_READABLE |
                              G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_PDFDOCUMENT,
        g_param_spec_pointer ("pdf-document",
                              "PDF Document",
                              "The document of pdf lib",
                              G_PARAM_READABLE |
                              G_PARAM_STATIC_STRINGS));
}

PdfDocument* pdf_document_new (void)
{
    return g_object_new (PDF_TYPE_DOCUMENT, NULL);
}

void pdf_document_lock (PdfDocument *self)
{
    g_return_if_fail (PDF_IS_DOCUMENT (self));

    g_mutex_lock (&self->priv->ctx_mutex);
}

void pdf_document_unlock (PdfDocument *self)
{
    g_return_if_fail (PDF_IS_DOCUMENT (self));

    g_mutex_unlock (&self->priv->ctx_mutex);
}

PdfDocument* clue_new_document (void)
{
    return pdf_document_new ();
}
