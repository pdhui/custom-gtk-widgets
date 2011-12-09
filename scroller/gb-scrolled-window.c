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

#include <clutter/clutter.h>
#include <clutter-gtk/clutter-gtk.h>
#include <glib/gi18n.h>

#include "gb-scrolled-child.h"
#include "gb-scrolled-window.h"

G_DEFINE_TYPE(GbScrolledWindow, gb_scrolled_window, GTK_TYPE_EVENT_BOX)

struct _GbScrolledWindowPrivate
{
   GtkWidget *scroller;
   GtkWidget *embed;

   ClutterActor *vbar;
   ClutterActor *hbar;
};

enum
{
   PROP_0,
   LAST_PROP
};

//static GParamSbec *gParamSpecs[LAST_PROP];

static void
gb_scrolled_window_add (GtkContainer *container,
                        GtkWidget    *child)
{
   GbScrolledWindowPrivate *priv;

   g_return_if_fail(GTK_IS_CONTAINER(container));
   g_return_if_fail(GTK_IS_WIDGET(child));

   priv = GB_SCROLLED_WINDOW(container)->priv;

   if (!GTK_CLUTTER_IS_EMBED(child)) {
      gtk_container_add(GTK_CONTAINER(priv->scroller), child);
      return;
   }

   GTK_CONTAINER_CLASS(gb_scrolled_window_parent_class)->add(container, child);
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
   GtkContainerClass *container_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_scrolled_window_finalize;
   g_type_class_add_private(object_class, sizeof(GbScrolledWindowPrivate));

   container_class = GTK_CONTAINER_CLASS(klass);
   container_class->add = gb_scrolled_window_add;
}

static void
gb_scrolled_window_init (GbScrolledWindow *window)
{
   GbScrolledWindowPrivate *priv;
   ClutterActor *actor;
   ClutterActor *stage;
   ClutterActor *rect;
   ClutterColor black = { 0, 0, 0, 102 };

   window->priv = priv =
      G_TYPE_INSTANCE_GET_PRIVATE(window,
                                  GB_TYPE_SCROLLED_WINDOW,
                                  GbScrolledWindowPrivate);

   priv->embed = g_object_new(GTK_CLUTTER_TYPE_EMBED,
                              "visible", TRUE,
                              NULL);
   stage = gtk_clutter_embed_get_stage(GTK_CLUTTER_EMBED(priv->embed));
   gtk_container_add(GTK_CONTAINER(window), priv->embed);

   priv->scroller = g_object_new(GB_TYPE_SCROLLED_CHILD,
                                 "visible", TRUE,
                                 NULL);
   actor = gtk_clutter_actor_new_with_contents(priv->scroller);
   clutter_container_add_actor(CLUTTER_CONTAINER(stage), actor);
   g_object_bind_property(stage, "width", actor, "width",
                          G_BINDING_SYNC_CREATE);
   g_object_bind_property(stage, "height", actor, "height",
                          G_BINDING_SYNC_CREATE);

   rect = g_object_new(CLUTTER_TYPE_RECTANGLE,
                       "color", &black,
                       "width", 4.0,
                       "height", 200.0,
                       "x", 5.0,
                       "y", 5.0,
                       NULL);
   clutter_container_add_actor(CLUTTER_CONTAINER(stage), rect);
}
