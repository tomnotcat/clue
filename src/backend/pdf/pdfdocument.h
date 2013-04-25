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
#ifndef __CTK_PDF_DOCUMENT_H__
#define __CTK_PDF_DOCUMENT_H__

#include "ctkdocument.h"

G_BEGIN_DECLS

#define PDF_TYPE_DOCUMENT (pdf_document_get_type())
#define PDF_DOCUMENT(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), PDF_TYPE_DOCUMENT, PdfDocument))
#define PDF_IS_DOCUMENT(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), PDF_TYPE_DOCUMENT))
#define PDF_DOCUMENT_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), PDF_TYPE_DOCUMENT, PdfDocumentClass))
#define PDF_IS_DOCUMENT_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), PDF_TYPE_DOCUMENT))
#define PDF_DOCUMENT_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), PDF_TYPE_DOCUMENT, PdfDocumentClass))

typedef struct _PdfDocument PdfDocument;
typedef struct _PdfDocumentPrivate PdfDocumentPrivate;
typedef struct _PdfDocumentClass PdfDocumentClass;

struct _PdfDocument {
    CtkDocument parent_instance;
    PdfDocumentPrivate *priv;
};

struct _PdfDocumentClass {
    CtkDocumentClass parent_class;
};

GType pdf_document_get_type (void) G_GNUC_CONST;

PdfDocument* pdf_document_new (void);

PdfDocument* clue_new_document (void);

G_END_DECLS

#endif /* __CTK_PDF_DOCUMENT_H__ */
