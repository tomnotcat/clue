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
#include "ctkdocmodel.h"
#include "ctkdocument.h"
#include "ctkmarshal.h"

G_DEFINE_TYPE (CtkDocModel, ctk_doc_model, G_TYPE_OBJECT)

enum {
    PROP_0,
    PROP_DOCUMENT,
    PROP_PAGE,
    PROP_ROTATION,
    PROP_SCALE,
    PROP_SIZING_MODE,
    PROP_CONTINUOUS,
    PROP_DUAL_PAGE,
    PROP_DUAL_PAGE_ODD_LEFT
};

enum {
    SIG_PAGECHANGED,
    LAST_SIGNAL
};

struct _CtkDocModelPrivate {
    CtkDocument *document;
    gint n_pages;
    gint page;
    gint rotation;
    gdouble scale;
    gdouble max_scale;
    gdouble min_scale;
    CtkSizingMode sizing_mode;
    guint continuous : 1;
    guint dual_page  : 1;
    guint dual_page_odd_left : 1;
};

static guint signals[LAST_SIGNAL] = { 0 };

static void ctk_doc_model_init (CtkDocModel *self)
{
    CtkDocModelPrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              CTK_TYPE_DOC_MODEL,
                                              CtkDocModelPrivate);
    priv = self->priv;

    priv->page = -1;
    priv->scale = 1.;
    priv->sizing_mode = CTK_SIZING_FIT_WIDTH;
    priv->continuous = TRUE;
    priv->min_scale = 0.;
    priv->max_scale = G_MAXDOUBLE;
}

static void ctk_doc_model_set_property (GObject *object,
                                        guint prop_id,
                                        const GValue *value,
                                        GParamSpec *pspec)
{
    CtkDocModel *self = CTK_DOC_MODEL (object);

    switch (prop_id) {
    case PROP_DOCUMENT:
        ctk_doc_model_set_document (self, (CtkDocument *)g_value_get_object (value));
        break;

    case PROP_PAGE:
        ctk_doc_model_set_page (self, g_value_get_int (value));
        break;

    case PROP_ROTATION:
        ctk_doc_model_set_rotation (self, g_value_get_int (value));
        break;

    case PROP_SCALE:
        ctk_doc_model_set_scale (self, g_value_get_double (value));
        break;

    case PROP_SIZING_MODE:
        ctk_doc_model_set_sizing_mode (self, g_value_get_enum (value));
        break;

    case PROP_CONTINUOUS:
        ctk_doc_model_set_continuous (self, g_value_get_boolean (value));
        break;

    case PROP_DUAL_PAGE:
        ctk_doc_model_set_dual_page (self, g_value_get_boolean (value));
        break;

    case PROP_DUAL_PAGE_ODD_LEFT:
        ctk_doc_model_set_dual_page_odd_pages_left (self, g_value_get_boolean (value));
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void ctk_doc_model_get_property (GObject *object,
                                        guint prop_id,
                                        GValue *value,
                                        GParamSpec *pspec)
{
    CtkDocModel *self = CTK_DOC_MODEL (object);
    CtkDocModelPrivate *priv = self->priv;

    switch (prop_id) {
    case PROP_DOCUMENT:
        g_value_set_object (value, priv->document);
        break;

    case PROP_PAGE:
        g_value_set_int (value, priv->page);
        break;

    case PROP_ROTATION:
        g_value_set_int (value, priv->rotation);
        break;

    case PROP_SCALE:
        g_value_set_double (value, priv->scale);
        break;

    case PROP_SIZING_MODE:
        g_value_set_enum (value, priv->sizing_mode);
        break;

    case PROP_CONTINUOUS:
        g_value_set_boolean (value, priv->continuous);
        break;

    case PROP_DUAL_PAGE:
        g_value_set_boolean (value, priv->dual_page);
        break;

    case PROP_DUAL_PAGE_ODD_LEFT:
        g_value_set_boolean (value, priv->dual_page_odd_left);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void ctk_doc_model_finalize (GObject *gobject)
{
    CtkDocModel *self = CTK_DOC_MODEL (gobject);
    CtkDocModelPrivate *priv = self->priv;

    if (priv->document)
        g_object_unref (priv->document);

    G_OBJECT_CLASS (ctk_doc_model_parent_class)->finalize (gobject);
}

static void ctk_doc_model_class_init (CtkDocModelClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->set_property = ctk_doc_model_set_property;
    gobject_class->get_property = ctk_doc_model_get_property;
    gobject_class->finalize = ctk_doc_model_finalize;

    g_type_class_add_private (gobject_class,
                              sizeof (CtkDocModelPrivate));

    /* Properties */
    g_object_class_install_property (
        gobject_class, PROP_DOCUMENT,
        g_param_spec_object ("document",
                             "Document",
                             "The current document",
                             CTK_TYPE_DOCUMENT,
                             G_PARAM_READWRITE |
                             G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_PAGE,
        g_param_spec_int ("page",
                          "Page",
                          "Current page",
                          -1, G_MAXINT, -1,
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

    g_object_class_install_property (
        gobject_class, PROP_SCALE,
        g_param_spec_double ("scale",
                             "Scale",
                             "Current scale factor",
                             0., G_MAXDOUBLE, 1.,
                             G_PARAM_READWRITE |
                             G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_SIZING_MODE,
        g_param_spec_enum ("sizing-mode",
                           "Sizing Mode",
                           "Current sizing mode",
                           CTK_TYPE_SIZING_MODE,
                           CTK_SIZING_FIT_WIDTH,
                           G_PARAM_READWRITE |
                           G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_CONTINUOUS,
        g_param_spec_boolean ("continuous",
                              "Continuous",
                              "Whether document is displayed in continuous mode",
                              TRUE,
                              G_PARAM_READWRITE |
                              G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_DUAL_PAGE,
        g_param_spec_boolean ("dual-page",
                              "Dual Page",
                              "Whether document is displayed in dual page mode",
                              FALSE,
                              G_PARAM_READWRITE |
                              G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_DUAL_PAGE_ODD_LEFT,
        g_param_spec_boolean ("dual-odd-left",
                              "Odd Pages Left",
                              "Whether odd pages are displayed on left side in dual mode",
                              FALSE,
                              G_PARAM_READWRITE |
                              G_PARAM_STATIC_STRINGS));

    /* Signals */
    signals[SIG_PAGECHANGED] =
            g_signal_new ("page-changed",
                          CTK_TYPE_DOC_MODEL,
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (CtkDocModelClass, page_changed),
                          NULL, NULL,
                          ctk_marshal_VOID__INT_INT,
                          G_TYPE_NONE,
                          2,
                          G_TYPE_INT,
                          G_TYPE_INT);
}

CtkDocModel* ctk_doc_model_new (void)
{
    return g_object_new (CTK_TYPE_DOC_MODEL, NULL);
}

CtkDocModel* ctk_doc_model_new_with_document (CtkDocument *document)
{
    g_return_val_if_fail (CTK_IS_DOCUMENT (document), NULL);

    return g_object_new (CTK_TYPE_DOC_MODEL,
                         "document", document, NULL);
}

void ctk_doc_model_set_document (CtkDocModel *self,
                                 CtkDocument *document)
{
    CtkDocModelPrivate *priv;

    g_return_if_fail (CTK_IS_DOC_MODEL (self));
    g_return_if_fail (CTK_IS_DOCUMENT (document));

    priv = self->priv;

    if (document == priv->document)
        return;

    if (priv->document)
        g_object_unref (priv->document);

    priv->document = g_object_ref (document);

    priv->n_pages = ctk_document_count_pages (document);
    ctk_doc_model_set_page (self, CLAMP (priv->page,
                                         0,
                                         priv->n_pages - 1));

    g_object_notify (G_OBJECT (self), "document");
}

/**
 * ctk_doc_model_get_document:
 * @self: a #CtkDocModel
 *
 * Get the document of the model.
 *
 * Returns: (allow-none) (transfer none): a #CtkDocument
 */
CtkDocument *ctk_doc_model_get_document (CtkDocModel *self)
{
    g_return_val_if_fail (CTK_IS_DOC_MODEL (self), NULL);

    return self->priv->document;
}

void ctk_doc_model_set_page (CtkDocModel *self,
                             gint page)
{
    CtkDocModelPrivate *priv;
    gint old_page;

    g_return_if_fail (CTK_IS_DOC_MODEL (self));

    priv = self->priv;

    if (priv->page == page)
        return;

    if (page < 0 || (priv->document && page >= priv->n_pages))
        return;

    old_page = priv->page;
    priv->page = page;
    g_signal_emit (self, signals[SIG_PAGECHANGED], 0, old_page, page);

    g_object_notify (G_OBJECT (self), "page");
}

gint ctk_doc_model_get_page (CtkDocModel *self)
{
    g_return_val_if_fail (CTK_IS_DOC_MODEL (self), -1);

    return self->priv->page;
}

void ctk_doc_model_set_scale (CtkDocModel *self,
                              gdouble scale)
{
    CtkDocModelPrivate *priv;

    g_return_if_fail (CTK_IS_DOC_MODEL (self));

    priv = self->priv;

    scale = CLAMP (scale,
                   priv->sizing_mode == CTK_SIZING_FREE ?
                   priv->min_scale : 0, priv->max_scale);

    if (scale == priv->scale)
        return;

    priv->scale = scale;

    g_object_notify (G_OBJECT (self), "scale");
}

gdouble ctk_doc_model_get_scale (CtkDocModel *self)
{
    g_return_val_if_fail (CTK_IS_DOC_MODEL (self), 1.0);

    return self->priv->scale;
}

void ctk_doc_model_set_max_scale (CtkDocModel *self,
                                  gdouble max_scale)
{
    CtkDocModelPrivate *priv;

    g_return_if_fail (CTK_IS_DOC_MODEL (self));

    priv = self->priv;

    if (max_scale == priv->max_scale)
        return;

    priv->max_scale = max_scale;

    if (priv->scale > max_scale)
        ctk_doc_model_set_scale (self, max_scale);
}

gdouble ctk_doc_model_get_max_scale (CtkDocModel *self)
{
    g_return_val_if_fail (CTK_IS_DOC_MODEL (self), 1.0);

    return self->priv->max_scale;
}

void ctk_doc_model_set_min_scale (CtkDocModel *self,
                                  gdouble min_scale)
{
    CtkDocModelPrivate *priv;

    g_return_if_fail (CTK_IS_DOC_MODEL (self));

    priv = self->priv;

    if (min_scale == priv->min_scale)
        return;

    priv->min_scale = min_scale;

    if (priv->scale < min_scale)
        ctk_doc_model_set_scale (self, min_scale);
}

gdouble ctk_doc_model_get_min_scale (CtkDocModel *self)
{
    g_return_val_if_fail (CTK_IS_DOC_MODEL (self), 0.);

    return self->priv->min_scale;
}

void ctk_doc_model_set_sizing_mode (CtkDocModel *self,
                                    CtkSizingMode mode)
{
    CtkDocModelPrivate *priv;

    g_return_if_fail (CTK_IS_DOC_MODEL (self));

    priv = self->priv;

    if (mode == priv->sizing_mode)
        return;

    priv->sizing_mode = mode;

    g_object_notify (G_OBJECT (self), "sizing-mode");
}

CtkSizingMode ctk_doc_model_get_sizing_mode (CtkDocModel *self)
{
    g_return_val_if_fail (CTK_IS_DOC_MODEL (self), CTK_SIZING_FIT_WIDTH);

    return self->priv->sizing_mode;
}

void ctk_doc_model_set_rotation (CtkDocModel *self,
                                 gint rotation)
{
    CtkDocModelPrivate *priv;

    g_return_if_fail (CTK_IS_DOC_MODEL (self));

    priv = self->priv;

    if (rotation >= 360)
        rotation -= 360;
    else if (rotation < 0)
        rotation += 360;

    if (rotation == priv->rotation)
        return;

    priv->rotation = rotation;

    g_object_notify (G_OBJECT (self), "rotation");
}

gint ctk_doc_model_get_rotation (CtkDocModel *self)
{
    g_return_val_if_fail (CTK_IS_DOC_MODEL (self), 0);

    return self->priv->rotation;
}

void ctk_doc_model_set_continuous (CtkDocModel *self,
                                   gboolean continuous)
{
    CtkDocModelPrivate *priv;

    g_return_if_fail (CTK_IS_DOC_MODEL (self));

    priv = self->priv;

    continuous = (continuous != FALSE);

    if (continuous == priv->continuous)
        return;

    priv->continuous = continuous;

    g_object_notify (G_OBJECT (self), "continuous");
}

gboolean ctk_doc_model_get_continuous (CtkDocModel *self)
{
    g_return_val_if_fail (CTK_IS_DOC_MODEL (self), TRUE);

    return self->priv->continuous;
}

void ctk_doc_model_set_dual_page (CtkDocModel *self,
                                  gboolean dual_page)
{
    CtkDocModelPrivate *priv;

    g_return_if_fail (CTK_IS_DOC_MODEL (self));

    priv = self->priv;

    dual_page = dual_page != FALSE;

    if (dual_page == priv->dual_page)
        return;

    priv->dual_page = dual_page;

    g_object_notify (G_OBJECT (self), "dual-page");

    if (dual_page && priv->dual_page_odd_left) {
        priv->dual_page_odd_left = FALSE;
        g_object_notify (G_OBJECT (self), "dual-odd-left");
    }
}

gboolean ctk_doc_model_get_dual_page (CtkDocModel *self)
{
    g_return_val_if_fail (CTK_IS_DOC_MODEL (self), FALSE);

    return self->priv->dual_page;
}

void ctk_doc_model_set_dual_page_odd_pages_left (CtkDocModel *self,
                                                 gboolean odd_left)
{
    CtkDocModelPrivate *priv;

    g_return_if_fail (CTK_IS_DOC_MODEL (self));

    priv = self->priv;

    odd_left = odd_left != FALSE;

    if (odd_left == priv->dual_page_odd_left)
        return;

    priv->dual_page_odd_left = odd_left;

    g_object_notify (G_OBJECT (self), "dual-odd-left");

    if (odd_left && priv->dual_page) {
        priv->dual_page = FALSE;
        g_object_notify (G_OBJECT (self), "dual-page");
    }
}

gboolean ctk_doc_model_get_dual_page_odd_pages_left (CtkDocModel *self)
{
    g_return_val_if_fail (CTK_IS_DOC_MODEL (self), FALSE);

    return self->priv->dual_page_odd_left;
}
