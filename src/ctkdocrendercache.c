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
#include "ctkdocmodel.h"
#include "ctkdocpage.h"
#include "ctkdocrendertask.h"
#include "ctkdocument.h"

#define RENDER_TASK_KEY "ctk-doc-render-cache-task"
#define MAX_PRELOADED_PAGES 3

G_DEFINE_TYPE (CtkDocRenderCache, ctk_doc_render_cache, G_TYPE_OBJECT)

enum {
    SIG_TASKFINISHED,
    LAST_SIGNAL
};

enum {
    PROP_0,
    PROP_DOCMODEL,
    PROP_THREADPOOL,
    PROP_MAXSIZE
};

typedef struct _CacheInfo {
    cairo_surface_t *surface;
    gdouble scale;
    gint rotation;
} CacheInfo;

struct _CtkDocRenderCachePrivate {
    CtkDocModel *model;
    OrenThreadPool *thread_pool;
    guint max_size;
    gint cache_index;
    gint cache_length;
    CacheInfo *cache_info;
};

static guint cache_signals[LAST_SIGNAL] = { 0 };

extern void ctk_get_page_size_for_scale_and_rotation (CtkDocument *document,
                                                      gint index,
                                                      gdouble scale,
                                                      gint rotation,
                                                      gdouble *page_width,
                                                      gdouble *page_height);

static void cache_info_destroy (CacheInfo *info)
{
    if (info->surface) {
        cairo_surface_destroy (info->surface);
        info->surface = NULL;
    }
}

static gint ctk_doc_render_cache_compare_find (gconstpointer a,
                                               gconstpointer b)
{
    CtkDocRenderTask *task = CTK_DOC_RENDER_TASK (a);
    CtkDocPage *page = ctk_doc_render_task_get_page (task);
    gint index = ctk_doc_page_get_index (page);

    return index - GPOINTER_TO_INT (b);
}

static gint ctk_doc_render_cache_compare_task (gconstpointer a,
                                               gconstpointer b)
{
    gconstpointer c;

    c = g_object_get_data (G_OBJECT (a), RENDER_TASK_KEY);

    return GPOINTER_TO_INT (c) - GPOINTER_TO_INT (b);
}

static CacheInfo* ctk_doc_render_cache_get_cache (CtkDocRenderCache *self,
                                                  gint page)
{
    CtkDocRenderCachePrivate *priv = self->priv;

    if (page < priv->cache_index)
        return NULL;

    if (page >= priv->cache_index + priv->cache_length)
        return NULL;

    return &priv->cache_info[page - priv->cache_index];
}

static void ctk_doc_render_cache_task_finished (CtkDocRenderTask *task,
                                                CtkDocRenderCache *cache)
{
    CtkDocPage *page;
    CacheInfo *info;
    gint page_index;

    page = ctk_doc_render_task_get_page (task);
    page_index = ctk_doc_page_get_index (page);

    info = ctk_doc_render_cache_get_cache (cache, page_index);
    if (NULL == info)
        return;

    cache_info_destroy (info);
    info->surface = ctk_doc_render_task_get_surface (task);

    if (info->surface)
        cairo_surface_reference (info->surface);

    g_object_get (G_OBJECT (task),
                  "scale", &info->scale,
                  "rotation", &info->rotation,
                  NULL);

    g_signal_emit (cache,
                   cache_signals[SIG_TASKFINISHED],
                   0,
                   page_index);
}

static gsize ctk_doc_render_cache_get_page_size (CtkDocRenderCache *self,
                                                 gint page_index,
                                                 gdouble scale,
                                                 gint rotation)
{
    CtkDocRenderCachePrivate *priv = self->priv;
    CtkDocument *document = ctk_doc_model_get_document (priv->model);
    gdouble width, height;

    ctk_get_page_size_for_scale_and_rotation (document,
                                              page_index,
                                              scale, rotation,
                                              &width, &height);

    return height * cairo_format_stride_for_width (CAIRO_FORMAT_ARGB32, width);
}

static void ctk_doc_render_cache_get_preload_range (CtkDocRenderCache *self,
                                                    gint begin_page,
                                                    gint end_page,
                                                    gdouble scale,
                                                    gint rotation,
                                                    gint *preload_begin,
                                                    gint *preload_end)
{
    CtkDocRenderCachePrivate *priv = self->priv;
    CtkDocument *document = ctk_doc_model_get_document (priv->model);
    gsize range_size = 0;
    gint preload_cache_size = 0;
    gint i, n_pages = ctk_document_count_pages (document);

    /* Get the size of the current range */
    for (i = begin_page; i < end_page; ++i) {
        range_size += ctk_doc_render_cache_get_page_size (self, i, scale, rotation);
    }

    if (range_size >= priv->max_size) {
        *preload_begin = begin_page;
        *preload_end = end_page;
        return;
    }

    i = 1;
    while (((begin_page - i >= 0) || (end_page + i - 1 < n_pages)) &&
           preload_cache_size < MAX_PRELOADED_PAGES)
    {
        gsize page_size;
        gboolean updated = FALSE;

        if (end_page + i - 1 < n_pages) {
            page_size = ctk_doc_render_cache_get_page_size (self,
                                                            end_page + i - 1,
                                                            scale,
                                                            rotation);
            if (page_size + range_size <= priv->max_size) {
                range_size += page_size;
                preload_cache_size++;
                updated = TRUE;
            }
            else {
                break;
            }
        }

        if (begin_page - i >= 0) {
            page_size = ctk_doc_render_cache_get_page_size (self,
                                                            begin_page - i,
                                                            scale,
                                                            rotation);
            if (page_size + range_size <= priv->max_size) {
                range_size += page_size;
                if (!updated)
                    preload_cache_size++;
            }
            else {
                break;
            }
        }

        i++;
    }

    *preload_begin = MAX (begin_page - preload_cache_size, 0);
    *preload_end = MIN (end_page + preload_cache_size, n_pages);
}

static void ctk_doc_render_cache_init (CtkDocRenderCache *self)
{
    CtkDocRenderCachePrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              CTK_TYPE_DOC_RENDER_CACHE,
                                              CtkDocRenderCachePrivate);
    priv = self->priv;

    priv->cache_info = NULL;
    priv->cache_index = -1;
    priv->cache_length = 0;
}

static void ctk_doc_render_cache_set_property (GObject *object,
                                               guint prop_id,
                                               const GValue *value,
                                               GParamSpec *pspec)
{
    CtkDocRenderCache *self = CTK_DOC_RENDER_CACHE (object);
    CtkDocRenderCachePrivate *priv = self->priv;

    switch (prop_id) {
    case PROP_DOCMODEL:
        priv->model = g_value_dup_object (value);
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
    case PROP_DOCMODEL:
        g_value_set_object (value, priv->model);
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
    gint i;

    if (priv->thread_pool) {
        oren_thread_pool_remove_tasks (priv->thread_pool,
                                       ctk_doc_render_cache_compare_task,
                                       self,
                                       TRUE);
        g_object_unref (priv->thread_pool);
    }

    for (i = 0; i < priv->cache_length; ++i)
        cache_info_destroy (&priv->cache_info[i]);

    if (priv->model)
        g_object_unref (priv->model);

    G_OBJECT_CLASS (ctk_doc_render_cache_parent_class)->finalize (gobject);
}

static void ctk_doc_render_cache_class_init (CtkDocRenderCacheClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->set_property = ctk_doc_render_cache_set_property;
    gobject_class->get_property = ctk_doc_render_cache_get_property;
    gobject_class->finalize = ctk_doc_render_cache_finalize;

    klass->task_finished = NULL;

    g_type_class_add_private (gobject_class,
                              sizeof (CtkDocRenderCachePrivate));

    cache_signals[SIG_TASKFINISHED] =
            g_signal_new ("task-finished",
                          G_OBJECT_CLASS_TYPE (gobject_class),
                          G_SIGNAL_RUN_FIRST,
                          G_STRUCT_OFFSET (CtkDocRenderCacheClass, task_finished),
                          NULL, NULL,
                          g_cclosure_marshal_VOID__INT,
                          G_TYPE_NONE,
                          1,
                          G_TYPE_INT);

    g_object_class_install_property (
        gobject_class, PROP_DOCMODEL,
        g_param_spec_object ("model",
                             "Document model",
                             "The document model",
                             CTK_TYPE_DOC_MODEL,
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

CtkDocRenderCache* ctk_doc_render_cache_new (CtkDocModel *model,
                                             OrenThreadPool *pool,
                                             guint max_size)
{
    return g_object_new (CTK_TYPE_DOC_RENDER_CACHE,
                         "model", model,
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
                                          gint end_page)
{
    CtkDocRenderCachePrivate *priv;
    CtkDocRenderTask *task;
    CtkDocument *document;
    CtkDocPage *page;
    GQueue *tasks;
    GList *it, *next;
    CacheInfo *info;
    gdouble scale;
    gint rotation;
    gint preload_begin;
    gint preload_end;
    gint i, j;

    g_return_if_fail (CTK_IS_DOC_RENDER_CACHE (self));

    priv = self->priv;

    document = ctk_doc_model_get_document (priv->model);
    scale = ctk_doc_model_get_scale (priv->model);
    rotation = ctk_doc_model_get_rotation (priv->model);

    ctk_doc_render_cache_get_preload_range (self,
                                            begin_page,
                                            end_page,
                                            scale,
                                            rotation,
                                            &preload_begin,
                                            &preload_end);
    /* Update the cache infos. */
    info = priv->cache_info;
    priv->cache_info = g_new0 (CacheInfo, preload_end - preload_begin);

    for (i = 0; i < priv->cache_length; ++i) {
        j = priv->cache_index + i;

        if (j < preload_begin || j >= preload_end)
            cache_info_destroy (&info[i]);
        else
            priv->cache_info[j - preload_begin] = info[i];
    }

    g_free (info);
    priv->cache_index = preload_begin;
    priv->cache_length = preload_end - preload_begin;

    /* Update pending render tasks. */
    tasks = oren_thread_pool_lock_tasks (priv->thread_pool);

    it = g_queue_peek_head_link (tasks);
    while (it) {
        next = it->next;
        task = it->data;
        page = ctk_doc_render_task_get_page (task);
        i = ctk_doc_page_get_index (page);

        if (i < preload_begin || i >= preload_end) {
            g_object_unref (task);
            g_queue_delete_link (tasks, it);
        }
        else {
            g_object_set (G_OBJECT (task),
                          "scale", scale,
                          "rotation", rotation,
                          NULL);
        }

        it = next;
    }

    /* Add new tasks. */
    for (i = preload_begin; i < preload_end; ++i) {
        info = ctk_doc_render_cache_get_cache (self, i);
        if (info->surface &&
            info->scale == scale &&
            info->rotation == rotation)
        {
            continue;
        }

        it = g_queue_find_custom (tasks,
                                  GINT_TO_POINTER (i),
                                  ctk_doc_render_cache_compare_find);
        if (NULL == it) {
            page = ctk_document_get_page (document, i);
            task = ctk_doc_render_task_new (page, scale, rotation);

            g_signal_connect (task,
                              "finished",
                              G_CALLBACK (ctk_doc_render_cache_task_finished),
                              self);

            g_queue_push_tail (tasks, task);
        }
    }

    oren_thread_pool_unlock_tasks (priv->thread_pool);
}

/**
 * ctk_doc_render_cache_get_surface:
 * @self: a #CtkDocRenderCache
 * @page: the page index
 *
 * Get the rendered pixbuf of the specified page.
 *
 * Returns: (allow-none) (transfer none): a #cairo_surface_t
 */
cairo_surface_t* ctk_doc_render_cache_get_surface (CtkDocRenderCache *self,
                                                   gint page)
{
    CacheInfo *info;

    g_return_val_if_fail (CTK_IS_DOC_RENDER_CACHE (self), NULL);

    info = ctk_doc_render_cache_get_cache (self, page);
    if (info)
        return info->surface;

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

    if (priv->cache_info) {
        gint i;

        for (i = 0; i < priv->cache_length; ++i)
            cache_info_destroy (&priv->cache_info[i]);

        g_free (priv->cache_info);

        priv->cache_info = NULL;
        priv->cache_index = 0;
        priv->cache_length = 0;
    }
}
