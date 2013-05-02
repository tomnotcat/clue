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
#ifndef __CTK_TXT_PAGE_H__
#define __CTK_TXT_PAGE_H__

#include "ctkdocpage.h"

G_BEGIN_DECLS

#define TXT_TYPE_PAGE (txt_page_get_type())
#define TXT_PAGE(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), TXT_TYPE_PAGE, TxtPage))
#define TXT_IS_PAGE(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), TXT_TYPE_PAGE))
#define TXT_PAGE_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), TXT_TYPE_PAGE, TxtPageClass))
#define TXT_IS_PAGE_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), TXT_TYPE_PAGE))
#define TXT_PAGE_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), TXT_TYPE_PAGE, TxtPageClass))

typedef struct _TxtPage TxtPage;
typedef struct _TxtPagePrivate TxtPagePrivate;
typedef struct _TxtPageClass TxtPageClass;

struct _TxtPage {
    CtkDocPage parent_instance;
    TxtPagePrivate *priv;
};

struct _TxtPageClass {
    CtkDocPageClass parent_class;
};

GType txt_page_get_type (void) G_GNUC_CONST;

CtkDocPage* txt_page_new (CtkDocument *doc, gint index);

G_END_DECLS

#endif /* __CTK_TXT_PAGE_H__ */
