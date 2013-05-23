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
#include "txtpage.h"

G_DEFINE_TYPE (TxtPage, txt_page, CTK_TYPE_DOC_PAGE)

struct _TxtPagePrivate {
    gpointer n;
};

static void _txt_page_get_size (CtkDocPage *self,
                                gdouble *width,
                                gdouble *height)
{
}

static gint _txt_page_text_length (CtkDocPage *page)
{
    return 0;
}

static void _txt_page_extract_text (CtkDocPage *page,
                                    gchar *texts,
                                    cairo_rectangle_int_t *rects)
{
}

static void _txt_page_render (CtkDocPage *self,
                              cairo_surface_t *surface,
                              const cairo_matrix_t *ctm,
                              const cairo_rectangle_int_t *area)
{
}

static void txt_page_init (TxtPage *self)
{
    TxtPagePrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              TXT_TYPE_PAGE,
                                              TxtPagePrivate);
    priv = self->priv;

    priv->n = NULL;
}

static void txt_page_finalize (GObject *gobject)
{
    TxtPage *self = TXT_PAGE (gobject);
    TxtPagePrivate *priv = self->priv;

    g_free (priv->n);

    G_OBJECT_CLASS (txt_page_parent_class)->finalize (gobject);
}

static void txt_page_class_init (TxtPageClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    CtkDocPageClass *page_class = CTK_DOC_PAGE_CLASS (klass);

    gobject_class->finalize = txt_page_finalize;

    page_class->get_size = _txt_page_get_size;
    page_class->text_length = _txt_page_text_length;
    page_class->extract_text = _txt_page_extract_text;
    page_class->render = _txt_page_render;

    g_type_class_add_private (gobject_class,
                              sizeof (TxtPagePrivate));
}

CtkDocPage* txt_page_new (CtkDocument *doc, gint index)
{
    return g_object_new (TXT_TYPE_PAGE,
                         "document", doc,
                         "index", index, NULL);
}
