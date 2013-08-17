/* img-view.c
 *
 * Copyright (C) 2013 Christian Hergert <christian@hergert.me>
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cairo-xlib.h>
#include <glib/gi18n.h>

#include "img-view.h"

static void gtk_scrollable_init (GtkScrollableInterface *iface);

G_DEFINE_TYPE_EXTENDED(ImgView,
                       img_view,
                       GTK_TYPE_DRAWING_AREA,
                       0,
                       G_IMPLEMENT_INTERFACE(GTK_TYPE_SCROLLABLE,
                                             gtk_scrollable_init))

struct _ImgViewPrivate
{
   cairo_surface_t *surface;
   GtkAdjustment   *hadjustment;
   GtkAdjustment   *vadjustment;
   guint            hadjustment_handler;
   guint            vadjustment_handler;
   guint            vadj_value;
};

enum
{
   PROP_0,
   PROP_HADJUSTMENT,
   PROP_HSCROLL_POLICY,
   PROP_VADJUSTMENT,
   PROP_VSCROLL_POLICY,
   PROP_SURFACE,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

ImgView *
img_view_new (void)
{
   return g_object_new(IMG_TYPE_VIEW, NULL);
}

cairo_surface_t *
img_view_get_surface (ImgView *view)
{
   g_return_val_if_fail(IMG_IS_VIEW(view), NULL);
   return view->priv->surface;
}

void
img_view_set_surface (ImgView         *view,
                      cairo_surface_t *surface)
{
   g_return_if_fail(IMG_IS_VIEW(view));
   g_clear_pointer(&view->priv->surface, cairo_surface_destroy);
   view->priv->surface = surface ? cairo_surface_reference(surface) : NULL;
   gtk_widget_queue_resize(GTK_WIDGET(view));
}

static void
vadj_changed (GtkAdjustment *adjustment,
              ImgView       *view)
{
   ImgViewPrivate *priv = view->priv;
   GdkWindow *window;
   guint value;

   window = gtk_widget_get_window(GTK_WIDGET(view));
   value = gtk_adjustment_get_value(adjustment);
   //g_print("value: %d\n", value);
   gdk_window_scroll(window, 0, priv->vadj_value - value);
   priv->vadj_value = value;
}

static void
value_changed (GtkAdjustment *adjustment,
               ImgView       *view)
{
   gtk_widget_queue_draw(GTK_WIDGET(view));
}

static void
img_view_set_vadjustment (ImgView       *view,
                          GtkAdjustment *vadjustment)
{
   g_return_if_fail(IMG_IS_VIEW(view));
   g_return_if_fail(!vadjustment || GTK_IS_ADJUSTMENT(vadjustment));

   if (view->priv->vadjustment_handler) {
      g_signal_handler_disconnect(view->priv->vadjustment,
                                  view->priv->vadjustment_handler);
      view->priv->vadjustment_handler = 0;
   }

   g_clear_object(&view->priv->vadjustment);

   if (vadjustment) {
      view->priv->vadjustment = g_object_ref(vadjustment);
   } else {
      view->priv->vadjustment = gtk_adjustment_new(0, 0, 0, 1, 1, 1);
   }

   view->priv->vadjustment_handler =
      g_signal_connect(view->priv->vadjustment,
                       "value-changed",
                       G_CALLBACK(vadj_changed),
                       view);

   gtk_widget_queue_resize(GTK_WIDGET(view));
}

static void
img_view_set_hadjustment (ImgView       *view,
                          GtkAdjustment *hadjustment)
{
   g_return_if_fail(IMG_IS_VIEW(view));
   g_return_if_fail(!hadjustment || GTK_IS_ADJUSTMENT(hadjustment));

   if (view->priv->hadjustment_handler) {
      g_signal_handler_disconnect(view->priv->hadjustment,
                                  view->priv->hadjustment_handler);
      view->priv->hadjustment_handler = 0;
   }

   g_clear_object(&view->priv->hadjustment);

   if (hadjustment) {
      view->priv->hadjustment = g_object_ref(hadjustment);
   } else {
      view->priv->hadjustment = gtk_adjustment_new(0, 0, 0, 0, 0, 0);
   }

   view->priv->hadjustment_handler =
      g_signal_connect(view->priv->hadjustment,
                       "value-changed",
                       G_CALLBACK(value_changed),
                       view);

   gtk_widget_queue_resize(GTK_WIDGET(view));
}

static void
img_view_size_allocate (GtkWidget     *widget,
                        GtkAllocation *allocation)
{
   ImgViewPrivate *priv = ((ImgView *)widget)->priv;
   int width;
   int height;

   GTK_WIDGET_CLASS(img_view_parent_class)->size_allocate(widget, allocation);

   if (priv->surface) {
      if (cairo_surface_get_type(priv->surface) == CAIRO_SURFACE_TYPE_IMAGE) {
         width = cairo_image_surface_get_width(priv->surface);
         height = cairo_image_surface_get_height(priv->surface);
      } else if (cairo_surface_get_type(priv->surface) == CAIRO_SURFACE_TYPE_XLIB) {
         width = cairo_xlib_surface_get_width(priv->surface);
         height = cairo_xlib_surface_get_height(priv->surface);
      } else {
         /* TODO: Generic surface size functions? */
         g_assert_not_reached();
      }

      gtk_adjustment_set_page_size(priv->vadjustment, allocation->height);
      gtk_adjustment_set_page_increment(priv->vadjustment, MAX(1, allocation->height * 0.75));
      gtk_adjustment_set_step_increment(priv->vadjustment, 1);
      gtk_adjustment_set_upper(priv->vadjustment, height);

      gtk_adjustment_set_page_size(priv->hadjustment, allocation->width);
      gtk_adjustment_set_page_increment(priv->hadjustment, MAX(1, allocation->width * 0.75));
      gtk_adjustment_set_step_increment(priv->hadjustment, 1);
      gtk_adjustment_set_upper(priv->hadjustment, width);
   }
}

static gboolean
img_view_draw (GtkWidget *widget,
               cairo_t   *cr)
{
   ImgViewPrivate *priv = IMG_VIEW(widget)->priv;
   GtkAllocation alloc;
   GdkWindow *window = gtk_widget_get_window(widget);
   //int xvalue;
   int yvalue;

   if (gtk_cairo_should_draw_window(cr, window) && priv->surface) {
      gtk_widget_get_allocation(widget, &alloc);

      //xvalue = gtk_adjustment_get_value(priv->hadjustment);
      yvalue = gtk_adjustment_get_value(priv->vadjustment);

      cairo_save(cr);
      cairo_rectangle(cr, 0, 0, alloc.width, alloc.height);
      cairo_set_source_surface(cr, priv->surface, 0, -yvalue);
      cairo_fill(cr);
      cairo_restore(cr);
   }

   return FALSE;
}

static void
img_view_realize (GtkWidget *widget)
{
   GTK_WIDGET_CLASS(img_view_parent_class)->realize(widget);
}

static void
img_view_finalize (GObject *object)
{
   ImgViewPrivate *priv = IMG_VIEW(object)->priv;

   if (priv->vadjustment_handler) {
      g_signal_handler_disconnect(priv->vadjustment,
                                  priv->vadjustment_handler);
      priv->vadjustment_handler = 0;
   }

   if (priv->hadjustment_handler) {
      g_signal_handler_disconnect(priv->hadjustment,
                                  priv->hadjustment_handler);
      priv->hadjustment_handler = 0;
   }

   g_clear_pointer(&priv->surface, cairo_surface_destroy);
   g_clear_object(&priv->hadjustment);
   g_clear_object(&priv->vadjustment);

   G_OBJECT_CLASS(img_view_parent_class)->finalize(object);
}

static void
img_view_get_property (GObject    *object,
                       guint       prop_id,
                       GValue     *value,
                       GParamSpec *pspec)
{
   ImgView *view = IMG_VIEW(object);

   switch (prop_id) {
   case PROP_VSCROLL_POLICY:
   case PROP_HSCROLL_POLICY:
      break;
   case PROP_HADJUSTMENT:
      g_value_set_object(value, view->priv->hadjustment);
      break;
   case PROP_VADJUSTMENT:
      g_value_set_object(value, view->priv->vadjustment);
      break;
   case PROP_SURFACE:
      g_value_set_boxed(value, img_view_get_surface(view));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
img_view_set_property (GObject      *object,
                       guint         prop_id,
                       const GValue *value,
                       GParamSpec   *pspec)
{
   ImgView *view = IMG_VIEW(object);

   switch (prop_id) {
   case PROP_VSCROLL_POLICY:
   case PROP_HSCROLL_POLICY:
      break;
   case PROP_HADJUSTMENT:
      img_view_set_hadjustment(view, g_value_get_object(value));
      break;
   case PROP_VADJUSTMENT:
      img_view_set_vadjustment(view, g_value_get_object(value));
      break;
   case PROP_SURFACE:
      img_view_set_surface(view, g_value_get_boxed(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
img_view_class_init (ImgViewClass *klass)
{
   GObjectClass *object_class;
   GtkWidgetClass *widget_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = img_view_finalize;
   object_class->get_property = img_view_get_property;
   object_class->set_property = img_view_set_property;
   g_type_class_add_private(object_class, sizeof(ImgViewPrivate));

   widget_class = GTK_WIDGET_CLASS(klass);
   widget_class->size_allocate = img_view_size_allocate;
   widget_class->draw = img_view_draw;
   widget_class->realize = img_view_realize;

   gParamSpecs[PROP_SURFACE] =
      g_param_spec_boxed("surface",
                         _("Surface"),
                         _("The cairo surface."),
                         CAIRO_GOBJECT_TYPE_SURFACE,
                         (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_SURFACE,
                                   gParamSpecs[PROP_SURFACE]);

   gParamSpecs[PROP_HADJUSTMENT] =
      g_param_spec_object("hadjustment",
                          _("HAdjustment"),
                          _("HAdjustment"),
                          GTK_TYPE_ADJUSTMENT,
                          (G_PARAM_READWRITE |
                           G_PARAM_STATIC_STRINGS |
                           G_PARAM_CONSTRUCT));
   g_object_class_install_property(object_class, PROP_HADJUSTMENT,
                                   gParamSpecs[PROP_HADJUSTMENT]);

   gParamSpecs[PROP_VADJUSTMENT] =
      g_param_spec_object("vadjustment",
                          _("HAdjustment"),
                          _("HAdjustment"),
                          GTK_TYPE_ADJUSTMENT,
                          (G_PARAM_READWRITE |
                           G_PARAM_STATIC_STRINGS |
                           G_PARAM_CONSTRUCT));
   g_object_class_install_property(object_class, PROP_VADJUSTMENT,
                                   gParamSpecs[PROP_VADJUSTMENT]);

   gParamSpecs[PROP_HSCROLL_POLICY] =
      g_param_spec_enum("hscroll-policy",
                        _("hscroll policy"),
                        _("hscroll policy."),
                        GTK_TYPE_SCROLLABLE_POLICY,
                        GTK_SCROLL_MINIMUM,
                        (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_HSCROLL_POLICY,
                                   gParamSpecs[PROP_HSCROLL_POLICY]);

   gParamSpecs[PROP_VSCROLL_POLICY] =
      g_param_spec_enum("vscroll-policy",
                        _("vscroll policy"),
                        _("vscroll policy."),
                        GTK_TYPE_SCROLLABLE_POLICY,
                        GTK_SCROLL_MINIMUM,
                        (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_VSCROLL_POLICY,
                                   gParamSpecs[PROP_VSCROLL_POLICY]);
}

static void
img_view_init (ImgView *view)
{
   view->priv = G_TYPE_INSTANCE_GET_PRIVATE(view,
                                            IMG_TYPE_VIEW,
                                            ImgViewPrivate);

   gtk_widget_add_events(GTK_WIDGET(view), GDK_SMOOTH_SCROLL_MASK);
}

static void
gtk_scrollable_init (GtkScrollableInterface *iface)
{
}
