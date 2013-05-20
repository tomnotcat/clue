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
#include "ctkdocrendertask.h"
#include "ctkdocpage.h"

G_DEFINE_TYPE (CtkDocRenderTask, ctk_doc_render_task, OREN_TYPE_TASK)

enum {
    PROP_0,
    PROP_PAGE,
    PROP_SCALE,
    PROP_ROTATION
};

struct _CtkDocRenderTaskPrivate {
    CtkDocPage *page;
    cairo_surface_t *surface;
    gdouble scale;
    gint rotation;
};

static void ctk_doc_render_task_init (CtkDocRenderTask *self)
{
    CtkDocRenderTaskPrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              CTK_TYPE_DOC_RENDER_TASK,
                                              CtkDocRenderTaskPrivate);
    priv = self->priv;

    priv->surface = NULL;
}

static void ctk_doc_render_task_set_property (GObject *object,
                                              guint prop_id,
                                              const GValue *value,
                                              GParamSpec *pspec)
{
    CtkDocRenderTask *self = CTK_DOC_RENDER_TASK (object);
    CtkDocRenderTaskPrivate *priv = self->priv;

    switch (prop_id) {
    case PROP_PAGE:
        priv->page = g_value_dup_object (value);
        break;

    case PROP_SCALE:
        priv->scale = g_value_get_double (value);
        break;

    case PROP_ROTATION:
        priv->rotation = g_value_get_int (value);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void ctk_doc_render_task_get_property (GObject *object,
                                              guint prop_id,
                                              GValue *value,
                                              GParamSpec *pspec)
{
    CtkDocRenderTask *self = CTK_DOC_RENDER_TASK (object);
    CtkDocRenderTaskPrivate *priv = self->priv;

    switch (prop_id) {
    case PROP_PAGE:
        g_value_set_object (value, priv->page);
        break;

    case PROP_SCALE:
        g_value_set_double (value, priv->scale);
        break;

    case PROP_ROTATION:
        g_value_set_int (value, priv->rotation);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void ctk_doc_render_task_finalize (GObject *gobject)
{
    CtkDocRenderTask *self = CTK_DOC_RENDER_TASK (gobject);
    CtkDocRenderTaskPrivate *priv = self->priv;

    if (priv->surface)
        cairo_surface_destroy (priv->surface);

    if (priv->page)
        g_object_unref (priv->page);

    G_OBJECT_CLASS (ctk_doc_render_task_parent_class)->finalize (gobject);
}

static void ctk_doc_render_task_run (OrenTask *task)
{
    CtkDocRenderTask *self = CTK_DOC_RENDER_TASK (task);
    CtkDocRenderTaskPrivate *priv = self->priv;
    gdouble width, height;
    cairo_matrix_t ctm;

    g_assert (NULL == priv->surface);

    ctk_doc_page_get_size (priv->page, &width, &height);

    if (priv->rotation == 90 || priv->rotation == 270) {
        width = height * priv->scale;
        height = width * priv->scale;
    }
    else {
        width = width * priv->scale;
        height = height * priv->scale;
    }

    cairo_matrix_init_scale (&ctm, priv->scale, priv->scale);
    cairo_matrix_rotate (&ctm, priv->rotation * G_PI / 180.0);

    priv->surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
                                                width + 0.5,
                                                height + 0.5);

    ctk_doc_page_render (priv->page, priv->surface, &ctm, NULL);
}

static void ctk_doc_render_task_class_init (CtkDocRenderTaskClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    OrenTaskClass *task_class = OREN_TASK_CLASS (klass);

    gobject_class->set_property = ctk_doc_render_task_set_property;
    gobject_class->get_property = ctk_doc_render_task_get_property;
    gobject_class->finalize = ctk_doc_render_task_finalize;

    task_class->run = ctk_doc_render_task_run;

    g_type_class_add_private (gobject_class,
                              sizeof (CtkDocRenderTaskPrivate));

    g_object_class_install_property (
        gobject_class, PROP_PAGE,
        g_param_spec_object ("page",
                             "Page",
                             "The page to render",
                             CTK_TYPE_DOC_PAGE,
                             G_PARAM_READABLE |
                             G_PARAM_WRITABLE |
                             G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_SCALE,
        g_param_spec_double ("scale",
                             "Scale",
                             "Current scale factor",
                             0., G_MAXDOUBLE, 1.,
                             G_PARAM_READWRITE |
                             G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_ROTATION,
        g_param_spec_int ("rotation",
                          "Rotation",
                          "Current rotation angle",
                          0, 360, 0,
                          G_PARAM_READWRITE |
                          G_PARAM_STATIC_STRINGS));
}

CtkDocRenderTask* ctk_doc_render_task_new (CtkDocPage *page,
                                           gdouble scale,
                                           gint rotation)
{
    return g_object_new (CTK_TYPE_DOC_RENDER_TASK,
                         "idle-finish", TRUE,
                         "page", page,
                         "scale", scale,
                         "rotation", rotation, NULL);
}

/**
 * ctk_doc_render_task_get_page:
 * @self: a #CtkDocRenderTask
 *
 * Get the document page of the task.
 *
 * Returns: (allow-none) (transfer none): a #CtkDocPage
 */
CtkDocPage* ctk_doc_render_task_get_page (CtkDocRenderTask *self)
{
    g_return_val_if_fail (CTK_IS_DOC_RENDER_TASK (self), NULL);

    return self->priv->page;
}

/**
 * ctk_doc_render_task_get_surface:
 * @self: a #CtkDocRenderTask
 *
 * Get the rendered pixbuf of the task.
 *
 * Returns: (allow-none) (transfer none): a #cairo_surface_t
 */
cairo_surface_t* ctk_doc_render_task_get_surface (CtkDocRenderTask *self)
{
    g_return_val_if_fail (CTK_IS_DOC_RENDER_TASK (self), NULL);

    return self->priv->surface;
}
