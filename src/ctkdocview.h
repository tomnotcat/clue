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
#ifndef __CTK_DOC_VIEW_H__
#define __CTK_DOC_VIEW_H__

#include "ctktypes.h"
#include <gtk/gtk.h>
#include <oren-threadpool.h>

G_BEGIN_DECLS

#define CTK_TYPE_DOC_VIEW (ctk_doc_view_get_type())
#define CTK_DOC_VIEW(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), CTK_TYPE_DOC_VIEW, CtkDocView))
#define CTK_IS_DOC_VIEW(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), CTK_TYPE_DOC_VIEW))
#define CTK_DOC_VIEW_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), CTK_TYPE_DOC_VIEW, CtkDocViewClass))
#define CTK_IS_DOC_VIEW_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), CTK_TYPE_DOC_VIEW))
#define CTK_DOC_VIEW_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), CTK_TYPE_DOC_VIEW, CtkDocViewClass))

typedef struct _CtkDocViewPrivate CtkDocViewPrivate;
typedef struct _CtkDocViewClass CtkDocViewClass;

struct _CtkDocView {
    GtkContainer parent_instance;
    CtkDocViewPrivate *priv;
};

struct _CtkDocViewClass {
    GtkContainerClass parent_class;
};

GType ctk_doc_view_get_type (void) G_GNUC_CONST;

CtkDocView* ctk_doc_view_new (OrenThreadPool *pool);

void ctk_doc_view_set_document (CtkDocView *self,
                                CtkDocument *doc);

CtkDocument* ctk_doc_view_get_document (CtkDocView *self);

void ctk_doc_view_set_page (CtkDocView *self,
                            gint page);

gint ctk_doc_view_get_page (CtkDocView *self);

void ctk_doc_view_set_scale (CtkDocView *self,
                             gdouble scale);

gdouble ctk_doc_view_get_scale (CtkDocView *self);

gboolean ctk_doc_view_can_zoom_in (CtkDocView *self);

gboolean ctk_doc_view_can_zoom_out (CtkDocView *self);

void ctk_doc_view_zoom_in (CtkDocView *self);

void ctk_doc_view_zoom_out (CtkDocView *self);

void ctk_doc_view_set_rotation (CtkDocView *self,
                                gint rotation);

gint ctk_doc_view_get_rotation (CtkDocView *self);

void ctk_doc_view_set_sizing_mode (CtkDocView *self,
                                   CtkSizingMode mode);

CtkSizingMode ctk_doc_view_get_sizing_mode (CtkDocView *self);

void ctk_doc_view_set_continuous (CtkDocView *self,
                                  gboolean continuous);

gboolean ctk_doc_view_get_continuous (CtkDocView *self);

void ctk_doc_view_set_dual_page (CtkDocView *self,
                                 gboolean dual_page);

gboolean ctk_doc_view_get_dual_page (CtkDocView *self);

void ctk_doc_view_set_dual_page_odd_pages_left (CtkDocView *self,
                                                gboolean odd_left);

gboolean ctk_doc_view_get_dual_page_odd_pages_left (CtkDocView *self);

G_END_DECLS

#endif /* __CTK_DOC_VIEW_H__ */
