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
#include "ctkdecoratorbutton.h"
#include "ctkdecoratorwidgetprivate.h"
#include <cairo-win32.h>

#define SENSITIVITY 5

enum {
    INDEX_LEFTTOP = 0,
    INDEX_MIDDLETOP,
    INDEX_RIGHTTOP,
    INDEX_LEFTMIDDLE,
    INDEX_CLIENT,
    INDEX_RIGHTMIDDLE,
    INDEX_LEFTBOTTOM,
    INDEX_MIDDLEBOTTOM,
    INDEX_RIGHTBOTTOM,
    MAX_SECTION
};

G_DEFINE_TYPE (CtkWindowDecorator, ctk_window_decorator, CTK_TYPE_BASE_DECORATOR)

struct _CtkWindowDecoratorPrivate {
    GSList *tools;
    CtkDecoratorWidget *hover_widget;
    CtkDecoratorWidget *press_widget;
    CtkDecoratorWidget *maximize_button;
    WNDPROC old_proc;
    HDC border_dc;
    HBITMAP border_bmp;
    HBITMAP border_bmp_old;
    HDC tools_dc;
    HBITMAP tools_bmp;
    HBITMAP tools_bmp_old;
    cairo_surface_t *tools_surface;
    cairo_t *tools_cr;
    HDC vert_dc;
    HBITMAP vert_bmp;
    HBITMAP vert_bmp_old;
    int tools_width;
    int vert_height;
    gboolean mouse_tracking;
    HRGN hrgn;
    RECT window_rect;
    RECT client_rect;
    RECT border_rect;
    POINT client_offset;
    RECT border_slice;
    SIZE border_size;
    RECT border_section[MAX_SECTION];
};

static BOOL _ctk_window_decorator_is_dialog (HWND hWnd)
{
    DWORD dwStyle = GetWindowLong (hWnd, GWL_STYLE);
    DWORD f1 = dwStyle & WS_DLGFRAME;
    DWORD f2 = dwStyle & DS_MODALFRAME;

    if ((f1 != 0) && (f2 != 0))
        return TRUE;

    return FALSE;
}

static BOOL _ctk_window_decorator_is_sizable (HWND hWnd)
{
    DWORD dwStyle, flag;

    if (_ctk_window_decorator_is_dialog (hWnd))
        return FALSE;

    dwStyle = GetWindowLong (hWnd, GWL_STYLE);
    flag = dwStyle & WS_THICKFRAME;

    return (flag != 0);
}

static BOOL _ctk_window_decorator_is_activate (HWND hWnd)
{
    WINDOWINFO wi;

    wi.cbSize = sizeof (WINDOWINFO);
    GetWindowInfo (hWnd, &wi);

    return (wi.dwWindowStatus == WS_ACTIVECAPTION);
}

static void _ctk_window_decorator_close_clicked (CtkDecoratorButton *self,
                                                 CtkWindowDecorator *deco)
{
    HWND hwnd = _ctk_base_decorator_get_hwnd (CTK_BASE_DECORATOR (deco));

    PostMessage (hwnd, WM_CLOSE, 0, 0);
}

static void _ctk_window_decorator_maximize_clicked (CtkDecoratorButton *self,
                                                    CtkWindowDecorator *deco)
{
    HWND hwnd = _ctk_base_decorator_get_hwnd (CTK_BASE_DECORATOR (deco));

    if (IsZoomed (hwnd)) {
        PostMessage (hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
    }
    else {
        PostMessage (hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
    }
}

static void _ctk_window_decorator_minimize_clicked (CtkDecoratorButton *self,
                                                    CtkWindowDecorator *deco)
{
    HWND hwnd = _ctk_base_decorator_get_hwnd (CTK_BASE_DECORATOR (deco));

    PostMessage (hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
}

static void _ctk_window_decorator_get_style_image_size (GtkStyleContext *context,
                                                        gint *width,
                                                        gint *height)
{
    cairo_pattern_t *pattern = NULL;

    gtk_style_context_get (context, gtk_style_context_get_state (context),
                           GTK_STYLE_PROPERTY_BACKGROUND_IMAGE, &pattern,
                           NULL);
    if (pattern) {
        cairo_surface_t *surface = NULL;

        cairo_pattern_get_surface (pattern, &surface);
        if (surface) {
            if (cairo_surface_get_type (surface) == CAIRO_SURFACE_TYPE_IMAGE) {
                *width = cairo_image_surface_get_width (surface);
                *height = cairo_image_surface_get_height (surface);
            }
        }

        cairo_pattern_destroy (pattern);
    }
}

static void _ctk_window_decorator_setup_tools (CtkWindowDecorator *self)
{
    CtkWindowDecoratorPrivate *priv = self->priv;
    CtkDecoratorWidget *button;
    GtkStyleContext *context;
    gint x, y, w, h;
    GtkAllocation allocation;
    GtkBorder margin;
    GtkWidget *window = ctk_base_decorator_get_window (CTK_BASE_DECORATOR (self));
    HWND hwnd = _ctk_base_decorator_get_hwnd (CTK_BASE_DECORATOR (self));
    DWORD dwStyle = GetWindowLong (hwnd, GWL_STYLE);

    if (!(dwStyle & WS_CAPTION))
        return;

    ctk_window_decorator_get_allocation (self, &allocation);
    w = h = 22;

    /* Close */
    button = ctk_decorator_button_new (window);
    context = ctk_decorator_widget_get_style_context (button);

    if (dwStyle & (WS_MAXIMIZEBOX | WS_MINIMIZEBOX))
        gtk_style_context_add_class (context, "caption-close");
    else
        gtk_style_context_add_class (context, "caption-close-only");

    gtk_style_context_invalidate (context);
    _ctk_window_decorator_get_style_image_size (context, &w, &h);
    gtk_style_context_get_margin (context, GTK_STATE_FLAG_NORMAL, &margin);
    x = allocation.x + allocation.width - w - margin.right;
    y = allocation.y + margin.top;

    ctk_window_decorator_add_tool (self, button, x, y, w, h, 1.0, 0.0, 1.0, 0.0);

    g_signal_connect (button,
                      "clicked",
                      G_CALLBACK (_ctk_window_decorator_close_clicked),
                      self);
    g_object_unref (button);
    x -= margin.left;

    /* Maximize */
    if (dwStyle & WS_MAXIMIZEBOX) {
        button = ctk_decorator_button_new (window);
        context = ctk_decorator_widget_get_style_context (button);
        gtk_style_context_add_class (context, "caption-maximize");
        gtk_style_context_invalidate (context);

        _ctk_window_decorator_get_style_image_size (context, &w, &h);
        gtk_style_context_get_margin (context, GTK_STATE_FLAG_NORMAL, &margin);
        x -= w;
        x -= margin.right;
        y = allocation.y + margin.top;

        ctk_window_decorator_add_tool (self, button, x, y, w, h, 1.0, 0.0, 1.0, 0.0);

        g_signal_connect (button,
                          "clicked",
                          G_CALLBACK (_ctk_window_decorator_maximize_clicked),
                          self);
        priv->maximize_button = button;
        x -= margin.left;
    }

    /* Minimize */
    if (dwStyle & WS_MINIMIZEBOX) {
        button = ctk_decorator_button_new (window);
        context = ctk_decorator_widget_get_style_context (button);
        gtk_style_context_add_class (context, "caption-minimize");
        gtk_style_context_invalidate (context);

        _ctk_window_decorator_get_style_image_size (context, &w, &h);
        gtk_style_context_get_margin (context, GTK_STATE_FLAG_NORMAL, &margin);
        x -= w;
        x -= margin.right;
        y = allocation.y + margin.top;

        ctk_window_decorator_add_tool (self, button, x, y, w, h, 1.0, 0.0, 1.0, 0.0);

        g_signal_connect (button,
                          "clicked",
                          G_CALLBACK (_ctk_window_decorator_minimize_clicked),
                          self);
        g_object_unref (button);
    }
}

static void _ctk_window_decorator_calculate_section_rect (CtkWindowDecorator *self,
                                                          const RECT *r)
{
    CtkWindowDecoratorPrivate *priv = self->priv;

    priv->border_section[INDEX_LEFTTOP].left = r->left;
    priv->border_section[INDEX_LEFTTOP].top = r->top;
    priv->border_section[INDEX_LEFTTOP].right = r->left + priv->border_slice.left;
    priv->border_section[INDEX_LEFTTOP].bottom = r->top + priv->border_slice.top;

    priv->border_section[INDEX_MIDDLETOP].left = r->left + priv->border_slice.left;
    priv->border_section[INDEX_MIDDLETOP].top = r->top;
    priv->border_section[INDEX_MIDDLETOP].right = r->right - priv->border_slice.right;
    priv->border_section[INDEX_MIDDLETOP].bottom = r->top + priv->border_slice.top;

    priv->border_section[INDEX_RIGHTTOP].left = r->right - priv->border_slice.right;
    priv->border_section[INDEX_RIGHTTOP].top = r->top;
    priv->border_section[INDEX_RIGHTTOP].right = r->right;
    priv->border_section[INDEX_RIGHTTOP].bottom = r->top + priv->border_slice.top;

    priv->border_section[INDEX_LEFTMIDDLE].left = r->left;
    priv->border_section[INDEX_LEFTMIDDLE].top = r->top + priv->border_slice.top;
    priv->border_section[INDEX_LEFTMIDDLE].right = r->left + priv->border_slice.right;
    priv->border_section[INDEX_LEFTMIDDLE].bottom = r->bottom - priv->border_slice.bottom;

    priv->border_section[INDEX_CLIENT].left = r->left + priv->border_slice.left;
    priv->border_section[INDEX_CLIENT].top = r->top + priv->border_slice.top;
    priv->border_section[INDEX_CLIENT].right = r->right - priv->border_slice.right;
    priv->border_section[INDEX_CLIENT].bottom = r->bottom - priv->border_slice.bottom;

    priv->border_section[INDEX_RIGHTMIDDLE].left = r->right - priv->border_slice.right;
    priv->border_section[INDEX_RIGHTMIDDLE].top = r->top + priv->border_slice.top;
    priv->border_section[INDEX_RIGHTMIDDLE].right = r->right;
    priv->border_section[INDEX_RIGHTMIDDLE].bottom = r->bottom - priv->border_slice.bottom;

    priv->border_section[INDEX_LEFTBOTTOM].left = r->left;
    priv->border_section[INDEX_LEFTBOTTOM].top = r->bottom - priv->border_slice.bottom;
    priv->border_section[INDEX_LEFTBOTTOM].right = r->left + priv->border_slice.right;
    priv->border_section[INDEX_LEFTBOTTOM].bottom = r->bottom;

    priv->border_section[INDEX_MIDDLEBOTTOM].left = r->left + priv->border_slice.left;
    priv->border_section[INDEX_MIDDLEBOTTOM].top = r->bottom - priv->border_slice.bottom;
    priv->border_section[INDEX_MIDDLEBOTTOM].right = r->right - priv->border_slice.right;
    priv->border_section[INDEX_MIDDLEBOTTOM].bottom = r->bottom;

    priv->border_section[INDEX_RIGHTBOTTOM].left = r->right - priv->border_slice.right;
    priv->border_section[INDEX_RIGHTBOTTOM].top = r->bottom - priv->border_slice.bottom;
    priv->border_section[INDEX_RIGHTBOTTOM].right = r->right;
    priv->border_section[INDEX_RIGHTBOTTOM].bottom = r->bottom;
}

static gboolean _ctk_window_decorator_is_resized (CtkWindowDecorator *self)
{
    CtkWindowDecoratorPrivate *priv = self->priv;
    HWND hwnd = _ctk_base_decorator_get_hwnd (CTK_BASE_DECORATOR (self));
    RECT rw, rc;

    GetWindowRect (hwnd, &rw);
    GetClientRect (hwnd, &rc);

    if ((rw.right - rw.left) != (priv->window_rect.right - priv->window_rect.left) ||
        (rw.bottom - rw.top) != (priv->window_rect.bottom - priv->window_rect.top) ||
        (rc.right - rc.left) != (priv->client_rect.right - priv->client_rect.left) ||
        (rc.bottom - rc.top) != (priv->client_rect.bottom - priv->client_rect.top))
    {
        return TRUE;
    }

    return FALSE;
}

static void _ctk_window_decorator_layout_tools (CtkWindowDecorator *self)
{
    CtkWindowDecoratorPrivate *priv = self->priv;
    GSList *it;
    CtkDecoratorWidget *widget;
    RECT *init_border;
    RECT *init_rect;
    RECT *rect;
    int width = priv->border_rect.right - priv->border_rect.left;
    int height = priv->border_rect.bottom - priv->border_rect.top;
    int cx, cy;

    it = priv->tools;
    while (it) {
        widget = it->data;
        init_border = &widget->priv->init_border;
        init_rect = &widget->priv->init_rect;
        rect = &widget->priv->rect;

        cx = width - (init_border->right - init_border->left);
        cy = height - (init_border->bottom - init_border->top);
        rect->left = init_rect->left + cx * widget->priv->left;
        rect->top = init_rect->top + cy * widget->priv->top;
        rect->right = init_rect->right + cx * widget->priv->right;
        rect->bottom = init_rect->bottom + cy * widget->priv->bottom;

        cx = priv->border_rect.left - init_border->left;
        cy = priv->border_rect.top - init_border->top;
        OffsetRect (rect, cx, cy);

        it = it->next;
    }
}

static void _ctk_window_decorator_redraw_caption (HWND hwnd)
{
    RECT rect;

    GetClientRect (hwnd, &rect);
    rect.bottom = GetSystemMetrics (SM_CYCAPTION);

    RedrawWindow (hwnd, &rect, NULL, RDW_ERASE|RDW_INVALIDATE|RDW_UPDATENOW);

    SendMessage (hwnd, WM_NCPAINT, 0, 0);
}

static gboolean _ctk_window_decorator_load_border_image (CtkWindowDecorator *self,
                                                         cairo_surface_t *surface,
                                                         GtkBorder *border_slice,
                                                         HDC hdc)
{
    CtkWindowDecoratorPrivate *priv = self->priv;
    unsigned char *data;
    unsigned char *line;
    guint8 *bits = NULL;
    int width, height, stride;
    int i, j;
    cairo_format_t format;
    BITMAPINFOHEADER bmpHeader;
    HDC tmp_dc;
    HBITMAP tmp_bmp;

    if (cairo_surface_get_type (surface) != CAIRO_SURFACE_TYPE_IMAGE)
        return FALSE;

    data = cairo_image_surface_get_data (surface);
    if (NULL == data)
        return FALSE;

    format = cairo_image_surface_get_format (surface);
    if (format != CAIRO_FORMAT_ARGB32)
        return FALSE;

    width = cairo_image_surface_get_width (surface);
    height = cairo_image_surface_get_height (surface);
    stride = cairo_image_surface_get_stride (surface);

    priv->border_dc = CreateCompatibleDC (hdc);
    priv->border_bmp = CreateCompatibleBitmap (hdc, width, height);
    priv->border_bmp_old = (HBITMAP) SelectObject (priv->border_dc, priv->border_bmp);
    priv->border_slice.left = border_slice->left;
    priv->border_slice.top = border_slice->top;
    priv->border_slice.right = border_slice->right;
    priv->border_slice.bottom = border_slice->bottom;
    priv->border_size.cx = width;
    priv->border_size.cy = height;

    bmpHeader.biSize = sizeof(bmpHeader);
    bmpHeader.biBitCount = 32;
    bmpHeader.biCompression = BI_RGB;
    bmpHeader.biWidth = width;
    bmpHeader.biHeight = height;
    bmpHeader.biPlanes = 1;
    tmp_dc = CreateCompatibleDC (hdc);
    tmp_bmp = CreateDIBSection (hdc, (BITMAPINFO *) &bmpHeader,
                                DIB_RGB_COLORS, (void **) &bits, NULL, 0);
    SelectObject (tmp_dc, tmp_bmp);

    line = data + stride * height;
    for (i = 0; i < bmpHeader.biHeight; ++i) {
        line -= stride;
        data = line;

        for (j = 0; j < bmpHeader.biWidth; ++j) {
            *bits++ = *data++;
            *bits++ = *data++;
            *bits++ = *data++;
            *bits++ = 0;
            data++;
        }
    }

    BitBlt (priv->border_dc, 0, 0, width, height, tmp_dc, 0, 0, SRCCOPY);

    DeleteObject (tmp_bmp);
    DeleteDC (tmp_dc);
    return TRUE;
}

static gboolean _ctk_window_decorator_load_border_info (CtkWindowDecorator *self,
                                                        GtkWidget *window,
                                                        HWND hwnd)
{
    CtkWindowDecoratorPrivate *priv = self->priv;
    GtkStyleContext *context;
    cairo_pattern_t *border_image = NULL;
    cairo_surface_t *surface = NULL;
    GtkBorder *border_slice = NULL;
    GtkStateFlags state;
    gint width, height;
    HDC hdc = NULL;
    gboolean result = FALSE;

    g_assert (NULL == priv->border_dc && NULL == priv->border_bmp);

    context = gtk_widget_get_style_context (window);
    state = gtk_style_context_get_state (context);

    gtk_style_context_get (context,
                           gtk_style_context_get_state (context),
                           "border-image-source",
                           &border_image,
                           NULL);
    if (NULL == border_image)
        goto done;

    cairo_pattern_get_surface (border_image, &surface);
    if (NULL == surface)
        goto done;

    gtk_style_context_get (context,
                           state,
                           "border-image-slice",
                           &border_slice,
                           NULL);
    if (NULL == border_slice)
        goto done;

    width = border_slice->left + border_slice->right;
    height = border_slice->top + border_slice->bottom;
    if (width <= 0 || height <= 0)
        goto done;

    hdc = GetDC (hwnd);
    result = _ctk_window_decorator_load_border_image (self,
                                                      surface,
                                                      border_slice,
                                                      hdc);
    ReleaseDC (hwnd, hdc);

done:
    if (border_image)
        cairo_pattern_destroy (border_image);

    if (border_slice)
        gtk_border_free (border_slice);

    return result;
}

static HRGN _ctk_window_decorator_create_rgn_from_bitmap (const BITMAP *bmp,
                                                          const BITMAPINFOHEADER *bih,
                                                          LPBYTE pBits,
                                                          LPDWORD clr_tbl,
                                                          int x,
                                                          int y,
                                                          int width,
                                                          int height,
                                                          COLORREF color)
{
    int i, j;
    int first = 0;  // left position of current scan line
    BYTE Bpp = bih->biBitCount >> 3;    // bytes per pixel
    DWORD dwAlignedWidthBytes = (bmp->bmWidthBytes & ~0x3) + (!!(bmp->bmWidthBytes & 0x3) << 2);
    // DIB image is flipped that's why we scan it from the last line
    LPBYTE	pColor = pBits + (bih->biHeight - y - 1) * dwAlignedWidthBytes + x * Bpp;
    DWORD	dwLineBackLen = dwAlignedWidthBytes + width * Bpp;	// offset of previous scan line
    // where mask was found
    gboolean wasfirst = FALSE;  // set when mask has been found in current scan line
    gboolean ismask = FALSE;    // set when current color is mask color

    // allocate memory for region data
    // region data here is set of regions that are rectangles with height 1 pixel (scan line)
    // that's why first allocation is <bm.biHeight> RECTs - number of scan lines in image
    const DWORD RGNDATAHEADER_SIZE	= sizeof(RGNDATAHEADER);
    const DWORD ADD_RECTS_COUNT		= 40;			// number of rects to be appended
    DWORD	dwRectsCount = height;			// number of rects in allocated buffer
    RGNDATAHEADER* pRgnData =
            (RGNDATAHEADER*) g_malloc (RGNDATAHEADER_SIZE + dwRectsCount * sizeof(RECT));
    // get pointer to RECT table
    LPRECT pRects = (LPRECT)((LPBYTE)pRgnData + RGNDATAHEADER_SIZE);
    DWORD dwCount;
    HRGN hRgn;
    // release region data
    DWORD dwSize;
    // zero region data header memory (header  part only)
    memset (pRgnData, 0, RGNDATAHEADER_SIZE + dwRectsCount * sizeof(RECT));
    // fill it by default
    pRgnData->dwSize = RGNDATAHEADER_SIZE;
    pRgnData->iType = RDH_RECTANGLES;

    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            // get color
            switch (bih->biBitCount) {
            case 8:
                ismask = (clr_tbl[*pColor] != color);
                break;

            case 16:
                ismask = (*(LPWORD)pColor != (WORD)color);
                break;

            case 24:
                ismask = ((*(LPDWORD)pColor & 0x00ffffff) != color);
                break;

            case 32:
                ismask = (*(LPDWORD)pColor != color);
            }

            // shift pointer to next color
            pColor += Bpp;
            // place part of scan line as RECT region if transparent color found after mask color or
            // mask color found at the end of mask image
            if (wasfirst) {
                if (!ismask) {
                    // save current RECT
                    RECT rr;

                    rr.left = first;
                    rr.top = i;
                    rr.right = j;
                    rr.bottom = i+1;

                    pRects[pRgnData->nCount++] = rr;

                    // if buffer full reallocate it with more room
                    if (pRgnData->nCount >= dwRectsCount) {
                        LPBYTE pRgnDataNew;
                        dwRectsCount += ADD_RECTS_COUNT;
                        // allocate new buffer
                        pRgnDataNew = g_malloc (RGNDATAHEADER_SIZE + dwRectsCount * sizeof(RECT));
                        // copy current region data to it
                        memcpy (pRgnDataNew, pRgnData, RGNDATAHEADER_SIZE + pRgnData->nCount * sizeof(RECT));
                        // delte old region data buffer
                        g_free (pRgnData);
                        // set pointer to new regiondata buffer to current
                        pRgnData = (RGNDATAHEADER*)pRgnDataNew;
                        // correct pointer to RECT table
                        pRects = (LPRECT)((LPBYTE)pRgnData + RGNDATAHEADER_SIZE);
                    }

                    wasfirst = FALSE;
                }
            }
            else if (ismask) {
                first = j;
                wasfirst = TRUE;
            }
        }

        if (wasfirst && ismask) {
            // save current RECT
            RECT rr;

            rr.left = first;
            rr.top = i;
            rr.right = j;
            rr.bottom = i+1;

            pRects[pRgnData->nCount++] = rr;

            // if buffer full reallocate it with more room
            if (pRgnData->nCount >= dwRectsCount) {
                LPBYTE pRgnDataNew;
                dwRectsCount += ADD_RECTS_COUNT;
                // allocate new buffer
                pRgnDataNew = g_malloc (RGNDATAHEADER_SIZE + dwRectsCount * sizeof(RECT));
                // copy current region data to it
                memcpy (pRgnDataNew, pRgnData, RGNDATAHEADER_SIZE + pRgnData->nCount * sizeof(RECT));
                // delte old region data buffer
                g_free (pRgnData);
                // set pointer to new regiondata buffer to current
                pRgnData = (RGNDATAHEADER*)pRgnDataNew;
                // correct pointer to RECT table
                pRects = (LPRECT)((LPBYTE)pRgnData + RGNDATAHEADER_SIZE);
            }

            wasfirst = FALSE;
        }

        pColor -= dwLineBackLen;
    }

    // create region
    dwCount = RGNDATAHEADER_SIZE + pRgnData->nCount * sizeof(RECT);
    hRgn = ExtCreateRegion (NULL, dwCount, (LPRGNDATA)pRgnData);
    // release region data
    dwSize  = GetRegionData (hRgn, 0, NULL);
    g_free (pRgnData);

    return hRgn;
}

static HRGN _ctk_window_decorator_create_rgn (CtkWindowDecorator *self)
{
    CtkWindowDecoratorPrivate *priv = self->priv;
    // get image properties
    HBITMAP hBmp = priv->border_bmp;
    BITMAP bmp = { 0 };
    HRGN result;
    HRGN hrgn;
    LPBITMAPINFO bi;
    BITMAPINFOHEADER *bih;
    // create temporary dc
    HDC dc = CreateICW (L"DISPLAY",NULL,NULL,NULL);
    // get extended information about image (length, compression, length of color table if exist, ...)
    DWORD res;
    // allocate memory for image data (colors)
    LPBYTE pBits;
    LPDWORD clr_tbl;
    LPBYTE pClr;
    COLORREF color = RGB(255,0,255);
    RECT rect;

    GetObject (hBmp, sizeof(BITMAP), &bmp);
    // allocate memory for extended image information
    bi = (LPBITMAPINFO) g_malloc (sizeof(BITMAPINFO) + 8);
    memset (bi, 0, sizeof(BITMAPINFO) + 8);
    bi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    res = GetDIBits (dc, hBmp, 0, bmp.bmHeight, 0, bi, DIB_RGB_COLORS);
    pBits = g_malloc (bi->bmiHeader.biSizeImage + 4);

    // allocate memory for color table
    if (bi->bmiHeader.biBitCount == 8) {
        // actually color table should be appended to this header(BITMAPINFO),
        // so we have to reallocate and copy it
        LPBITMAPINFO old_bi = bi;
        // 255 - because there is one in BITMAPINFOHEADER
        bi = (LPBITMAPINFO)g_malloc (sizeof(BITMAPINFO) + 255 * sizeof(RGBQUAD));
        memcpy (bi, old_bi, sizeof(BITMAPINFO));
        // release old header
        g_free (old_bi);
    }

    // get bitmap info header
    bih = &bi->bmiHeader;
    // get color table (for 256 color mode contains 256 entries of RGBQUAD(=DWORD))
    clr_tbl = (LPDWORD)&bi->bmiColors;
    // fill bits buffer
    res = GetDIBits (dc, hBmp, 0, bih->biHeight, pBits, bi, DIB_RGB_COLORS);
    DeleteDC (dc);

    // shift bits and byte per pixel (for comparing colors)
    pClr = (LPBYTE)&color;

    //==============================================================================
    //BEGIN根据www.codeproject.com网站上的评论进行了修改，使得能够正确处理16bits的图片
    //==============================================================================
    // swap red and blue components
    // DON'T NEED TO SWAP ANY COLOR !!!!
    //BYTE tmp = pClr[0]; pClr[0] = pClr[2]; pClr[2] = tmp;
    // convert color if curent DC is 16-bit (5:6:5) or 15-bit (5:5:5)
    if (bih->biBitCount == 16) {
        pClr[0] = pClr[0] & 0x1F;
        pClr[1] = pClr[1] & 0x3F;
        pClr[2] = pClr[2] & 0x1F;

        color = (DWORD)((pClr[0]<<(5+6))|(pClr[1]<<5)|pClr[2]);
    }

    rect = priv->border_rect;
    result = CreateRectRgn (rect.left, rect.top, rect.left, rect.top);

    /* left top */
    hrgn = _ctk_window_decorator_create_rgn_from_bitmap (&bmp,
                                                         bih,
                                                         pBits,
                                                         clr_tbl,
                                                         0,
                                                         0,
                                                         priv->border_slice.left, 
                                                         priv->border_slice.top, 
                                                         color);
    OffsetRgn (hrgn, rect.left, rect.top);
    CombineRgn (result, result, hrgn, RGN_OR);
    DeleteObject (hrgn);

    /* right top */
    hrgn = _ctk_window_decorator_create_rgn_from_bitmap (&bmp,
                                                         bih,
                                                         pBits,
                                                         clr_tbl,
                                                         priv->border_size.cx - priv->border_slice.right,
                                                         0,
                                                         priv->border_slice.right, 
                                                         priv->border_slice.top, 
                                                         color);
    OffsetRgn (hrgn, rect.right - priv->border_slice.right, rect.top);
    CombineRgn (result, result, hrgn, RGN_OR);
    DeleteObject (hrgn);

    /* left bottom */
    hrgn = _ctk_window_decorator_create_rgn_from_bitmap (&bmp,
                                                         bih,
                                                         pBits,
                                                         clr_tbl,
                                                         0,
                                                         priv->border_size.cy - priv->border_slice.bottom, 
                                                         priv->border_slice.left, 
                                                         priv->border_slice.bottom, 
                                                         color);
    OffsetRgn (hrgn, rect.left, rect.bottom - priv->border_slice.bottom);
    CombineRgn (result, result, hrgn, RGN_OR);
    DeleteObject (hrgn);

    /* right bottom */
    hrgn = _ctk_window_decorator_create_rgn_from_bitmap (&bmp,
                                                         bih,
                                                         pBits,
                                                         clr_tbl,
                                                         priv->border_size.cx - priv->border_slice.right,
                                                         priv->border_size.cy - priv->border_slice.bottom, 
                                                         priv->border_slice.right, 
                                                         priv->border_slice.bottom, 
                                                         color);
    OffsetRgn (hrgn,
               rect.right - priv->border_slice.right,
               rect.bottom - priv->border_slice.bottom);
    CombineRgn (result, result, hrgn, RGN_OR);
    DeleteObject (hrgn);

    /* left */
    hrgn = CreateRectRgn (rect.left,
                          rect.top + priv->border_slice.top,
                          rect.left + priv->border_slice.left,
                          rect.bottom - priv->border_slice.bottom);
    CombineRgn (result, result, hrgn, RGN_OR);
    DeleteObject (hrgn);

    /* top */
    hrgn = CreateRectRgn (rect.left + priv->border_slice.left,
                          rect.top,
                          rect.right - priv->border_slice.right,
                          rect.top + priv->border_slice.top);
    CombineRgn (result, result, hrgn, RGN_OR);
    DeleteObject (hrgn);

    /* right */
    hrgn = CreateRectRgn (rect.right - priv->border_slice.right,
                          rect.top + priv->border_slice.top,
                          rect.right,
                          rect.bottom - priv->border_slice.bottom);
    CombineRgn (result, result, hrgn, RGN_OR);
    DeleteObject (hrgn);

    /* bottom */
    hrgn = CreateRectRgn (rect.left + priv->border_slice.left,
                          rect.bottom - priv->border_slice.bottom,
                          rect.right - priv->border_slice.right,
                          rect.bottom);
    CombineRgn (result, result, hrgn, RGN_OR);
    DeleteObject (hrgn);

    /* center */
    hrgn = CreateRectRgn (rect.left + priv->border_slice.left,
                          rect.top + priv->border_slice.top,
                          rect.right - priv->border_slice.right,
                          rect.bottom - priv->border_slice.bottom);
    CombineRgn (result, result, hrgn, RGN_OR);
    DeleteObject (hrgn);

    // release image data
    g_free (pBits);
    g_free (bi);

    return result;
}


static CtkDecoratorWidget* _ctk_window_decorator_widget_at (CtkWindowDecorator *self,
                                                            POINT pt)
{
    CtkWindowDecoratorPrivate *priv = self->priv;
    GSList *it;
    CtkDecoratorWidget *widget;

    it = priv->tools;
    while (it) {
        widget = it->data;
        if (PtInRect (&widget->priv->rect, pt))
            return widget;

        it = it->next;
    }

    return NULL;
}

static void _ctk_window_decorator_size_changed (CtkWindowDecorator *self)
{
    CtkWindowDecoratorPrivate *priv = self->priv;
    HDC hWinDC = NULL;
    HRGN hrgn;
    HWND hwnd = _ctk_base_decorator_get_hwnd (CTK_BASE_DECORATOR (self));
    int w1, h1, w2, h2, cx, cy;

    GetWindowRect (hwnd, &priv->window_rect);
    priv->border_rect = priv->window_rect;
    priv->border_rect.right -= priv->border_rect.left;
    priv->border_rect.bottom -= priv->border_rect.top;
    priv->border_rect.left = 0;
    priv->border_rect.top = 0;

    w1 = priv->window_rect.right - priv->window_rect.left;
    h1 = priv->window_rect.bottom - priv->window_rect.top;

    GetClientRect (hwnd, &priv->client_rect);
    w2 = priv->client_rect.right - priv->client_rect.left;
    h2 = priv->client_rect.bottom - priv->client_rect.top;

    cx = (w1 - w2) - (priv->border_slice.left + priv->border_slice.right);
    cy = (h1 - h2) - (priv->border_slice.top + priv->border_slice.bottom);

    priv->border_rect.left += cx / 2;
    priv->border_rect.top += cy / 2;
    priv->border_rect.right -= cx / 2 + cx % 2;
    priv->border_rect.bottom -= cy / 2 + cy % 2;

    if (priv->border_rect.left < 0)
        priv->border_rect.left = 0;

    if (priv->border_rect.top < 0)
        priv->border_rect.top = 0;

    w1 = priv->window_rect.right - priv->window_rect.left;
    h1 = priv->window_rect.bottom - priv->window_rect.top;

    if (priv->border_rect.right > w1)
        priv->border_rect.right = w1;

    if (priv->border_rect.bottom > h1)
        priv->border_rect.bottom = h1;

    _ctk_window_decorator_calculate_section_rect (self, &priv->border_rect);

    w1 = priv->border_rect.right - priv->border_rect.left;
    h1 = priv->border_rect.bottom - priv->border_rect.top;

    if (priv->tools_width < w1) {
        hWinDC = GetWindowDC (hwnd);

        if (NULL == priv->tools_dc)
            priv->tools_dc = CreateCompatibleDC (hWinDC);

        if (priv->tools_bmp_old)
            SelectObject (priv->tools_dc, priv->tools_bmp_old);

        if (priv->tools_bmp)
            DeleteObject (priv->tools_bmp);

        priv->tools_bmp = CreateCompatibleBitmap (hWinDC,
                                                  w1,
                                                  MAX (priv->border_slice.top, priv->border_slice.bottom));
        priv->tools_bmp_old = SelectObject (priv->tools_dc, priv->tools_bmp);
        priv->tools_width = w1;

        if (priv->tools_cr)
            cairo_destroy (priv->tools_cr);

        if (priv->tools_surface) {
            cairo_surface_finish (priv->tools_surface);
            cairo_surface_destroy (priv->tools_surface);
        }

        priv->tools_surface = cairo_win32_surface_create (priv->tools_dc);
        priv->tools_cr = cairo_create (priv->tools_surface);
    }

    if (priv->vert_height < h1) {
        if (NULL == hWinDC)
            hWinDC = GetWindowDC (hwnd);

        if (NULL == priv->vert_dc)
            priv->vert_dc = CreateCompatibleDC (hWinDC);

        if (priv->vert_bmp_old)
            SelectObject (priv->vert_dc, priv->vert_bmp_old);

        if (priv->vert_bmp)
            DeleteObject (priv->vert_bmp);

        priv->vert_bmp = CreateCompatibleBitmap (hWinDC,
                                                 MAX (priv->border_slice.left, priv->border_slice.right),
                                                 h1);
        priv->vert_bmp_old = SelectObject (priv->vert_dc, priv->vert_bmp);
        priv->vert_height = h1;
    }

    if (hWinDC)
        ReleaseDC (hwnd, hWinDC);

    _ctk_window_decorator_layout_tools (self);

    hrgn = _ctk_window_decorator_create_rgn (self);

    if (SetWindowRgn (hwnd, hrgn, TRUE)) {
        DeleteObject (priv->hrgn);
        priv->hrgn = hrgn;
    }
    else {
        DeleteObject (hrgn);
    }
}

struct _CtkRect {
    int x, y, width, height;
};

static void _ctk_window_decorator_on_nc_paint (CtkWindowDecorator *self,
                                               HWND hwnd,
                                               BOOL draw_tools_only)
{
    CtkWindowDecoratorPrivate *priv = self->priv;
    HWND hWnd = hwnd;
    BOOL activated;
    HDC hWinDC;
    gint x, y, width, height;
    struct _CtkRect r;
    struct _CtkRect m;
    int old_mode = -1;

    if (_ctk_window_decorator_is_resized (self))
        _ctk_window_decorator_size_changed (self);

    x = priv->border_rect.left;
    y = priv->border_rect.top;
    width = priv->border_rect.right - priv->border_rect.left;
    height = priv->border_rect.bottom - priv->border_rect.top;

    activated = _ctk_window_decorator_is_activate (hWnd);

    hWinDC = GetWindowDC (hWnd);
    r.x = 0;
    r.y = 0;
    r.width = priv->border_size.cx;
    r.height = priv->border_size.cy;
    m.x = priv->border_slice.left;
    m.y = priv->border_slice.top;
    m.width = priv->border_slice.right;
    m.height = priv->border_slice.bottom;

    old_mode = SetStretchBltMode (hWinDC, COLORONCOLOR);

    if (draw_tools_only)
        goto tools;

    /* bottom-left corner */
    BitBlt (hWinDC, x, y + height - m.height, m.x, m.height,
            priv->border_dc, r.x, r.y + r.height - m.height, SRCCOPY);

    /* bottom-right corner */
    BitBlt (hWinDC, x + width - m.width,
            y + height - m.height,
            m.width, m.height,
            priv->border_dc, r.x + r.width - m.width,
            r.y + r.height - m.height, SRCCOPY);

    /* bottom side */
    StretchBlt (priv->tools_dc, 0, 0,
                width - m.x - m.width, m.height,
                priv->border_dc, r.x + m.x, r.y + r.height - m.height,
                r.width - m.x - m.width, m.height, SRCCOPY);

    BitBlt (hWinDC, x + m.x, y + height - m.height,
            width - m.x - m.width, m.height,
            priv->tools_dc, 0, 0, SRCCOPY);

    /* left side */
    StretchBlt (priv->vert_dc, 0, 0,
                m.x, height - m.y - m.height,
                priv->border_dc, r.x, r.y + m.y,
                m.x, r.height - m.y - m.height, SRCCOPY);

    BitBlt (hWinDC, x, y + m.y,
            m.x, height - m.y - m.height,
            priv->vert_dc, 0, 0, SRCCOPY);

    /* right side */
    StretchBlt (priv->vert_dc, 0, 0,
                m.width, height - m.y - m.height,
                priv->border_dc, r.x + r.width - m.width, r.y + m.y,
                m.width, r.height - m.y - m.height, SRCCOPY);

    BitBlt (hWinDC, x + width - m.width, y + m.y,
            m.width, height - m.y - m.height,
            priv->vert_dc, 0, 0, SRCCOPY);

tools:
    /* top-left corner */
    BitBlt (priv->tools_dc, 0, 0, m.x, m.y, priv->border_dc, r.x, r.y, SRCCOPY);

    /* top-right corner */
    BitBlt (priv->tools_dc, 0 + width - m.width, 0, m.width, m.y,
            priv->border_dc, r.x + r.width - m.width, r.y, SRCCOPY);

    /* top side */
    StretchBlt (priv->tools_dc, 0 + m.x, 0,
                width - m.x - m.width, m.y,
                priv->border_dc, r.x + m.x, r.y,
                r.width - m.x - m.width, m.y, SRCCOPY);

    if (NULL == priv->tools) {
        _ctk_window_decorator_setup_tools (self);
    }

    if (priv->tools) {
        GSList *it;
        CtkDecoratorWidget *widget;
        RECT *rect;

        if (priv->maximize_button) {
            GtkStyleContext *context;

            context = ctk_decorator_widget_get_style_context (priv->maximize_button);
            if (IsZoomed (hwnd)) {
                if (!gtk_style_context_has_class (context, "caption-restore")) {
                    gtk_style_context_remove_class (context, "caption-maximize");
                    gtk_style_context_add_class (context, "caption-restore");
                    gtk_style_context_invalidate (context);
                }
            }
            else {
                if (!gtk_style_context_has_class (context, "caption-maximize")) {
                    gtk_style_context_remove_class (context, "caption-restore");
                    gtk_style_context_add_class (context, "caption-maximize");
                    gtk_style_context_invalidate (context);
                }
            }
        }

        it = priv->tools;
        while (it) {
            widget = it->data;
            rect = &widget->priv->rect;

            cairo_save (priv->tools_cr);
            cairo_translate (priv->tools_cr,
                             rect->left - priv->border_rect.left,
                             rect->top - priv->border_rect.top);

            cairo_rectangle (priv->tools_cr,
                             0, 0,
                             rect->right - rect->left,
                             rect->bottom - rect->top);
            cairo_clip (priv->tools_cr);

            if (gdk_cairo_get_clip_rectangle (priv->tools_cr, NULL)) {
                _ctk_decorator_widget_draw (widget, priv->tools_cr);
            }

            cairo_restore (priv->tools_cr);
            it = it->next;
        }

        cairo_surface_flush (priv->tools_surface);
    }

    BitBlt (hWinDC, x, y, width, priv->border_slice.top,
            priv->tools_dc, 0, 0, SRCCOPY);

    SetStretchBltMode (hWinDC, old_mode);
    ReleaseDC (hWnd, hWinDC);
}

static LRESULT _ctk_window_decorator_on_nc_activate (CtkWindowDecorator *self,
                                                     HWND hwnd,
                                                     BOOL bActive)
{
    CtkWindowDecoratorPrivate *priv = self->priv;
    LRESULT lr;

    DWORD dwStyle = GetWindowLong (hwnd, GWL_STYLE);
    SetWindowLong (hwnd, GWL_STYLE, WS_POPUP);

    lr = _ctk_base_decorator_call_old_proc (CTK_BASE_DECORATOR (self),
                                            hwnd,
                                            WM_NCACTIVATE,
                                            bActive,
                                            0);
    SetWindowLong (hwnd, GWL_STYLE, dwStyle);

    SendMessage (hwnd, WM_NCPAINT, 0, 0);
    return lr;
}

static LRESULT _ctk_window_decorator_nc_lbutton_down (CtkWindowDecorator *self,
                                                      HWND hwnd,
                                                      UINT nHitTest,
                                                      POINT point)
{
    CtkWindowDecoratorPrivate *priv = self->priv;
    CtkDecoratorWidget *widget;
    LRESULT result = 0;
    POINT pt = point;
    RECT rect;
    DWORD dwStyle;
    DWORD sysmenu;

    GetWindowRect (hwnd, &rect);
    pt.x -= rect.left;
    pt.y -= rect.top;
    widget = _ctk_window_decorator_widget_at (self, pt);
    if (widget) {
        _ctk_decorator_widget_button_press (widget,
                                            pt.x - widget->priv->rect.left,
                                            pt.y - widget->priv->rect.top);
        priv->press_widget = widget;

        _ctk_window_decorator_on_nc_paint (self, hwnd, TRUE);

        SetCapture (hwnd);
        return 0;
    }

    if (!ctk_is_win7 () && IsZoomed (hwnd))
        return 0;

    dwStyle = GetWindowLong (hwnd, GWL_STYLE);
    sysmenu = (dwStyle & WS_SYSMENU);

    SetWindowLong (hwnd, GWL_STYLE, dwStyle & (~WS_SYSMENU));

    result = _ctk_base_decorator_call_old_proc (CTK_BASE_DECORATOR (self),
                                                hwnd,
                                                WM_NCLBUTTONDOWN,
                                                nHitTest,
                                                MAKELONG(point.x, point.y));
    if (sysmenu) {
        dwStyle = GetWindowLong (hwnd, GWL_STYLE);
        SetWindowLong (hwnd, GWL_STYLE, dwStyle | WS_SYSMENU);
    }

    return result;
}

static LRESULT _ctk_window_decorator_nc_lbutton_up (CtkWindowDecorator *self,
                                                    HWND hwnd,
                                                    UINT nHitTest,
                                                    POINT point,
                                                    gboolean nc_message)
{
    CtkWindowDecoratorPrivate *priv = self->priv;
    LRESULT result = 0;

    if (priv->press_widget) {
        POINT pt = point;
        RECT rect;

        GetWindowRect (hwnd, &rect);
        pt.x -= rect.left;
        pt.y -= rect.top;

        _ctk_decorator_widget_button_release (priv->press_widget,
                                              pt.x - priv->press_widget->priv->rect.left,
                                              pt.y - priv->press_widget->priv->rect.top);
        priv->press_widget = NULL;

        _ctk_window_decorator_on_nc_paint (self, hwnd, TRUE);

        ReleaseCapture ();
        return 0;
    }

    if (nc_message) {
        result = _ctk_base_decorator_call_old_proc (CTK_BASE_DECORATOR (self),
                                                    hwnd,
                                                    WM_NCLBUTTONUP,
                                                    nHitTest,
                                                    MAKELONG(point.x, point.y));
    }

    return result;
}

static LRESULT _ctk_window_decorator_nc_lbutton_dblclk (CtkWindowDecorator *self,
                                                        HWND hwnd,
                                                        UINT nHitTest,
                                                        POINT point)
{
    CtkWindowDecoratorPrivate *priv = self->priv;
    CtkDecoratorWidget *widget;
    POINT pt = point;
    RECT rect;
    LRESULT result = 0;

    GetWindowRect (hwnd, &rect);
    pt.x -= rect.left;
    pt.y -= rect.top;

    widget = _ctk_window_decorator_widget_at (self, pt);
    if (widget) {
        _ctk_decorator_widget_button_press (widget,
                                            pt.x - widget->priv->rect.left,
                                            pt.y - widget->priv->rect.top);
        priv->press_widget = widget;

        _ctk_window_decorator_on_nc_paint (self, hwnd, TRUE);
        return 0;
    }

    result = _ctk_base_decorator_call_old_proc (CTK_BASE_DECORATOR (self),
                                                hwnd,
                                                WM_NCLBUTTONDBLCLK,
                                                nHitTest,
                                                MAKELONG(point.x, point.y));
    return result;
}

static UINT _ctk_window_decorator_nc_hit_test (CtkWindowDecorator *self,
                                               HWND hwnd,
                                               POINT point)
{
    CtkWindowDecoratorPrivate *priv = self->priv;
    RECT r;
    DWORD dwStyle = GetWindowLong (hwnd, GWL_STYLE);

    if ((dwStyle & WS_CAPTION) && _ctk_window_decorator_widget_at (self, point))
        return HTCAPTION;

    if (IsZoomed (hwnd) || !_ctk_window_decorator_is_sizable (hwnd)) {
        if (dwStyle & WS_CAPTION) {
            if (PtInRect (&priv->border_section[INDEX_MIDDLETOP], point)||
                PtInRect (&priv->border_section[INDEX_LEFTTOP], point) ||
                PtInRect (&priv->border_section[INDEX_RIGHTTOP], point))
            {
                return HTCAPTION;
            }
        }

        if (PtInRect (&priv->border_section[INDEX_CLIENT], point))
            return HTCLIENT;

        return HTBORDER;
    }

    r = priv->border_section[INDEX_LEFTTOP];
    r.bottom = r.top + SENSITIVITY;
    r.right = r.left + SENSITIVITY;
    if (PtInRect (&r, point)) {
        return HTTOPLEFT;
    }
    else if (PtInRect (&priv->border_section[INDEX_LEFTTOP], point)) {
        r = priv->border_section[INDEX_LEFTTOP];
        r.left += SENSITIVITY;

        if ((dwStyle & WS_CAPTION) && PtInRect (&r, point))
            return HTCAPTION;

        return HTLEFT;
    }

    r = priv->border_section[INDEX_MIDDLETOP];
    r.bottom = r.top + SENSITIVITY;
    if (PtInRect (&r, point)) {
        return HTTOP;
    }

    if (PtInRect (&priv->border_section[INDEX_MIDDLETOP], point)) {
        if (dwStyle & WS_CAPTION)
            return HTCAPTION;

        return HTBORDER;
    }

    r = priv->border_section[INDEX_RIGHTTOP];
    r.bottom = r.top + SENSITIVITY;
    r.left = r.right - SENSITIVITY;
    if (PtInRect (&r, point)) {
        return HTTOPRIGHT;
    }
    else if (PtInRect (&priv->border_section[INDEX_RIGHTTOP], point)) {
        r = priv->border_section[INDEX_RIGHTTOP];
        r.right -= SENSITIVITY;

        if ((dwStyle & WS_CAPTION) && PtInRect (&r, point))
            return HTCAPTION;

        return HTRIGHT;
    }

    if (PtInRect (&priv->border_section[INDEX_RIGHTMIDDLE], point))
        return HTRIGHT;

    if (PtInRect (&priv->border_section[INDEX_LEFTMIDDLE], point))
        return HTLEFT;

    r = priv->border_section[INDEX_RIGHTBOTTOM];
    r.top = r.bottom - SENSITIVITY;
    r.left = r.right - SENSITIVITY;
    if (PtInRect (&r, point))
        return HTBOTTOMRIGHT;

    if (PtInRect (&priv->border_section[INDEX_RIGHTBOTTOM], point))
        return HTBOTTOM;

    r = priv->border_section[INDEX_LEFTBOTTOM];
    r.top = r.bottom - SENSITIVITY;
    r.right = r.left + SENSITIVITY;
    if (PtInRect (&r, point))
        return HTBOTTOMLEFT;

    if (PtInRect (&priv->border_section[INDEX_LEFTBOTTOM], point))
        return HTBOTTOM;

    if (PtInRect (&priv->border_section[INDEX_MIDDLEBOTTOM], point))
        return HTBOTTOM;

    if (PtInRect (&priv->border_section[INDEX_CLIENT], point))
        return HTCLIENT;

    return HTERROR;
}

static void _ctk_window_decorator_mouse_move (CtkWindowDecorator *self,
                                              HWND hwnd,
                                              UINT nHitTest,
                                              POINT point)
{
    CtkWindowDecoratorPrivate *priv = self->priv;
    CtkDecoratorWidget *widget;
    POINT pt = point;
    RECT rect;

    GetWindowRect (hwnd, &rect);
    pt.x -= rect.left;
    pt.y -= rect.top;

    widget = _ctk_window_decorator_widget_at (self, pt);
    if (widget != priv->hover_widget) {

        if (priv->hover_widget) {
            _ctk_decorator_widget_leave_notify (priv->hover_widget);
            priv->hover_widget = NULL;
        }

        if (widget) {
            if (NULL == priv->press_widget || widget == priv->press_widget) {
                _ctk_decorator_widget_enter_notify (widget);

                priv->hover_widget = widget;
            }
        }

        if (!priv->press_widget) {
            if (priv->hover_widget && !priv->mouse_tracking) {
                TRACKMOUSEEVENT tme;

                tme.cbSize = sizeof (TRACKMOUSEEVENT);
                tme.dwFlags = TME_NONCLIENT | TME_LEAVE;
                tme.hwndTrack = hwnd;

                if (TrackMouseEvent (&tme))
                    priv->mouse_tracking = TRUE;
            }
        }

        /* Draw tool widgets */
        _ctk_window_decorator_on_nc_paint (self, hwnd, TRUE);
    }
}

static gboolean _ctk_window_decorator_attach (CtkBaseDecorator *_self)
{
    CtkWindowDecorator *self = CTK_WINDOW_DECORATOR (_self);
    CtkWindowDecoratorPrivate *priv = self->priv;
    GtkWidget *window = ctk_base_decorator_get_window (_self);
    HWND hwnd = _ctk_base_decorator_get_hwnd (_self);
    RECT rect;

    if (!_ctk_window_decorator_load_border_info (self, window, hwnd))
        return FALSE;

    priv->client_offset.x = 0;
    priv->client_offset.y = 0;
    ClientToScreen (hwnd, &priv->client_offset);
    GetWindowRect (hwnd, &rect);
    priv->client_offset.x -= rect.left;
    priv->client_offset.y -= rect.top;

    RedrawWindow (hwnd, NULL, NULL,
                  RDW_FRAME|RDW_UPDATENOW|RDW_ERASE|RDW_INVALIDATE);
    return TRUE;
}

static void _ctk_window_decorator_detach (CtkBaseDecorator *_self)
{
    CtkWindowDecorator *self = CTK_WINDOW_DECORATOR (_self);
    CtkWindowDecoratorPrivate *priv = self->priv;

    if (priv->border_dc) {
        SelectObject (priv->border_dc, priv->border_bmp_old);
        DeleteDC (priv->border_dc);
        priv->border_dc = NULL;
        priv->border_bmp_old = NULL;
    }

    if (priv->border_bmp) {
        DeleteObject (priv->border_bmp);
        priv->border_bmp = NULL;
    }

    if (priv->hrgn) {
        DeleteObject (priv->hrgn);
        priv->hrgn = NULL;
    }

    if (priv->maximize_button) {
        g_object_unref (priv->maximize_button);
        priv->maximize_button = NULL;
    }

    priv->window_rect.left = 0;
    priv->window_rect.top = 0;
    priv->window_rect.right = 0;
    priv->window_rect.bottom = 0;
    priv->client_rect = priv->window_rect;
    priv->border_rect = priv->window_rect;
    priv->client_offset.x = 0;
    priv->client_offset.y = 0;
    priv->border_slice.left = 0;
    priv->border_slice.top = 0;
    priv->border_slice.right = 0;
    priv->border_slice.bottom = 0;
    priv->border_size.cx = 0;
    priv->border_size.cy = 0;

    if (priv->tools) {
        g_slist_free_full (priv->tools, g_object_unref);
        priv->tools = NULL;
    }

    if (priv->tools_surface) {
        cairo_surface_destroy (priv->tools_surface);
        priv->tools_surface = NULL;
    }

    if (priv->tools_cr) {
        cairo_destroy (priv->tools_cr);
        priv->tools_cr = NULL;
    }

    if (priv->tools_dc) {
        SelectObject (priv->tools_dc, priv->tools_bmp_old);
        DeleteDC (priv->tools_dc);
        priv->tools_dc = NULL;
        priv->tools_bmp_old = NULL;
    }

    if (priv->tools_bmp) {
        DeleteObject (priv->tools_bmp);
        priv->tools_bmp = NULL;
    }

    if (priv->vert_dc) {
        SelectObject (priv->vert_dc, priv->vert_bmp_old);
        DeleteDC (priv->vert_dc);
        priv->vert_dc = NULL;
        priv->vert_bmp_old = NULL;
    }

    if (priv->vert_bmp) {
        DeleteObject (priv->vert_bmp);
        priv->vert_bmp = NULL;
    }
}

static gboolean _ctk_window_decorator_proc (CtkBaseDecorator *_self,
                                            HWND hwnd,
                                            UINT msg,
                                            WPARAM wparam,
                                            LPARAM lparam,
                                            LRESULT *result)
{
    CtkWindowDecorator *self = CTK_WINDOW_DECORATOR (_self);
    CtkWindowDecoratorPrivate *priv = self->priv;

    switch(msg) {
    case WM_NCLBUTTONDOWN:
        {
            POINT pt;

            pt.x = GET_X_LPARAM(lparam);
            pt.y = GET_Y_LPARAM(lparam);

            *result = _ctk_window_decorator_nc_lbutton_down (self, hwnd, wparam, pt);
            return TRUE;
        }

    case WM_NCLBUTTONUP:
        {
            POINT pt;

            pt.x = GET_X_LPARAM(lparam);
            pt.y = GET_Y_LPARAM(lparam);

            *result = _ctk_window_decorator_nc_lbutton_up (self, hwnd, wparam, pt, TRUE);
            return TRUE;
        }

    case WM_LBUTTONUP:
        if (priv->press_widget) {
            POINT pt;

            pt.x = GET_X_LPARAM(lparam);
            pt.y = GET_Y_LPARAM(lparam);
            ClientToScreen (hwnd, &pt);

            *result = _ctk_window_decorator_nc_lbutton_up (self, hwnd, 0, pt, FALSE);
            return TRUE;
        }
        break;

    case WM_NCLBUTTONDBLCLK:
        {
            POINT pt;

            pt.x = GET_X_LPARAM(lparam);
            pt.y = GET_Y_LPARAM(lparam);

            *result = _ctk_window_decorator_nc_lbutton_dblclk (self, hwnd, wparam, pt);
            return TRUE;
        }
        break;

    case WM_NCRBUTTONDOWN:
    case WM_NCRBUTTONUP:
    case WM_NCRBUTTONDBLCLK:
        {
            POINT pt;
            RECT r;

            GetWindowRect (hwnd, &r);
            pt.x = GET_X_LPARAM(lparam) - r.left;
            pt.y = GET_Y_LPARAM(lparam) - r.top;

            if (_ctk_window_decorator_widget_at (self, pt))
                return TRUE;
        }
        break;

    case WM_NCHITTEST:
        {
            POINT pt;
            RECT r;

            GetWindowRect (hwnd, &r);
            pt.x = GET_X_LPARAM(lparam) - r.left;
            pt.y = GET_Y_LPARAM(lparam) - r.top;

            *result = _ctk_window_decorator_nc_hit_test (self, hwnd, pt);
            return TRUE;
        }
        break;

    case WM_NCMOUSEMOVE:
        {
            DWORD dwStyle = GetWindowLong (hwnd, GWL_STYLE);
            POINT pt;

            SetWindowLong (hwnd, GWL_STYLE, dwStyle & (~WS_SYSMENU));

            *result = _ctk_base_decorator_call_old_proc (CTK_BASE_DECORATOR (self),
                                                         hwnd,
                                                         msg,
                                                         wparam,
                                                         lparam);
            SetWindowLong (hwnd, GWL_STYLE, dwStyle);

            pt.x = GET_X_LPARAM(lparam);
            pt.y = GET_Y_LPARAM(lparam);

            _ctk_window_decorator_mouse_move (self, hwnd, wparam, pt);
            return TRUE;
        }
        break;

    case WM_MOUSEMOVE:
        if (priv->press_widget) {
            POINT pt;

            pt.x = GET_X_LPARAM(lparam);
            pt.y = GET_Y_LPARAM(lparam);

            ClientToScreen (hwnd, &pt);

            _ctk_window_decorator_mouse_move (self, hwnd, wparam, pt);
            return TRUE;
        }
        break;

    case WM_NCMOUSELEAVE:
        if (priv->mouse_tracking) {
            if (priv->hover_widget &&
                priv->hover_widget != priv->press_widget)
            {
                _ctk_decorator_widget_leave_notify (priv->hover_widget);
                _ctk_window_decorator_on_nc_paint (self, hwnd, TRUE);
                priv->hover_widget = NULL;
            }

            priv->mouse_tracking = FALSE;
        }
        break;

    case WM_SETCURSOR:
        {
            DWORD dwStyle = GetWindowLong (hwnd , GWL_STYLE);

            SetWindowLong (hwnd, GWL_STYLE, dwStyle & (~WS_SYSMENU));

            *result = _ctk_base_decorator_call_old_proc (CTK_BASE_DECORATOR (self),
                                                         hwnd,
                                                         msg,
                                                         wparam,
                                                         lparam);
            SetWindowLong (hwnd, GWL_STYLE, dwStyle);
            return TRUE;
        }
        break;

    case WM_NCCALCSIZE:
        if (!IsIconic (hwnd)) {
            NCCALCSIZE_PARAMS *lpncsp = (NCCALCSIZE_PARAMS *) lparam;

            /* NOTE: There is a problem with Win7 when the client area of
             *       a resiable window is bigger then calculated by GTK proc,
             *       the window will expand forever. */
            if (_ctk_window_decorator_is_sizable (hwnd)) {
                int dx, dy;

                *result = _ctk_base_decorator_call_old_proc (CTK_BASE_DECORATOR (self),
                                                             hwnd,
                                                             msg,
                                                             wparam,
                                                             lparam);
                dx = priv->client_offset.x - priv->border_slice.left;
                dy = priv->client_offset.y - priv->border_slice.top;

                OffsetRect (&lpncsp->rgrc[0],
                            priv->border_rect.left - dx,
                            priv->border_rect.top - dy);
            }
            else {
                lpncsp->rgrc[0].left += priv->border_slice.left;
                lpncsp->rgrc[0].top += priv->border_slice.top;
                lpncsp->rgrc[0].right -= priv->border_slice.right;
                lpncsp->rgrc[0].bottom -= priv->border_slice.bottom;
            }

            return TRUE;
        }
        break;

    case WM_GETMINMAXINFO:
        {
            MINMAXINFO *lpMMI = (MINMAXINFO *) lparam;
            int width, height;

            width = priv->border_slice.left + priv->border_slice.right;
            height = priv->border_slice.top + priv->border_slice.bottom;

            _ctk_base_decorator_call_old_proc (CTK_BASE_DECORATOR (self),
                                               hwnd,
                                               msg,
                                               wparam,
                                               lparam);

            if (lpMMI->ptMinTrackSize.x < width)
                lpMMI->ptMinTrackSize.x = width;

            if (lpMMI->ptMinTrackSize.y < height)
                lpMMI->ptMinTrackSize.y = height;
        }
        return TRUE;

    case WM_NCPAINT:
        if (IsIconic (hwnd))
            return FALSE;

        _ctk_window_decorator_on_nc_paint (self, hwnd, FALSE);
        return TRUE;

    case WM_NCACTIVATE:
        *result = _ctk_window_decorator_on_nc_activate (self, hwnd, wparam);
        return TRUE;

    case WM_INITMENU:
    case WM_INITMENUPOPUP:
        {
            DWORD dwStyle = GetWindowLong (hwnd, GWL_STYLE);
            DWORD magicStyle = (WS_POPUP|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS);

            SetWindowLong (hwnd, GWL_STYLE, magicStyle);

            *result = _ctk_base_decorator_call_old_proc (CTK_BASE_DECORATOR (self),
                                                         hwnd,
                                                         msg,
                                                         wparam,
                                                         lparam);
            SetWindowLong (hwnd, GWL_STYLE, dwStyle);

            _ctk_window_decorator_redraw_caption (hwnd);
            return TRUE;
        }
        break;

    default:
        break;
    }

    return FALSE;
}

static void ctk_window_decorator_init (CtkWindowDecorator *self)
{
    CtkWindowDecoratorPrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              CTK_TYPE_WINDOW_DECORATOR,
                                              CtkWindowDecoratorPrivate);
    priv = self->priv;

    priv->tools = NULL;
    priv->hover_widget = NULL;
    priv->press_widget = NULL;
    priv->maximize_button = NULL;
    priv->mouse_tracking = FALSE;
    priv->border_dc = NULL;
    priv->border_bmp = NULL;
    priv->border_bmp_old = NULL;
    priv->tools_dc = NULL;
    priv->tools_bmp = NULL;
    priv->tools_bmp_old = NULL;
    priv->tools_cr = NULL;
    priv->tools_surface = NULL;
    priv->tools_width = 0;
    priv->vert_dc = NULL;
    priv->vert_bmp = NULL;
    priv->vert_bmp_old = NULL;
    priv->vert_height = 0;
    priv->hrgn = NULL;
    priv->window_rect.left = 0;
    priv->window_rect.top = 0;
    priv->window_rect.right = 0;
    priv->window_rect.bottom = 0;
    priv->client_rect = priv->window_rect;
    priv->border_rect = priv->window_rect;
    priv->client_offset.x = 0;
    priv->client_offset.y = 0;
    priv->border_slice.left = 0;
    priv->border_slice.top = 0;
    priv->border_slice.right = 0;
    priv->border_slice.bottom = 0;
    priv->border_size.cx = 0;
    priv->border_size.cy = 0;
}

static void ctk_window_decorator_finalize (GObject *gobject)
{
    G_OBJECT_CLASS (ctk_window_decorator_parent_class)->finalize (gobject);
}

static void ctk_window_decorator_class_init (CtkWindowDecoratorClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    CtkBaseDecoratorClass *decorator_class = CTK_BASE_DECORATOR_CLASS (klass);

    gobject_class->finalize = ctk_window_decorator_finalize;

    decorator_class->attach = _ctk_window_decorator_attach;
    decorator_class->detach = _ctk_window_decorator_detach;
    decorator_class->proc = _ctk_window_decorator_proc;

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
    CtkWindowDecoratorPrivate *priv;
    CtkDecoratorWidgetPrivate *toolp;
    GtkAllocation allocation;

    g_return_if_fail (CTK_IS_WINDOW_DECORATOR (self));
    g_return_if_fail (CTK_IS_DECORATOR_WIDGET (tool));

    priv = self->priv;
    toolp = tool->priv;

    ctk_window_decorator_get_allocation (self, &allocation);

    toolp->init_border.left = allocation.x;
    toolp->init_border.top = allocation.y;
    toolp->init_border.right = allocation.x + allocation.width;
    toolp->init_border.bottom = allocation.y + allocation.height;
    toolp->init_rect.left = x;
    toolp->init_rect.top = y;
    toolp->init_rect.right = x + width;
    toolp->init_rect.bottom = y + height;
    toolp->rect = toolp->init_rect;
    toolp->left = left;
    toolp->top = top;
    toolp->right = right;
    toolp->bottom = bottom;

    priv->tools = g_slist_prepend (priv->tools,
                                   g_object_ref (tool));
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
    CtkWindowDecoratorPrivate *priv;

    g_return_if_fail (CTK_IS_WINDOW_DECORATOR (self));

    priv = self->priv;

    if (_ctk_window_decorator_is_resized (self))
        _ctk_window_decorator_size_changed (self);

    allocation->x = priv->border_rect.left;
    allocation->y = priv->border_rect.top;
    allocation->width = priv->border_rect.right - priv->border_rect.left;
    allocation->height = priv->border_rect.bottom - priv->border_rect.top;
}
