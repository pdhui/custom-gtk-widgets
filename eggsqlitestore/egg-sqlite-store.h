/**
 * egg-sqlite-store.h - GtkTreeStore using SQLite for storage.
 * 
 * Copyright (C) 2007   Christian Hergert <chrisian.hergert@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
**/

#ifndef __EGG_SQLITE_STORE__
#define __EGG_SQLITE_STORE__

#include <gtk/gtk.h>

#define EGG_TYPE_SQLITE_STORE             (egg_sqlite_store_get_type())
#define EGG_SQLITE_STORE(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj), \
                                           EGG_TYPE_SQLITE_STORE,            \
                                           EggSqliteStore))
#define EGG_SQLITE_STORE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass),  \
                                           EGG_TYPE_SQLITE_STORE,GObject))
#define EGG_IS_SQLITE_STORE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj), \
                                           EGG_TYPE_SQLITE_STORE))
#define EGG_IS_SQLITE_STORE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass),  \
                                           EGG_TYPE_SQLITE_STORE))
#define EGG_SQLITE_STORE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj),  \
                                           EGG_TYPE_SQLITE_SQLITE_STORE,     \
                                           EggSqliteStoreClass))

typedef struct _EggSqliteStore EggSqliteStore;
typedef struct _EggSqliteStoreClass EggSqliteStoreClass;

struct _EggSqliteStore {
	 GObject parent;
	 gint    n_columns;
	 gint    stamp;
};

struct _EggSqliteStoreClass {
    GObjectClass parent_class;
};

GType           egg_sqlite_store_get_type    (void) G_GNUC_CONST;
GtkTreeModel*   egg_sqlite_store_new         (void);

void            egg_sqlite_store_set_filename  (EggSqliteStore  *self,
                                                const gchar     *filename,
                                                GError         **error);
void            egg_sqlite_store_set_table     (EggSqliteStore  *self,
                                                const gchar     *table,
                                                GError         **error);
const gchar*    egg_sqlite_store_get_table     (EggSqliteStore  *self);
void            egg_sqlite_store_append        (EggSqliteStore  *self,
                                                GtkTreeIter     *iter);
void            egg_sqlite_store_set           (EggSqliteStore  *self,
                                                GtkTreeIter     *iter,
                                                ...);
void            egg_sqlite_store_remove        (EggSqliteStore  *self,
                                                GtkTreeIter     *iter);
void            egg_sqlite_store_clear         (EggSqliteStore  *self);
gboolean        egg_sqlite_store_iter_is_valid (EggSqliteStore  *self,
                                                GtkTreeIter     *iter);

#endif /* __EGG_SQLITE_STORE__ */
