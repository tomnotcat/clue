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
#include "ctkmousemanager.h"

#define TID_MOUSELEAVE 113344

struct _CTKMouseManager {
    HHOOK hook_mouse;
    HWND hwnd_leave;
    GSList *items;
};

struct _TrackItem {
    HWND hwnd;
    CtkMouseTrackFlags mask;
};

static struct _CTKMouseManager mouse_manager;

static struct _TrackItem* _ctk_get_mouse_item (HWND hwnd,
                                               gboolean create)
{
    GSList *it = mouse_manager.items;
    struct _TrackItem *item;

    while (it) {
        item = it->data;
        if (item->hwnd == hwnd)
            return item;

        it = it->next;
    }

    if (create) {
        item = g_malloc (sizeof *item);
        item->hwnd = hwnd;
        item->mask = 0;
        mouse_manager.items = g_slist_prepend (mouse_manager.items, item);
    }
    else {
        item = NULL;
    }

    return item;
}

static void _ctk_destroy_mouse_item (struct _TrackItem *item)
{
    g_free (item);
}

static HWND _ctk_get_parent_owner (HWND hWnd)
{
    return (GetWindowLong (hWnd, GWL_STYLE) & WS_CHILD) ?
        GetParent (hWnd) : GetWindow (hWnd, GW_OWNER);
}

static gboolean _ctk_is_top_parent_active (HWND hWnd)
{
    HWND hwndForeground = GetForegroundWindow ();
	HWND hWndT;
    HWND hwndActivePopup;

	while ((hWndT = _ctk_get_parent_owner (hWnd)) != NULL)
		hWnd = hWndT;

	hwndActivePopup = GetLastActivePopup (hWnd);
	if (hwndForeground == hwndActivePopup)
		return TRUE;

	return FALSE;
}

static void CALLBACK _ctk_track_mouse_timer_proc (HWND hWnd, 
                                                  UINT uMsg,
                                                  UINT idEvent,
                                                  DWORD dwTime)
{
    RECT rect;
    POINT pt;
    BOOL top_parent_active;

    if (!IsWindow (hWnd)) {
        KillTimer (hWnd, idEvent);
        mouse_manager.hwnd_leave = NULL;
        return;
    }

    GetWindowRect (hWnd, &rect);
    GetCursorPos (&pt);

    top_parent_active = GetParent (hWnd) == 0 || 
                        (GetWindowLong (hWnd, GWL_STYLE) & WS_POPUP) || 
                        _ctk_is_top_parent_active (hWnd);

    if (!PtInRect (&rect, pt) || !top_parent_active) {
        KillTimer (hWnd, idEvent);
        mouse_manager.hwnd_leave = NULL;

        PostMessage (hWnd, CTK_MOUSE_MSG_MOUSELEAVE, 0, 0);
    }
}

static LRESULT CALLBACK _ctk_mouse_hook_proc (int nCode, 
                                              WPARAM wParam,
                                              LPARAM lParam)
{
    PMOUSEHOOKSTRUCT pHook = (PMOUSEHOOKSTRUCT) lParam;
    POINT point;
    GSList *it, *next;
    guint mask;
    guint msg;
    struct _TrackItem *item;

	if (nCode != HC_ACTION)
		return CallNextHookEx (mouse_manager.hook_mouse, nCode, wParam, lParam);

	point = pHook->pt;

    if (wParam >= WM_MOUSEFIRST) {
        mask = 1 << (wParam - WM_MOUSEFIRST);
        msg = (CTK_MOUSE_MSG_MOUSEMOVE) + (wParam - WM_MOUSEFIRST);
    }
    else {
        mask = 1 << (10 + wParam - WM_NCMOUSEMOVE);
        msg = (CTK_MOUSE_MSG_NCMOUSEMOVE) + (wParam - WM_NCMOUSEMOVE);
    }

    it = mouse_manager.items;
    while (it) {
        next = it->next;
        item = it->data;
        if (item->mask & mask) {
            if (IsWindow (item->hwnd)) {
                PostMessage (item->hwnd,
                             msg,
                             pHook->wHitTestCode,
                             MAKELONG(pHook->pt.x, pHook->pt.y));
            }
            else {
                _ctk_destroy_mouse_item (item);
                mouse_manager.items = g_slist_delete_link (mouse_manager.items, it);

                if (NULL == mouse_manager.items && mouse_manager.hook_mouse) {
                    UnhookWindowsHookEx (mouse_manager.hook_mouse);
                    mouse_manager.hook_mouse = NULL;
                }
            }
        }

        it = next;
    }

	return CallNextHookEx (mouse_manager.hook_mouse, nCode, wParam, lParam);
}

void ctk_mouse_manager_track_leave (HWND hwnd)
{
    if (mouse_manager.hwnd_leave == hwnd)
        return;

	if (mouse_manager.hwnd_leave && 
        hwnd != mouse_manager.hwnd_leave && 
        IsWindow (mouse_manager.hwnd_leave))
    {
		SendMessage (mouse_manager.hwnd_leave, CTK_MOUSE_MSG_MOUSELEAVE, 0, 0);
    }

	mouse_manager.hwnd_leave = hwnd;

	SetTimer (hwnd, TID_MOUSELEAVE, 100, 
              (TIMERPROC) _ctk_track_mouse_timer_proc);
}

void ctk_mouse_manager_add_track (HWND hwnd,
                                  CtkMouseTrackFlags mask)
{
    struct _TrackItem *item;

    item = _ctk_get_mouse_item (hwnd, TRUE);
    item->mask |= mask;

    if (item->mask && NULL == mouse_manager.hook_mouse) {
        mouse_manager.hook_mouse = SetWindowsHookEx (WH_MOUSE,
                                                     _ctk_mouse_hook_proc, 
                                                     0,
                                                     GetCurrentThreadId ());
    }
}

void ctk_mouse_manager_remove_track (HWND hwnd,
                                     CtkMouseTrackFlags mask)
{
    struct _TrackItem *item;

    item = _ctk_get_mouse_item (hwnd, FALSE);
    if (NULL == item)
        return;

    item->mask &= ~mask;
    if (0 == item->mask) {
        mouse_manager.items = g_slist_remove (mouse_manager.items, item);
        _ctk_destroy_mouse_item (item);

        if (NULL == mouse_manager.items && mouse_manager.hook_mouse) {
            UnhookWindowsHookEx (mouse_manager.hook_mouse);
            mouse_manager.hook_mouse = NULL;
        }
    }
}
