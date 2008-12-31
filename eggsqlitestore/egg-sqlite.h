/**
 * sqlite.h - GtkTreeStore using SQLite for storage.
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

#ifndef __EGG_SQLITE_H__
#define __EGG_SQLITE_H__

#include <sqlite3.h>
#include <glib.h>

gint    egg_sqlite_count_rows      (sqlite3 *sqlite, gchar *table);
gint    egg_sqlite_fetch_row_pos   (sqlite3 *sqlite, gchar *table, gchar *oid);
GPtrArray* egg_sqlite_fetch_next      (sqlite3 *sqlite, gchar *table, gchar *last_oid);
GPtrArray* egg_sqlite_fetch_row       (sqlite3 *sqlite, gchar *table, gchar *oid);
GPtrArray* egg_sqlite_fetch_nth_row   (sqlite3 *sqlite, gchar *table, gint index);
gint    egg_sqlite_fetch_n_columns (sqlite3 *sqlite, gchar *table);

#endif /* __EGG_SQLITE_H__ */
