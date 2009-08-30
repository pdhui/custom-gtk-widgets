/* gtkmodebutton.h
 * Copyright (C) 2008 Christian Hergert <chris@dronelabs.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef __GTK_MODE_BUTTON_H__
#define __GTK_MODE_BUTTON_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTK_TYPE_MODE_BUTTON gtk_mode_button_get_type()

#define GTK_MODE_BUTTON(obj) (              \
        G_TYPE_CHECK_INSTANCE_CAST ((obj),  \
        GTK_TYPE_MODE_BUTTON,               \
        GtkModeButton))

#define GTK_MODE_BUTTON_CLASS(klass) (      \
        G_TYPE_CHECK_CLASS_CAST ((klass),   \
        GTK_TYPE_MODE_BUTTON,               \
        GtkModeButtonClass))

#define GTK_IS_MODE_BUTTON(obj) (           \
        G_TYPE_CHECK_INSTANCE_TYPE ((obj),  \
        GTK_TYPE_MODE_BUTTON))

#define GTK_IS_MODE_BUTTON_CLASS(klass) (   \
        G_TYPE_CHECK_CLASS_TYPE ((klass),   \
        GTK_TYPE_MODE_BUTTON))

#define GTK_MODE_BUTTON_GET_CLASS(obj) (    \
        G_TYPE_INSTANCE_GET_CLASS ((obj),   \
        GTK_TYPE_MODE_BUTTON,               \
        GtkModeButtonClass))

typedef struct
{
	GtkEventBox parent;
} GtkModeButton;

typedef struct
{
	GtkEventBoxClass parent_class;
	
	gboolean (*select) (GtkModeButton *mode_button, gint index);
} GtkModeButtonClass;

GType      gtk_mode_button_get_type     (void);
GtkWidget* gtk_mode_button_new          (void);
gint       gtk_mode_button_get_selected (GtkModeButton *mode_button);
void       gtk_mode_button_set_selected (GtkModeButton *mode_button, gint index);
gint       gtk_mode_button_get_hovered  (GtkModeButton *mode_button);

gint       gtk_mode_button_append       (GtkModeButton *mode_button, GtkWidget *widget);
gint       gtk_mode_button_append_button (GtkModeButton *mode_button, GtkWidget *button);
void       gtk_mode_button_remove       (GtkModeButton *mode_button, gint index);

G_END_DECLS

#endif /* __GTK_MODE_BUTTON_H__ */

