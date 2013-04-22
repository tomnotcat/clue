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
#include "ctkwindowdecorator.h"

G_DEFINE_TYPE (CtkWindowDecorator, ctk_window_decorator, CTK_TYPE_BASE_DECORATOR)

struct _CtkWindowDecoratorPrivate {
    gpointer dummy;
};

static void ctk_window_decorator_init (CtkWindowDecorator *self)
{
    CtkWindowDecoratorPrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              CTK_TYPE_WINDOW_DECORATOR,
                                              CtkWindowDecoratorPrivate);
    priv = self->priv;

    priv->dummy = NULL;
}

static void ctk_window_decorator_finalize (GObject *gobject)
{
    G_OBJECT_CLASS (ctk_window_decorator_parent_class)->finalize (gobject);
}

static void ctk_window_decorator_class_init (CtkWindowDecoratorClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = ctk_window_decorator_finalize;

    g_type_class_add_private (gobject_class,
                              sizeof (CtkWindowDecoratorPrivate));
}

CtkWindowDecorator* ctk_window_decorator_new (void)
{
    return g_object_new (CTK_TYPE_WINDOW_DECORATOR, NULL);
}

void ctk_window_decorator_add_tool (CtkWindowDecorator *self,
                                    CtkDecoratorWidget *tool,
                                    gint x, gint y,
                                    gint width, gint height,
                                    gfloat left, gfloat top,
                                    gfloat right, gfloat bottom)
{
    g_return_if_fail (CTK_IS_WINDOW_DECORATOR (self));
}

/**
 * ctk_window_decorator_get_allocation:
 * @self: an #CtkWindowDecorator
 * @allocation: (out): a pointer to a #GtkAllocation to copy to
 *
 * Retrieves the decorator's allocation.
 */
void ctk_window_decorator_get_allocation (CtkWindowDecorator *self,
                                          GtkAllocation *allocation)
{
    g_return_if_fail (CTK_IS_WINDOW_DECORATOR (self));

    allocation->x = 0;
    allocation->y = 0;
    allocation->width = 0;
    allocation->height = 0;
}
