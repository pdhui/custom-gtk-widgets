#include <stdlib.h>
#include <gtk/gtk.h>
#include "egg-panel.h"

gint
main (gint   argc,
      gchar *argv[])
{
	GtkWidget *panel1;
	GtkBuilder *builder;
	GtkWidget *child;

	gtk_init(&argc, &argv);

	panel1 = egg_panel_new();
	egg_panel_set_title(EGG_PANEL(panel1), "Sources");
	gtk_widget_show(panel1);

	builder = gtk_builder_new();
	gtk_builder_add_from_file(builder, "main.ui", NULL);
	child = GTK_WIDGET(gtk_builder_get_object(builder, "table1"));
	gtk_widget_unparent(child);
	gtk_container_set_border_width(GTK_CONTAINER(child), 12);
	gtk_container_add(GTK_CONTAINER(panel1), child);
	gtk_widget_show(child);

	gtk_main();

	return EXIT_SUCCESS;
}
