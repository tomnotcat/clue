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
#ifndef __CTK_DOC_PAGE_H__
#define __CTK_DOC_PAGE_H__

#include "ctktypes.h"
#include <cairo.h>

G_BEGIN_DECLS

#define CTK_TYPE_DOC_PAGE (ctk_doc_page_get_type())
#define CTK_DOC_PAGE(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), CTK_TYPE_DOC_PAGE, CtkDocPage))
#define CTK_IS_DOC_PAGE(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), CTK_TYPE_DOC_PAGE))
#define CTK_DOC_PAGE_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), CTK_TYPE_DOC_PAGE, CtkDocPageClass))
#define CTK_IS_DOC_PAGE_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), CTK_TYPE_DOC_PAGE))
#define CTK_DOC_PAGE_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), CTK_TYPE_DOC_PAGE, CtkDocPageClass))

typedef struct _CtkDocPagePrivate CtkDocPagePrivate;
typedef struct _CtkDocPageClass CtkDocPageClass;

struct _CtkDocPage {
    GObject parent_instance;
    CtkDocPagePrivate *priv;
};

struct _CtkDocPageClass {
    GObjectClass parent_class;
    void (*get_size) (CtkDocPage *self,
                      gdouble *width,
                      gdouble *height);
    void (*render) (CtkDocPage *self,
                    cairo_surface_t *surface,
                    const cairo_matrix_t *ctm,
                    const cairo_rectangle_int_t *area);
    void (*close) (CtkDocPage *self);
};

GType ctk_doc_page_get_type (void) G_GNUC_CONST;

void ctk_doc_page_get_size (CtkDocPage *self,
                            gdouble *width,
                            gdouble *height);

void ctk_doc_page_render (CtkDocPage *self,
                          cairo_surface_t *surface,
                          const cairo_matrix_t *ctm,
                          const cairo_rectangle_int_t *area);

CtkDocument* ctk_doc_page_get_document (CtkDocPage *self);

gint ctk_doc_page_get_index (CtkDocPage *self);

G_END_DECLS

#endif /* __CTK_DOC_PAGE_H__ */
