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
#ifndef __CTK_DOC_RENDER_CACHE_H__
#define __CTK_DOC_RENDER_CACHE_H__

#include "ctktypes.h"
#include <cairo.h>
#include <oren-threadpool.h>

G_BEGIN_DECLS

#define CTK_TYPE_DOC_RENDER_CACHE (ctk_doc_render_cache_get_type())
#define CTK_DOC_RENDER_CACHE(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), CTK_TYPE_DOC_RENDER_CACHE, CtkDocRenderCache))
#define CTK_IS_DOC_RENDER_CACHE(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), CTK_TYPE_DOC_RENDER_CACHE))
#define CTK_DOC_RENDER_CACHE_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), CTK_TYPE_DOC_RENDER_CACHE, CtkDocRenderCacheClass))
#define CTK_IS_DOC_RENDER_CACHE_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), CTK_TYPE_DOC_RENDER_CACHE))
#define CTK_DOC_RENDER_CACHE_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), CTK_TYPE_DOC_RENDER_CACHE, CtkDocRenderCacheClass))

typedef struct _CtkDocRenderCachePrivate CtkDocRenderCachePrivate;
typedef struct _CtkDocRenderCacheClass CtkDocRenderCacheClass;

struct _CtkDocRenderCache {
    GObject parent_instance;
    CtkDocRenderCachePrivate *priv;
};

struct _CtkDocRenderCacheClass {
    GObjectClass parent_class;
};

GType ctk_doc_render_cache_get_type (void) G_GNUC_CONST;

CtkDocRenderCache* ctk_doc_render_cache_new (CtkDocument *doc,
                                             OrenThreadPool *pool);

void ctk_doc_render_cache_set_page_range (CtkDocRenderCache *self,
                                          gint begin_page,
                                          gint end_page);

cairo_surface_t* ctk_doc_render_cache_get_surface (CtkDocRenderCache *self,
                                                   gint page);

void ctk_doc_render_cache_clear (CtkDocRenderCache *self);

G_END_DECLS

#endif /* __CTK_DOC_RENDER_CACHE_H__ */
