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
#ifndef __CTK_WINDOW_DECORATOR_H__
#define __CTK_WINDOW_DECORATOR_H__

#include "ctkbasedecorator.h"

G_BEGIN_DECLS

#define CTK_TYPE_WINDOW_DECORATOR (ctk_window_decorator_get_type())
#define CTK_WINDOW_DECORATOR(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), CTK_TYPE_WINDOW_DECORATOR, CtkWindowDecorator))
#define CTK_IS_WINDOW_DECORATOR(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), CTK_TYPE_WINDOW_DECORATOR))
#define CTK_WINDOW_DECORATOR_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), CTK_TYPE_WINDOW_DECORATOR, CtkWindowDecoratorClass))
#define CTK_IS_WINDOW_DECORATOR_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), CTK_TYPE_WINDOW_DECORATOR))
#define CTK_WINDOW_DECORATOR_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), CTK_TYPE_WINDOW_DECORATOR, CtkWindowDecoratorClass))

typedef struct _CtkWindowDecoratorPrivate CtkWindowDecoratorPrivate;
typedef struct _CtkWindowDecoratorClass CtkWindowDecoratorClass;

struct _CtkWindowDecorator {
    CtkBaseDecorator parent_instance;
    CtkWindowDecoratorPrivate *priv;
};

struct _CtkWindowDecoratorClass {
    CtkBaseDecoratorClass parent_class;
};

GType ctk_window_decorator_get_type (void) G_GNUC_CONST;

CtkWindowDecorator* ctk_window_decorator_new (void);

void ctk_window_decorator_add_tool (CtkWindowDecorator *self,
                                    CtkDecoratorWidget *tool,
                                    gint x, gint y,
                                    gint width, gint height,
                                    gfloat left, gfloat top,
                                    gfloat right, gfloat bottom);

void ctk_window_decorator_get_allocation (CtkWindowDecorator *self,
                                          GtkAllocation *allocation);

G_END_DECLS

#endif /* __CTK_WINDOW_DECORATOR_H__ */
