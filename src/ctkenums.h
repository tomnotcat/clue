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
#ifndef __CTK_ENUMS_H__
#define __CTK_ENUMS_H__

#include <glib-object.h>

G_BEGIN_DECLS

/**
 * CtkCloseMode:
 * @CTK_CLOSE_MODE_NONE: none
 * @CTK_CLOSE_MODE_LOST_FOCUS: close when lost focus
 * @CTK_CLOSE_MODE_MOUSE_LEAVE: close when mouse leave
 */
typedef enum {
    CTK_CLOSE_MODE_NONE,
    CTK_CLOSE_MODE_LOST_FOCUS,
    CTK_CLOSE_MODE_MOUSE_LEAVE
} CtkCloseMode;

GType ctk_close_mode_get_type (void) G_GNUC_CONST;
#define CTK_TYPE_CLOSE_MODE (ctk_close_mode_get_type ())

G_END_DECLS

#endif /* __CTK_ENUMS_H__ */
