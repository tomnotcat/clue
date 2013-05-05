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
#ifndef __CTK_DOCUMENT_H__
#define __CTK_DOCUMENT_H__

#include "ctktypes.h"
#include <gio/gio.h>

G_BEGIN_DECLS

#define CTK_TYPE_DOCUMENT (ctk_document_get_type())
#define CTK_DOCUMENT(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), CTK_TYPE_DOCUMENT, CtkDocument))
#define CTK_IS_DOCUMENT(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), CTK_TYPE_DOCUMENT))
#define CTK_DOCUMENT_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), CTK_TYPE_DOCUMENT, CtkDocumentClass))
#define CTK_IS_DOCUMENT_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), CTK_TYPE_DOCUMENT))
#define CTK_DOCUMENT_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), CTK_TYPE_DOCUMENT, CtkDocumentClass))

#define CTK_DOCUMENT_ERROR ctk_document_error_quark ()

typedef enum {
    CTK_DOCUMENT_ERROR_INVALID
} CtkDocumentError;

typedef struct _CtkDocumentPrivate CtkDocumentPrivate;
typedef struct _CtkDocumentClass CtkDocumentClass;

struct _CtkDocument {
    GObject parent_instance;
    CtkDocumentPrivate *priv;
};

struct _CtkDocumentClass {
    GObjectClass parent_class;
    gboolean (*load) (CtkDocument *self,
                      GInputStream *stream,
                      GError **error);
    void (*close) (CtkDocument *self);
    gint (*count_pages) (CtkDocument *self);
    CtkDocPage* (*get_page) (CtkDocument *self,
                             gint index);
};

GType ctk_document_get_type (void) G_GNUC_CONST;

GQuark ctk_document_error_quark (void);

gboolean ctk_document_load (CtkDocument *self,
                            GInputStream *stream,
                            GError **error);

void ctk_document_close (CtkDocument *self);

gint ctk_document_count_pages (CtkDocument *self);

CtkDocPage* ctk_document_get_page (CtkDocument *self,
                                   gint index);

gboolean ctk_document_get_uniform_page_size (CtkDocument *self,
                                             gdouble *width,
                                             gdouble *height);

void ctk_document_get_max_page_size (CtkDocument *self,
                                     gdouble *width,
                                     gdouble *height);

void ctk_document_get_min_page_size (CtkDocument *self,
                                     gdouble *width,
                                     gdouble *height);

gboolean ctk_document_load_from_file (CtkDocument *self,
                                      const gchar *filename,
                                      GError **error);

CtkDocument* ctk_load_document_from_file (gpointer context,
                                          const gchar *filename,
                                          GError **error);

G_END_DECLS

#endif /* __CTK_DOCUMENT_H__ */
