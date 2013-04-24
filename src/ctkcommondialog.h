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
#ifndef __CTK_COMMON_DIALOG_H__
#define __CTK_COMMON_DIALOG_H__

#include "ctktypes.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

gchar* ctk_open_single_file (GtkWidget *parent,
                             const gchar *name,
                             const gchar *dir,
                             const gchar *title,
                             gchar **filters);

G_END_DECLS

#endif /* __CTK_COMMON_DIALOG_H__ */
