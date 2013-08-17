/* img-view.h
 *
 * Copyright (C) 2013 Christian Hergert <christian@hergert.me>
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef IMG_VIEW_H
#define IMG_VIEW_H

#include <gtk/gtk.h>
#include <cairo-gobject.h>

G_BEGIN_DECLS

#define IMG_TYPE_VIEW            (img_view_get_type())
#define IMG_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), IMG_TYPE_VIEW, ImgView))
#define IMG_VIEW_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), IMG_TYPE_VIEW, ImgView const))
#define IMG_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  IMG_TYPE_VIEW, ImgViewClass))
#define IMG_IS_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IMG_TYPE_VIEW))
#define IMG_IS_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  IMG_TYPE_VIEW))
#define IMG_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  IMG_TYPE_VIEW, ImgViewClass))

typedef struct _ImgView        ImgView;
typedef struct _ImgViewClass   ImgViewClass;
typedef struct _ImgViewPrivate ImgViewPrivate;

struct _ImgView
{
   GtkDrawingArea parent;

   /*< private >*/
   ImgViewPrivate *priv;
};

struct _ImgViewClass
{
   GtkDrawingAreaClass parent_class;
};

ImgView         *img_view_new         (void);
cairo_surface_t *img_view_get_surface (ImgView         *self);
GType            img_view_get_type    (void) G_GNUC_CONST;
void             img_view_set_surface (ImgView         *self,
                                       cairo_surface_t *surface);

G_END_DECLS

#endif /* IMG_VIEW_H */
