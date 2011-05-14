/* chat-avatar.c
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

#include "chat-avatar.h"

G_DEFINE_TYPE(ChatAvatar, chat_avatar, GTK_TYPE_EVENT_BOX)

struct _ChatAvatarPrivate
{
	GtkWidget *image;
};

/**
 * chat_avatar_finalize:
 * @object: (in): A #ChatAvatar.
 *
 * Finalizer for a #ChatAvatar instance.  Frees any resources held by
 * the instance.
 *
 * Returns: None.
 * Side effects: None.
 */
static void
chat_avatar_finalize (GObject *object)
{
	G_OBJECT_CLASS(chat_avatar_parent_class)->finalize(object);
}

/**
 * chat_avatar_class_init:
 * @klass: (in): A #ChatAvatarClass.
 *
 * Initializes the #ChatAvatarClass and prepares the vtable.
 *
 * Returns: None.
 * Side effects: None.
 */
static void
chat_avatar_class_init (ChatAvatarClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS(klass);
	object_class->finalize = chat_avatar_finalize;
	g_type_class_add_private(object_class, sizeof(ChatAvatarPrivate));
}

/**
 * chat_avatar_init:
 * @avatar: (in): A #ChatAvatar.
 *
 * Initializes the newly created #ChatAvatar instance.
 *
 * Returns: None.
 * Side effects: None.
 */
static void
chat_avatar_init (ChatAvatar *avatar)
{
	ChatAvatarPrivate *priv;

	avatar->priv = priv =
		G_TYPE_INSTANCE_GET_PRIVATE(avatar,
		                            CHAT_TYPE_AVATAR,
		                            ChatAvatarPrivate);

	g_object_set(avatar,
	             "visible-window", FALSE,
	             NULL);

	priv->image = g_object_new(GTK_TYPE_IMAGE,
	                           "icon-size", GTK_ICON_SIZE_SMALL_TOOLBAR,
	                           "icon-name", "avatar-default",
	                           "visible", TRUE,
	                           NULL);
	gtk_container_add(GTK_CONTAINER(avatar), priv->image);
}
