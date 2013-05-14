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
#ifndef __CTK_MARSHAL_H__
#define __CTK_MARSHAL_H__

#include "ctktypes.h"

G_BEGIN_DECLS

void ctk_marshal_VOID__INT_INT (GClosure *closure,
                                GValue *return_value G_GNUC_UNUSED,
                                guint n_param_values,
                                const GValue *param_values,
                                gpointer invocation_hint G_GNUC_UNUSED,
                                gpointer marshal_data);

G_END_DECLS

#endif /* __CTK_MARSHAL_H__ */
