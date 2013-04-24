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
#ifndef __CTK_TXT_DOCUMENT_H__
#define __CTK_TXT_DOCUMENT_H__

#include "ctkdocument.h"

G_BEGIN_DECLS

#define TXT_TYPE_DOCUMENT (txt_document_get_type())
#define TXT_DOCUMENT(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), TXT_TYPE_DOCUMENT, TxtDocument))
#define TXT_IS_DOCUMENT(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), TXT_TYPE_DOCUMENT))
#define TXT_DOCUMENT_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), TXT_TYPE_DOCUMENT, TxtDocumentClass))
#define TXT_IS_DOCUMENT_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), TXT_TYPE_DOCUMENT))
#define TXT_DOCUMENT_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), TXT_TYPE_DOCUMENT, TxtDocumentClass))

typedef struct _TxtDocument TxtDocument;
typedef struct _TxtDocumentPrivate TxtDocumentPrivate;
typedef struct _TxtDocumentClass TxtDocumentClass;

struct _TxtDocument {
    CtkDocument parent_instance;
    TxtDocumentPrivate *priv;
};

struct _TxtDocumentClass {
    CtkDocumentClass parent_class;
};

GType txt_document_get_type (void) G_GNUC_CONST;

TxtDocument* txt_document_new (void);

G_END_DECLS

#endif /* __CTK_TXT_DOCUMENT_H__ */
