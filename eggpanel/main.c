#include <stdlib.h>
#include <gtk/gtk.h>
#include "egg-panel.h"

gint
main (gint   argc,
      gchar *argv[])
{
	GtkWidget *panel1;

	gtk_init(&argc, &argv);

	panel1 = egg_panel_new();
	egg_panel_set_title(EGG_PANEL(panel1), "Panel 1");
	gtk_widget_show(panel1);

	gtk_main();

	return EXIT_SUCCESS;
}
