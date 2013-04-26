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
#ifndef __CTK_DOC_MODEL_H__
#define __CTK_DOC_MODEL_H__

#include "ctktypes.h"

G_BEGIN_DECLS

#define CTK_TYPE_DOC_MODEL (ctk_doc_model_get_type())
#define CTK_DOC_MODEL(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), CTK_TYPE_DOC_MODEL, CtkDocModel))
#define CTK_IS_DOC_MODEL(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), CTK_TYPE_DOC_MODEL))
#define CTK_DOC_MODEL_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), CTK_TYPE_DOC_MODEL, CtkDocModelClass))
#define CTK_IS_DOC_MODEL_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), CTK_TYPE_DOC_MODEL))
#define CTK_DOC_MODEL_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), CTK_TYPE_DOC_MODEL, CtkDocModelClass))

typedef struct _CtkDocModelPrivate CtkDocModelPrivate;
typedef struct _CtkDocModelClass CtkDocModelClass;

struct _CtkDocModel {
    GObject parent_instance;
    CtkDocModelPrivate *priv;
};

struct _CtkDocModelClass {
    GObjectClass parent_class;
};

GType ctk_doc_model_get_type (void) G_GNUC_CONST;

CtkDocModel* ctk_doc_model_new (void);

void ctk_doc_model_set_document (CtkDocModel *self,
                                 CtkDocument *doc);

CtkDocument* ctk_doc_model_get_document (CtkDocModel *self);

void ctk_doc_model_set_page (CtkDocModel *self,
                             gint page);

gint ctk_doc_model_get_page (CtkDocModel *self);

void ctk_doc_model_set_scale (CtkDocModel *self,
                              gdouble scale);

gdouble ctk_doc_model_get_scale (CtkDocModel *self);

G_END_DECLS

#endif /* __CTK_DOC_MODEL_H__ */
