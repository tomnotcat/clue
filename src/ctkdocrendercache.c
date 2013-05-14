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
#include "ctkdocrendercache.h"
#include "ctkdocument.h"

#define RENDER_TASK_KEY "ctk-doc-render-cache-task"

G_DEFINE_TYPE (CtkDocRenderCache, ctk_doc_render_cache, G_TYPE_OBJECT)

enum {
    PROP_0,
    PROP_DOCUMENT,
    PROP_THREADPOOL,
    PROP_MAXSIZE
};

struct _CtkDocRenderCachePrivate {
    CtkDocument *document;
    OrenThreadPool *thread_pool;
    guint max_size;
};

static gint ctk_doc_render_cache_compare_task (gconstpointer a,
                                               gconstpointer b)
{
    gconstpointer c;

    c = g_object_get_data (G_OBJECT (a), RENDER_TASK_KEY);

    return GPOINTER_TO_INT (c) - GPOINTER_TO_INT (b);
}

/*
static void ctk_doc_render_cache_task_finished (CtkDocRenderTask *task,
                                                CtkDocRenderCache *cache)
{
}
*/

static void ctk_doc_render_cache_init (CtkDocRenderCache *self)
{
    CtkDocRenderCachePrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              CTK_TYPE_DOC_RENDER_CACHE,
                                              CtkDocRenderCachePrivate);
    priv = self->priv;

    (void) priv;
}

static void ctk_doc_render_cache_set_property (GObject *object,
                                               guint prop_id,
                                               const GValue *value,
                                               GParamSpec *pspec)
{
    CtkDocRenderCache *self = CTK_DOC_RENDER_CACHE (object);
    CtkDocRenderCachePrivate *priv = self->priv;

    switch (prop_id) {
    case PROP_DOCUMENT:
        priv->document = g_value_dup_object (value);
        break;

    case PROP_THREADPOOL:
        priv->thread_pool = g_value_dup_object (value);
        break;

    case PROP_MAXSIZE:
        ctk_doc_render_cache_set_max_size (self, g_value_get_uint (value));
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void ctk_doc_render_cache_get_property (GObject *object,
                                               guint prop_id,
                                               GValue *value,
                                               GParamSpec *pspec)
{
    CtkDocRenderCache *self = CTK_DOC_RENDER_CACHE (object);
    CtkDocRenderCachePrivate *priv = self->priv;

    switch (prop_id) {
    case PROP_DOCUMENT:
        g_value_set_object (value, priv->document);
        break;

    case PROP_THREADPOOL:
        g_value_set_object (value, priv->thread_pool);
        break;

    case PROP_MAXSIZE:
        g_value_set_uint (value, priv->max_size);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void ctk_doc_render_cache_finalize (GObject *gobject)
{
    CtkDocRenderCache *self = CTK_DOC_RENDER_CACHE (gobject);
    CtkDocRenderCachePrivate *priv = self->priv;

    if (priv->thread_pool) {
        oren_thread_pool_remove_tasks (priv->thread_pool,
                                       ctk_doc_render_cache_compare_task,
                                       self,
                                       TRUE);
        g_object_unref (priv->thread_pool);
    }

    if (priv->document)
        g_object_unref (priv->document);

    G_OBJECT_CLASS (ctk_doc_render_cache_parent_class)->finalize (gobject);
}

static void ctk_doc_render_cache_class_init (CtkDocRenderCacheClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->set_property = ctk_doc_render_cache_set_property;
    gobject_class->get_property = ctk_doc_render_cache_get_property;
    gobject_class->finalize = ctk_doc_render_cache_finalize;

    g_type_class_add_private (gobject_class,
                              sizeof (CtkDocRenderCachePrivate));

    g_object_class_install_property (
        gobject_class, PROP_DOCUMENT,
        g_param_spec_object ("document",
                             "Document",
                             "The document to render",
                             CTK_TYPE_DOCUMENT,
                             G_PARAM_READABLE |
                             G_PARAM_WRITABLE |
                             G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_THREADPOOL,
        g_param_spec_object ("thread-pool",
                             "Thread pool",
                             "The thread pool for render page",
                             OREN_TYPE_THREAD_POOL,
                             G_PARAM_READABLE |
                             G_PARAM_WRITABLE |
                             G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_MAXSIZE,
        g_param_spec_uint ("max-size",
                           "Max cache size",
                           "The max cache size",
                           0, G_MAXUINT, 0,
                           G_PARAM_READWRITE |
                           G_PARAM_STATIC_STRINGS));
}

CtkDocRenderCache* ctk_doc_render_cache_new (CtkDocument *doc,
                                             OrenThreadPool *pool,
                                             guint max_size)
{
    return g_object_new (CTK_TYPE_DOC_RENDER_CACHE,
                         "document", doc,
                         "thread-pool", pool,
                         "max-size", max_size, NULL);
}

void ctk_doc_render_cache_set_max_size (CtkDocRenderCache *self,
                                        guint max_size)
{
    CtkDocRenderCachePrivate *priv;

    g_return_if_fail (CTK_IS_DOC_RENDER_CACHE (self));

    priv = self->priv;

    if (max_size != priv->max_size) {
        if (max_size < priv->max_size)
            ctk_doc_render_cache_clear (self);

        priv->max_size = max_size;
    }
}

void ctk_doc_render_cache_set_page_range (CtkDocRenderCache *self,
                                          gint begin_page,
                                          gint end_page,
                                          gdouble scale,
                                          gint rotation)
{
}

cairo_surface_t* ctk_doc_render_cache_get_surface (CtkDocRenderCache *self,
                                                   gint page)
{
    return NULL;
}

void ctk_doc_render_cache_clear (CtkDocRenderCache *self)
{
    CtkDocRenderCachePrivate *priv;

    g_return_if_fail (CTK_IS_DOC_RENDER_CACHE (self));

    priv = self->priv;

    if (priv->thread_pool) {
        oren_thread_pool_remove_tasks (priv->thread_pool,
                                       ctk_doc_render_cache_compare_task,
                                       self,
                                       TRUE);
    }
}
