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
#ifndef __CTK_DOC_RENDER_TASK_H__
#define __CTK_DOC_RENDER_TASK_H__

#include "ctktypes.h"
#include <cairo.h>
#include <oren-task.h>

G_BEGIN_DECLS

#define CTK_TYPE_DOC_RENDER_TASK (ctk_doc_render_task_get_type())
#define CTK_DOC_RENDER_TASK(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), CTK_TYPE_DOC_RENDER_TASK, CtkDocRenderTask))
#define CTK_IS_DOC_RENDER_TASK(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), CTK_TYPE_DOC_RENDER_TASK))
#define CTK_DOC_RENDER_TASK_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), CTK_TYPE_DOC_RENDER_TASK, CtkDocRenderTaskClass))
#define CTK_IS_DOC_RENDER_TASK_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), CTK_TYPE_DOC_RENDER_TASK))
#define CTK_DOC_RENDER_TASK_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), CTK_TYPE_DOC_RENDER_TASK, CtkDocRenderTaskClass))

typedef struct _CtkDocRenderTaskPrivate CtkDocRenderTaskPrivate;
typedef struct _CtkDocRenderTaskClass CtkDocRenderTaskClass;

struct _CtkDocRenderTask {
    OrenTask parent_instance;
    CtkDocRenderTaskPrivate *priv;
};

struct _CtkDocRenderTaskClass {
    OrenTaskClass parent_class;
};

GType ctk_doc_render_task_get_type (void) G_GNUC_CONST;

CtkDocRenderTask* ctk_doc_render_task_new (CtkDocPage *page,
                                           gdouble scale,
                                           gint rotation);

CtkDocPage* ctk_doc_render_task_get_page (CtkDocRenderTask *self);

cairo_surface_t* ctk_doc_render_task_get_surface (CtkDocRenderTask *self);

G_END_DECLS

#endif /* __CTK_DOC_RENDER_TASK_H__ */
