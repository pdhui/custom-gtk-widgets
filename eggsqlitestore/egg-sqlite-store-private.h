/**
 * egg-sqlite-store-private.h - GtkTreeStore using SQLite for storage.
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

#ifndef __EGG_SQLITE_STORE_PRIVATE_H__
#define __EGG_SQLITE_STORE_PRIVATE_H__

#include <sqlite3.h>

#define EGG_SQLITE_STORE_ERROR g_quark_from_string("EggSqliteStore")

#define EGG_SQLITE_STORE_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), \
                                         EGG_TYPE_SQLITE_STORE,           \
                                         EggSqliteStorePrivate))

typedef struct _EggSqliteStorePrivate EggSqliteStorePrivate;
struct _EggSqliteStorePrivate {
    gchar   *table;
    sqlite3 *dbh;
    GTree   *cache;  /* data cache indexed by oid (gchar*)      */
	GTree   *rcache; /* data cache indexed by row offset (gint) */
};

/* GObject implementations */
static void              egg_sqlite_store_init            (EggSqliteStore       *self);
static void              egg_sqlite_store_class_init      (EggSqliteStoreClass  *klass);
static void              egg_sqlite_store_finalize        (GObject              *obj);

/* GtkTreeModelIface implementation */
static void              egg_sqlite_store_tree_model_init (GtkTreeModelIface *iface);
static GtkTreeModelFlags egg_sqlite_store_get_flags       (GtkTreeModel      *tree_model);
static gint              egg_sqlite_store_get_n_columns   (GtkTreeModel      *tree_model);
static GType             egg_sqlite_store_get_column_type (GtkTreeModel      *tree_model,
                                                           gint               index);
static gboolean          egg_sqlite_store_get_iter        (GtkTreeModel      *tree_model,
                                                           GtkTreeIter       *iter,
                                                           GtkTreePath       *path);
static GtkTreePath*      egg_sqlite_store_get_path        (GtkTreeModel      *tree_model,
                                                           GtkTreeIter       *iter);
static void              egg_sqlite_store_get_value       (GtkTreeModel      *tree_model,
                                                           GtkTreeIter       *iter,
                                                           gint               column,
                                                           GValue            *value);
static gboolean          egg_sqlite_store_iter_next       (GtkTreeModel      *tree_model,
                                                           GtkTreeIter       *iter);
static gboolean          egg_sqlite_store_iter_children   (GtkTreeModel      *tree_model,
                                                           GtkTreeIter       *iter,
                                                           GtkTreeIter       *parent);
static gboolean          egg_sqlite_store_iter_has_child  (GtkTreeModel      *tree_model,
                                                           GtkTreeIter       *iter);
static gint              egg_sqlite_store_iter_n_children (GtkTreeModel      *tree_model,
                                                           GtkTreeIter       *iter);
static gboolean          egg_sqlite_store_iter_nth_child  (GtkTreeModel      *tree_model,
                                                           GtkTreeIter       *iter,
                                                           GtkTreeIter       *parent,
                                                           gint               n);
static gboolean          egg_sqlite_store_iter_parent     (GtkTreeModel      *tree_model,
                                                           GtkTreeIter       *iter,
                                                           GtkTreeIter       *child);

#endif /* __EGG_SQLITE_STORE_PRIVATE_H__ */
