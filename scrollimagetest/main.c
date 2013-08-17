#include <gtk/gtk.h>

#include "gb-animation.h"
#include "img-view.h"

static guint realize_handler;

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

cairo_surface_t *
image_to_xlib_surface (GdkWindow       *window,
                       cairo_surface_t *surface)
{
   cairo_surface_t *xlib_surface;
   cairo_t *cr;
   int width;
   int height;

   width = cairo_image_surface_get_width(surface);
   height = cairo_image_surface_get_height(surface);
   xlib_surface = gdk_window_create_similar_surface(window, CAIRO_CONTENT_COLOR, width, height);

   cr = cairo_create(xlib_surface);
   cairo_set_source_surface(cr, surface, 0, 0);
   cairo_rectangle(cr, 0, 0, width, height);
   cairo_fill(cr);
   cairo_destroy(cr);

   return xlib_surface;
}

static void
make_surface (GtkWidget       *widget,
              cairo_surface_t *surface)
{
   cairo_surface_t *xlib_surface;
   GdkWindow *window;

   window = gtk_widget_get_window(widget);
   xlib_surface = image_to_xlib_surface(window, surface);
   g_assert(xlib_surface);
   img_view_set_surface(IMG_VIEW(widget), xlib_surface);
   //cairo_surface_destroy(surface);

   g_signal_handler_disconnect(widget, realize_handler);
   realize_handler = 0;
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
   gboolean use_xlib;

   gtk_init(&argc, &argv);

   use_xlib = (argc == 1);

   surface = cairo_image_surface_create_from_png("testimage.png");

   window = g_object_new(GTK_TYPE_WINDOW,
                         "default-height", 1400,
                         "default-width", 1200,
                         "title", "Image Viewer",
                         "window-position", GTK_WIN_POS_CENTER,
                         NULL);

   scroller = g_object_new(GTK_TYPE_SCROLLED_WINDOW,
                           "visible", TRUE,
                           NULL);
   gtk_container_add(GTK_CONTAINER(window), scroller);

   view = g_object_new(IMG_TYPE_VIEW,
                       "visible", TRUE,
                       NULL);
   if (use_xlib) {
      g_print("Using Xlib surface\n");
      realize_handler = g_signal_connect_after(view,
                                               "realize",
                                               G_CALLBACK(make_surface),
                                               surface);
   } else {
      g_print("Using Image surface\n");
      img_view_set_surface(IMG_VIEW(view), surface);
      cairo_surface_destroy(surface);
   }
   gtk_container_add(GTK_CONTAINER(scroller), view);

   adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(scroller));

   g_timeout_add(1000, scroll1, adj);
   g_timeout_add(5000, scroll2, adj);
   g_timeout_add(8000, scroll3, adj);

   g_signal_connect(window, "delete-event", gtk_main_quit, NULL);

   gtk_window_present(window);

   gtk_main();

   return 0;
}
