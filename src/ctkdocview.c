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
#include "ctkdocview.h"
#include "ctkdocmodel.h"
#include "ctkdocpage.h"
#include "ctkdocrendercache.h"
#include "ctkdocument.h"

#define HEIGHT_CACHE_KEY "ctk-doc-view-height-cache"
#define ZOOM_IN_FACTOR  1.2
#define ZOOM_OUT_FACTOR (1.0/ZOOM_IN_FACTOR)

G_DEFINE_TYPE_WITH_CODE (CtkDocView, ctk_doc_view, GTK_TYPE_CONTAINER,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_SCROLLABLE, NULL))

enum {
    PROP_0,
    PROP_HADJUSTMENT,
    PROP_VADJUSTMENT,
    PROP_HSCROLL_POLICY,
    PROP_VSCROLL_POLICY,
    PROP_THREADPOOL
};

typedef enum {
    SCROLL_TO_KEEP_POSITION,
    SCROLL_TO_PAGE_POSITION,
    SCROLL_TO_CENTER,
    SCROLL_TO_FIND_LOCATION
} PendingScroll;

typedef struct _CtkHeightCache {
    gint rotation;
    gboolean dual_even_left;
    gdouble *height_to_page;
    gdouble *dual_height_to_page;
} CtkHeightCache;

struct _CtkDocViewPrivate {
    CtkDocModel *model;
    CtkDocument *document;
    /* Cache the height of pages layout */
    CtkHeightCache *height_cache;
    OrenThreadPool *thread_pool;
    CtkDocRenderCache *render_cache;
    guint render_cache_size;

    /* Visible pages */
    gint begin_page;
    gint end_page;
    gint cur_page;
    GtkRequisition requisition;
    gboolean internal_size_request;

    /* Find */
    gboolean jump_to_find_result;
    gboolean highlight_find_results;

    /* Scrolling */
    GtkAdjustment *hadjustment;
    GtkAdjustment *vadjustment;

    /* GtkScrollablePolicy needs to be checked when
     * driving the scrollable adjustment values */
    guint hscroll_policy : 2;
    guint vscroll_policy : 2;
    gdouble scroll_x;
    gdouble scroll_y;

    PendingScroll pending_scroll;
    gboolean pending_resize;
    gdouble pending_x;
    gdouble pending_y;

    gint rotation;
    gdouble scale;
    gdouble spacing;

    gboolean continuous;
    gboolean dual_page;
    gboolean dual_even_left;
    CtkSizingMode sizing_mode;
};

static void ctk_doc_view_height_cache_free (CtkHeightCache *cache)
{
    if (cache->height_to_page) {
        g_free (cache->height_to_page);
        cache->height_to_page = NULL;
    }

    if (cache->dual_height_to_page) {
        g_free (cache->dual_height_to_page);
        cache->dual_height_to_page = NULL;
    }

    g_free (cache);
}

static void ctk_doc_view_build_height_cache (CtkDocView *self,
                                             CtkHeightCache *cache)
{
    CtkDocViewPrivate *priv = self->priv;
    CtkDocPage *page;
    gboolean swap, uniform;
    gint page_count, i;
    gdouble uniform_height, page_height, next_page_height;
    gdouble saved_height;
    gdouble u_width, u_height;

    swap = (priv->rotation == 90 || priv->rotation == 270);

    uniform = ctk_document_get_uniform_page_size (priv->document, &u_width, &u_height);
    page_count = ctk_document_count_pages (priv->document);

    g_free (cache->height_to_page);
    g_free (cache->dual_height_to_page);

    cache->rotation = priv->rotation;
    cache->dual_even_left = priv->dual_even_left;
    cache->height_to_page = g_new0 (gdouble, page_count + 1);
    cache->dual_height_to_page = g_new0 (gdouble, page_count + 2);

    saved_height = 0;
    for (i = 0; i <= page_count; ++i) {
        if (uniform) {
            uniform_height = swap ? u_width : u_height;
            cache->height_to_page[i] = i * uniform_height;
        }
        else {
            if (i < page_count) {
                gdouble w, h;

                page = ctk_document_get_page (priv->document, i);
                ctk_doc_page_get_size (page, &w, &h);
                page_height = swap ? w : h;
            }
            else {
                page_height = 0;
            }

            cache->height_to_page[i] = saved_height;
            saved_height += page_height;
        }
    }

    if (cache->dual_even_left && !uniform) {
        gdouble w, h;

        page = ctk_document_get_page (priv->document, 0);
        ctk_doc_page_get_size (page, &w, &h);
        saved_height = swap ? w : h;
    }
    else {
        saved_height = 0;
    }

    for (i = cache->dual_even_left; i < page_count + 2; i += 2) {
        if (uniform) {
            uniform_height = swap ? u_width : u_height;
            cache->dual_height_to_page[i] = ((i + cache->dual_even_left) / 2) * uniform_height;
            if (i + 1 < page_count + 2)
                cache->dual_height_to_page[i + 1] = ((i + cache->dual_even_left) / 2) * uniform_height;
        }
        else {
            if (i + 1 < page_count) {
                gdouble w, h;

                page = ctk_document_get_page (priv->document, i + 1);
                ctk_doc_page_get_size (page, &w, &h);
                next_page_height = swap ? w : h;
            }
            else {
                next_page_height = 0;
            }

            if (i < page_count) {
                gdouble w, h;

                page = ctk_document_get_page (priv->document, i);
                ctk_doc_page_get_size (page, &w, &h);
                page_height = swap ? w : h;
            }
            else {
                page_height = 0;
            }

            if (i + 1 < page_count + 2) {
                cache->dual_height_to_page[i] = saved_height;
                cache->dual_height_to_page[i + 1] = saved_height;
                saved_height += MAX(page_height, next_page_height);
            }
            else {
                cache->dual_height_to_page[i] = saved_height;
            }
        }
    }
}

static CtkHeightCache* ctk_doc_view_get_height_cache (CtkDocView *self)
{
    CtkDocViewPrivate *priv = self->priv;
    CtkHeightCache *cache;

    if (!priv->document)
        return NULL;

    cache = g_object_get_data (G_OBJECT (priv->document), HEIGHT_CACHE_KEY);
    if (!cache) {
        cache = g_new0 (CtkHeightCache, 1);
        ctk_doc_view_build_height_cache (self, cache);
        g_object_set_data_full (G_OBJECT (priv->document),
                                HEIGHT_CACHE_KEY,
                                cache,
                                (GDestroyNotify) ctk_doc_view_height_cache_free);
    }

    return cache;
}

static void _render_cache_task_finished (CtkDocRenderCache *cache,
                                         gint page,
                                         CtkDocView *view)
{
    gtk_widget_queue_draw (GTK_WIDGET (view));
}

static void ctk_doc_view_setup_caches (CtkDocView *self)
{
    CtkDocViewPrivate *priv = self->priv;

    priv->height_cache = ctk_doc_view_get_height_cache (self);

    priv->render_cache = ctk_doc_render_cache_new (priv->model,
                                                   priv->thread_pool,
                                                   0);

    ctk_doc_render_cache_set_max_size (priv->render_cache,
                                       priv->render_cache_size);

    g_signal_connect (priv->render_cache,
                      "task-finished",
                      G_CALLBACK (_render_cache_task_finished),
                      self);
}

static void ctk_doc_view_clear_caches (CtkDocView *self)
{
    CtkDocViewPrivate *priv = self->priv;

    if (priv->render_cache) {
        g_object_unref (priv->render_cache);
        priv->render_cache = NULL;
    }
}

static void ctk_doc_view_get_max_page_size (CtkDocView *self,
                                            gdouble *max_width,
                                            gdouble *max_height)
{
    CtkDocViewPrivate *priv = self->priv;
    gdouble width, height;

    ctk_document_get_max_page_size (priv->document, &width, &height);

    width *= priv->scale;
    height *= priv->scale;

    if (max_width)
        *max_width = (priv->rotation % 180) ? height : width;

    if (max_height)
        *max_height = (priv->rotation % 180) ? width : height;
}

static void ctk_doc_view_compute_border (CtkDocView *self,
                                         gdouble width,
                                         gdouble height,
                                         GtkBorder *border)
{
    border->left = 1;
    border->top = 1;

    if (width < 100) {
        border->right = 2;
        border->bottom = 2;
    }
    else if (width < 500) {
        border->right = 3;
        border->bottom = 3;
    }
    else {
        border->right = 4;
        border->bottom = 4;
    }
}

static void ctk_doc_view_get_height_to_page (CtkDocView *self,
                                             gint page,
                                             gdouble *height,
                                             gdouble *dual_height)
{
    CtkDocViewPrivate *priv = self->priv;
    CtkHeightCache *cache;

    if (NULL == priv->height_cache)
        return;

    cache = priv->height_cache;
    if (cache->rotation != priv->rotation ||
        cache->dual_even_left != priv->dual_even_left)
    {
        ctk_doc_view_build_height_cache (self, cache);
    }

    if (height)
        *height = cache->height_to_page[page] * priv->scale;

    if (dual_height)
        *dual_height = cache->dual_height_to_page[page] * priv->scale;
}

static void ctk_doc_view_get_page_y_offset (CtkDocView *self,
                                            int page,
                                            gdouble *offset)
{
    CtkDocViewPrivate *priv = self->priv;
    gdouble temp;
    GtkBorder border;

    ctk_doc_view_get_max_page_size (self, &temp, NULL);
    ctk_doc_view_compute_border (self, temp, temp, &border);

    if (priv->dual_page) {
        ctk_doc_view_get_height_to_page (self, page, NULL, offset);
        temp = ((page + priv->dual_even_left) / 2 + 1) * priv->spacing +
               ((page + priv->dual_even_left) / 2 ) * (border.top + border.bottom);
    }
    else {
        ctk_doc_view_get_height_to_page (self, page, offset, NULL);
        temp = (page + 1) * priv->spacing + page * (border.top + border.bottom);
    }

    *offset += temp;
}

void ctk_get_page_size_for_scale_and_rotation (CtkDocument *document,
                                               gint index,
                                               gdouble scale,
                                               gint rotation,
                                               gint *page_width,
                                               gint *page_height)
{
    CtkDocPage *page;
    gdouble width, height;

    page = ctk_document_get_page (document, index);
    g_assert (page);

    ctk_doc_page_get_size (page, &width, &height);
    width *= scale;
    height *= scale;

    if (rotation == 0 || rotation == 180) {
        if (page_width)
            *page_width = width + 0.5;

        if (page_height)
            *page_height = height + 0.5;
    }
    else {
        if (page_width)
            *page_width = height + 0.5;

        if (page_height)
            *page_height = width + 0.5;
    }
}

static void ctk_doc_view_get_view_page_size (CtkDocView *self,
                                             gint index,
                                             gint *page_width,
                                             gint *page_height)
{
    CtkDocViewPrivate *priv = self->priv;

    ctk_get_page_size_for_scale_and_rotation (priv->document,
                                              index,
                                              priv->scale,
                                              priv->rotation,
                                              page_width,
                                              page_height);
}

static void ctk_doc_view_get_doc_page_size (CtkDocView *self,
                                            gint index,
                                            gint *page_width,
                                            gint *page_height)
{
    CtkDocViewPrivate *priv = self->priv;

    ctk_get_page_size_for_scale_and_rotation (priv->document,
                                              index,
                                              1.0,
                                              priv->rotation,
                                              page_width,
                                              page_height);
}

static void ctk_doc_view_get_page_extents (CtkDocView *self,
                                           gint page,
                                           GdkRectangle *page_area,
                                           GtkBorder *border)
{
    CtkDocViewPrivate *priv = self->priv;
    GtkWidget *widget = GTK_WIDGET (self);
    gint width, height;
    GtkAllocation allocation;

    gtk_widget_get_allocation (widget, &allocation);

    /* Get the size of the page */
    ctk_doc_view_get_view_page_size (self, page, &width, &height);
    ctk_doc_view_compute_border (self, width, height, border);
    page_area->width = width + border->left + border->right;
    page_area->height = height + border->top + border->bottom;

    if (priv->continuous) {
        gdouble max_width;
        gdouble x, y;

        ctk_doc_view_get_max_page_size (self, &max_width, NULL);
        max_width = max_width + border->left + border->right;

        /* Get the location of the bounding box */
        if (priv->dual_page) {
            x = priv->spacing + ((page % 2 == priv->dual_even_left) ? 0 : 1) * (max_width + priv->spacing);
            x = x + MAX (0, allocation.width - (max_width * 2 + priv->spacing * 3)) / 2;
            if (page % 2 == priv->dual_even_left)
                x = x + (max_width - width - border->left - border->right);
        }
        else {
            x = priv->spacing;
            x = x + MAX (0, allocation.width - (width + priv->spacing * 2)) / 2;
        }

        ctk_doc_view_get_page_y_offset (self, page, &y);

        page_area->x = x;
        page_area->y = y;
    }
    else {
        gint x, y;

        if (priv->dual_page) {
            gint width_2, height_2;
            gint max_width = width;
            gint max_height = height;
            GtkBorder overall_border;
            gint other_page;

            other_page = (page % 2 == priv->dual_even_left) ? page + 1: page - 1;

            /* First, we get the bounding box of the two pages */
            if (other_page < ctk_document_count_pages (priv->document)
                && (0 <= other_page))
            {
                ctk_doc_view_get_view_page_size (self, other_page,
                                                 &width_2, &height_2);
                if (width_2 > width)
                    max_width = width_2;

                if (height_2 > height)
                    max_height = height_2;
            }

            ctk_doc_view_compute_border (self, max_width, max_height, &overall_border);

            /* Find the offsets */
            x = priv->spacing;
            y = priv->spacing;

            /* Adjust for being the left or right page */
            if (page % 2 == priv->dual_even_left)
                x = x + max_width - width;
            else
                x = x + (max_width + overall_border.left + overall_border.right) + priv->spacing;

            y = y + (max_height - height)/2;

            /* Adjust for extra allocation */
            x = x + MAX (0, allocation.width -
                         ((max_width + overall_border.left + overall_border.right) * 2 + priv->spacing * 3))/2;
            y = y + MAX (0, allocation.height - (height + priv->spacing * 2))/2;
        }
        else {
            x = priv->spacing;
            y = priv->spacing;

            /* Adjust for extra allocation */
            x = x + MAX (0, allocation.width - (width + border->left + border->right + priv->spacing * 2)) / 2;
            y = y + MAX (0, allocation.height - (height + border->top + border->bottom + priv->spacing * 2)) / 2;
        }

        page_area->x = x;
        page_area->y = y;
    }
}

static gint ctk_doc_view_get_scrollbar_size (CtkDocView *self,
                                             GtkOrientation orientation)
{
    CtkDocViewPrivate *priv = self->priv;
    GtkWidget *widget = GTK_WIDGET (self);
    GtkWidget *swindow = gtk_widget_get_parent (widget);
    GtkWidget *sb;
    GtkAllocation allocation;
    GtkRequisition req;
    gint spacing;

    if (!GTK_IS_SCROLLED_WINDOW (swindow))
        return 0;

    gtk_widget_get_allocation (widget, &allocation);

    if (orientation == GTK_ORIENTATION_VERTICAL) {
        if (allocation.height >= priv->requisition.height)
            sb = gtk_scrolled_window_get_vscrollbar (GTK_SCROLLED_WINDOW (swindow));
        else
            return 0;
    }
    else {
        if (allocation.width >= priv->requisition.width)
            sb = gtk_scrolled_window_get_hscrollbar (GTK_SCROLLED_WINDOW (swindow));
        else
            return 0;
    }

    gtk_widget_style_get (swindow, "scrollbar_spacing", &spacing, NULL);
    gtk_widget_get_preferred_size (sb, &req, NULL);

    return (orientation == GTK_ORIENTATION_VERTICAL ? req.width : req.height) + spacing;
}

static void ctk_doc_point_to_view_point (CtkDocView *self,
                                         gint page,
                                         gdouble doc_x,
                                         gdouble doc_y,
                                         GdkPoint *view_point)
{
    CtkDocViewPrivate *priv = self->priv;
    GdkRectangle page_area;
    GtkBorder border;
    gint x, y, view_x, view_y;
    gint width, height;

    ctk_doc_view_get_doc_page_size (self, page, &width, &height);
    if (priv->rotation == 0) {
        x = doc_x;
        y = doc_y;
    }
    else if (priv->rotation == 90) {
        x = width - doc_y;
        y = doc_x;
    }
    else if (priv->rotation == 180) {
        x = width - doc_x;
        y = height - doc_y;
    } else if (priv->rotation == 270) {
        x = doc_y;
        y = height - doc_x;
    }
    else {
        g_assert_not_reached ();
    }

    ctk_doc_view_get_page_extents (self, page, &page_area, &border);

    view_x = CLAMP (x * priv->scale, 0, page_area.width);
    view_y = CLAMP (y * priv->scale, 0, page_area.height);
    view_point->x = view_x + page_area.x;
    view_point->y = view_y + page_area.y;
}

static gdouble ctk_doc_view_zoom_for_size_fit_width (gdouble doc_width,
                                                     gdouble doc_height,
                                                     gdouble target_width,
                                                     gdouble target_height)
{
    return target_width / doc_width;
}

static gdouble ctk_doc_view_zoom_for_size_best_fit (gdouble doc_width,
                                                    gdouble doc_height,
                                                    gdouble target_width,
                                                    gdouble target_height)
{
    gdouble w_scale;
    gdouble h_scale;

    w_scale = target_width / doc_width;
    h_scale = target_height / doc_height;

    return MIN (w_scale, h_scale);
}

static void ctk_doc_view_zoom_for_size_continuous_and_dual_page (CtkDocView *self,
                                                                 gdouble width,
                                                                 gdouble height)
{
    CtkDocViewPrivate *priv = self->priv;
    gdouble doc_width, doc_height;
    GtkBorder border;
    gdouble scale;
    gint sb_size;

    ctk_document_get_max_page_size (priv->document, &doc_width, &doc_height);
    if (priv->rotation == 90 || priv->rotation == 270) {
        gdouble tmp;

        tmp = doc_width;
        doc_width = doc_height;
        doc_height = tmp;
    }

    ctk_doc_view_compute_border (self, doc_width, doc_height, &border);

    doc_width *= 2;
    width -= (2 * (border.left + border.right) + 3 * priv->spacing);
    height -= (border.top + border.bottom + 2 * priv->spacing - 1);

    sb_size = ctk_doc_view_get_scrollbar_size (self, GTK_ORIENTATION_VERTICAL);

    if (priv->sizing_mode == CTK_SIZING_FIT_WIDTH) {
        scale = ctk_doc_view_zoom_for_size_fit_width (doc_width,
                                                      doc_height,
                                                      width - sb_size,
                                                      height);
    }
    else if (priv->sizing_mode == CTK_SIZING_BEST_FIT) {
        scale = ctk_doc_view_zoom_for_size_best_fit (doc_width,
                                                     doc_height,
                                                     width - sb_size,
                                                     height);
    }
    else {
        g_assert_not_reached ();
    }

    ctk_doc_model_set_scale (priv->model, scale);
}

static void ctk_doc_view_zoom_for_size_continuous (CtkDocView *self,
                                                   gdouble width,
                                                   gdouble height)
{
    CtkDocViewPrivate *priv = self->priv;
    gdouble doc_width, doc_height;
    GtkBorder border;
    gdouble scale;
    gint sb_size;

    ctk_document_get_max_page_size (priv->document, &doc_width, &doc_height);
    if (priv->rotation == 90 || priv->rotation == 270) {
        gdouble tmp;

        tmp = doc_width;
        doc_width = doc_height;
        doc_height = tmp;
    }

    ctk_doc_view_compute_border (self, doc_width, doc_height, &border);

    width -= (border.left + border.right + 2 * priv->spacing);
    height -= (border.top + border.bottom + 2 * priv->spacing - 1);

    sb_size = ctk_doc_view_get_scrollbar_size (self, GTK_ORIENTATION_VERTICAL);

    if (priv->sizing_mode == CTK_SIZING_FIT_WIDTH) {
        scale = ctk_doc_view_zoom_for_size_fit_width (doc_width,
                                                      doc_height,
                                                      width - sb_size,
                                                      height);
    }
    else if (priv->sizing_mode == CTK_SIZING_BEST_FIT) {
        scale = ctk_doc_view_zoom_for_size_best_fit (doc_width,
                                                     doc_height,
                                                     width - sb_size,
                                                     height);
    }
    else {
        g_assert_not_reached ();
    }

    ctk_doc_model_set_scale (priv->model, scale);
}

static void ctk_doc_view_zoom_for_size_dual_page (CtkDocView *self,
                                                  gdouble width,
                                                  gdouble height)
{
    CtkDocViewPrivate *priv = self->priv;
    GtkBorder border;
    gint doc_width, doc_height;
    gdouble scale;
    gint other_page;

    other_page = priv->cur_page ^ 1;

    /* Find the largest of the two. */
    ctk_doc_view_get_doc_page_size (self, priv->cur_page, &doc_width, &doc_height);
    if (other_page < ctk_document_count_pages (priv->document)) {
        gint width_2, height_2;

        ctk_doc_view_get_doc_page_size (self, other_page, &width_2, &height_2);
        if (width_2 > doc_width)
            doc_width = width_2;

        if (height_2 > doc_height)
            doc_height = height_2;
    }

    ctk_doc_view_compute_border (self, width, height, &border);

    doc_width = doc_width * 2;
    width -= ((border.left + border.right)* 2 + 3 * priv->spacing);
    height -= (border.top + border.bottom + 2 * priv->spacing);

    if (priv->sizing_mode == CTK_SIZING_FIT_WIDTH) {
        gint sb_size;

        sb_size = ctk_doc_view_get_scrollbar_size (self, GTK_ORIENTATION_VERTICAL);
        scale = ctk_doc_view_zoom_for_size_fit_width (doc_width,
                                                      doc_height,
                                                      width - sb_size,
                                                      height);
    }
    else if (priv->sizing_mode == CTK_SIZING_BEST_FIT) {
        scale = ctk_doc_view_zoom_for_size_best_fit (doc_width,
                                                     doc_height,
                                                     width,
                                                     height);
    }
    else {
        g_assert_not_reached ();
    }

    ctk_doc_model_set_scale (priv->model, scale);
}

static void ctk_doc_view_zoom_for_size_single_page (CtkDocView *self,
                                                    gint width,
                                                    gint height)
{
    CtkDocViewPrivate *priv = self->priv;
    gint doc_width, doc_height;
    GtkBorder border;
    gdouble scale;

    ctk_doc_view_get_doc_page_size (self, priv->cur_page, &doc_width, &doc_height);

    /* Get an approximate border */
    ctk_doc_view_compute_border (self, width, height, &border);

    width -= (border.left + border.right + 2 * priv->spacing);
    height -= (border.top + border.bottom + 2 * priv->spacing);

    if (priv->sizing_mode == CTK_SIZING_FIT_WIDTH) {
        gint sb_size;

        sb_size = ctk_doc_view_get_scrollbar_size (self, GTK_ORIENTATION_VERTICAL);
        scale = ctk_doc_view_zoom_for_size_fit_width (doc_width,
                                                      doc_height,
                                                      width - sb_size,
                                                      height);
    }
    else if (priv->sizing_mode == CTK_SIZING_BEST_FIT) {
        scale = ctk_doc_view_zoom_for_size_best_fit (doc_width,
                                                     doc_height,
                                                     width,
                                                     height);
    }
    else {
        g_assert_not_reached ();
    }

    ctk_doc_model_set_scale (priv->model, scale);
}

static void ctk_doc_view_zoom_for_size (CtkDocView *self,
                                        gint width,
                                        gint height)
{
    CtkDocViewPrivate *priv = self->priv;

    g_assert (priv->sizing_mode == CTK_SIZING_FIT_WIDTH ||
              priv->sizing_mode == CTK_SIZING_BEST_FIT);
    g_assert (width >= 0 && height >= 0);

    if (NULL == priv->document)
        return;

    if (priv->continuous && priv->dual_page)
        ctk_doc_view_zoom_for_size_continuous_and_dual_page (self, width, height);
    else if (priv->continuous)
        ctk_doc_view_zoom_for_size_continuous (self, width, height);
    else if (priv->dual_page)
        ctk_doc_view_zoom_for_size_dual_page (self, width, height);
    else
        ctk_doc_view_zoom_for_size_single_page (self, width, height);
}

static void ctk_doc_view_update_visible_pages (CtkDocView *self)
{
    CtkDocViewPrivate *priv = self->priv;
    gint begin = priv->begin_page;
    gint end = priv->end_page;
    gint page_count;

    page_count = ctk_document_count_pages (priv->document);

    if (priv->continuous) {
        GdkRectangle current_area, unused, page_area;
        GtkBorder border;
        gboolean found = FALSE;
        gint area_max = -1, area;
        gint best_current_page = -1;
        gint i, j = 0;

        if (!(priv->vadjustment && priv->hadjustment))
            return;

        current_area.x = gtk_adjustment_get_value (priv->hadjustment);
        current_area.width = gtk_adjustment_get_page_size (priv->hadjustment);
        current_area.y = gtk_adjustment_get_value (priv->vadjustment);
        current_area.height = gtk_adjustment_get_page_size (priv->vadjustment);

        for (i = 0; i < page_count; ++i) {
            ctk_doc_view_get_page_extents (self, i, &page_area, &border);

            if (gdk_rectangle_intersect (&current_area, &page_area, &unused)) {
                area = unused.width * unused.height;

                if (!found) {
                    area_max = area;
                    priv->begin_page = i;
                    found = TRUE;
                    best_current_page = i;
                }

                if (area > area_max) {
                    best_current_page = (area == area_max) ? MIN (i, best_current_page) : i;
                    area_max = area;
                }

                priv->end_page = i + 1;
                j = 0;
            }
            else if (found && priv->cur_page < priv->end_page) {
                if (priv->dual_page && j < 1) {
                    /* In dual mode we stop searching
                     * after two consecutive non-visible pages.
                     */
                    j++;
                    continue;
                }
                break;
            }
        }

        if (priv->pending_scroll == SCROLL_TO_KEEP_POSITION) {
            best_current_page = MAX (best_current_page, priv->begin_page);

            if (priv->cur_page != best_current_page) {
                priv->cur_page = best_current_page;
                ctk_doc_model_set_page (priv->model, best_current_page);
            }
        }
    }
    else if (priv->dual_page) {
        if (priv->cur_page % 2 == priv->dual_even_left) {
            priv->begin_page = priv->cur_page;

            if (priv->cur_page + 1 < page_count)
                priv->end_page = priv->begin_page + 2;
            else
                priv->end_page = priv->begin_page + 1;
        }
        else {
            if (priv->cur_page < 1)
                priv->begin_page = priv->cur_page;
            else
                priv->begin_page = priv->cur_page - 1;

            priv->end_page = priv->cur_page + 1;
        }
    }
    else {
        priv->begin_page = priv->cur_page;
        priv->end_page = priv->cur_page + 1;
    }

    if (priv->begin_page == -1 || priv->end_page == -1)
        return;

    if (begin != priv->begin_page || end != priv->end_page) {
    }

    ctk_doc_render_cache_set_page_range (priv->render_cache,
                                         priv->begin_page,
                                         priv->end_page);

    if (ctk_doc_render_cache_get_surface (priv->render_cache,
                                          priv->cur_page))
    {
        gtk_widget_queue_draw (GTK_WIDGET (self));
    }
}

static void ctk_doc_view_scroll_to_point (CtkDocView *self,
                                          gdouble x,
                                          gdouble y,
                                          GtkOrientation orientation)
{
    CtkDocViewPrivate *priv = self->priv;
    gdouble page_size;
    gdouble upper, lower;

    if (orientation == GTK_ORIENTATION_VERTICAL) {
        page_size = gtk_adjustment_get_page_size (priv->vadjustment);
        upper = gtk_adjustment_get_upper (priv->vadjustment);
        lower = gtk_adjustment_get_lower (priv->vadjustment);

        y = CLAMP (y, lower, upper - page_size);
        if (priv->continuous) {
            gtk_adjustment_clamp_page (priv->vadjustment,
                                       y,
                                       y + page_size);
        }
        else {
            gtk_adjustment_set_value (priv->vadjustment, y);
        }
     }
    else {
        page_size = gtk_adjustment_get_page_size (priv->hadjustment);
        upper = gtk_adjustment_get_upper (priv->hadjustment);
        lower = gtk_adjustment_get_lower (priv->hadjustment);

        x = CLAMP (x, lower, upper - page_size);
        if (priv->dual_page) {
            gtk_adjustment_clamp_page (priv->hadjustment,
                                       x,
                                       x + page_size);
        }
        else {
            gtk_adjustment_set_value (priv->hadjustment, x);
        }
    }
}

static void ctk_doc_view_scroll_to_page_position (CtkDocView *self,
                                                  GtkOrientation orientation)
{
    CtkDocViewPrivate *priv = self->priv;
    gdouble x, y;

    if (!priv->document)
        return;

    if ((orientation == GTK_ORIENTATION_VERTICAL && priv->pending_y == 0) ||
        (orientation == GTK_ORIENTATION_HORIZONTAL && priv->pending_x == 0))
    {
        GdkRectangle page_area;
        GtkBorder border;

        ctk_doc_view_get_page_extents (self, priv->cur_page, &page_area, &border);
        x = page_area.x;
        y = page_area.y;
    }
    else {
        GdkPoint view_point;

        ctk_doc_point_to_view_point (self,
                                     priv->cur_page,
                                     priv->pending_x,
                                     priv->pending_y,
                                     &view_point);
        x = view_point.x;
        y = view_point.y;
    }

    ctk_doc_view_scroll_to_point (self, x, y, orientation);
}

static void ctk_doc_view_adjustment_value_changed (GtkAdjustment *adjustment,
                                                   CtkDocView *view)
{
    CtkDocViewPrivate *priv = view->priv;
    GtkWidget *widget = GTK_WIDGET (view);
    gdouble dx = 0, dy = 0;
    gdouble value;

    if (!gtk_widget_get_realized (widget))
        return;

    if (priv->hadjustment) {
        value = gtk_adjustment_get_value (priv->hadjustment);
        dx = priv->scroll_x - value;
        priv->scroll_x = value;
    }
    else {
        priv->scroll_x = 0;
    }

    if (priv->vadjustment) {
        value = gtk_adjustment_get_value (priv->vadjustment);
        dy = priv->scroll_y - value;
        priv->scroll_y = value;
    }
    else {
        priv->scroll_y = 0;
    }

    if (priv->pending_resize) {
        gtk_widget_queue_draw (widget);
    }
    else {
        gdk_window_scroll (gtk_widget_get_window (widget), dx, dy);
    }

    if (priv->document)
        ctk_doc_view_update_visible_pages (view);
}

static void ctk_doc_view_set_adjustment_values (CtkDocView *self,
                                                GtkOrientation orientation)
{
    CtkDocViewPrivate *priv = self->priv;
    GtkWidget *widget = GTK_WIDGET (self);
    GtkAdjustment *adjustment;
    GtkAllocation allocation;
    gint req_size;
    gint alloc_size;
    gdouble page_size;
    gdouble value;
    gdouble upper;
    double factor;
    gint new_value;

    gtk_widget_get_allocation (widget, &allocation);

    if (orientation == GTK_ORIENTATION_HORIZONTAL)  {
        req_size = priv->requisition.width;
        alloc_size = allocation.width;
        adjustment = priv->hadjustment;
    }
    else {
        req_size = priv->requisition.height;
        alloc_size = allocation.height;
        adjustment = priv->vadjustment;
    }

    if (!adjustment)
        return;

    factor = 1.0;
    value = gtk_adjustment_get_value (adjustment);
    upper = gtk_adjustment_get_upper (adjustment);
    page_size = gtk_adjustment_get_page_size (adjustment);

    switch (priv->pending_scroll) {
    case SCROLL_TO_KEEP_POSITION:
    case SCROLL_TO_FIND_LOCATION:
        factor = value / upper;
        break;

    case SCROLL_TO_PAGE_POSITION:
        break;

    case SCROLL_TO_CENTER:
        factor = (value + page_size * 0.5) / upper;
        break;
    }

    upper = MAX (alloc_size, req_size);
    page_size = alloc_size;

    gtk_adjustment_set_page_size (adjustment, page_size);
    gtk_adjustment_set_step_increment (adjustment, alloc_size * 0.1);
    gtk_adjustment_set_page_increment (adjustment, alloc_size * 0.9);
    gtk_adjustment_set_lower (adjustment, 0);
    gtk_adjustment_set_upper (adjustment, upper);

    /*
     * We add 0.5 to the values before to average out our rounding errors.
     */
    switch (priv->pending_scroll) {
    case SCROLL_TO_KEEP_POSITION:
    case SCROLL_TO_FIND_LOCATION:
        new_value = CLAMP (upper * factor + 0.5, 0, upper - page_size);
        gtk_adjustment_set_value (adjustment, (int) new_value);
        break;

    case SCROLL_TO_PAGE_POSITION:
        ctk_doc_view_scroll_to_page_position (self, orientation);
        break;

    case SCROLL_TO_CENTER:
        new_value = CLAMP (upper * factor - page_size * 0.5 + 0.5,
                           0, upper - page_size);
        gtk_adjustment_set_value (adjustment, (int) new_value);
        break;
    }

    gtk_adjustment_changed (adjustment);
}

static void ctk_doc_view_set_scroll_adjustment (CtkDocView *self,
                                                GtkOrientation orientation,
                                                GtkAdjustment *adjustment)
{
    CtkDocViewPrivate *priv = self->priv;
    GtkAdjustment **to_set;
    const gchar *prop_name;

    if (orientation == GTK_ORIENTATION_HORIZONTAL) {
        to_set = &priv->hadjustment;
        prop_name = "hadjustment";
    }
    else {
        to_set = &priv->vadjustment;
        prop_name = "vadjustment";
    }

    if (adjustment && adjustment == *to_set)
        return;

    if (*to_set) {
        g_signal_handlers_disconnect_by_data (*to_set, self);
        g_object_unref (*to_set);
    }

    if (!adjustment)
        adjustment = gtk_adjustment_new (0.0, 0.0, 0.0, 0.0, 0.0, 0.0);

    g_signal_connect (adjustment,
                      "value-changed",
                      G_CALLBACK (ctk_doc_view_adjustment_value_changed),
                      self);

    *to_set = g_object_ref_sink (adjustment);
    ctk_doc_view_set_adjustment_values (self, orientation);

    g_object_notify (G_OBJECT (self), prop_name);
}

static void ctk_doc_view_remove_all (CtkDocView *self)
{
    gtk_container_foreach (GTK_CONTAINER (self),
                           (GtkCallback) gtk_widget_destroy,
                           NULL);
}

static void ctk_doc_view_change_page (CtkDocView *self,
                                      gint new_page)
{
    CtkDocViewPrivate *priv = self->priv;

    priv->cur_page = new_page;
    priv->pending_scroll = SCROLL_TO_PAGE_POSITION;

    /*
    gint x, y;
    hide_loading_window (view);

    ev_document_misc_get_pointer_position (GTK_WIDGET (view), &x, &y);
    ev_view_handle_cursor_over_xy (view, x, y);
    */

    gtk_widget_queue_resize (GTK_WIDGET (self));
}

static void ctk_doc_view_page_changed (CtkDocModel *model,
                                       gint old_page,
                                       gint new_page,
                                       CtkDocView *view)
{
    CtkDocViewPrivate *priv = view->priv;

    if (!priv->document)
        return;

    if (priv->cur_page != new_page) {
        ctk_doc_view_change_page (view, new_page);
    }
    else {
        gtk_widget_queue_draw (GTK_WIDGET (view));
    }
}

static void ctk_doc_view_document_changed (CtkDocModel *model,
                                           GParamSpec *pspec,
                                           CtkDocView *view)
{
    CtkDocViewPrivate *priv = view->priv;
    CtkDocument *document = ctk_doc_model_get_document (model);

    if (document != priv->document) {
        gint cur_page;

        ctk_doc_view_remove_all (view);
        ctk_doc_view_clear_caches (view);

        if (priv->document)
            g_object_unref (priv->document);

        priv->document = document;

        if (priv->document) {
            g_object_ref (priv->document);
            ctk_doc_view_setup_caches (view);
        }

        cur_page = ctk_doc_model_get_page (model);
        if (priv->cur_page != cur_page) {
            ctk_doc_view_change_page (view, cur_page);
        }
        else {
            priv->pending_scroll = SCROLL_TO_KEEP_POSITION;
            gtk_widget_queue_resize (GTK_WIDGET (view));
        }
    }
}

static void ctk_doc_view_rotation_changed (CtkDocModel *model,
                                           GParamSpec *pspec,
                                           CtkDocView *view)
{
    CtkDocViewPrivate *priv = view->priv;
    gint rotation = ctk_doc_model_get_rotation (model);

    priv->rotation = rotation;

    if (priv->render_cache) {
        ctk_doc_render_cache_clear (priv->render_cache);

        if (!ctk_document_get_uniform_page_size (priv->document, NULL, NULL))
            priv->pending_scroll = SCROLL_TO_PAGE_POSITION;

        gtk_widget_queue_resize (GTK_WIDGET (view));
    }

    ctk_doc_view_remove_all (view);
}

static void ctk_doc_view_sizing_mode_changed (CtkDocModel *model,
                                              GParamSpec *pspec,
                                              CtkDocView *view)
{
    CtkSizingMode mode = ctk_doc_model_get_sizing_mode (model);

    view->priv->sizing_mode = mode;

    if (mode != CTK_SIZING_FREE)
        gtk_widget_queue_resize (GTK_WIDGET (view));
}

#define EPSILON 0.0000001
static void ctk_doc_view_scale_changed (CtkDocModel *model,
                                        GParamSpec *pspec,
                                        CtkDocView *view)
{
    CtkDocViewPrivate *priv = view->priv;
    gdouble scale = ctk_doc_model_get_scale (model);

    if (ABS (priv->scale - scale) < EPSILON)
        return;

    priv->scale = scale;
    priv->pending_resize = TRUE;

    if (priv->sizing_mode == CTK_SIZING_FREE)
        gtk_widget_queue_resize (GTK_WIDGET (view));
}

static void ctk_doc_view_continuous_changed (CtkDocModel *model,
                                             GParamSpec *pspec,
                                             CtkDocView *view)
{
    CtkDocViewPrivate *priv = view->priv;
    gboolean continuous = ctk_doc_model_get_continuous (model);

    priv->continuous = continuous;
    priv->pending_scroll = SCROLL_TO_PAGE_POSITION;
    gtk_widget_queue_resize (GTK_WIDGET (view));
}

static void ctk_doc_view_dual_page_changed (CtkDocModel *model,
                                            GParamSpec *pspec,
                                            CtkDocView *view)
{
    CtkDocViewPrivate *priv = view->priv;
    gboolean dual_page = ctk_doc_model_get_dual_page (model);

    priv->dual_page = dual_page || ctk_doc_model_get_dual_page_odd_pages_left (model);
    priv->pending_scroll = SCROLL_TO_PAGE_POSITION;

    /* FIXME: if we're keeping the pixbuf cache around, we should
     * extend the preload_cache_size to be 2 if dual_page is set.
     */
    gtk_widget_queue_resize (GTK_WIDGET (view));
}

static void ctk_doc_view_dual_odd_left_changed (CtkDocModel *model,
                                                GParamSpec *pspec,
                                                CtkDocView *view)
{
    CtkDocViewPrivate *priv = view->priv;
    gboolean dual_odd_left = ctk_doc_model_get_dual_page_odd_pages_left (model);

    priv->dual_page = dual_odd_left || ctk_doc_model_get_dual_page (model);
    priv->dual_even_left = !dual_odd_left;

    priv->pending_scroll = SCROLL_TO_PAGE_POSITION;
    gtk_widget_queue_resize (GTK_WIDGET (view));
}

static void ctk_doc_view_init (CtkDocView *self)
{
    CtkDocViewPrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              CTK_TYPE_DOC_VIEW,
                                              CtkDocViewPrivate);
    priv = self->priv;

    gtk_widget_set_has_window (GTK_WIDGET (self), TRUE);
    gtk_widget_set_can_focus (GTK_WIDGET (self), TRUE);
    gtk_widget_set_redraw_on_allocate (GTK_WIDGET (self), FALSE);
    gtk_container_set_resize_mode (GTK_CONTAINER (self), GTK_RESIZE_QUEUE);

    gtk_widget_set_events (GTK_WIDGET (self),
                           GDK_EXPOSURE_MASK |
                           GDK_BUTTON_PRESS_MASK |
                           GDK_BUTTON_RELEASE_MASK |
                           GDK_SCROLL_MASK |
                           GDK_KEY_PRESS_MASK |
                           GDK_POINTER_MOTION_MASK |
                           GDK_POINTER_MOTION_HINT_MASK |
                           GDK_ENTER_NOTIFY_MASK |
                           GDK_LEAVE_NOTIFY_MASK);
    priv->model = NULL;
    priv->document = NULL;
    priv->height_cache = NULL;
    priv->thread_pool = NULL;
    priv->render_cache = NULL;
    priv->render_cache_size = 0;
    priv->begin_page = -1;
    priv->end_page = -1;
    priv->cur_page = -1;
    priv->requisition.width = 0;
    priv->requisition.height = 0;
    priv->internal_size_request = FALSE;
    priv->jump_to_find_result = TRUE;
    priv->highlight_find_results = FALSE;
    priv->hadjustment = NULL;
    priv->vadjustment = NULL;
    priv->hscroll_policy = 0;
    priv->vscroll_policy = 0;
    priv->scroll_x = 0;
    priv->scroll_y = 0;
    priv->pending_scroll = SCROLL_TO_KEEP_POSITION;
    priv->pending_resize = FALSE;
    priv->pending_x = 0;
    priv->pending_y = 0;
    priv->rotation = 0;
    priv->scale = 1.0;
    priv->spacing = 5.0;
    priv->continuous = TRUE;
    priv->dual_page = FALSE;
    priv->dual_even_left = TRUE;
    priv->sizing_mode = CTK_SIZING_FIT_WIDTH;
}

static void ctk_doc_view_set_property (GObject *object,
                                       guint prop_id,
                                       const GValue *value,
                                       GParamSpec *pspec)
{
    CtkDocView *self = CTK_DOC_VIEW (object);
    CtkDocViewPrivate *priv = self->priv;

    switch (prop_id) {
    case PROP_HADJUSTMENT:
        ctk_doc_view_set_scroll_adjustment (self,
                                            GTK_ORIENTATION_HORIZONTAL,
                                            g_value_get_object (value));
        break;

    case PROP_VADJUSTMENT:
        ctk_doc_view_set_scroll_adjustment (self,
                                            GTK_ORIENTATION_VERTICAL,
                                            g_value_get_object (value));
        break;

    case PROP_HSCROLL_POLICY:
        priv->hscroll_policy = g_value_get_enum (value);
        gtk_widget_queue_resize (GTK_WIDGET (self));
        break;

    case PROP_VSCROLL_POLICY:
        priv->vscroll_policy = g_value_get_enum (value);
        gtk_widget_queue_resize (GTK_WIDGET (self));
        break;

    case PROP_THREADPOOL:
        priv->thread_pool = g_value_dup_object (value);
        if (NULL == priv->thread_pool)
            priv->thread_pool = oren_thread_pool_new (1);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void ctk_doc_view_get_property (GObject *object,
                                       guint prop_id,
                                       GValue *value,
                                       GParamSpec *pspec)
{
    CtkDocView *self = CTK_DOC_VIEW (object);
    CtkDocViewPrivate *priv = self->priv;

    switch (prop_id) {
    case PROP_HADJUSTMENT:
        g_value_set_object (value, priv->hadjustment);
        break;

    case PROP_VADJUSTMENT:
        g_value_set_object (value, priv->vadjustment);
        break;

    case PROP_HSCROLL_POLICY:
        g_value_set_enum (value, priv->hscroll_policy);
        break;

    case PROP_VSCROLL_POLICY:
        g_value_set_enum (value, priv->vscroll_policy);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void ctk_doc_view_dispose (GObject *gobject)
{
    CtkDocView *self = CTK_DOC_VIEW (gobject);
    CtkDocViewPrivate *priv = self->priv;

    if (priv->render_cache) {
        g_object_unref (priv->render_cache);
        priv->render_cache = NULL;
    }

    if (priv->thread_pool) {
        g_object_unref (priv->thread_pool);
        priv->thread_pool = NULL;
    }

    if (priv->document) {
        g_object_unref (priv->document);
        priv->document = NULL;
    }

    if (priv->model) {
        g_object_unref (priv->model);
        priv->model = NULL;
    }

    gtk_scrollable_set_hadjustment (GTK_SCROLLABLE (self), NULL);
    gtk_scrollable_set_vadjustment (GTK_SCROLLABLE (self), NULL);

    G_OBJECT_CLASS (ctk_doc_view_parent_class)->dispose (gobject);
}

static void ctk_doc_view_finalize (GObject *gobject)
{
    G_OBJECT_CLASS (ctk_doc_view_parent_class)->finalize (gobject);
}

static void ctk_doc_view_size_request_continuous_dual_page (CtkDocView *self,
                                                            GtkRequisition *requisition)
{
    g_print ("size_request_continuous_dual_page\n");
}

static void ctk_doc_view_size_request_continuous (CtkDocView *self,
                                                  GtkRequisition *requisition)
{
    CtkDocViewPrivate *priv = self->priv;
    gdouble height;
    gint page_count;

    page_count = ctk_document_count_pages (priv->document);
    ctk_doc_view_get_page_y_offset (self, page_count, &height);
    requisition->height = height;

    switch (priv->sizing_mode) {
    case CTK_SIZING_FIT_WIDTH:
    case CTK_SIZING_BEST_FIT:
        requisition->width = 1;
        break;

    case CTK_SIZING_FREE:
        {
            gdouble max_width;
            GtkBorder border;

            ctk_doc_view_get_max_page_size (self, &max_width, NULL);
            ctk_doc_view_compute_border (self, max_width, max_width, &border);
            requisition->width = max_width + (priv->spacing * 2) +
                                 border.left + border.right;
        }
        break;

    default:
        g_assert_not_reached ();
    }
}

static void ctk_doc_view_size_request_dual_page (CtkDocView *self,
                                                 GtkRequisition *requisition)
{
    g_print ("size_request_dual_page\n");
}

static void ctk_doc_view_size_request_single_page (CtkDocView *self,
                                                   GtkRequisition *requisition)
{
    g_print ("size_request_single_page\n");
}

static void ctk_doc_view_size_request (GtkWidget *widget,
                                       GtkRequisition *requisition)
{
    CtkDocView *self = CTK_DOC_VIEW (widget);
    CtkDocViewPrivate *priv = self->priv;

    if (NULL == priv->document) {
        priv->requisition.width = 1;
        priv->requisition.height = 1;

        *requisition = priv->requisition;
        return;
    }

    /* Get zoom for size here when not called from
     * ctk_doc_view_size_allocate ()
     */
    if (!priv->internal_size_request &&
        (priv->sizing_mode == CTK_SIZING_FIT_WIDTH ||
         priv->sizing_mode == CTK_SIZING_BEST_FIT))
    {
        GtkAllocation allocation;

        gtk_widget_get_allocation (widget, &allocation);
        ctk_doc_view_zoom_for_size (self,
                                    allocation.width,
                                    allocation.height);
    }

    if (priv->continuous && priv->dual_page)
        ctk_doc_view_size_request_continuous_dual_page (self, &priv->requisition);
    else if (priv->continuous)
        ctk_doc_view_size_request_continuous (self, &priv->requisition);
    else if (priv->dual_page)
        ctk_doc_view_size_request_dual_page (self, &priv->requisition);
    else
        ctk_doc_view_size_request_single_page (self, &priv->requisition);

    *requisition = priv->requisition;
}

static void ctk_doc_view_get_preferred_width (GtkWidget *widget,
                                              gint *minimum,
                                              gint *natural)
{
    GtkRequisition requisition;

    ctk_doc_view_size_request (widget, &requisition);

    *minimum = *natural = requisition.width;
}

static void ctk_doc_view_get_preferred_height (GtkWidget *widget,
                                               gint *minimum,
                                               gint *natural)
{
    GtkRequisition requisition;

    ctk_doc_view_size_request (widget, &requisition);

    *minimum = *natural = requisition.height;
}

static void ctk_doc_view_size_allocate (GtkWidget *widget,
                                        GtkAllocation  *allocation)
{
    CtkDocView *self = CTK_DOC_VIEW (widget);
    CtkDocViewPrivate *priv = self->priv;

    gtk_widget_set_allocation (widget, allocation);

    if (gtk_widget_get_realized (widget)) {
        gdk_window_move_resize (gtk_widget_get_window (widget),
                                allocation->x,
                                allocation->y,
                                allocation->width,
                                allocation->height);
    }

    if (NULL == priv->document)
        return;

    if (priv->sizing_mode == CTK_SIZING_FIT_WIDTH ||
        priv->sizing_mode == CTK_SIZING_BEST_FIT)
    {
        GtkRequisition req;

        ctk_doc_view_zoom_for_size (self,
                                    allocation->width,
                                    allocation->height);
        priv->internal_size_request = TRUE;
        ctk_doc_view_size_request (widget, &req);
        priv->internal_size_request = FALSE;
    }

    ctk_doc_view_set_adjustment_values (self, GTK_ORIENTATION_HORIZONTAL);
    ctk_doc_view_set_adjustment_values (self, GTK_ORIENTATION_VERTICAL);

    if (priv->document)
        ctk_doc_view_update_visible_pages (self);

    priv->pending_scroll = SCROLL_TO_KEEP_POSITION;
    priv->pending_resize = FALSE;
    priv->pending_x = 0;
    priv->pending_y = 0;
}

static gboolean ctk_doc_view_scroll_event (GtkWidget *widget,
                                           GdkEventScroll *event)
{
    CtkDocView *self = CTK_DOC_VIEW (widget);
    CtkDocViewPrivate *priv = self->priv;
    guint state;

    state = event->state & gtk_accelerator_get_default_mod_mask ();

    if (state == GDK_CONTROL_MASK) {
        ctk_doc_model_set_sizing_mode (priv->model, CTK_SIZING_FREE);
        if (event->direction == GDK_SCROLL_UP ||
            event->direction == GDK_SCROLL_LEFT)
        {
            if (ctk_doc_view_can_zoom_in (self)) {
                ctk_doc_view_zoom_in (self);
            }
        }
        else {
            if (ctk_doc_view_can_zoom_out (self)) {
                ctk_doc_view_zoom_out (self);
            }
        }

        return TRUE;
    }

    priv->jump_to_find_result = FALSE;

    /* Shift+Wheel scrolls the in the perpendicular direction */
    if (state & GDK_SHIFT_MASK) {
        if (event->direction == GDK_SCROLL_UP)
            event->direction = GDK_SCROLL_LEFT;
        else if (event->direction == GDK_SCROLL_LEFT)
            event->direction = GDK_SCROLL_UP;
        else if (event->direction == GDK_SCROLL_DOWN)
            event->direction = GDK_SCROLL_RIGHT;
        else if (event->direction == GDK_SCROLL_RIGHT)
            event->direction = GDK_SCROLL_DOWN;

        event->state &= ~GDK_SHIFT_MASK;
        state &= ~GDK_SHIFT_MASK;
    }

    if (state == 0 &&
        priv->sizing_mode == CTK_SIZING_BEST_FIT &&
        !priv->continuous)
    {
        switch (event->direction) {
        case GDK_SCROLL_DOWN:
        case GDK_SCROLL_RIGHT:
            ctk_doc_view_next_page (self);
            break;

        case GDK_SCROLL_UP:
        case GDK_SCROLL_LEFT:
            ctk_doc_view_previous_page (self);
            break;

        default:
            break;
        }

        return TRUE;
    }

    return FALSE;
}

static void ctk_doc_view_realize (GtkWidget *widget)
{
    GtkAllocation allocation;
    GdkWindow *window;
    GdkWindowAttr attributes;
    gint attributes_mask;

    gtk_widget_set_realized (widget, TRUE);

    gtk_widget_get_allocation (widget, &allocation);

    attributes.window_type = GDK_WINDOW_CHILD;
    attributes.x = allocation.x;
    attributes.y = allocation.y;
    attributes.width = allocation.width;
    attributes.height = allocation.height;
    attributes.wclass = GDK_INPUT_OUTPUT;
    attributes.visual = gtk_widget_get_visual (widget);
    attributes.event_mask = gtk_widget_get_events (widget);

    attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL;

    window = gdk_window_new (gtk_widget_get_parent_window (widget),
                             &attributes,
                             attributes_mask);
    gtk_widget_set_window (widget, window);
    gdk_window_set_user_data (window, widget);

    gtk_style_context_set_background (gtk_widget_get_style_context (widget),
                                      window);
}

static void ctk_doc_view_draw_page_bkgnd (GtkWidget *widget,
                                          cairo_t *cr,
                                          GdkRectangle *area,
                                          GtkBorder *border,
                                          gboolean highlight,
                                          gboolean inverted_colors)
{
    GtkStyleContext *context = gtk_widget_get_style_context (widget);
    GtkStateFlags state = gtk_widget_get_state_flags (widget);
    GdkRGBA fg, bg, shade_bg;

    gtk_style_context_get_background_color (context, state, &bg);
    gtk_style_context_get_color (context, state, &fg);
    gtk_style_context_get_color (context, GTK_STATE_FLAG_INSENSITIVE, &shade_bg);

    gdk_cairo_set_source_rgba (cr, highlight ? &fg : &shade_bg);
    gdk_cairo_rectangle (cr, area);
    cairo_fill (cr);

    if (inverted_colors)
        cairo_set_source_rgb (cr, 0, 0, 0);
    else
        cairo_set_source_rgb (cr, 1, 1, 1);

    cairo_rectangle (cr,
                     area->x + border->left,
                     area->y + border->top,
                     area->width - (border->left + border->right),
                     area->height - (border->top + border->bottom));
    cairo_fill (cr);

    gdk_cairo_set_source_rgba (cr, &bg);
    cairo_rectangle (cr,
                     area->x,
                     area->y + area->height - (border->bottom - border->top),
                     border->bottom - border->top,
                     border->bottom - border->top);
    cairo_fill (cr);

    cairo_rectangle (cr,
                     area->x + area->width - (border->right - border->left),
                     area->y,
                     border->right - border->left,
                     border->right - border->left);
    cairo_fill (cr);
}

static void ctk_doc_view_draw_page (CtkDocView *self,
                                    cairo_t *cr,
                                    gint page,
                                    GdkRectangle *page_area,
                                    GtkBorder *border,
                                    GdkRectangle *expose_area,
                                    gboolean *page_ready)
{
    CtkDocViewPrivate *priv = self->priv;
    GdkRectangle overlap;
    GdkRectangle real_page_area;

    if (!gdk_rectangle_intersect (page_area, expose_area, &overlap))
        return;

    /* Render the document itself */
    real_page_area = *page_area;

    real_page_area.x += border->left;
    real_page_area.y += border->top;
    real_page_area.width -= (border->left + border->right);
    real_page_area.height -= (border->top + border->bottom);
    *page_ready = TRUE;

    ctk_doc_view_draw_page_bkgnd (GTK_WIDGET (self),
                                  cr,
                                  page_area,
                                  border,
                                  page == priv->cur_page,
                                  FALSE);

    if (gdk_rectangle_intersect (&real_page_area, expose_area, &overlap)) {
        cairo_surface_t *page_surface;
        gint width, height;
        gint page_width, page_height;

        page_surface = ctk_doc_render_cache_get_surface (priv->render_cache, page);
        if (NULL == page_surface) {
            /* TODO: show loading page. */
            return;
        }

        ctk_doc_view_get_view_page_size (self, page, &width, &height);
        page_width = cairo_image_surface_get_width (page_surface);
        page_height = cairo_image_surface_get_height (page_surface);

        cairo_save (cr);
        cairo_translate (cr, overlap.x, overlap.y);

        if (width != page_width || height != page_height) {
            cairo_pattern_set_filter (cairo_get_source (cr),
                                      CAIRO_FILTER_FAST);
            cairo_scale (cr,
                         (gdouble) width / page_width,
                         (gdouble) height / page_height);
        }

        cairo_surface_set_device_offset (page_surface,
                                         overlap.x - real_page_area.x,
                                         overlap.y - real_page_area.y);
        cairo_set_source_surface (cr, page_surface, 0, 0);
        cairo_paint (cr);
        cairo_restore (cr);
    }
}

static gboolean ctk_doc_view_draw (GtkWidget *widget,
                                   cairo_t *cr)
{
    CtkDocView *self = CTK_DOC_VIEW (widget);
    CtkDocViewPrivate *priv = self->priv;
    gint i;
    GdkRectangle clip_rect;

    if (NULL == priv->document)
        return FALSE;

    if (!gdk_cairo_get_clip_rectangle (cr, &clip_rect))
        return FALSE;

    for (i = priv->begin_page; i < priv->end_page; ++i) {
        GdkRectangle page_area;
        GtkBorder border;
        gboolean page_ready = TRUE;

        ctk_doc_view_get_page_extents (self, i, &page_area, &border);

        page_area.x -= priv->scroll_x;
        page_area.y -= priv->scroll_y;

        ctk_doc_view_draw_page (self, cr, i, &page_area, &border,
                                &clip_rect, &page_ready);
    }

    return GTK_WIDGET_CLASS (ctk_doc_view_parent_class)->draw (widget, cr);
}

static void ctk_doc_view_destroy (GtkWidget *widget)
{
    GTK_WIDGET_CLASS (ctk_doc_view_parent_class)->destroy (widget);
}

static void ctk_doc_view_add (GtkContainer *container,
                              GtkWidget *child)
{
}

static void ctk_doc_view_remove (GtkContainer *container,
                                 GtkWidget *widget)
{
}

static void ctk_doc_view_forall (GtkContainer *container,
                                 gboolean include_internals,
                                 GtkCallback callback,
                                 gpointer callback_data)
{
}

static void ctk_doc_view_class_init (CtkDocViewClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
    GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);

    gobject_class->set_property = ctk_doc_view_set_property;
    gobject_class->get_property = ctk_doc_view_get_property;
    gobject_class->dispose = ctk_doc_view_dispose;
    gobject_class->finalize = ctk_doc_view_finalize;

    widget_class->realize = ctk_doc_view_realize;
    widget_class->draw = ctk_doc_view_draw;
    widget_class->destroy = ctk_doc_view_destroy;
    widget_class->get_preferred_width = ctk_doc_view_get_preferred_width;
    widget_class->get_preferred_height = ctk_doc_view_get_preferred_height;
    widget_class->size_allocate = ctk_doc_view_size_allocate;
    widget_class->scroll_event = ctk_doc_view_scroll_event;

    container_class->add = ctk_doc_view_add;
    container_class->remove = ctk_doc_view_remove;
    container_class->forall = ctk_doc_view_forall;

    g_type_class_add_private (gobject_class,
                              sizeof (CtkDocViewPrivate));

    /* GtkScrollable interface */
    g_object_class_override_property (gobject_class, PROP_HADJUSTMENT, "hadjustment");
    g_object_class_override_property (gobject_class, PROP_VADJUSTMENT, "vadjustment");
    g_object_class_override_property (gobject_class, PROP_HSCROLL_POLICY, "hscroll-policy");
    g_object_class_override_property (gobject_class, PROP_VSCROLL_POLICY, "vscroll-policy");

    g_object_class_install_property (
        gobject_class, PROP_THREADPOOL,
        g_param_spec_object ("thread-pool",
                             "Thread pool",
                             "The thread pool for render page",
                             OREN_TYPE_THREAD_POOL,
                             G_PARAM_WRITABLE |
                             G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_STATIC_STRINGS));
}

CtkDocView* ctk_doc_view_new (OrenThreadPool *pool)
{
    return g_object_new (CTK_TYPE_DOC_VIEW,
                         "thread-pool", pool, NULL);
}

void ctk_doc_view_set_model (CtkDocView *self,
                             CtkDocModel *model)
{
    CtkDocViewPrivate *priv;

    g_return_if_fail (CTK_IS_DOC_VIEW (self));

    priv = self->priv;

    if (model == priv->model)
        return;

    if (priv->model) {
        g_signal_handlers_disconnect_by_data (priv->model, self);
        g_object_unref (priv->model);
    }

    priv->model = model ? g_object_ref (model) : NULL;

    if (NULL == priv->model)
        return;

    priv->rotation = ctk_doc_model_get_rotation (priv->model);
    priv->sizing_mode = ctk_doc_model_get_sizing_mode (priv->model);
    priv->scale = ctk_doc_model_get_scale (priv->model);
    priv->continuous = ctk_doc_model_get_continuous (priv->model);
    priv->dual_page = ctk_doc_model_get_dual_page (priv->model);

    ctk_doc_view_document_changed (priv->model, NULL, self);

    g_signal_connect (priv->model, "notify::document",
                      G_CALLBACK (ctk_doc_view_document_changed),
                      self);

    g_signal_connect (priv->model, "notify::rotation",
                      G_CALLBACK (ctk_doc_view_rotation_changed),
                      self);

    g_signal_connect (priv->model, "notify::sizing-mode",
                      G_CALLBACK (ctk_doc_view_sizing_mode_changed),
                      self);

    g_signal_connect (priv->model, "notify::scale",
                      G_CALLBACK (ctk_doc_view_scale_changed),
                      self);

    g_signal_connect (priv->model, "notify::continuous",
                      G_CALLBACK (ctk_doc_view_continuous_changed),
                      self);

    g_signal_connect (priv->model, "notify::dual-page",
                      G_CALLBACK (ctk_doc_view_dual_page_changed),
                      self);

    g_signal_connect (priv->model, "notify::dual-odd-left",
                      G_CALLBACK (ctk_doc_view_dual_odd_left_changed),
                      self);

    g_signal_connect (priv->model, "page-changed",
                      G_CALLBACK (ctk_doc_view_page_changed),
                      self);
}

/**
 * ctk_doc_view_get_model:
 * @self: a #CtkDocView
 *
 * Returns the #CtkDocModel referenced by the view.
 *
 * Returns: (transfer none): a #CtkDocModel
 */
CtkDocModel* ctk_doc_view_get_model (CtkDocView *self)
{
    g_return_val_if_fail (CTK_IS_DOC_VIEW (self), NULL);

    return self->priv->model;
}

void ctk_doc_view_set_render_cache_size (CtkDocView *self,
                                         guint cache_size)
{
    CtkDocViewPrivate *priv;

    g_return_if_fail (CTK_IS_DOC_VIEW (self));

    priv = self->priv;

    priv->render_cache_size = cache_size;

    if (priv->render_cache) {
        ctk_doc_render_cache_set_max_size (priv->render_cache,
                                           cache_size);
    }
}

gboolean ctk_doc_view_can_zoom_in (CtkDocView *self)
{
    CtkDocViewPrivate *priv;

    g_return_val_if_fail (CTK_IS_DOC_VIEW (self), FALSE);

    priv = self->priv;

    if (!priv->model)
        return FALSE;

    return priv->scale * ZOOM_IN_FACTOR <=
            ctk_doc_model_get_max_scale (priv->model);
}

gboolean ctk_doc_view_can_zoom_out (CtkDocView *self)
{
    CtkDocViewPrivate *priv;

    g_return_val_if_fail (CTK_IS_DOC_VIEW (self), FALSE);

    priv = self->priv;

    if (!priv->model)
        return FALSE;

    return priv->scale * ZOOM_OUT_FACTOR >=
            ctk_doc_model_get_min_scale (priv->model);
}

void ctk_doc_view_zoom_in (CtkDocView *self)
{
    CtkDocViewPrivate *priv;
    gdouble scale;

    g_return_if_fail (CTK_IS_DOC_VIEW (self));

    priv = self->priv;

    priv->pending_scroll = SCROLL_TO_CENTER;
    scale = ctk_doc_model_get_scale (priv->model) * ZOOM_IN_FACTOR;
    ctk_doc_model_set_scale (priv->model, scale);
}

void ctk_doc_view_zoom_out (CtkDocView *self)
{
    CtkDocViewPrivate *priv;
    gdouble scale;

    g_return_if_fail (CTK_IS_DOC_VIEW (self));

    priv = self->priv;

    priv->pending_scroll = SCROLL_TO_CENTER;
    scale = ctk_doc_model_get_scale (priv->model) * ZOOM_OUT_FACTOR;
    ctk_doc_model_set_scale (priv->model, scale);
}

gboolean ctk_doc_view_next_page (CtkDocView *self)
{
    CtkDocViewPrivate *priv;
    int page, n_pages;

    g_return_val_if_fail (CTK_IS_DOC_VIEW (self), FALSE);

    priv = self->priv;

    if (!priv->document)
        return FALSE;

    page = ctk_doc_model_get_page (priv->model);
    n_pages = ctk_document_count_pages (priv->document);

    if (priv->dual_page)
        page = page + 2;
    else
        page = page + 1;

    if (page < n_pages) {
        ctk_doc_model_set_page (priv->model, page);
        return TRUE;
    }
    else if (priv->dual_page && page == n_pages) {
        ctk_doc_model_set_page (priv->model, page - 1);
        return TRUE;
    }

    return FALSE;
}

gboolean ctk_doc_view_previous_page (CtkDocView *self)
{
    CtkDocViewPrivate *priv;
    int page;

    g_return_val_if_fail (CTK_IS_DOC_VIEW (self), FALSE);

    priv = self->priv;

    if (!priv->document)
        return FALSE;

    page = ctk_doc_model_get_page (priv->model);

    if (priv->dual_page)
        page = page - 2;
    else
        page = page - 1;

    if (page >= 0) {
        ctk_doc_model_set_page (priv->model, page);
        return TRUE;
    }
    else if (priv->dual_page && page == -1) {
        ctk_doc_model_set_page (priv->model, 0);
        return TRUE;
    }

    return FALSE;
}
