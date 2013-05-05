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
#include "ctkdocpage.h"
#include "ctkfileutils.h"
#include <gimo.h>
#include <string.h>

G_DEFINE_ABSTRACT_TYPE (CtkDocument, ctk_document, G_TYPE_OBJECT)

struct _CtkDocumentPrivate {
    GPtrArray *pages;
    gboolean uniform;
    gdouble uniform_width;
    gdouble uniform_height;
    gdouble max_width;
    gdouble max_height;
    gdouble min_width;
    gdouble min_height;
};

static void _doc_page_destroy (gpointer page)
{
    if (page) {
        CTK_DOC_PAGE_GET_CLASS (page)->close (page);
        g_object_unref (page);
    }
}

static void ctk_document_init (CtkDocument *self)
{
    CtkDocumentPrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              CTK_TYPE_DOCUMENT,
                                              CtkDocumentPrivate);
    priv = self->priv;

    priv->pages = NULL;
    priv->uniform = FALSE;
    priv->uniform_width = 0;
    priv->uniform_height = 0;
    priv->min_width = 0;
    priv->min_height = 0;
    priv->max_width = 0;
    priv->max_height = 0;
}

static void ctk_document_finalize (GObject *gobject)
{
    CtkDocument *self = CTK_DOCUMENT (gobject);
    CtkDocumentPrivate *priv = self->priv;

    g_assert (NULL == priv->pages);

    G_OBJECT_CLASS (ctk_document_parent_class)->finalize (gobject);
}

static void ctk_document_class_init (CtkDocumentClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = ctk_document_finalize;

    klass->load = NULL;
    klass->count_pages = NULL;
    klass->get_page = NULL;

    g_type_class_add_private (gobject_class,
                              sizeof (CtkDocumentPrivate));
}

GQuark ctk_document_error_quark (void)
{
    return g_quark_from_static_string ("ctk-document-error-quark");
}

gboolean ctk_document_load (CtkDocument *self,
                            GInputStream *stream,
                            GError **error)
{
    CtkDocumentPrivate *priv;
    CtkDocPage *page;
    gint i, count;
    gdouble page_width, page_height;
    gboolean result;

    g_return_val_if_fail (CTK_IS_DOCUMENT (self), FALSE);

    priv = self->priv;

    if (priv->pages) {
        g_set_error (error,
                     CTK_DOCUMENT_ERROR,
                     CTK_DOCUMENT_ERROR_INVALID,
                     "Document already loaded");
        return FALSE;
    }

    result = CTK_DOCUMENT_GET_CLASS (self)->load (self, stream, error);
    if (!result)
        return FALSE;

    count = ctk_document_count_pages (self);
    priv->pages = g_ptr_array_new_full (count, _doc_page_destroy);
    g_ptr_array_set_size (priv->pages, count);

    priv->uniform = TRUE;
    for (i = 0; i < count; ++i) {
        page = ctk_document_get_page (self, i);
        ctk_doc_page_get_size (page, &page_width, &page_height);

        if (i == 0) {
            priv->uniform_width = page_width;
            priv->uniform_height = page_height;
            priv->max_width = priv->uniform_width;
            priv->max_height = priv->uniform_height;
            priv->min_width = priv->uniform_width;
            priv->min_height = priv->uniform_height;
        }
        else if (priv->uniform &&
                 (priv->uniform_width != page_width ||
                  priv->uniform_height != page_height))
        {
            priv->uniform = FALSE;
        }

        if (!priv->uniform) {
            if (page_width > priv->max_width)
                priv->max_width = page_width;

            if (page_width < priv->min_width)
                priv->min_width = page_width;

            if (page_height > priv->max_height)
                priv->max_height = page_height;

            if (page_height < priv->min_height)
                priv->min_height = page_height;
        }
    }

    return result;
}

void ctk_document_close (CtkDocument *self)
{
    CtkDocumentPrivate *priv;

    g_return_if_fail (CTK_IS_DOCUMENT (self));

    priv = self->priv;

    if (priv->pages) {
        g_ptr_array_unref (priv->pages);
        priv->pages = NULL;
    }

    CTK_DOCUMENT_GET_CLASS (self)->close (self);

    priv->uniform = FALSE;
    priv->uniform_width = 0;
    priv->uniform_height = 0;
    priv->min_width = 0;
    priv->min_height = 0;
    priv->max_width = 0;
    priv->max_height = 0;

    g_assert (NULL == priv->pages);
}

gint ctk_document_count_pages (CtkDocument *self)
{
    return CTK_DOCUMENT_GET_CLASS (self)->count_pages (self);
}

/**
 * ctk_document_get_page:
 * @self: a #CtkDocument
 * @index: the page index
 *
 * Get a page object from the document.
 *
 * Returns: (allow-none) (transfer none): a #CtkPage
 */
CtkDocPage* ctk_document_get_page (CtkDocument *self,
                                   gint index)
{
    CtkDocumentPrivate *priv;
    CtkDocPage *page;

    g_return_val_if_fail (CTK_IS_DOCUMENT (self), NULL);

    priv = self->priv;

    if (!priv->pages)
        return NULL;

    if (index < 0 || index >= priv->pages->len)
        return NULL;

    page = g_ptr_array_index (priv->pages, index);
    if (page)
        return page;

    page = CTK_DOCUMENT_GET_CLASS (self)->get_page (self, index);
    g_ptr_array_index (priv->pages, index) = page;

    return page;
}

gboolean ctk_document_get_uniform_page_size (CtkDocument *self,
                                             gdouble *width,
                                             gdouble *height)
{
    CtkDocumentPrivate *priv;

    g_return_val_if_fail (CTK_IS_DOCUMENT (self), FALSE);

    priv = self->priv;

    if (priv->uniform) {
        if (width)
            *width = priv->uniform_width;

        if (height)
            *height = priv->uniform_height;

        return TRUE;
    }

    return FALSE;
}

void ctk_document_get_max_page_size (CtkDocument *self,
                                     gdouble *width,
                                     gdouble *height)
{
    CtkDocumentPrivate *priv;

    g_return_if_fail (CTK_IS_DOCUMENT (self));

    priv = self->priv;

    if (width)
        *width = priv->max_width;

    if (height)
        *height = priv->max_height;
}

void ctk_document_get_min_page_size (CtkDocument *self,
                                     gdouble *width,
                                     gdouble *height)
{
    CtkDocumentPrivate *priv;

    g_return_if_fail (CTK_IS_DOCUMENT (self));

    priv = self->priv;

    if (width)
        *width = priv->min_width;

    if (height)
        *height = priv->min_height;
}

gboolean ctk_document_load_from_file (CtkDocument *self,
                                      const gchar *filename,
                                      GError **error)
{
    gchar *uri;
    GFile *file;
    GFileInputStream *stream;
    gboolean result = FALSE;

    uri = g_filename_to_uri (filename, NULL, error);
    if (!uri)
        return FALSE;

    file = g_file_new_for_uri (uri);
    if (file) {
        stream = g_file_read (file, NULL, error);
        if (stream) {
            result = ctk_document_load (self,
                                        G_INPUT_STREAM (stream),
                                        error);
            g_object_unref (stream);
        }

        g_object_unref (file);
    }

    g_free (uri);
    return result;
}

/**
 * ctk_load_document_from_file:
 * @context: (type Gimo.Context): the plugin context
 * @filename: the document file name
 * @error: return location for an error, or %NULL
 *
 * Load the document from local file.
 *
 * Returns: (allow-none) (transfer full): a #CtkDocument
 */
CtkDocument* ctk_load_document_from_file (gpointer context,
                                          const gchar *filename,
                                          GError **error)
{
    GPtrArray *backends;
    GimoExtension *ext;
    const gchar *ext_mime = NULL;
    gchar *uri = NULL;
    gchar *file_mime = NULL;
    gchar *sub;
    CtkDocument *doc = NULL;
    guint i;

    backends = gimo_context_query_extensions (context,
                                              "org.clue.backend");
    if (NULL == backends)
        return NULL;

    uri = g_filename_to_uri (filename, NULL, error);
    if (!uri)
        goto out;

    file_mime = ctk_file_get_mime_type (uri, TRUE, error);
    if (!file_mime)
        goto out;

    for (i = 0; i < backends->len; ++i) {
        ext = g_ptr_array_index (backends, i);
        ext_mime = gimo_extension_get_config_value (ext, "mime-type");
        sub = strstr (ext_mime, file_mime);

        if (sub) {
            size_t len = strlen (file_mime);

            if ((sub == ext_mime || sub[-1] == ';') &&
                (!sub[len] || sub[len] == ';'))
            {
                GimoPlugin *plugin;
                GObject *obj;

                plugin = gimo_extension_query_plugin (ext);
                obj = gimo_plugin_resolve (plugin, "clue_new_document");
                g_object_unref (plugin);

                doc = CTK_DOCUMENT (obj);
                if (doc) {
                    if (ctk_document_load_from_file (doc, filename, error))
                        break;
                }

                g_warning ("backend load document error: %s: %s",
                           G_OBJECT_TYPE_NAME (obj), uri);
                g_object_unref (obj);
            }
        }
    }

out:
    g_free (uri);
    g_free (file_mime);
    g_ptr_array_unref (backends);
    return doc;
}
