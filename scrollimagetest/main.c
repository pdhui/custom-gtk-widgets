#include <gtk/gtk.h>

#include "gb-animation.h"
#include "img-view.h"

static gboolean
scroll1 (gpointer adj)
{
   gb_object_animate(adj, GB_ANIMATION_EASE_IN_OUT_QUAD, 3500,
                     "value", 3000.0,
                     NULL);
   return FALSE;
}

static gboolean
scroll2 (gpointer adj)
{
   gb_object_animate(adj, GB_ANIMATION_EASE_IN_OUT_QUAD, 1000,
                     "value", 000.0,
                     NULL);
   return FALSE;
}

static gboolean
scroll3 (gpointer adj)
{
   gb_object_animate(adj, GB_ANIMATION_EASE_IN_OUT_QUAD, 1000,
                     "value", 10450.0,
                     NULL);
   return FALSE;
}

gint
main (gint   argc,
      gchar *argv[])
{
   GtkWindow *window;
   GtkWidget *scroller;
   GtkWidget *view;
   GtkAdjustment *adj;
   cairo_surface_t *surface;

   gtk_init(&argc, &argv);

   surface = cairo_image_surface_create_from_png("testimage.png");

   window = g_object_new(GTK_TYPE_WINDOW,
                         "default-height", 480,
                         "default-width", 640,
                         "title", "Image Viewer",
                         "window-position", GTK_WIN_POS_CENTER,
                         NULL);

   scroller = g_object_new(GTK_TYPE_SCROLLED_WINDOW,
                           "visible", TRUE,
                           NULL);
   gtk_container_add(GTK_CONTAINER(window), scroller);

   view = g_object_new(IMG_TYPE_VIEW,
                       "surface", surface,
                       "visible", TRUE,
                       NULL);
   gtk_container_add(GTK_CONTAINER(scroller), view);

   adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(scroller));

   g_timeout_add(1000, scroll1, adj);
   g_timeout_add(5000, scroll2, adj);
   g_timeout_add(8000, scroll3, adj);

   g_signal_connect(window, "delete-event", gtk_main_quit, NULL);

   gtk_window_present(window);

   cairo_surface_destroy(surface);

   gtk_main();

   return 0;
}
