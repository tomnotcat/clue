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
#ifndef __CTK_PDF_PAGE_H__
#define __CTK_PDF_PAGE_H__

#include "ctkdocpage.h"

G_BEGIN_DECLS

#define PDF_TYPE_PAGE (pdf_page_get_type())
#define PDF_PAGE(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), PDF_TYPE_PAGE, PdfPage))
#define PDF_IS_PAGE(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), PDF_TYPE_PAGE))
#define PDF_PAGE_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), PDF_TYPE_PAGE, PdfPageClass))
#define PDF_IS_PAGE_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), PDF_TYPE_PAGE))
#define PDF_PAGE_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), PDF_TYPE_PAGE, PdfPageClass))

typedef struct _PdfPage PdfPage;
typedef struct _PdfPagePrivate PdfPagePrivate;
typedef struct _PdfPageClass PdfPageClass;

struct _PdfPage {
    CtkDocPage parent_instance;
    PdfPagePrivate *priv;
};

struct _PdfPageClass {
    CtkDocPageClass parent_class;
};

GType pdf_page_get_type (void) G_GNUC_CONST;

CtkDocPage* pdf_page_new (CtkDocument *doc, gint index);

G_END_DECLS

#endif /* __CTK_PDF_PAGE_H__ */
