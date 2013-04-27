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
#include "ctkfileutils.h"
#include <gimo.h>
#include <string.h>

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

    klass->load = NULL;

    g_type_class_add_private (gobject_class,
                              sizeof (CtkDocumentPrivate));
}

gboolean ctk_document_load (CtkDocument *self,
                            const gchar *uri,
                            GError **error)
{
    return CTK_DOCUMENT_GET_CLASS (self)->load (self, uri, error);
}

/**
 * ctk_load_document:
 * @context: (type Gimo.Context): the plugin context
 * @filepath: the document file path
 *
 * Load the document from the specified path.
 *
 * Returns: (allow-none) (transfer full): a #CtkDocument
 */
CtkDocument* ctk_load_document (gpointer context,
                                const gchar *filepath)
{
    GPtrArray *backends;
    GimoExtension *ext;
    const gchar *ext_mime;
    gchar *uri;
    gchar *file_mime;
    gchar *sub;
    GError *error = NULL;
    CtkDocument *doc = NULL;
    guint i;

    backends = gimo_context_query_extensions (context,
                                              "org.clue.backend");
    if (NULL == backends)
        return NULL;

    uri = g_filename_to_uri (filepath, NULL, NULL);
    file_mime = ctk_file_get_mime_type (uri, TRUE, &error);
    if (file_mime) {
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
                        if (ctk_document_load (doc, uri, NULL))
                            break;
                    }

                    g_warning ("backend load document error: %s",
                               G_OBJECT_TYPE_NAME (obj));
                    g_object_unref (obj);
                }
            }
        }
    }
    else if (error) {
        g_warning ("get mime type error: %s", error->message);
        g_clear_error (&error);
    }

    g_free (uri);
    g_free (file_mime);
    g_ptr_array_unref (backends);
    return doc;
}
