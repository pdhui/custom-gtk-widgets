#ifndef __BDB_LIST_STORE_H__
#define __BDB_LIST_STORE_H__

#include <glib-object.h>
#include <gtk/gtktreemodel.h>
#include <db.h>

G_BEGIN_DECLS

#define BDB_TYPE_LIST_STORE bdb_list_store_get_type()

#define BDB_LIST_STORE(obj) ( \
	G_TYPE_CHECK_INSTANCE_CAST ((obj), \
	BDB_TYPE_LIST_STORE, \
	BdbListStore))

#define BDB_LIST_STORE_CLASS(klass) ( \
	G_TYPE_CHECK_CLASS_CAST ((klass), \
	BDB_TYPE_LIST_STORE, \
	BdbListStoreClass))

#define BDB_IS_LIST_STORE(obj) ( \
	G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
	BDB_TYPE_LIST_STORE))

#define BDB_IS_LIST_STORE_CLASS(klass) ( \
	G_TYPE_CHECK_CLASS_TYPE ((klass), \
	BDB_TYPE_LIST_STORE))

#define BDB_LIST_STORE_GET_CLASS(obj) ( \
	G_TYPE_INSTANCE_GET_CLASS ((obj), \
	BDB_TYPE_LIST_STORE, \
	BdbListStoreClass))

typedef struct _BdbListStore      BdbListStore;
typedef struct _BdbListStoreClass BdbListStoreClass;

struct _BdbListStore
{
	GObject parent;
};

struct _BdbListStoreClass
{
	GObjectClass parent_class;
};

GType         bdb_list_store_get_type  (void);
BdbListStore* bdb_list_store_new       (void);

void          bdb_list_store_append    (BdbListStore *self, GtkTreeIter *iter);
gboolean      bdb_list_store_remove    (BdbListStore *self, GtkTreeIter *iter);
gboolean      bdb_list_store_set_db    (BdbListStore *self, DB *db, GError **error);
DB*           bdb_list_store_get_db    (BdbListStore *self);
void          bdb_list_store_set_value (BdbListStore *self,
                                        GtkTreeIter  *iter,
                                        gint          column,
                                        GValue       *value);

G_END_DECLS

#endif /* __BDB_LIST_STORE_H__ */
