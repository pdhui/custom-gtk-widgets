/* gb-anim-bin.c
 *
 * Copyright (C) 2011 Christian Hergert <chris@dronelabs.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib/gi18n.h>

#include "gb-anim-bin.h"
#include "gb-animation.h"

G_DEFINE_TYPE_EXTENDED(GbAnimBin, gb_anim_bin, GTK_TYPE_EVENT_BOX, 0,
                       G_IMPLEMENT_INTERFACE(GTK_TYPE_ORIENTABLE,
                                             NULL))

struct _GbAnimBinPrivate
{
   GbAnimation *animation;
   GbAnimationMode mode;
   guint duration;
   guint fps;
   guint last_child_height;
   guint last_child_width;
   GtkOrientation orientation;
};

enum
{
   PROP_0,
   PROP_DURATION,
   PROP_FRAME_RATE,
   PROP_ORIENTATION,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

GtkWidget *
gb_anim_bin_new (void)
{
   return g_object_new(GB_TYPE_ANIM_BIN,
                       "orientation", GTK_ORIENTATION_VERTICAL,
                       NULL);
}

guint
gb_anim_bin_get_duration (GbAnimBin *bin)
{
   g_return_val_if_fail(GB_IS_ANIM_BIN(bin), 0);
   return bin->priv->duration;
}

void
gb_anim_bin_set_duration (GbAnimBin *bin,
                          guint      duration)
{
   g_return_if_fail(GB_IS_ANIM_BIN(bin));
   bin->priv->duration = duration;
   g_object_notify_by_pspec(G_OBJECT(bin), gParamSpecs[PROP_DURATION]);
}

guint
gb_anim_bin_get_frame_rate (GbAnimBin *bin)
{
   g_return_val_if_fail(GB_IS_ANIM_BIN(bin), 0);
   return bin->priv->fps;
}

void
gb_anim_bin_set_frame_rate (GbAnimBin *bin,
                            guint      frame_rate)
{
   g_return_if_fail(GB_IS_ANIM_BIN(bin));
   bin->priv->fps = frame_rate;
   g_object_notify_by_pspec(G_OBJECT(bin), gParamSpecs[PROP_FRAME_RATE]);
}

static void
gb_anim_bin_cancel_animation (GbAnimBin *bin)
{
   GbAnimBinPrivate *priv;
   GbAnimation *animation;

   g_return_if_fail(GB_IS_ANIM_BIN(bin));

   priv = bin->priv;

   if ((animation = priv->animation)) {
      priv->animation = NULL;
      g_object_remove_weak_pointer(G_OBJECT(animation),
                                   (gpointer *)&priv->animation);
   }
}

static void
gb_anim_bin_hide_done (GtkWidget *widget)
{
   GbAnimBinPrivate *priv;
   GbAnimBin *bin = (GbAnimBin *)widget;

   g_return_if_fail(GB_IS_ANIM_BIN(bin));

   priv = bin->priv;

   GTK_WIDGET_CLASS(gb_anim_bin_parent_class)->hide(widget);

   if (priv->orientation == GTK_ORIENTATION_VERTICAL) {
      priv->last_child_height = 0;
      g_object_set(widget, "height-request", -1, NULL);
   } else {
      priv->last_child_width = 0;
      g_object_set(widget, "width-request", -1, NULL);
   }

   g_object_unref(widget);
}

static void
gb_anim_bin_hide (GtkWidget *widget)
{
   GbAnimBinPrivate *priv;
   GtkAllocation alloc;
   GbAnimBin *bin = (GbAnimBin *)widget;
   GtkWidget *child;

   g_return_if_fail(GB_IS_ANIM_BIN(bin));

   priv = bin->priv;

   gb_anim_bin_cancel_animation(bin);

   if ((child = gtk_bin_get_child(GTK_BIN(bin)))) {
      gtk_widget_get_allocation(child, &alloc);

      if (priv->orientation == GTK_ORIENTATION_VERTICAL) {
         priv->last_child_height = alloc.height;
      } else {
         priv->last_child_width = alloc.width;
      }

      gtk_widget_get_allocation(widget, &alloc);

      if (priv->orientation == GTK_ORIENTATION_VERTICAL) {
         g_object_set(widget, "height-request", alloc.height, NULL);
      } else {
         g_object_set(widget, "width-request", alloc.width, NULL);
      }

      priv->animation =
         gb_object_animate_full(widget, priv->mode, priv->duration, priv->fps,
                                (GDestroyNotify)gb_anim_bin_hide_done,
                                g_object_ref(widget),
                                (priv->orientation == GTK_ORIENTATION_VERTICAL) ? "height-request" : "width-request", 0,
                                NULL);
      g_object_add_weak_pointer(G_OBJECT(priv->animation),
                                (gpointer *)&priv->animation);
   } else {
      GTK_WIDGET_CLASS(gb_anim_bin_parent_class)->hide(widget);
   }
}

static void
gb_anim_bin_show_done (GtkWidget *widget)
{
   GbAnimBinPrivate *priv;
   GbAnimBin *bin = (GbAnimBin *)widget;

   g_return_if_fail(GB_IS_ANIM_BIN(widget));

   priv = bin->priv;

   if (priv->orientation == GTK_ORIENTATION_VERTICAL) {
      g_object_set(widget, "height-request", -1, NULL);
   } else {
      g_object_set(widget, "width-request", -1, NULL);
   }

   g_object_unref(widget);
}

static void
gb_anim_bin_show (GtkWidget *widget)
{
   GbAnimBinPrivate *priv;
   GbAnimBin *bin = (GbAnimBin *)widget;
   GtkWidget *child;
   gint value;

   g_return_if_fail(GB_IS_ANIM_BIN(bin));

   priv = bin->priv;

   gb_anim_bin_cancel_animation(bin);

   if ((child = gtk_bin_get_child(GTK_BIN(bin)))) {
      if (priv->orientation == GTK_ORIENTATION_VERTICAL) {
         g_object_set(widget, "height-request", 0, NULL);
      } else {
         g_object_set(widget, "width-request", 0, NULL);
      }

      GTK_WIDGET_CLASS(gb_anim_bin_parent_class)->show(widget);

      if (priv->orientation == GTK_ORIENTATION_VERTICAL) {
         gtk_widget_get_preferred_height(child, NULL, &value);
      } else {
         gtk_widget_get_preferred_width(child, NULL, &value);
      }

      priv->animation =
         gb_object_animate_full(widget, priv->mode, priv->duration, priv->fps,
                                (GDestroyNotify)gb_anim_bin_show_done,
                                g_object_ref(widget),
                                (priv->orientation == GTK_ORIENTATION_VERTICAL) ? "height-request" : "width-request", value,
                                NULL);
      g_object_add_weak_pointer(G_OBJECT(priv->animation),
                                (gpointer *)&priv->animation);
   } else {
      GTK_WIDGET_CLASS(gb_anim_bin_parent_class)->show(widget);
   }
}

static void
gb_anim_bin_get_preferred_height (GtkWidget *widget,
                                  gint      *min_height,
                                  gint      *natural_height)
{
   GbAnimBinPrivate *priv = GB_ANIM_BIN(widget)->priv;
   GtkWidget *child;
   gint height;

   if (priv->orientation == GTK_ORIENTATION_VERTICAL) {
      g_object_get(widget, "height-request", &height, NULL);
      if (height < 0) {
         if ((child = gtk_bin_get_child(GTK_BIN(widget)))) {
            gtk_widget_get_preferred_height(child, min_height, natural_height);
            return;
         }
      }
      if (min_height) {
         *min_height = height;
      }
      if (natural_height) {
         *natural_height = height;
      }
   } else {
      GTK_WIDGET_CLASS(gb_anim_bin_parent_class)->
         get_preferred_height(widget, min_height, natural_height);
   }
}

static void
gb_anim_bin_get_preferred_width (GtkWidget *widget,
                                 gint      *min_width,
                                 gint      *natural_width)
{
   GbAnimBinPrivate *priv = GB_ANIM_BIN(widget)->priv;
   GtkWidget *child;
   gint width;

   if (priv->orientation == GTK_ORIENTATION_HORIZONTAL) {
      g_object_get(widget, "width-request", &width, NULL);
      if (width < 0) {
         if ((child = gtk_bin_get_child(GTK_BIN(widget)))) {
            gtk_widget_get_preferred_width(child, min_width, natural_width);
            return;
         }
      }
      if (min_width) {
         *min_width = width;
      }
      if (natural_width) {
         *natural_width = width;
      }
   } else {
      GTK_WIDGET_CLASS(gb_anim_bin_parent_class)->
         get_preferred_width(widget, min_width, natural_width);
   }
}

static void
gb_anim_bin_size_allocate (GtkWidget     *widget,
                           GtkAllocation *alloc)
{
   GbAnimBinPrivate *priv;
   GtkAllocation child_alloc;
   GbAnimBin *bin = (GbAnimBin *)widget;
   GtkWidget *child;

   g_return_if_fail(GB_IS_ANIM_BIN(bin));

   priv = bin->priv;

   GTK_WIDGET_CLASS(gb_anim_bin_parent_class)->size_allocate(widget, alloc);

   if ((child = gtk_bin_get_child(GTK_BIN(bin)))) {
      if (!gtk_widget_get_has_window(widget)) {
         child_alloc.x = alloc->x;
         child_alloc.y = alloc->y;
      } else {
         child_alloc.x = 0;
         child_alloc.y = 0;
      }
      child_alloc.width = alloc->width;
      child_alloc.height = alloc->height;
      if ((priv->orientation == GTK_ORIENTATION_VERTICAL) && priv->last_child_height) {
         child_alloc.height = priv->last_child_height;
         gtk_widget_size_allocate(child, &child_alloc);
      } else if ((priv->orientation == GTK_ORIENTATION_HORIZONTAL) && priv->last_child_width) {
         child_alloc.width = priv->last_child_width;
         gtk_widget_size_allocate(child, &child_alloc);
      }
   }
}

static GtkOrientation
gb_anim_bin_get_orientation (GbAnimBin *bin)
{
   g_return_val_if_fail(GB_IS_ANIM_BIN(bin), 0);
   return bin->priv->orientation;
}

static void
gb_anim_bin_set_orientation (GbAnimBin      *bin,
                             GtkOrientation  orientation)
{
   g_return_if_fail(GB_IS_ANIM_BIN(bin));

   if (bin->priv->animation) {
      g_warning("Cannot change orientation while in an animation!");
      return;
   }

   bin->priv->orientation = orientation;
   g_object_notify(G_OBJECT(bin), "orientation");
}

static void
gb_anim_bin_finalize (GObject *object)
{
   G_OBJECT_CLASS(gb_anim_bin_parent_class)->finalize(object);
}

static void
gb_anim_bin_get_property (GObject    *object,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
   GbAnimBin *bin = GB_ANIM_BIN(object);

   switch (prop_id) {
   case PROP_DURATION:
      g_value_set_uint(value, gb_anim_bin_get_duration(bin));
      break;
   case PROP_FRAME_RATE:
      g_value_set_uint(value, gb_anim_bin_get_frame_rate(bin));
      break;
   case PROP_ORIENTATION:
      g_value_set_enum(value, gb_anim_bin_get_orientation(bin));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_anim_bin_set_property (GObject      *object,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
   GbAnimBin *bin = GB_ANIM_BIN(object);

   switch (prop_id) {
   case PROP_DURATION:
      gb_anim_bin_set_duration(bin, g_value_get_uint(value));
      break;
   case PROP_FRAME_RATE:
      gb_anim_bin_set_frame_rate(bin, g_value_get_uint(value));
      break;
   case PROP_ORIENTATION:
      gb_anim_bin_set_orientation(bin, g_value_get_enum(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_anim_bin_class_init (GbAnimBinClass *klass)
{
   GObjectClass *object_class;
   GtkWidgetClass *widget_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_anim_bin_finalize;
   object_class->get_property = gb_anim_bin_get_property;
   object_class->set_property = gb_anim_bin_set_property;
   g_type_class_add_private(object_class, sizeof(GbAnimBinPrivate));

   widget_class = GTK_WIDGET_CLASS(klass);
   widget_class->get_preferred_height = gb_anim_bin_get_preferred_height;
   widget_class->get_preferred_width = gb_anim_bin_get_preferred_width;
   widget_class->hide = gb_anim_bin_hide;
   widget_class->show = gb_anim_bin_show;
   widget_class->size_allocate = gb_anim_bin_size_allocate;

   g_object_class_override_property(object_class,
                                    PROP_ORIENTATION,
                                    "orientation");

   gParamSpecs[PROP_DURATION] =
      g_param_spec_uint("duration",
                          _("Duration"),
                          _("The duration of the animation."),
                          0,
                          G_MAXUINT,
                          500,
                          G_PARAM_READWRITE);
   g_object_class_install_property(object_class, PROP_DURATION,
                                   gParamSpecs[PROP_DURATION]);

   gParamSpecs[PROP_FRAME_RATE] =
      g_param_spec_uint("frame-rate",
                          _("Frame Rate"),
                          _("The number of frames per second."),
                          1,
                          G_MAXUINT,
                          60,
                          G_PARAM_READWRITE);
   g_object_class_install_property(object_class, PROP_FRAME_RATE,
                                   gParamSpecs[PROP_FRAME_RATE]);
}

static void
gb_anim_bin_init (GbAnimBin *bin)
{
   GbAnimBinPrivate *priv;

   bin->priv = priv =
      G_TYPE_INSTANCE_GET_PRIVATE(bin,
                                  GB_TYPE_ANIM_BIN,
                                  GbAnimBinPrivate);

   priv->mode = GB_ANIMATION_EASE_IN_OUT_QUAD;
   priv->duration = 500;
   priv->fps = 60;
   priv->orientation = GTK_ORIENTATION_VERTICAL;
}
