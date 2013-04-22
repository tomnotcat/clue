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
#include "ctkenums.h"

GType ctk_close_mode_get_type (void)
{
    static volatile gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile)) {
        static const GEnumValue values[] = {
            {CTK_CLOSE_MODE_NONE, "CTK_CLOSE_MODE_NONE", "NONE"},
            {CTK_CLOSE_MODE_LOST_FOCUS, "CTK_CLOSE_MODE_LOST_FOCUS", "LOST_FOCUS"},
            {CTK_CLOSE_MODE_MOUSE_LEAVE, "CTK_CLOSE_MODE_MOUSE_LEAVE", "MOUSE_LEAVE"},
            {0, NULL, NULL}
        };
        GType g_define_type_id =
                g_enum_register_static (g_intern_static_string ("CtkCloseMode"), values);
        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}
