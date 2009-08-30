#include <gtk/gtk.h>
#include "gtkmodebutton.h"

void
switched (GtkModeButton *mode_button, gint selected, gpointer user_data)
{
	GtkNotebook *notebook = GTK_NOTEBOOK (user_data);
	
	g_debug ("New selected item %d", selected);
	
	if (selected < 2)
		gtk_notebook_set_page (notebook, selected);
}

void
remove_at (GtkButton *button, gpointer user_data)
{
	GtkModeButton *mode_button = GTK_MODE_BUTTON (user_data);
	
	gint selected = gtk_mode_button_get_selected (mode_button);
	if (selected >= 0)
	{
		gtk_mode_button_remove (mode_button, selected);
	}
}

gint
main (gint argc, gchar *argv[])
{
	GtkWidget *window;
	GtkWidget *mode_button;
	GtkWidget *vbox;
	GtkWidget *remove;
	GtkWidget *notebook;
	GtkWidget *page1;
	GtkWidget *page2;
	GtkWidget *hour, *day, *week, *month, *year;
	GtkWidget *btn;
	
	gtk_init (&argc, &argv);
	
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "GtkModeButton");
	gtk_window_set_default_size (GTK_WINDOW (window), 300, -1);
	gtk_container_set_border_width (GTK_CONTAINER (window), 12);
	g_signal_connect (window, "destroy", gtk_main_quit, NULL);
	gtk_widget_show (window);
	
	vbox = gtk_vbox_new (FALSE, 6);
	gtk_container_add (GTK_CONTAINER (window), vbox);
	gtk_widget_show (vbox);
	
	mode_button = gtk_mode_button_new ();
	gtk_box_pack_start (GTK_BOX (vbox), mode_button, FALSE, TRUE, 0);
	gtk_widget_show (mode_button);
	
	hour = gtk_label_new ("Hour");
	day = gtk_label_new ("Day");
	week = gtk_label_new ("Week");
	month = gtk_label_new ("Month");
	year = gtk_label_new ("Year");

	btn = gtk_button_new_from_stock (GTK_STOCK_OPEN);
	
	gtk_mode_button_append (GTK_MODE_BUTTON (mode_button), hour);
	gtk_mode_button_append (GTK_MODE_BUTTON (mode_button), day);
	gtk_mode_button_append (GTK_MODE_BUTTON (mode_button), week);
	gtk_mode_button_append (GTK_MODE_BUTTON (mode_button), month);
	gtk_mode_button_append (GTK_MODE_BUTTON (mode_button), year);
	gtk_mode_button_append_button (GTK_MODE_BUTTON (mode_button), btn);
	
	gtk_widget_show (hour);
	gtk_widget_show (day);
	gtk_widget_show (week);
	gtk_widget_show (month);
	gtk_widget_show (year);
	
	remove = gtk_button_new_from_stock (GTK_STOCK_REMOVE);
	gtk_box_pack_start (GTK_BOX (vbox), remove, FALSE, TRUE, 0);
	gtk_widget_show (remove);
	
	notebook = gtk_notebook_new ();
	gtk_container_add (GTK_CONTAINER (vbox), notebook);
	gtk_notebook_set_show_border (GTK_NOTEBOOK (notebook), FALSE);
	gtk_notebook_set_show_tabs (GTK_NOTEBOOK (notebook), FALSE);
	gtk_widget_show (notebook);
	
	page1 = gtk_text_view_new ();
	gtk_notebook_insert_page (GTK_NOTEBOOK (notebook), page1, NULL, -1);
	gtk_widget_show (page1);
	
	page2 = gtk_text_view_new ();
	gtk_notebook_insert_page (GTK_NOTEBOOK (notebook), page2, NULL, -1);
	gtk_widget_show (page2);
	
	g_signal_connect (mode_button, "switched", G_CALLBACK (switched), notebook);
	g_signal_connect (remove, "clicked", G_CALLBACK (remove_at), mode_button);
	
	gtk_main ();
	
	return 0;
}
