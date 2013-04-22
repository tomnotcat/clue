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

typedef struct _CtkDocumentPrivate CtkDocumentPrivate;
typedef struct _CtkDocumentClass CtkDocumentClass;

struct _CtkDocument {
    GObject parent_instance;
    CtkDocumentPrivate *priv;
};

struct _CtkDocumentClass {
    GObjectClass parent_class;
};

GType ctk_document_get_type (void) G_GNUC_CONST;

CtkDocument* ctk_document_new (void);

G_END_DECLS

#endif /* __CTK_DOCUMENT_H__ */
