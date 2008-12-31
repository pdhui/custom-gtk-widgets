#include <glib.h>
#include <gtk/gtk.h>

#include <db.h>
#include <stdlib.h>
#include <stdio.h>

#include "bdb-list-store.h"

static BdbListStore *store    = NULL;
static GtkWidget    *treeview = NULL;

void
add_clicked (GtkButton *add)
{
	GtkTreeIter iter;
	GValue value = {0,};
	
	bdb_list_store_append (store, &iter);
	
	g_value_init (&value, G_TYPE_STRING);
	g_value_take_string (&value, g_strdup_printf ("This is row %d", GPOINTER_TO_INT (iter.user_data)));
	
	bdb_list_store_set_value (store, &iter, 0, &value);
	
	g_value_unset (&value);
}

void
remove_clicked (GtkButton *remove)
{
	GtkTreeIter   iter;
	GtkTreeModel *model;
	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
	
	if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
		if (bdb_list_store_remove (store, &iter)) {
			gtk_tree_selection_select_iter (selection, &iter);
		}
	}
}

void quit (void)
{
	DB *db = bdb_list_store_get_db (store);
	g_assert (db != NULL);
	db->close (db, 0);
	gtk_main_quit ();
}

gint
main (int argc, char *argv[])
{
	GtkWidget         *window;
	GtkWidget         *vbox;
	GtkWidget         *scroller;
	GtkTreeViewColumn *column;
	GtkCellRenderer   *ctext;
	GtkWidget         *hbox;
	GtkWidget         *add;
	GtkWidget         *remove;
	GError            *error = NULL;
	
	gtk_init (&argc, &argv);
	
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width (GTK_CONTAINER (window), 12);
	g_signal_connect (window, "destroy", G_CALLBACK (quit), NULL);
	gtk_window_set_default_size (GTK_WINDOW (window), 300, 400);
	gtk_widget_show (window);
	
	vbox = gtk_vbox_new (FALSE, 6);
	gtk_container_add (GTK_CONTAINER (window), vbox);
	gtk_widget_show (vbox);
	
	scroller = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroller),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	gtk_container_add (GTK_CONTAINER (vbox), scroller);
	gtk_widget_show (scroller);
	
	treeview = gtk_tree_view_new ();
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview), TRUE);
	gtk_container_add (GTK_CONTAINER (scroller), treeview);
	gtk_widget_show (treeview);
	
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, "Data");
	gtk_tree_view_column_set_alignment (column, 0.5f);
	ctext = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, ctext, TRUE);
	gtk_tree_view_column_add_attribute (column, ctext, "text", 0);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	
	hbox = gtk_hbox_new (TRUE, 2);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);
	gtk_widget_show (hbox);
	
	add = gtk_button_new_from_stock (GTK_STOCK_ADD);
	g_signal_connect (add, "clicked", G_CALLBACK (add_clicked), NULL);
	gtk_box_pack_start (GTK_BOX (hbox), add, TRUE, TRUE, 0);
	gtk_widget_show (add);
	
	remove = gtk_button_new_from_stock (GTK_STOCK_REMOVE);
	g_signal_connect (remove, "clicked", G_CALLBACK (remove_clicked), NULL);
	gtk_box_pack_start (GTK_BOX (hbox), remove, TRUE, TRUE, 0);
	gtk_widget_show (remove);
	
	DB     *dbp          = NULL;
	DB_ENV *db_env       = NULL;
	int     ret          = 0;
	int     db_env_flags = DB_CREATE
	                     | DB_INIT_LOG
			     | DB_INIT_LOCK
			     | DB_INIT_MPOOL
			     | DB_INIT_TXN
			     | DB_PRIVATE;
	
	if ((ret = db_env_create (&db_env, 0)) != 0)
		g_error ("db_env_create: %s", db_strerror (ret));
	
	db_env->set_errpfx (db_env, "bdbliststore");
	db_env->set_errfile (db_env, stderr);
	
	if ((ret = db_env->open (db_env, ".", db_env_flags, 0)) != 0)
		g_error ("db_env_open: %s", db_strerror (ret));
	
	if ((ret = db_create (&dbp, db_env, 0)) != 0)
		g_error ("db_create: %s", db_strerror (ret));
	
	dbp->set_flags (dbp, DB_RENUMBER);

	if ((ret = dbp->open (dbp, NULL, "test.db", NULL, DB_RECNO, DB_CREATE, 0)) != 0)
		g_error ("db_open: %s", db_strerror (ret));
	
	store = bdb_list_store_new ();
	if (!bdb_list_store_set_db (store, dbp, &error)) {
		if (error) {
			g_printerr ("Could not attach database: %s\n", error->message);
			g_error_free (error);
		}
		return EXIT_FAILURE;
	}
	
	gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), GTK_TREE_MODEL (store));

	gtk_main ();
	
	return 0;
}
