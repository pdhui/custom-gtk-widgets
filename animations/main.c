#include <gtk/gtk.h>

#include "gb-animation.h"

static GtkWidget *window;
static GtkWidget *fixed;

static gboolean
timeout_cb (gpointer data)
{
	GtkWidget *l;
	gint i;

	for (i = 0; i < 10; i++) {
		l = g_object_new(GTK_TYPE_LABEL,
						 "visible", TRUE,
						 "label", "XXX",
						 NULL);
		gtk_container_add_with_properties(GTK_CONTAINER(fixed), l,
		                                  "x", i * 20,
		                                  "y", i * 20,
		                                  NULL);
		gb_object_animate(l, GB_ANIMATION_EASE_IN_OUT_QUAD, 1000,
						  "x", 500,
						  "y", 50 * i,
						  NULL);
	}

	return FALSE;
}

static gboolean
timeout_cb2 (gpointer data)
{
	GtkWidget *hbox = data;
	GtkWidget *vscroller;
	GtkAdjustment *adj;
	gint i;

	for (i = 0; i < 3; i++) {
		adj = g_object_new(GTK_TYPE_ADJUSTMENT,
						   "lower", 0.0,
						   "upper", 1.0,
						   "value", 0.0,
						   NULL);
		vscroller = g_object_new(GTK_TYPE_VSCROLLBAR,
								 "visible", TRUE,
								 "adjustment", adj,
								 NULL);
		gtk_container_add_with_properties(GTK_CONTAINER(hbox), vscroller,
		                                  "expand", FALSE,
		                                  NULL);
		gb_object_animate(adj, GB_ANIMATION_EASE_IN_OUT_QUAD, 2000 / (i + 1),
						  "value", 1.0,
						  NULL);
	}

	return FALSE;
}

gint
main (gint   argc,
	  gchar *argv[])
{
	GtkWidget *hbox;

	gtk_init(&argc, &argv);

	window = g_object_new(GTK_TYPE_WINDOW,
						  "title", "Animations",
						  "default-width", 640,
						  "default-height", 480,
						  NULL);
	hbox = g_object_new(GTK_TYPE_HBOX,
						"visible", TRUE,
						NULL);
	gtk_container_add(GTK_CONTAINER(window), hbox);
	fixed = g_object_new(GTK_TYPE_FIXED,
						 "visible", TRUE,
						 NULL);
	gtk_container_add(GTK_CONTAINER(hbox), fixed);

	g_timeout_add(500, timeout_cb, NULL);
	g_timeout_add(1500, timeout_cb2, hbox);

	g_signal_connect(window, "delete-event", gtk_main_quit, NULL);
	gtk_window_present(GTK_WINDOW(window));
	gtk_main();

	return 0;
}
