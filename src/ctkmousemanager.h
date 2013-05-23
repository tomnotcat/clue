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
#ifndef __CTK_MOUSE_MANAGER_H__
#define __CTK_MOUSE_MANAGER_H__

#include "ctktypes.h"
#include <Windows.h>

G_BEGIN_DECLS

typedef enum {
    CTK_MOUSE_TRACK_MOUSEMOVE     = 1 << 0,
    CTK_MOUSE_TRACK_LBUTTONDOWN   = 1 << 1,
    CTK_MOUSE_TRACK_LBUTTONUP     = 1 << 2,
    CTK_MOUSE_TRACK_LBUTTONDBLCLK = 1 << 3,
    CTK_MOUSE_TRACK_RBUTTONDOWN   = 1 << 4,
    CTK_MOUSE_TRACK_RBUTTONUP     = 1 << 5,
    CTK_MOUSE_TRACK_RBUTTONDBLCLK = 1 << 6,
    CTK_MOUSE_TRACK_MBUTTONDOWN   = 1 << 7,
    CTK_MOUSE_TRACK_MBUTTONUP     = 1 << 8,
    CTK_MOUSE_TRACK_MBUTTONDBLCLK = 1 << 9,
    CTK_MOUSE_TRACK_NCMOUSEMOVE     = 1 << 10,
    CTK_MOUSE_TRACK_NCLBUTTONDOWN   = 1 << 11,
    CTK_MOUSE_TRACK_NCLBUTTONUP     = 1 << 12,
    CTK_MOUSE_TRACK_NCLBUTTONDBLCLK = 1 << 13,
    CTK_MOUSE_TRACK_NCRBUTTONDOWN   = 1 << 14,
    CTK_MOUSE_TRACK_NCRBUTTONUP     = 1 << 15,
    CTK_MOUSE_TRACK_NCRBUTTONDBLCLK = 1 << 16,
    CTK_MOUSE_TRACK_NCMBUTTONDOWN   = 1 << 17,
    CTK_MOUSE_TRACK_NCMBUTTONUP     = 1 << 18,
    CTK_MOUSE_TRACK_NCMBUTTONDBLCLK = 1 << 19
} CtkMouseTrackFlags;

typedef enum {
    CTK_MOUSE_MSG_MOUSEMOVE   = WM_USER + 2114,
    CTK_MOUSE_MSG_LBUTTONDOWN,
    CTK_MOUSE_MSG_LBUTTONUP,
    CTK_MOUSE_MSG_LBUTTONDBLCLK,
    CTK_MOUSE_MSG_RBUTTONDOWN,
    CTK_MOUSE_MSG_RBUTTONUP,
    CTK_MOUSE_MSG_RBUTTONDBLCLK,
    CTK_MOUSE_MSG_MBUTTONDOWN,
    CTK_MOUSE_MSG_MBUTTONUP,
    CTK_MOUSE_MSG_MBUTTONDBLCLK,
    CTK_MOUSE_MSG_NCMOUSEMOVE,
    CTK_MOUSE_MSG_NCLBUTTONDOWN,
    CTK_MOUSE_MSG_NCLBUTTONUP,
    CTK_MOUSE_MSG_NCLBUTTONDBLCLK,
    CTK_MOUSE_MSG_NCRBUTTONDOWN,
    CTK_MOUSE_MSG_NCRBUTTONUP,
    CTK_MOUSE_MSG_NCRBUTTONDBLCLK,
    CTK_MOUSE_MSG_NCMBUTTONDOWN,
    CTK_MOUSE_MSG_NCMBUTTONUP,
    CTK_MOUSE_MSG_NCMBUTTONDBLCLK,
    CTK_MOUSE_MSG_MOUSELEAVE
} CtkMouseMessage;

void ctk_mouse_manager_track_leave (HWND hwnd);

void ctk_mouse_manager_add_track (HWND hwnd,
                                  CtkMouseTrackFlags mask);

void ctk_mouse_manager_remove_track (HWND hwnd,
                                     CtkMouseTrackFlags mask);

G_END_DECLS

#endif /* __CTK_MOUSE_MANAGER_H__ */
