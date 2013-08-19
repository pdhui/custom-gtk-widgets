#include <clutter/clutter.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

gint
main (gint   argc,
      gchar *argv[])
{
   ClutterContent *image;
   ClutterActor *stage;
   ClutterActor *box;
   GdkPixbuf *pixbuf;

   if (clutter_init(&argc, &argv) != CLUTTER_INIT_SUCCESS) {
      g_printerr("Failed to initialize clutter.\n");
      return 1;
   }

   stage = clutter_stage_new();
   clutter_stage_set_title (CLUTTER_STAGE (stage), "Image Viewer");
   clutter_actor_set_size(stage, 1400, 1200);
   clutter_actor_set_position(stage, 100, 100);
   g_signal_connect (stage, "destroy", G_CALLBACK (clutter_main_quit), NULL);
   clutter_actor_show(stage);

   pixbuf = gdk_pixbuf_new_from_file("testimage_8000.png", NULL);
   g_assert(pixbuf);

   image = clutter_image_new();
   clutter_image_set_data(CLUTTER_IMAGE(image),
                          gdk_pixbuf_get_pixels(pixbuf),
                          gdk_pixbuf_get_has_alpha(pixbuf) ?
                             COGL_PIXEL_FORMAT_RGBA_8888 :
                             COGL_PIXEL_FORMAT_RGB_888,
                          gdk_pixbuf_get_width(pixbuf),
                          gdk_pixbuf_get_height(pixbuf),
                          gdk_pixbuf_get_rowstride(pixbuf),
                          NULL);
   g_object_unref(pixbuf);

   box = clutter_actor_new();
   clutter_actor_set_position(box, 0, 0);
   clutter_actor_set_height(box, 8000);
   clutter_actor_set_width(box, 1200);
   //clutter_actor_add_constraint(box, clutter_bind_constraint_new(stage, CLUTTER_BIND_WIDTH, 0.0));
   clutter_actor_set_content(box, image);
   clutter_actor_add_child(stage, box);
   clutter_actor_show(box);

   clutter_stage_set_fullscreen(CLUTTER_STAGE(stage), TRUE);

   clutter_actor_animate(box,
                         CLUTTER_EASE_IN_OUT_QUAD,
                         4000,
                         "y", -8000.0,
                         NULL);

   clutter_main();

   return 0;
}
