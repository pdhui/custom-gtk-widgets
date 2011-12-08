#include <gtk/gtk.h>

#include "gb-animation.h"
#include "gb-scrolled-window.h"

static gboolean
scroll_to_middle (gpointer user_data)
{
   GtkAdjustment *adj = gtk_scrollable_get_vadjustment(user_data);
   gdouble upper = gtk_adjustment_get_upper(adj);

   gb_object_animate(adj, GB_ANIMATION_EASE_IN_OUT_QUAD, 1500,
                     "value", upper / 3.0,
                     NULL);

   return FALSE;
}

gint
main (gint   argc,
      gchar *argv[])
{
   GtkWindow *window;
   GtkWidget *scroller;
   GtkWidget *textview;

   gtk_init(&argc, &argv);
   window = g_object_new(GTK_TYPE_WINDOW,
                         "border-width", 12,
                         "default-width", 640,
                         "default-height", 480,
                         "title", "Test Window",
                         "window-position", GTK_WIN_POS_CENTER,
                         NULL);
   scroller = g_object_new(GB_TYPE_SCROLLED_WINDOW,
                           "visible", TRUE,
                           NULL);
   gtk_container_add(GTK_CONTAINER(window), scroller);
   textview = g_object_new(GTK_TYPE_TEXT_VIEW,
                           "visible", TRUE,
                           NULL);
   {
      gchar *data = NULL;
      gsize len = 0;

      g_file_get_contents("gb-scrolled-window.c", &data, &len, NULL);
      gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview)),
                               data, len);
      g_free(data);
   }
   gtk_container_add(GTK_CONTAINER(scroller), textview);
   g_signal_connect(window, "delete-event", gtk_main_quit, NULL);
   gtk_widget_show_all(GTK_WIDGET(window));
   gtk_window_present(window);
   g_timeout_add(100, scroll_to_middle, textview);
   gtk_main();
   return 0;
}
