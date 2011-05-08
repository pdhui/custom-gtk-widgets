#include "gb-anim-bin.h"

static gboolean
show_widget (gpointer data)
{
	gtk_widget_show(data);
	return FALSE;
}

static gboolean
hide_widget (gpointer data)
{
	g_timeout_add(5000, show_widget, data);
	gtk_widget_hide(data);
	return FALSE;
}

gint
main (gint   argc,
      gchar *argv[])
{
	GtkWindow *window;
	GtkWidget *box;
	GtkWidget *progress;
	GtkWidget *vbox;
	GtkWidget *sw;
	GtkWidget *tv;
	gint i;

	gtk_init(&argc, &argv);

	window = g_object_new(GTK_TYPE_WINDOW,
	                      "title", "anim test",
	                      "border-width", 12,
						  //"resizable", FALSE,
	                      NULL);
	g_signal_connect(window, "delete-event", gtk_main_quit, NULL);
	vbox = g_object_new(GTK_TYPE_VBOX,
						"visible", TRUE,
						NULL);
	gtk_container_add(GTK_CONTAINER(window), vbox);

	sw = g_object_new(GTK_TYPE_SCROLLED_WINDOW,
					  "visible", TRUE,
					  "height-request", 300,
					  NULL);
	gtk_container_add(GTK_CONTAINER(vbox), sw);

	tv = g_object_new(GTK_TYPE_TREE_VIEW,
					  "visible", TRUE,
					  NULL);
	gtk_container_add(GTK_CONTAINER(sw), tv);

	for (i = 0; i < 5; i++) {
		box = g_object_new(GB_TYPE_ANIM_BIN,
						   "visible", TRUE,
						   NULL);
		gtk_container_add_with_properties(GTK_CONTAINER(vbox), box,
		                                  "expand", FALSE,
		                                  NULL);

		progress = g_object_new(GTK_TYPE_PROGRESS_BAR,
								"visible", TRUE,
								"fraction", (i / 5.0),
								NULL);
		gtk_container_add(GTK_CONTAINER(box), progress);

		g_timeout_add((i + 2) * 1000, hide_widget, box);
	}

	gtk_window_present(window);
	gtk_main();

	return 0;
}
