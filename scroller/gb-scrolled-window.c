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
#include "gb-scrolled-window.h"

G_DEFINE_TYPE(GbScrolledWindow, gb_scrolled_window, GTK_TYPE_BIN)

struct _GbScrolledWindowPrivate
{
   GdkWindow *window;

   GtkAdjustment *hadj;
   GtkAdjustment *vadj;

   GtkAdjustment *opacity;
   GbAnimation *opacity_anim;

   gint sb_min_height;
   gint sb_padding;
   gint sb_radius;
   gint sb_width;
};

enum
{
   PROP_0,
   LAST_PROP
};

//static GParamSpec *gParamSpecs[LAST_PROP];

gboolean
gb_scrolled_window_draw (GtkWidget *widget,
                         cairo_t   *cr,
                         gpointer   user_data)
{
   GbScrolledWindowPrivate *priv;
   GtkAllocation a;
   gdouble opacity;
   gdouble lower;
   gdouble page_size;
   gdouble upper;
   gdouble value;
   gdouble ratio;
   gdouble height;
   gdouble y;

   priv = GB_SCROLLED_WINDOW(user_data)->priv;

   gtk_widget_get_allocation(widget, &a);

   g_object_get(priv->opacity,
                "value", &opacity,
                NULL);

   g_object_get(priv->vadj,
                "lower", &lower,
                "upper", &upper,
                "value", &value,
                "page-size", &page_size,
                NULL);

   ratio = page_size / (upper - lower);
   height = ratio * (a.height - (2 * priv->sb_padding));
   height = MAX(height, 20);

   ratio = (value - lower) / (upper - lower);
   y = ratio * (a.height - (2 * priv->sb_padding)) + priv->sb_padding;

   cairo_rectangle(cr,
                   a.width - 9 - priv->sb_padding,
                   y, 9, height);

   cairo_set_source_rgba(cr, 0, 0, 0, opacity);
   cairo_fill(cr);

   return FALSE;
}

static void
cancel_animation (GbScrolledWindow *window)
{
   GbScrolledWindowPrivate *priv;
   GbAnimation *a;

   g_return_if_fail(GB_IS_SCROLLED_WINDOW(window));

   priv = window->priv;

   if ((a = priv->opacity_anim)) {
      priv->opacity_anim = NULL;
      g_object_remove_weak_pointer(G_OBJECT(a),
                                   (gpointer *)&priv->opacity_anim);
      gb_animation_stop(a);
   }
}

static gboolean
gb_scrolled_window_enter_notify (GtkWidget        *widget,
                                 GdkEventCrossing *crossing,
                                 gpointer          user_data)
{
   GbScrolledWindowPrivate *priv;
   GbScrolledWindow *window = user_data;
   gdouble upper;

   g_return_val_if_fail(GB_IS_SCROLLED_WINDOW(window), FALSE);

   priv = window->priv;

   cancel_animation(user_data);
   g_object_get(priv->opacity, "upper", &upper, NULL);
   priv->opacity_anim = gb_object_animate(priv->opacity,
                                          GB_ANIMATION_EASE_OUT_QUAD,
                                          1000,
                                          "value", upper,
                                          NULL);
   g_object_add_weak_pointer(G_OBJECT(priv->opacity_anim),
                             (gpointer *)&priv->opacity_anim);

   return FALSE;
}

static gboolean
gb_scrolled_window_leave_notify (GtkWidget        *widget,
                                 GdkEventCrossing *crossing,
                                 gpointer          user_data)
{
   GbScrolledWindowPrivate *priv;
   GbScrolledWindow *window = user_data;

   g_return_val_if_fail(GB_IS_SCROLLED_WINDOW(window), FALSE);

   priv = window->priv;

   cancel_animation(user_data);
   priv->opacity_anim = gb_object_animate(priv->opacity,
                                          GB_ANIMATION_EASE_IN_QUAD,
                                          1000,
                                          "value", 0.0,
                                          NULL);
   g_object_add_weak_pointer(G_OBJECT(priv->opacity_anim),
                             (gpointer *)&priv->opacity_anim);

   return FALSE;
}

static void
gb_scrolled_window_add (GtkContainer *container,
                        GtkWidget    *child)
{
   GbScrolledWindowPrivate *priv;
   GbScrolledWindow *window = (GbScrolledWindow *)container;
   GtkWidget *prev_child;
   GtkBin *bin;

   g_return_if_fail(GB_IS_SCROLLED_WINDOW(window));
   g_return_if_fail(GTK_IS_WIDGET(child));

   priv = window->priv;

   bin = GTK_BIN(container);
   prev_child = gtk_bin_get_child(bin);
   g_return_if_fail(prev_child == NULL);

   GTK_CONTAINER_CLASS(gb_scrolled_window_parent_class)->add(container, child);

   if (!GTK_IS_SCROLLABLE(child)) {
      g_warning("gb_scrolled_window_add(): cannot add non scrollable widget "
                "use gtk_scrolled_window_add_with_viewport() instead");
      return;
   }

   g_object_set(child,
                "hadjustment", priv->hadj,
                "vadjustment", priv->vadj,
                NULL);

   /*
    * XXX: This should probably be done with an input only window
    *      that sits above the childs window and then passes evenst
    *      on to the child.
    */
   gtk_widget_add_events(child,
                         GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);

   g_signal_connect_after(child, "draw",
                          G_CALLBACK(gb_scrolled_window_draw),
                          container);
   g_signal_connect_after(child, "enter-notify-event",
                          G_CALLBACK(gb_scrolled_window_enter_notify),
                          container);
   g_signal_connect_after(child, "leave-notify-event",
                          G_CALLBACK(gb_scrolled_window_leave_notify),
                          container);
}

static void
gb_scrolled_window_remove (GtkContainer *container,
                           GtkWidget    *child)
{
   GbScrolledWindow *window = (GbScrolledWindow *)container;

   g_return_if_fail(GB_IS_SCROLLED_WINDOW(window));
   g_return_if_fail(GTK_IS_WIDGET(child));
   g_return_if_fail(gtk_bin_get_child(GTK_BIN(container)) == child);

   g_object_set(child,
                "hadjustment", NULL,
                "vadjustment", NULL,
                NULL);

   GTK_CONTAINER_CLASS(gb_scrolled_window_parent_class)->remove(container, child);
}

static gdouble
gb_scrolled_window_get_wheel_delta (GtkAdjustment      *adj,
                                    GdkScrollDirection  direction)
{
   gdouble delta;

   g_return_val_if_fail(GTK_IS_ADJUSTMENT(adj), 0.0);

   delta = gtk_adjustment_get_step_increment(adj) * 2;

   if ((direction == GDK_SCROLL_UP) ||
       (direction == GDK_SCROLL_LEFT)) {
      delta = -delta;
   }

   return delta;
}

static gboolean
gb_scrolled_window_scroll_event (GtkWidget      *widget,
                                 GdkEventScroll *event)
{
   GbScrolledWindowPrivate *priv;
   GtkAdjustment *adj;
   gdouble delta;

   g_return_val_if_fail(GTK_IS_WIDGET(widget), FALSE);
   g_return_val_if_fail(event != NULL, FALSE);

   priv = GB_SCROLLED_WINDOW(widget)->priv;

   if ((event->direction == GDK_SCROLL_UP) || (event->direction == GDK_SCROLL_DOWN)) {
      adj = priv->vadj;
   } else {
      adj = priv->hadj;
   }

   delta = gb_scrolled_window_get_wheel_delta(adj, event->direction);

   gtk_adjustment_set_value(adj, gtk_adjustment_get_value(adj) + delta);

   return TRUE;
}

static gboolean
gb_scrolled_window_focus (GtkWidget        *widget,
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
gb_scrolled_window_size_allocate (GtkWidget     *widget,
                                  GtkAllocation *allocation)
{
   GbScrolledWindow *window = (GbScrolledWindow *)widget;
   GtkWidget *child;

   g_return_if_fail(GB_IS_SCROLLED_WINDOW(window));
   g_return_if_fail(allocation != NULL);

   gtk_widget_set_allocation(widget, allocation);

   child = gtk_bin_get_child(GTK_BIN(widget));
   gtk_widget_size_allocate(child, allocation);
}

static void
gb_scrolled_window_destroy (GtkWidget *widget)
{
   GbScrolledWindow *window = (GbScrolledWindow *)widget;

   g_return_if_fail(GB_IS_SCROLLED_WINDOW(window));

   GTK_WIDGET_CLASS(gb_scrolled_window_parent_class)->destroy(widget);

   g_clear_object(&window->priv->hadj);
   g_clear_object(&window->priv->vadj);
}

static void
gb_scrolled_window_vadj_changed (GtkAdjustment    *adj,
                                 GbScrolledWindow *scroller)
{
   GtkAllocation alloc;
   GtkWidget *child;

   g_return_if_fail(GTK_IS_ADJUSTMENT(adj));
   g_return_if_fail(GB_IS_SCROLLED_WINDOW(scroller));

   if ((child = gtk_bin_get_child(GTK_BIN(scroller)))) {
      if (gtk_widget_get_visible(child)) {
         gtk_widget_get_allocation(child, &alloc);
         gtk_widget_queue_draw_area(child,
                                    alloc.width - 20,
                                    0,
                                    20,
                                    alloc.height);
      }
   }
}

static void
gb_scrolled_window_opacity_changed (GtkAdjustment    *adj,
                                    GbScrolledWindow *window)
{
   g_return_if_fail(GTK_IS_ADJUSTMENT(adj));
   g_return_if_fail(GB_IS_SCROLLED_WINDOW(window));

   gb_scrolled_window_vadj_changed(window->priv->vadj, window);
}

static void
gb_scrolled_window_finalize (GObject *object)
{
   G_OBJECT_CLASS(gb_scrolled_window_parent_class)->finalize(object);
}

static void
gb_scrolled_window_class_init (GbScrolledWindowClass *klass)
{
   GObjectClass *object_class;
   GtkWidgetClass *widget_class;
   GtkContainerClass *container_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_scrolled_window_finalize;
   g_type_class_add_private(object_class, sizeof(GbScrolledWindowPrivate));

   widget_class = GTK_WIDGET_CLASS(klass);
   widget_class->destroy = gb_scrolled_window_destroy;
   widget_class->focus = gb_scrolled_window_focus;
   widget_class->scroll_event = gb_scrolled_window_scroll_event;
   widget_class->size_allocate = gb_scrolled_window_size_allocate;

   container_class = GTK_CONTAINER_CLASS(klass);
   container_class->add = gb_scrolled_window_add;
   container_class->remove = gb_scrolled_window_remove;
}

static void
gb_scrolled_window_init (GbScrolledWindow *window)
{
   window->priv =
      G_TYPE_INSTANCE_GET_PRIVATE(window,
                                  GB_TYPE_SCROLLED_WINDOW,
                                  GbScrolledWindowPrivate);

   window->priv->hadj = g_object_ref(g_object_new(GTK_TYPE_ADJUSTMENT, NULL));
   window->priv->vadj = g_object_ref(g_object_new(GTK_TYPE_ADJUSTMENT, NULL));
   window->priv->opacity = g_object_new(GTK_TYPE_ADJUSTMENT,
                                        "lower", 0.0,
                                        "upper", 0.4,
                                        "value", 0.4,
                                        NULL);

   g_signal_connect(window->priv->opacity,
                    "value-changed",
                    G_CALLBACK(gb_scrolled_window_opacity_changed),
                    window);
   g_signal_connect(window->priv->vadj,
                    "changed",
                    G_CALLBACK(gb_scrolled_window_vadj_changed),
                    window);
   g_signal_connect(window->priv->vadj,
                    "value-changed",
                    G_CALLBACK(gb_scrolled_window_vadj_changed),
                    window);

   window->priv->sb_min_height = 20;
   window->priv->sb_padding = 5;
   window->priv->sb_radius = 5;
   window->priv->sb_width = 10;
}
