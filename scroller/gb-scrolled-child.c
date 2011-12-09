/* gb-scrolled-window.c
 *
 * Copyright (C) 2011 Christian Hergert <chris@dronelabs.com>
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

#include <glib/gi18n.h>

#include "gb-animation.h"
#include "gb-scrolled-child.h"

G_DEFINE_TYPE(GbScrolledChild, gb_scrolled_child, GTK_TYPE_BIN)

struct _GbScrolledChildPrivate
{
   GdkWindow *window;

   GtkAdjustment *hadj;
   GbAnimation *hadj_anim;
   gdouble hadj_target;

   GtkAdjustment *vadj;
   GbAnimation *vadj_anim;
   gdouble vadj_target;
};

enum
{
   PROP_0,
   LAST_PROP
};

//static GParamSpec *gParamSpecs[LAST_PROP];

static void
gb_scrolled_child_add (GtkContainer *container,
                        GtkWidget    *child)
{
   GbScrolledChildPrivate *priv;
   GbScrolledChild *window = (GbScrolledChild *)container;
   GtkWidget *prev_child;
   GtkBin *bin;

   g_return_if_fail(GB_IS_SCROLLED_CHILD(window));
   g_return_if_fail(GTK_IS_WIDGET(child));

   priv = window->priv;

   bin = GTK_BIN(container);
   prev_child = gtk_bin_get_child(bin);
   g_return_if_fail(prev_child == NULL);

   GTK_CONTAINER_CLASS(gb_scrolled_child_parent_class)->add(container, child);

   if (!GTK_IS_SCROLLABLE(child)) {
      g_warning("gb_scrolled_child_add(): cannot add non scrollable widget "
                "use gtk_scrolled_child_add_with_viewport() instead");
      return;
   }

   g_object_set(child,
                "hadjustment", priv->hadj,
                "vadjustment", priv->vadj,
                NULL);
}

static void
gb_scrolled_child_remove (GtkContainer *container,
                           GtkWidget    *child)
{
   GbScrolledChild *window = (GbScrolledChild *)container;

   g_return_if_fail(GB_IS_SCROLLED_CHILD(window));
   g_return_if_fail(GTK_IS_WIDGET(child));
   g_return_if_fail(gtk_bin_get_child(GTK_BIN(container)) == child);

   g_object_set(child,
                "hadjustment", NULL,
                "vadjustment", NULL,
                NULL);

   GTK_CONTAINER_CLASS(gb_scrolled_child_parent_class)->remove(container, child);
}

static gdouble
gb_scrolled_child_get_wheel_delta (GtkAdjustment      *adj,
                                    GdkScrollDirection  direction)
{
   gdouble delta;

   g_return_val_if_fail(GTK_IS_ADJUSTMENT(adj), 0.0);

   delta = gtk_adjustment_get_step_increment(adj) * (4.0/3.0);

   if ((direction == GDK_SCROLL_UP) ||
       (direction == GDK_SCROLL_LEFT)) {
      delta = -delta;
   }

   return delta;
}

static gboolean
gb_scrolled_child_scroll_event (GtkWidget      *widget,
                                GdkEventScroll *event)
{
   GbScrolledChildPrivate *priv;
   GtkAdjustment *adj;
   GbAnimation **anim = NULL;
   gdouble delta;
   gdouble value = 0;
   gdouble *target = NULL;

   g_return_val_if_fail(GTK_IS_WIDGET(widget), FALSE);
   g_return_val_if_fail(event != NULL, FALSE);

   priv = GB_SCROLLED_CHILD(widget)->priv;

   if ((event->direction == GDK_SCROLL_UP) || (event->direction == GDK_SCROLL_DOWN)) {
      adj = priv->vadj;
      anim = &priv->vadj_anim;
      target = &priv->vadj_target;
   } else {
      adj = priv->hadj;
      anim = &priv->hadj_anim;
      target = &priv->hadj_target;
   }

   delta = gb_scrolled_child_get_wheel_delta(adj, event->direction);
   value = gtk_adjustment_get_value(adj);

   if (*anim) {
      g_object_remove_weak_pointer(G_OBJECT(*anim), (gpointer *)anim);
      gb_animation_stop(*anim);
      *anim = NULL;
      value = *target;
   }

   value += delta;
   *anim = gb_object_animate(adj, GB_ANIMATION_EASE_OUT_QUAD, 200,
                             "value", value,
                             NULL);
   g_object_add_weak_pointer(G_OBJECT(*anim), (gpointer *)anim);
   *target = value;

   return TRUE;
}

static gboolean
gb_scrolled_child_focus (GtkWidget        *widget,
                          GtkDirectionType  direction)
{
   GtkWidget *child;

   g_return_val_if_fail(GTK_IS_BIN(widget), FALSE);

   if (gtk_widget_is_focus(widget)) {
      return FALSE;
   }

   if ((child = gtk_bin_get_child(GTK_BIN(widget)))) {
      if (gtk_widget_child_focus(child, direction)) {
         return TRUE;
      }
   } else {
      gtk_widget_grab_focus(widget);
      return TRUE;
   }

   return FALSE;
}

static void
gb_scrolled_child_size_allocate (GtkWidget     *widget,
                                  GtkAllocation *allocation)
{
   GbScrolledChild *window = (GbScrolledChild *)widget;
   GtkWidget *child;

   g_return_if_fail(GB_IS_SCROLLED_CHILD(window));
   g_return_if_fail(allocation != NULL);

   gtk_widget_set_allocation(widget, allocation);

   child = gtk_bin_get_child(GTK_BIN(widget));
   gtk_widget_size_allocate(child, allocation);
}

static void
gb_scrolled_child_destroy (GtkWidget *widget)
{
   GbScrolledChild *window = (GbScrolledChild *)widget;

   g_return_if_fail(GB_IS_SCROLLED_CHILD(window));

   GTK_WIDGET_CLASS(gb_scrolled_child_parent_class)->destroy(widget);

   g_clear_object(&window->priv->hadj);
   g_clear_object(&window->priv->vadj);
}

static void
gb_scrolled_child_finalize (GObject *object)
{
   G_OBJECT_CLASS(gb_scrolled_child_parent_class)->finalize(object);
}

static void
gb_scrolled_child_class_init (GbScrolledChildClass *klass)
{
   GObjectClass *object_class;
   GtkWidgetClass *widget_class;
   GtkContainerClass *container_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_scrolled_child_finalize;
   g_type_class_add_private(object_class, sizeof(GbScrolledChildPrivate));

   widget_class = GTK_WIDGET_CLASS(klass);
   widget_class->destroy = gb_scrolled_child_destroy;
   widget_class->focus = gb_scrolled_child_focus;
   widget_class->scroll_event = gb_scrolled_child_scroll_event;
   widget_class->size_allocate = gb_scrolled_child_size_allocate;

   container_class = GTK_CONTAINER_CLASS(klass);
   container_class->add = gb_scrolled_child_add;
   container_class->remove = gb_scrolled_child_remove;
}

static void
gb_scrolled_child_init (GbScrolledChild *window)
{
   window->priv =
      G_TYPE_INSTANCE_GET_PRIVATE(window,
                                  GB_TYPE_SCROLLED_CHILD,
                                  GbScrolledChildPrivate);

   window->priv->hadj = g_object_ref(g_object_new(GTK_TYPE_ADJUSTMENT, NULL));
   window->priv->vadj = g_object_ref(g_object_new(GTK_TYPE_ADJUSTMENT, NULL));
}
