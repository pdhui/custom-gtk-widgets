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

G_DEFINE_TYPE(GbAnimBin, gb_anim_bin, GTK_TYPE_EVENT_BOX)

struct _GbAnimBinPrivate
{
   GbAnimationMode mode;
   guint duration;
   guint fps;
   guint last_child_height;
};

enum
{
   PROP_0,
   LAST_PROP
};

static void
gb_anim_bin_hide_done (GtkWidget *widget)
{
   GbAnimBinPrivate *priv;
   GbAnimBin *bin = (GbAnimBin *)widget;

   g_return_if_fail(GB_IS_ANIM_BIN(bin));

   priv = bin->priv;

   GTK_WIDGET_CLASS(gb_anim_bin_parent_class)->hide(widget);

   priv->last_child_height = 0;
   g_object_set(widget, "height-request", -1, NULL);
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

   if ((child = gtk_bin_get_child(GTK_BIN(bin)))) {
      gtk_widget_get_allocation(child, &alloc);
      priv->last_child_height = alloc.height;

      gtk_widget_get_allocation(widget, &alloc);
      g_object_set(widget, "height-request", alloc.height, NULL);

      gb_object_animate_full(widget, priv->mode, priv->duration, priv->fps,
                             (GDestroyNotify)gb_anim_bin_hide_done,
                             g_object_ref(widget),
                             "height-request", 0,
                             NULL);
   } else {
      GTK_WIDGET_CLASS(gb_anim_bin_parent_class)->hide(widget);
   }
}

static void
gb_anim_bin_show_done (GtkWidget *widget)
{
   g_return_if_fail(GB_IS_ANIM_BIN(widget));

   g_object_set(widget, "height-request", -1, NULL);
   g_object_unref(widget);
}

static void
gb_anim_bin_show (GtkWidget *widget)
{
   GbAnimBinPrivate *priv;
   GbAnimBin *bin = (GbAnimBin *)widget;
   GtkWidget *child;
   gint height;

   g_return_if_fail(GB_IS_ANIM_BIN(bin));

   priv = bin->priv;

   if ((child = gtk_bin_get_child(GTK_BIN(bin)))) {
      g_object_set(widget, "height-request", 0, NULL);
      GTK_WIDGET_CLASS(gb_anim_bin_parent_class)->show(widget);

      gtk_widget_get_preferred_height(child, NULL, &height);
      gb_object_animate_full(widget, priv->mode, priv->duration, priv->fps,
                             (GDestroyNotify)gb_anim_bin_show_done,
                             g_object_ref(widget),
                             "height-request", height,
                             NULL);
   } else {
      GTK_WIDGET_CLASS(gb_anim_bin_parent_class)->show(widget);
   }
}

static void
gb_anim_bin_get_preferred_height (GtkWidget *widget,
                                  gint      *min_height,
                                  gint      *natural_height)
{
   GtkWidget *child;
   gint height;

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
      if (priv->last_child_height) {
         child_alloc.height = priv->last_child_height;
         gtk_widget_size_allocate(child, &child_alloc);
      }
   }
}

/**
 * gb_anim_bin_finalize:
 * @object: (in): A #GbAnimBin.
 *
 * Finalizer for a #GbAnimBin instance.  Frees any resources held by
 * the instance.
 *
 * Returns: None.
 * Side effects: None.
 */
static void
gb_anim_bin_finalize (GObject *object)
{
   G_OBJECT_CLASS(gb_anim_bin_parent_class)->finalize(object);
}

/**
 * gb_anim_bin_class_init:
 * @klass: (in): A #GbAnimBinClass.
 *
 * Initializes the #GbAnimBinClass and prepares the vtable.
 *
 * Returns: None.
 * Side effects: None.
 */
static void
gb_anim_bin_class_init (GbAnimBinClass *klass)
{
   GObjectClass *object_class;
   GtkWidgetClass *widget_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_anim_bin_finalize;
   g_type_class_add_private(object_class, sizeof(GbAnimBinPrivate));

   widget_class = GTK_WIDGET_CLASS(klass);
   widget_class->get_preferred_height = gb_anim_bin_get_preferred_height;
   widget_class->hide = gb_anim_bin_hide;
   widget_class->show = gb_anim_bin_show;
   widget_class->size_allocate = gb_anim_bin_size_allocate;
}

/**
 * gb_anim_bin_init:
 * @: (in): A #GbAnimBin.
 *
 * Initializes the newly created #GbAnimBin instance.
 *
 * Returns: None.
 * Side effects: None.
 */
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
}
