/* chat-grid.c
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

#include <glib/gi18n.h>

#include "chat-grid.h"
#include "ppg-animation.h"

G_DEFINE_TYPE(ChatGrid, chat_grid, GTK_TYPE_FIXED)

struct _ChatGridPrivate
{
	GdkRectangle item_rect;
	guint row_spacing;
	guint column_spacing;
	gboolean need_relayout;
	guint stride;
};

enum
{
	PROP_0,
	PROP_COLUMN_SPACING,
	PROP_ROW_SPACING,
	LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];
static GQuark      gQuarkAnimation;

static void
chat_grid_add (GtkContainer *parent,
               GtkWidget    *child)
{
	ChatGridPrivate *priv;
	GtkRequisition req = { 0 };

	GTK_CONTAINER_CLASS(chat_grid_parent_class)->add(parent, child);

	priv = CHAT_GRID(parent)->priv;

	gtk_widget_get_preferred_size(child, NULL, &req);

	if (req.width > priv->item_rect.width) {
		priv->item_rect.width = req.width;
	}

	if (req.height > priv->item_rect.height) {
		priv->item_rect.height = req.height;
	}

	priv->need_relayout = TRUE;
	gtk_widget_queue_resize(GTK_WIDGET(parent));
}

static void
chat_grid_remove (GtkContainer *parent,
                  GtkWidget    *child)
{
	ChatGridPrivate *priv;
	ChatGrid *grid = (ChatGrid *)parent;

	g_return_if_fail(CHAT_IS_GRID(grid));

	priv = grid->priv;

	GTK_CONTAINER_CLASS(chat_grid_parent_class)->remove(parent, child);

	priv->need_relayout = TRUE;
	gtk_widget_queue_resize(GTK_WIDGET(parent));
}

static void
chat_grid_get_preferred_width (GtkWidget *widget,
                               gint      *minimum_width,
                               gint      *natural_width)
{
	ChatGridPrivate *priv;
	ChatGrid *grid = (ChatGrid *)widget;
	guint border_width;
	gint width;

	g_return_if_fail(CHAT_IS_GRID(grid));

	priv = grid->priv;

	g_object_get(widget, "border-width", &border_width, NULL);
	width = priv->item_rect.width + (border_width * 2);

	if (minimum_width) {
		*minimum_width = width;
	}

	if (natural_width) {
		*natural_width = width;
	}
}

static void
chat_grid_size_allocate (GtkWidget     *widget,
                         GtkAllocation *allocation)
{
	ChatGridPrivate *priv;
	PpgAnimation *anim;
	GtkWidget *child;
	ChatGrid *grid = (ChatGrid *)widget;
	GList *children;
	GList *iter;
	gint border_width;
	gint stride = 0;
	gint width;
	gint x;
	gint y;
	gint cx;
	gint cy;

	g_return_if_fail(CHAT_IS_GRID(grid));

	priv = grid->priv;

	GTK_WIDGET_CLASS(chat_grid_parent_class)->size_allocate(widget, allocation);

	g_object_get(widget, "border-width", &border_width, NULL);

	for (width = priv->item_rect.width;
	     width < (allocation->width - (2 * border_width));
	     width += priv->column_spacing + priv->item_rect.width) {
		stride++;
	}

	if (stride == priv->stride) {
		if (!priv->need_relayout) {
			return;
		}
	}

	priv->stride = stride;

	/*
	 * TODO: There seems to be something wierd causing all the items to align
	 *       to X=0 when animations are disabled. Even though we seem to be
	 *       setting it right.
	 */

	children = gtk_container_get_children(GTK_CONTAINER(widget));
	width = allocation->width - (2 * border_width);
	x = 0;
	y = 0;

	for (iter = children; iter; iter = iter->next) {
		child = iter->data;
		if ((x + priv->item_rect.width) >= width) {
			x = 0;
			y += priv->item_rect.height + priv->row_spacing;
		}
		gtk_container_child_get(GTK_CONTAINER(grid), child,
		                        "x", &cx, "y", &cy, NULL);
		if (cx != x || cy != y) {
			if (!g_getenv("CHAT_DISABLE_ANIMATIONS")) {
				anim = g_object_get_qdata(G_OBJECT(child), gQuarkAnimation);
				if (anim) {
					ppg_animation_stop(anim);
				}
				anim = g_object_animate(child,
				                        PPG_ANIMATION_EASE_IN_OUT_QUAD, 300,
				                        "x", x, "y", y, NULL);
				g_object_set_qdata_full(G_OBJECT(child), gQuarkAnimation,
				                        g_object_ref(anim), g_object_unref);
			} else {
				gtk_container_child_set(GTK_CONTAINER(grid), child,
				                        "x", x, "y", y, NULL);
			}
		}
		x += priv->item_rect.width + priv->column_spacing;
	}

	g_list_free(children);

	priv->need_relayout = FALSE;
}

/**
 * chat_grid_finalize:
 * @object: (in): A #ChatGrid.
 *
 * Finalizer for a #ChatGrid instance.  Frees any resources held by
 * the instance.
 *
 * Returns: None.
 * Side effects: None.
 */
static void
chat_grid_finalize (GObject *object)
{
	G_OBJECT_CLASS(chat_grid_parent_class)->finalize(object);
}

/**
 * chat_grid_set_property:
 * @object: (in): A #GObject.
 * @prop_id: (in): The property identifier.
 * @value: (out): The given property.
 * @pspec: (in): A #ParamSpec.
 *
 * Get a given #GObject property.
 */
static void
chat_grid_get_property (GObject    *object,
                        guint       prop_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
	ChatGrid *grid = CHAT_GRID(object);

	switch (prop_id) {
	case PROP_COLUMN_SPACING:
		g_value_set_uint(value, grid->priv->column_spacing);
		break;
	case PROP_ROW_SPACING:
		g_value_set_uint(value, grid->priv->row_spacing);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

/**
 * chat_grid_set_property:
 * @object: (in): A #GObject.
 * @prop_id: (in): The property identifier.
 * @value: (in): The given property.
 * @pspec: (in): A #ParamSpec.
 *
 * Set a given #GObject property.
 */
static void
chat_grid_set_property (GObject      *object,
                        guint         prop_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
	ChatGrid *grid = CHAT_GRID(object);

	switch (prop_id) {
	case PROP_COLUMN_SPACING:
		grid->priv->column_spacing = g_value_get_uint(value);
		gtk_widget_queue_resize(GTK_WIDGET(grid));
		break;
	case PROP_ROW_SPACING:
		grid->priv->row_spacing = g_value_get_uint(value);
		gtk_widget_queue_resize(GTK_WIDGET(grid));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

/**
 * chat_grid_class_init:
 * @klass: (in): A #ChatGridClass.
 *
 * Initializes the #ChatGridClass and prepares the vtable.
 *
 * Returns: None.
 * Side effects: None.
 */
static void
chat_grid_class_init (ChatGridClass *klass)
{
	GObjectClass *object_class;
	GtkContainerClass *container_class;
	GtkWidgetClass *widget_class;

	object_class = G_OBJECT_CLASS(klass);
	object_class->finalize = chat_grid_finalize;
	object_class->get_property = chat_grid_get_property;
	object_class->set_property = chat_grid_set_property;
	g_type_class_add_private(object_class, sizeof(ChatGridPrivate));

	container_class = GTK_CONTAINER_CLASS(klass);
	container_class->add = chat_grid_add;
	container_class->remove = chat_grid_remove;

	widget_class = GTK_WIDGET_CLASS(klass);
	widget_class->get_preferred_width = chat_grid_get_preferred_width;
	widget_class->size_allocate = chat_grid_size_allocate;

	gParamSpecs[PROP_COLUMN_SPACING] =
		g_param_spec_uint("column-spacing",
		                  _("Column Spacing"),
		                  _("Amount of column spacing between grid items."),
		                  0,
		                  G_MAXUINT,
		                  0,
		                  G_PARAM_READWRITE);
	g_object_class_install_property(object_class, PROP_COLUMN_SPACING,
	                                gParamSpecs[PROP_COLUMN_SPACING]);

	gParamSpecs[PROP_ROW_SPACING] =
		g_param_spec_uint("row-spacing",
		                  _("Row Spacing"),
		                  _("Amount of row spacing between grid items."),
		                  0,
		                  G_MAXUINT,
		                  0,
		                  G_PARAM_READWRITE);
	g_object_class_install_property(object_class, PROP_ROW_SPACING,
	                                gParamSpecs[PROP_ROW_SPACING]);

	gQuarkAnimation = g_quark_from_static_string("chat-grid-animation");
}

/**
 * chat_grid_init:
 * @grid: (in): A #ChatGrid.
 *
 * Initializes the newly created #ChatGrid instance.
 *
 * Returns: None.
 * Side effects: None.
 */
static void
chat_grid_init (ChatGrid *grid)
{
	grid->priv =
		G_TYPE_INSTANCE_GET_PRIVATE(grid,
		                            CHAT_TYPE_GRID,
		                            ChatGridPrivate);

	grid->priv->need_relayout = TRUE;
}
