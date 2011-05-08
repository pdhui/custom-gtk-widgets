/* gb-anim-bin.h
 *
 * Copyright (C) 2011 Christian Hergert <chris@dronelabs.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GB_ANIM_BIN_H
#define GB_ANIM_BIN_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GB_TYPE_ANIM_BIN            (gb_anim_bin_get_type())
#define GB_ANIM_BIN(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_ANIM_BIN, GbAnimBin))
#define GB_ANIM_BIN_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_ANIM_BIN, GbAnimBin const))
#define GB_ANIM_BIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_ANIM_BIN, GbAnimBinClass))
#define GB_IS_ANIM_BIN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_ANIM_BIN))
#define GB_IS_ANIM_BIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_ANIM_BIN))
#define GB_ANIM_BIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_ANIM_BIN, GbAnimBinClass))

typedef struct _GbAnimBin        GbAnimBin;
typedef struct _GbAnimBinClass   GbAnimBinClass;
typedef struct _GbAnimBinPrivate GbAnimBinPrivate;

struct _GbAnimBin
{
	GtkEventBox parent;

	/*< private >*/
	GbAnimBinPrivate *priv;
};

struct _GbAnimBinClass
{
	GtkEventBoxClass parent_class;
};

GType gb_anim_bin_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* GB_ANIM_BIN_H */
