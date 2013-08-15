#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>

#include "gb-animation.h"

static gboolean
begin_scroll (gpointer data)
{
   GtkAdjustment *adj = gtk_scrollable_get_vadjustment (data);
   gdouble value = gtk_adjustment_get_upper (adj);

   gb_object_animate(adj, GB_ANIMATION_EASE_IN_OUT_QUAD, 5000,
                     "value", value / 2.0,
                     NULL);

   return G_SOURCE_REMOVE;
}

static gboolean
begin_scroll2 (gpointer data)
{
   GtkAdjustment *adj = gtk_scrollable_get_vadjustment (data);

   gb_object_animate(adj, GB_ANIMATION_EASE_IN_OUT_QUAD, 3000,
                     "value", 0.0,
                     NULL);

   return G_SOURCE_REMOVE;
}

static gboolean
begin_scroll3 (gpointer data)
{
   GtkAdjustment *adj = gtk_scrollable_get_vadjustment (data);
   gdouble value = gtk_adjustment_get_upper (adj);

   gb_object_animate(adj, GB_ANIMATION_EASE_IN_OUT_QUAD, 4000,
                     "value", value / 5.0,
                     NULL);

   return G_SOURCE_REMOVE;
}

static void
setup (void)
{
   GtkTextBuffer *buffer;
   GtkWidget *window;
   GtkWidget *scrolled_window;
   GtkWidget *text_view;
   gsize length = 0;
   char *contents = NULL;

   text_view = g_object_new(GTK_SOURCE_TYPE_VIEW,
                            "show-line-numbers", TRUE,
                            "visible", TRUE,
                            NULL);
   gtk_widget_override_font(GTK_WIDGET(text_view),
                            pango_font_description_from_string("Monospace"));
   buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
   g_file_get_contents("gb-animation.c", &contents, &length, NULL);
   gtk_text_buffer_set_text(GTK_TEXT_BUFFER(buffer), contents, length);
   g_free(contents);

   scrolled_window = g_object_new(GTK_TYPE_SCROLLED_WINDOW,
                                  "visible", TRUE,
                                  NULL);
   gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);

   window = g_object_new(GTK_TYPE_WINDOW,
                         "default-width", 600,
                         "default-height", 600,
                         "window-position", GTK_WIN_POS_CENTER,
                         "title", "Scroll Test",
                         NULL);
   g_signal_connect(window, "delete-event", gtk_main_quit, NULL);
   gtk_container_add(GTK_CONTAINER(window), scrolled_window);
   gtk_window_present(GTK_WINDOW(window));

   g_timeout_add(500, begin_scroll, text_view);
   g_timeout_add(6000, begin_scroll2, text_view);
   g_timeout_add(10000, begin_scroll3, text_view);
}

int
main (int   argc,
      char *argv[])
{
   gtk_init(&argc, &argv);
   setup();
   gtk_main();
   return 0;
}
