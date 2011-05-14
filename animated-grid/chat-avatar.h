/* chat-avatar.h
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

#ifndef CHAT_AVATAR_H
#define CHAT_AVATAR_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define CHAT_TYPE_AVATAR            (chat_avatar_get_type())
#define CHAT_AVATAR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CHAT_TYPE_AVATAR, ChatAvatar))
#define CHAT_AVATAR_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), CHAT_TYPE_AVATAR, ChatAvatar const))
#define CHAT_AVATAR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  CHAT_TYPE_AVATAR, ChatAvatarClass))
#define CHAT_IS_AVATAR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CHAT_TYPE_AVATAR))
#define CHAT_IS_AVATAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  CHAT_TYPE_AVATAR))
#define CHAT_AVATAR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  CHAT_TYPE_AVATAR, ChatAvatarClass))

typedef struct _ChatAvatar        ChatAvatar;
typedef struct _ChatAvatarClass   ChatAvatarClass;
typedef struct _ChatAvatarPrivate ChatAvatarPrivate;

struct _ChatAvatar
{
	GtkEventBox parent;

	/*< private >*/
	ChatAvatarPrivate *priv;
};

struct _ChatAvatarClass
{
	GtkEventBoxClass parent_class;
};

GType chat_avatar_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* CHAT_AVATAR_H */
