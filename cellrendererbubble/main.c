#include <gtk/gtk.h>
#include "gtkcellrendererbubble.h"

gint
main (int argc, char *argv[])
{
  GtkWidget *window,
            *treeview;
  GtkTreeViewColumn *column;
  GtkCellRenderer *cell;
  GtkListStore *model;
  GtkTreeIter iter;
  
  gtk_init (&argc, &argv);
  
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
  g_signal_connect (window, "delete-event", G_CALLBACK (gtk_main_quit), NULL);
  gtk_widget_show (window);
  
  treeview = gtk_tree_view_new ();
  gtk_container_add (GTK_CONTAINER (window), treeview);
  gtk_widget_show (treeview);
  
  column = gtk_tree_view_column_new ();
  
  cell = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (column, cell, TRUE);
  gtk_tree_view_column_add_attribute (column, cell, "text", 0);
  
  cell = gtk_cell_renderer_bubble_new ();
  gtk_tree_view_column_pack_start (column, cell, FALSE);
  gtk_tree_view_column_add_attribute (column, cell, "markup", 1);
  gtk_tree_view_column_add_attribute (column, cell, "show-bubble", 2);
  
  gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
  
  model = gtk_list_store_new (3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN);
  
  gtk_list_store_append (model, &iter);
  gtk_list_store_set (model, &iter, 0, "Some Title", 1, "<small>10</small>", 2, TRUE, -1);
  
  gtk_list_store_append (model, &iter);
  gtk_list_store_set (model, &iter, 0, "Another Title", 1, "<small>33</small>", 2, FALSE, -1);
  
  gtk_list_store_append (model, &iter);
  gtk_list_store_set (model, &iter, 0, "Great Title", 1, "<small>20</small>", 2, TRUE, -1);
  
  gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), GTK_TREE_MODEL (model));
  
  gtk_main ();
  
  return 0;
}
