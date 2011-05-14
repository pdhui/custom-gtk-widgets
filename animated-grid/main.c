/* main.c
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
#include <gtk/gtk.h>

#include "chat-avatar.h"
#include "chat-grid.h"

gint
main (gint argc,
      gchar *argv[])
{
	GtkStyleContext *context;
	GtkWidget *a;
	GtkWidget *avatar;
	GtkWidget *grid;
	GtkWidget *l;
	GtkWidget *main_vbox;
	GtkWidget *scroller;
	GtkWidget *toolbar;
	GtkWidget *window;
	GtkWidget *vbox;
	GtkWidget *viewport;
	gint i;

	gtk_init(&argc, &argv);

	window = g_object_new(GTK_TYPE_WINDOW,
	                      "title", _("Simply Chat"),
	                      NULL);

	main_vbox = g_object_new(GTK_TYPE_VBOX,
	                         "visible", TRUE,
	                         NULL);
	gtk_container_add(GTK_CONTAINER(window), main_vbox);

	toolbar = g_object_new(GTK_TYPE_TOOLBAR,
	                       "show-arrow", FALSE,
	                       "visible", TRUE,
	                       NULL);
	gtk_style_context_add_class(gtk_widget_get_style_context(toolbar),
	                            GTK_STYLE_CLASS_PRIMARY_TOOLBAR);
	gtk_container_add_with_properties(GTK_CONTAINER(main_vbox), toolbar,
	                                  "expand", FALSE,
	                                  NULL);

	l = g_object_new(GTK_TYPE_COMBO_BOX,
	                 "has-entry", TRUE,
	                 "visible", TRUE,
	                 NULL);
	gtk_entry_set_text(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(l))), "Available");
	a = g_object_new(GTK_TYPE_TOOL_ITEM,
	                 "child", l,
	                 "visible", TRUE,
	                 NULL);
	gtk_container_add_with_properties(GTK_CONTAINER(toolbar), a,
	                                  "expand", TRUE,
	                                  NULL);

	scroller = g_object_new(GTK_TYPE_SCROLLED_WINDOW,
	                        "visible", TRUE,
	                        NULL);
	gtk_container_add(GTK_CONTAINER(main_vbox), scroller);

	viewport = g_object_new(GTK_TYPE_VIEWPORT,
	                        "visible", TRUE,
	                        NULL);
	context = gtk_widget_get_style_context(viewport);
	gtk_style_context_add_class(context, GTK_STYLE_CLASS_VIEW);
	gtk_container_add(GTK_CONTAINER(scroller), viewport);

	vbox = g_object_new(GTK_TYPE_VBOX,
	                    "border-width", 6,
	                    "spacing", 6,
	                    "visible", TRUE,
	                    NULL);
	gtk_container_add(GTK_CONTAINER(viewport), vbox);

	a = g_object_new(GTK_TYPE_ALIGNMENT,
	                 "left-padding", 12,
	                 "right-padding", 6,
	                 "visible", TRUE,
	                 NULL);
	grid = g_object_new(CHAT_TYPE_GRID,
	                    "column-spacing", 6,
	                    "row-spacing", 6,
	                    "visible", TRUE,
	                    NULL);
	l = g_object_new(GTK_TYPE_LABEL,
	                 "label", "<b>_Frequent <span size='smaller'>(20)</span></b>",
	                 "mnemonic-widget", grid,
	                 "use-markup", TRUE,
	                 "use-underline", TRUE,
	                 "visible", TRUE,
	                 "xalign", 0.0f,
	                 NULL);
	gtk_container_add_with_properties(GTK_CONTAINER(vbox), l,
	                                  "expand", FALSE,
	                                  NULL);
	gtk_container_add(GTK_CONTAINER(a), grid);
	gtk_container_add_with_properties(GTK_CONTAINER(vbox), a,
	                                  "expand", FALSE,
	                                  NULL);

	for (i = 0; i < 20; i++) {
		avatar = g_object_new(CHAT_TYPE_AVATAR,
		                      "visible", TRUE,
		                      NULL);
		gtk_container_add(GTK_CONTAINER(grid), avatar);
	}

	a = g_object_new(GTK_TYPE_ALIGNMENT,
	                 "left-padding", 12,
	                 "right-padding", 6,
	                 "visible", TRUE,
	                 NULL);
	grid = g_object_new(CHAT_TYPE_GRID,
	                    "column-spacing", 6,
	                    "row-spacing", 6,
	                    "visible", TRUE,
	                    NULL);
	l = g_object_new(GTK_TYPE_LABEL,
	                 "label", "<b>_Buddies <span size='smaller'>(100)</span></b>",
	                 "mnemonic-widget", grid,
	                 "use-markup", TRUE,
	                 "use-underline", TRUE,
	                 "visible", TRUE,
	                 "xalign", 0.0f,
	                 NULL);
	gtk_container_add_with_properties(GTK_CONTAINER(vbox), l,
	                                  "expand", FALSE,
	                                  NULL);
	gtk_container_add(GTK_CONTAINER(a), grid);
	gtk_container_add_with_properties(GTK_CONTAINER(vbox), a,
	                                  "expand", FALSE,
	                                  NULL);

	for (i = 0; i < 100; i++) {
		avatar = g_object_new(CHAT_TYPE_AVATAR,
		                      "visible", TRUE,
		                      NULL);
		gtk_container_add(GTK_CONTAINER(grid), avatar);
	}

	g_signal_connect(window, "delete-event", gtk_main_quit, NULL);

	gtk_window_present(GTK_WINDOW(window));
	gtk_main();

	return 0;
}
