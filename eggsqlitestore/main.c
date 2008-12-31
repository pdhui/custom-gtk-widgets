/**
 * main.c - 
 * 
 * Copyright (C) 2007   Christian Hergert <chrisian.hergert@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
**/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <glib.h>
#include <bonobo.h>
#include <gnome.h>
#include <glade/glade.h>

#include "egg-sqlite-store.h"

/* For testing propose use the local (not installed) glade file */
/* #define GLADE_FILE PACKAGE_DATA_DIR"/marina/glade/marina.glade" */
#define GLADE_FILE "marina.glade"

int
main (int argc, char *argv[])
{
	GtkWidget         *window;
	GtkWidget         *scroller;
	GtkWidget         *treeview;
	GtkTreeViewColumn *column;
	GtkCellRenderer   *cell;
	GtkTreeModel      *store;
	
	GError *error = NULL;

#ifdef ENABLE_NLS
	bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
#endif

	gnome_program_init (PACKAGE, VERSION, LIBGNOMEUI_MODULE,
                      argc, argv,
                      GNOME_PARAM_APP_DATADIR, PACKAGE_DATA_DIR,
                      NULL);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size (GTK_WINDOW (window), 150, 400);
	gtk_container_set_border_width (GTK_CONTAINER (window), 12);
	g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
	gtk_widget_show (window);
	
	scroller = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scroller),
										 GTK_SHADOW_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroller),
									GTK_POLICY_AUTOMATIC,
									GTK_POLICY_AUTOMATIC);
	gtk_container_add (GTK_CONTAINER (window), scroller);
	gtk_widget_show (scroller);
	
	treeview = gtk_tree_view_new ();
	gtk_container_add (GTK_CONTAINER (scroller), treeview);
	gtk_widget_show (treeview);
	
	store = egg_sqlite_store_new ();
	egg_sqlite_store_set_filename (EGG_SQLITE_STORE (store), "/tmp/test.db", &error);
	
	if (error) {
		g_print ("Error: %s\n", error->message);
		return 1;
	}
	
	egg_sqlite_store_set_table (EGG_SQLITE_STORE (store), "entries", NULL);
	gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), store);
	
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, "OID");
	cell = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, cell, FALSE);
	gtk_tree_view_column_add_attribute (column, cell, "text", 0);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, "TITLE");
	cell = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, cell, FALSE);
	gtk_tree_view_column_add_attribute (column, cell, "text", 2);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, "DATE");
	cell = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, cell, FALSE);
	gtk_tree_view_column_add_attribute (column, cell, "text", 3);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	
	gtk_main ();
	
	return 0;
}
