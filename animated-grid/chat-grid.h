/* chat-grid.h
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

#ifndef CHAT_GRID_H
#define CHAT_GRID_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define CHAT_TYPE_GRID            (chat_grid_get_type())
#define CHAT_GRID(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CHAT_TYPE_GRID, ChatGrid))
#define CHAT_GRID_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), CHAT_TYPE_GRID, ChatGrid const))
#define CHAT_GRID_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  CHAT_TYPE_GRID, ChatGridClass))
#define CHAT_IS_GRID(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CHAT_TYPE_GRID))
#define CHAT_IS_GRID_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  CHAT_TYPE_GRID))
#define CHAT_GRID_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  CHAT_TYPE_GRID, ChatGridClass))

typedef struct _ChatGrid        ChatGrid;
typedef struct _ChatGridClass   ChatGridClass;
typedef struct _ChatGridPrivate ChatGridPrivate;

struct _ChatGrid
{
	GtkFixed parent;

	/*< private >*/
	ChatGridPrivate *priv;
};

struct _ChatGridClass
{
	GtkFixedClass parent_class;
};

GType chat_grid_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* CHAT_GRID_H */
