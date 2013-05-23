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
#ifndef __CTK_DECORATOR_BUTTON_H__
#define __CTK_DECORATOR_BUTTON_H__

#include "ctkdecoratorwidget.h"

G_BEGIN_DECLS

#define CTK_TYPE_DECORATOR_BUTTON (ctk_decorator_button_get_type())
#define CTK_DECORATOR_BUTTON(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), CTK_TYPE_DECORATOR_BUTTON, CtkDecoratorButton))
#define CTK_IS_DECORATOR_BUTTON(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), CTK_TYPE_DECORATOR_BUTTON))
#define CTK_DECORATOR_BUTTON_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), CTK_TYPE_DECORATOR_BUTTON, CtkDecoratorButtonClass))
#define CTK_IS_DECORATOR_BUTTON_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), CTK_TYPE_DECORATOR_BUTTON))
#define CTK_DECORATOR_BUTTON_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), CTK_TYPE_DECORATOR_BUTTON, CtkDecoratorButtonClass))

typedef struct _CtkDecoratorButtonPrivate CtkDecoratorButtonPrivate;
typedef struct _CtkDecoratorButtonClass CtkDecoratorButtonClass;

struct _CtkDecoratorButton {
    CtkDecoratorWidget parent_instance;
    CtkDecoratorButtonPrivate *priv;
};

struct _CtkDecoratorButtonClass {
    CtkDecoratorWidgetClass parent_class;
    void (*clicked) (CtkDecoratorButton *self);
};

GType ctk_decorator_button_get_type (void) G_GNUC_CONST;

CtkDecoratorWidget* ctk_decorator_button_new (GtkWidget *window);

G_END_DECLS

#endif /* __CTK_DECORATOR_BUTTON_H__ */
