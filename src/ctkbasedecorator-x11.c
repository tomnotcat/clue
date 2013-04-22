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
#include "ctkbasedecorator.h"

G_DEFINE_TYPE (CtkBaseDecorator, ctk_base_decorator, G_TYPE_OBJECT)

struct _CtkBaseDecoratorPrivate {
    gpointer dummy;
};

static void ctk_base_decorator_init (CtkBaseDecorator *self)
{
    CtkBaseDecoratorPrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              CTK_TYPE_BASE_DECORATOR,
                                              CtkBaseDecoratorPrivate);
    priv = self->priv;

    priv->dummy = NULL;
}

static void ctk_base_decorator_finalize (GObject *gobject)
{
    G_OBJECT_CLASS (ctk_base_decorator_parent_class)->finalize (gobject);
}

static void ctk_base_decorator_class_init (CtkBaseDecoratorClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = ctk_base_decorator_finalize;

    klass->attach = NULL;
    klass->detach = NULL;

    g_type_class_add_private (gobject_class,
                              sizeof (CtkBaseDecoratorPrivate));
}

CtkBaseDecorator* ctk_base_decorator_new (void)
{
    return g_object_new (CTK_TYPE_BASE_DECORATOR, NULL);
}

gboolean ctk_base_decorator_attach (CtkBaseDecorator *self,
                                    GtkWidget *widget)
{
    g_return_val_if_fail (CTK_IS_BASE_DECORATOR (self), FALSE);

    if (CTK_BASE_DECORATOR_GET_CLASS (self)->attach)
        return CTK_BASE_DECORATOR_GET_CLASS (self)->attach (self);

    return TRUE;
}

void ctk_base_decorator_detach (CtkBaseDecorator *self)
{
    g_return_if_fail (CTK_IS_BASE_DECORATOR (self));

    if (CTK_BASE_DECORATOR_GET_CLASS (self)->detach)
        CTK_BASE_DECORATOR_GET_CLASS (self)->detach (self);
}

/**
 * ctk_base_decorator_get_window:
 * @self: an #CtkBaseDecorator
 *
 * Get the attached window.
 *
 * Return value: (transfer none): a #GtkWidget
 */
GtkWidget* ctk_base_decorator_get_window (CtkBaseDecorator *self)
{
    g_return_val_if_fail (CTK_IS_BASE_DECORATOR (self), NULL);

    return self->priv->dummy;
}

void ctk_base_decorator_set_close_mode (CtkBaseDecorator *self,
                                        CtkCloseMode mode,
                                        guint timeout)
{
    g_return_if_fail (CTK_IS_BASE_DECORATOR (self));
}

gboolean ctk_is_win7 (void)
{
    return FALSE;
}
