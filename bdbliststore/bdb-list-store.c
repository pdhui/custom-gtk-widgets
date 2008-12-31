#include "bdb-list-store.h"

#include <glib.h>
#include <string.h>

#define CLEAR_DBT(dbt)   (memset(&(dbt), 0, sizeof(dbt)))
#define FREE_DBT(dbt)    if ((dbt.flags & (DB_DBT_MALLOC|DB_DBT_REALLOC)) && \
                              dbt.data != NULL) { g_free(dbt.data); dbt.data = NULL; }

static void tree_model_init (GtkTreeModelIface *iface);

G_DEFINE_TYPE_EXTENDED (BdbListStore, bdb_list_store, G_TYPE_OBJECT, 0,
                        G_IMPLEMENT_INTERFACE (GTK_TYPE_TREE_MODEL, tree_model_init));

#define LIST_STORE_PRIVATE(o)              \
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), \
	BDB_TYPE_LIST_STORE,               \
	BdbListStorePrivate))

#define BDB_QUARK (g_quark_from_static_string("bdb-list-store"))

typedef struct _BdbListStorePrivate BdbListStorePrivate;

struct _BdbListStorePrivate
{
	gint      stamp;
	GType     g_type;
	DB       *dbp;
	gboolean  dirty;
	gint      n_keys;
};

static void
bdb_list_store_get_property (GObject    *object,
                              guint       property_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
	switch (property_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
bdb_list_store_set_property (GObject      *object,
                              guint         property_id,
                              const GValue *value,
			      GParamSpec   *pspec)
{
	switch (property_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
bdb_list_store_dispose (GObject *object)
{
	if (G_OBJECT_CLASS (bdb_list_store_parent_class)->dispose)
		G_OBJECT_CLASS (bdb_list_store_parent_class)->dispose (object);
}

static void
bdb_list_store_finalize (GObject *object)
{
	G_OBJECT_CLASS (bdb_list_store_parent_class)->finalize (object);
}

static void
bdb_list_store_class_init (BdbListStoreClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (BdbListStorePrivate));

	object_class->get_property = bdb_list_store_get_property;
	object_class->set_property = bdb_list_store_set_property;
	object_class->dispose      = bdb_list_store_dispose;
	object_class->finalize     = bdb_list_store_finalize;
}

static gint
get_n_keys (BdbListStore *self)
{
	g_return_val_if_fail (BDB_IS_LIST_STORE (self), 0);
	
	BdbListStorePrivate *priv = LIST_STORE_PRIVATE (self);
	if (!priv->dirty)
		return priv->n_keys;
	
	g_return_val_if_fail (priv->dbp != NULL, 0);
	
	DB_BTREE_STAT *stat;
	if (priv->dbp->stat (priv->dbp, NULL, &stat, DB_FAST_STAT) == 0) {
		priv->n_keys = stat->bt_nkeys;
		priv->dirty = FALSE;
	}
	
	g_free (stat);
	
	return priv->n_keys;
}

static GtkTreeModelFlags
get_flags (GtkTreeModel *tree_model)
{
	return GTK_TREE_MODEL_LIST_ONLY;
}

static gboolean
get_iter (GtkTreeModel *tree_model, GtkTreeIter *iter, GtkTreePath *path)
{
	g_return_val_if_fail (BDB_IS_LIST_STORE (tree_model), FALSE);
	
	BdbListStorePrivate *priv    = LIST_STORE_PRIVATE (tree_model);
	gint                *indices = gtk_tree_path_get_indices (path);
	gint                 depth   = gtk_tree_path_get_depth (path);
	
	if (depth == 0)
		return FALSE;
	
	gint n_keys = get_n_keys (BDB_LIST_STORE (tree_model));
	if (n_keys <= indices[0])
		return FALSE;
	
	iter->stamp = priv->stamp;
	iter->user_data = GINT_TO_POINTER (indices[0] + 1);
	
	return TRUE;
}

static gint
get_n_columns (GtkTreeModel *tree_model)
{
	return 1;
}

static GType
get_column_type (GtkTreeModel *tree_model, gint index)
{
	return G_TYPE_STRING;
	//return LIST_STORE_PRIVATE (tree_model)->g_type;
}

static gboolean
iter_next (GtkTreeModel *tree_model, GtkTreeIter *iter)
{
	g_return_val_if_fail (BDB_IS_LIST_STORE (tree_model), FALSE);
	
	BdbListStorePrivate *priv = LIST_STORE_PRIVATE (tree_model);
	g_return_val_if_fail (iter->stamp == priv->stamp, FALSE);
	
	gint n_keys = get_n_keys (BDB_LIST_STORE (tree_model));
	gint next = GPOINTER_TO_INT (iter->user_data) + 1;
	
	if (n_keys < next)
		return FALSE;
	
	iter->user_data = GINT_TO_POINTER (next);
	
	return TRUE;
}

static gboolean
iter_nth_child (GtkTreeModel *tree_model,
                GtkTreeIter *iter,
		GtkTreeIter *parent,
		gint n)
{
	g_return_val_if_fail (BDB_IS_LIST_STORE (tree_model), FALSE);
	
	if (parent)
		return FALSE;
	
	gint n_keys = 0;
	if ((n_keys = get_n_keys (BDB_LIST_STORE (tree_model))) <= n)
		return FALSE;
	
	BdbListStorePrivate *priv = LIST_STORE_PRIVATE (tree_model);
	
	iter->stamp = priv->stamp;
	iter->user_data = GINT_TO_POINTER (n);
	
	return TRUE;
}

static void
get_value (GtkTreeModel *tree_model,
	   GtkTreeIter  *iter,
           gint          column,
	   GValue       *value)
{
	g_return_if_fail (BDB_IS_LIST_STORE (tree_model));
	
	BdbListStorePrivate *priv = LIST_STORE_PRIVATE (tree_model);
	g_return_if_fail (priv->stamp == iter->stamp);
	
	DBT key, data;
	db_recno_t keydata;
	DB_TXN *txn = NULL;
	gint ret;
	gint flags = 0;
	
	CLEAR_DBT (key);
	CLEAR_DBT (data);
	
	keydata = GPOINTER_TO_INT (iter->user_data);
	key.data = &keydata;
	data.flags = DB_DBT_MALLOC;
	
	if ((ret = priv->dbp->get (priv->dbp, txn, &key, &data, flags)) != 0) {
		g_warning ("get_value: %s", db_strerror (ret));
		return;
	}
	
	g_value_init (value, G_TYPE_STRING);
	g_value_set_string (value, data.data);
	
	FREE_DBT (key);
	FREE_DBT (data);
}

static gboolean
iter_children (GtkTreeModel *tree_model,
               GtkTreeIter  *iter,
               GtkTreeIter  *parent)
{
	return FALSE;
}

static gboolean
iter_has_child (GtkTreeModel *tree_model,
                GtkTreeIter  *iter)
{
	return FALSE;
}

static int
iter_n_children (GtkTreeModel *tree_model, GtkTreeIter *iter)
{
	g_return_val_if_fail (BDB_IS_LIST_STORE (tree_model), 0);
	if (iter)
		return 0;
	return get_n_keys (BDB_LIST_STORE (tree_model));
}

static gboolean
iter_parent (GtkTreeModel *tree_model,
             GtkTreeIter  *iter,
             GtkTreeIter  *parent)
{
	return FALSE;
}

static GtkTreePath*
get_path (GtkTreeModel *tree_model, GtkTreeIter *iter)
{
	BdbListStorePrivate *priv = LIST_STORE_PRIVATE (tree_model);
	g_return_val_if_fail (priv->stamp == iter->stamp, NULL);
	
	gint index = GPOINTER_TO_INT (iter->user_data);
	return gtk_tree_path_new_from_indices (index - 1, -1);
}

static void
tree_model_init (GtkTreeModelIface *iface)
{
	iface->get_flags       = get_flags;
	iface->get_iter        = get_iter;
	iface->get_n_columns   = get_n_columns;
	iface->get_column_type = get_column_type;
	iface->iter_next       = iter_next;
	iface->iter_nth_child  = iter_nth_child;
	iface->get_value       = get_value;
	iface->iter_children   = iter_children;
	iface->iter_has_child  = iter_has_child;
	iface->iter_n_children = iter_n_children;
	iface->iter_parent     = iter_parent;
	iface->get_path        = get_path;
}

static void
bdb_list_store_init (BdbListStore *self)
{
	BdbListStorePrivate *priv = LIST_STORE_PRIVATE (self);
	
	priv->stamp = g_random_int ();
	priv->n_keys = 0;
	priv->dirty = TRUE;
}

BdbListStore*
bdb_list_store_new (void)
{
	return g_object_new (BDB_TYPE_LIST_STORE, NULL);
}

DB*
bdb_list_store_get_db (BdbListStore *self)
{
	g_return_val_if_fail (BDB_IS_LIST_STORE (self), NULL);
	return LIST_STORE_PRIVATE (self)->dbp;
}

gboolean
bdb_list_store_set_db (BdbListStore *self, DB *db, GError **error)
{
	g_return_val_if_fail (BDB_IS_LIST_STORE (self), FALSE);
	g_return_val_if_fail (db != NULL, FALSE);
	
	if (db->type != DB_RECNO) {
		if (error && *error == NULL)
			*error = g_error_new_literal (BDB_QUARK, 0, "DB must be of type DB_RECNO");
		return FALSE;
	}
	
	BdbListStorePrivate *priv = LIST_STORE_PRIVATE (self);
	
	if (priv->dbp != NULL) {
		if (error && *error == NULL)
			*error = g_error_new (BDB_QUARK, 0, "Cannot set db twice");
		return FALSE;
	}
	
	guint flags = 0;
	gint ret = 0;
	
	if ((ret = db->get_flags (db, &flags)) != 0) {
		if (error && *error == NULL)
			*error = g_error_new (BDB_QUARK, 0, "Cannot retrieve db flags");
		return FALSE;
	}
	
	if (!(flags & DB_RENUMBER)) {
		if (error && *error == NULL)
			*error = g_error_new (BDB_QUARK, 0, "Please alter DB with db->set_flags(DB_RENUMBER)");
		return FALSE;
	}
	
	priv->dbp = db;
	
	return TRUE;
}

void
bdb_list_store_append (BdbListStore *self, GtkTreeIter *iter)
{
	g_return_if_fail (BDB_IS_LIST_STORE (self));
	
	BdbListStorePrivate *priv = LIST_STORE_PRIVATE (self);
	
	DBT key, data;
	DB_TXN *txn = NULL;
	db_recno_t recno;
	gint ret;
	
	CLEAR_DBT (key);
	CLEAR_DBT (data);
	
	recno = 0;
	key.data = &recno;
	key.size = sizeof (db_recno_t);
	key.ulen = key.size;
	key.flags = DB_DBT_USERMEM;
	
	data.data = "";
	data.size = sizeof (char);
	data.ulen = strlen ("") + 1;
	data.flags = DB_DBT_USERMEM;
	
	if ((ret = priv->dbp->put (priv->dbp, txn, &key, &data, DB_APPEND)) != 0)
		g_warning ("bdb_list_store_append: %s", db_strerror (ret));
	
	priv->dirty = TRUE;
	
	iter->stamp = priv->stamp;
	iter->user_data = GINT_TO_POINTER (get_n_keys (self));
	
	FREE_DBT (key);
	FREE_DBT (data);
	
	GtkTreePath *path = gtk_tree_model_get_path (GTK_TREE_MODEL (self), iter);
	gtk_tree_model_row_inserted (GTK_TREE_MODEL (self), path, iter);
	gtk_tree_path_free (path);
}

void
bdb_list_store_set_value (BdbListStore *self,
                          GtkTreeIter *iter,
                          gint column,
                          GValue *value)
{
	g_return_if_fail (BDB_IS_LIST_STORE (self));
	
	BdbListStorePrivate *priv = LIST_STORE_PRIVATE (self);
	g_return_if_fail (priv->stamp == iter->stamp);
	g_return_if_fail (G_VALUE_HOLDS_STRING (value));
	
	DBT key, data;
	DB_TXN *txn = NULL;
	db_recno_t recno = GPOINTER_TO_INT (iter->user_data);
	const gchar *str = g_value_get_string (value);
	gint ret = 0;
	
	CLEAR_DBT (key);
	CLEAR_DBT (value);
	
	key.data = &recno;
	key.size = sizeof (db_recno_t);
	key.ulen = key.size;
	key.flags = DB_DBT_USERMEM;
	
	data.data = (void*)str;
	data.size = strlen (str) + 1;
	data.ulen = data.size;
	data.flags = DB_DBT_USERMEM;
	
	if ((ret = priv->dbp->put (priv->dbp, txn, &key, &data, 0)) != 0)
		g_warning ("bdb_list_store_set_value: %s", db_strerror (ret));
	else
		priv->dirty = TRUE;
	
	FREE_DBT (key);
	FREE_DBT (data);
	
	GtkTreePath *path = gtk_tree_model_get_path (GTK_TREE_MODEL (self), iter);
	gtk_tree_model_row_changed (GTK_TREE_MODEL (self), path, iter);
	gtk_tree_path_free (path);
}

gboolean
bdb_list_store_remove (BdbListStore *self, GtkTreeIter *iter)
{
	BdbListStorePrivate *priv = LIST_STORE_PRIVATE (self);
	g_return_val_if_fail (priv->stamp == iter->stamp, FALSE);
	g_return_val_if_fail (priv->dbp != NULL, FALSE);
	
	DBT key;
	DB_TXN *txn = NULL;
	gint ret = 0;
	gint flags = 0;
	GtkTreePath *path;
	
	CLEAR_DBT (key);
	
	db_recno_t recno = GPOINTER_TO_INT (iter->user_data);
	key.data = &recno;
	key.size = sizeof (db_recno_t);
	key.ulen = key.size;
	key.flags = DB_DBT_USERMEM;
	
	if ((ret = priv->dbp->del (priv->dbp, txn, &key, flags)) != 0)
		g_warning ("Could not remove ");
	
	priv->dirty = TRUE;
	path = get_path (GTK_TREE_MODEL (self), iter);
	
	gboolean is_valid = FALSE;
	
	if (get_n_keys (self) >= GPOINTER_TO_INT (iter->user_data)) {
		is_valid = TRUE;
	}
	else {
		iter->stamp = 0;
		iter->user_data = NULL;
	}
	
	gtk_tree_model_row_deleted (GTK_TREE_MODEL (self), path);
	gtk_tree_path_free (path);
	
	FREE_DBT (key);
	
	return is_valid;
}
