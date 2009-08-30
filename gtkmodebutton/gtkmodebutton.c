/* gtkmodebutton.h
 * Copyright (C) 2008 Christian Hergert <chris@dronelabs.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include "gtkmodebutton.h"
#include "gtkmodemarshal.h"

#define MODE_BUTTON_PRIVATE(o)             \
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), \
	GTK_TYPE_MODE_BUTTON,              \
	GtkModeButtonPrivate))

typedef struct
{
	GtkWidget *box;
	GtkWidget *button;
	gint       selected;
	gint       hovered;
	gint       n_children;
} GtkModeButtonPrivate;

enum
{
	PROP_0,
	PROP_SELECTED,
	PROP_HOVERED,
};

enum
{
	ADDED,
	REMOVED,
	SELECT,
	SWITCHED,
	LAST_SIGNAL
};

static guint mode_button_signals[LAST_SIGNAL] = {0,};

static gboolean _gtk_mode_button_select              (GtkModeButton *mode_button, gint index);
static gboolean _gtk_mode_button_expose_event        (GtkWidget *widget, GdkEventExpose *event);
static void     _gtk_mode_button_show                (GtkWidget *widget);
static gboolean _gtk_mode_button_motion_notify_event (GtkWidget *widget, GdkEventMotion *event);
static gboolean _gtk_mode_button_leave_notify_event  (GtkWidget *widget, GdkEventCrossing *event);
static gboolean _gtk_mode_button_button_press_event  (GtkWidget *widget, GdkEventButton *event);

G_DEFINE_TYPE (GtkModeButton, gtk_mode_button, GTK_TYPE_EVENT_BOX);

static void
gtk_mode_button_get_property (GObject    *object,
                              guint       property_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
	switch (property_id) {
	case PROP_SELECTED:
		g_value_set_int (value, gtk_mode_button_get_selected (GTK_MODE_BUTTON (object)));
		break;
	case PROP_HOVERED:
		g_value_set_int (value, gtk_mode_button_get_hovered (GTK_MODE_BUTTON (object)));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
gtk_mode_button_set_property (GObject      *object,
                              guint         property_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
	switch (property_id) {
	case PROP_SELECTED:
		gtk_mode_button_set_selected (GTK_MODE_BUTTON (object), g_value_get_int (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
gtk_mode_button_dispose (GObject *object)
{
	if (G_OBJECT_CLASS (gtk_mode_button_parent_class)->dispose)
		G_OBJECT_CLASS (gtk_mode_button_parent_class)->dispose (object);
}

static void
gtk_mode_button_finalize (GObject *object)
{
	G_OBJECT_CLASS (gtk_mode_button_parent_class)->finalize (object);
}

static void
gtk_mode_button_class_init (GtkModeButtonClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	g_type_class_add_private (klass, sizeof (GtkModeButtonPrivate));

	object_class->get_property = gtk_mode_button_get_property;
	object_class->set_property = gtk_mode_button_set_property;
	object_class->dispose = gtk_mode_button_dispose;
	object_class->finalize = gtk_mode_button_finalize;
	klass->select = _gtk_mode_button_select;
	widget_class->expose_event = _gtk_mode_button_expose_event;
	widget_class->show = _gtk_mode_button_show;
	widget_class->motion_notify_event = _gtk_mode_button_motion_notify_event;
	widget_class->leave_notify_event = _gtk_mode_button_leave_notify_event;
	widget_class->button_press_event = _gtk_mode_button_button_press_event;
	
	g_object_class_install_property (object_class,
	                                 PROP_SELECTED,
	                                 g_param_spec_int ("selected",
	                                                   "Selected",
	                                                   "The index of the selected widget",
	                                                   -1,
	                                                   G_MAXINT,
	                                                   -1,
	                                                   G_PARAM_READWRITE));
	
	g_object_class_install_property (object_class,
	                                 PROP_HOVERED,
	                                 g_param_spec_int ("hovered",
	                                                   "Hovered",
	                                                   "The index of the hovered widget",
	                                                   -1,
	                                                   G_MAXINT,
	                                                   -1,
	                                                   G_PARAM_READABLE));
	
	mode_button_signals[ADDED] = g_signal_new ("added",
	                                           G_OBJECT_CLASS_TYPE (object_class),
	                                           G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
	                                           0,
	                                           NULL, NULL,
	                                           g_cclosure_marshal_VOID__OBJECT,
	                                           G_TYPE_NONE,
	                                           1,
	                                           GTK_TYPE_WIDGET);
	
	mode_button_signals[REMOVED] = g_signal_new ("removed",
	                                             G_OBJECT_CLASS_TYPE (object_class),
	                                             G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
	                                             0,
	                                             NULL, NULL,
	                                             g_cclosure_marshal_VOID__OBJECT,
	                                             G_TYPE_NONE,
	                                             1,
	                                             GTK_TYPE_WIDGET);
	
	mode_button_signals[SELECT] = g_signal_new ("select",
	                                            G_OBJECT_CLASS_TYPE (object_class),
	                                            G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
	                                            G_STRUCT_OFFSET (GtkModeButtonClass, select),
	                                            NULL, NULL,
	                                            _gtk_marshal_BOOLEAN__INT,
	                                            G_TYPE_BOOLEAN,
	                                            1,
	                                            G_TYPE_INT);

	mode_button_signals[SWITCHED] = g_signal_new ("switched",
	                                              G_OBJECT_CLASS_TYPE (object_class),
	                                              G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
	                                              0,
	                                              NULL, NULL,
	                                              g_cclosure_marshal_VOID__INT,
	                                              G_TYPE_NONE,
	                                              1,
	                                              G_TYPE_INT);
}

static void
gtk_mode_button_init (GtkModeButton *self)
{
	GtkModeButtonPrivate *priv = MODE_BUTTON_PRIVATE (self);
	
	priv->hovered = -1;
	priv->selected = -1;
	
	gtk_widget_add_events (GTK_WIDGET (self), GDK_POINTER_MOTION_MASK);
	
	priv->box = gtk_hbox_new (TRUE, 6);
	gtk_container_set_border_width (GTK_CONTAINER (priv->box), 6);
	gtk_container_add (GTK_CONTAINER (self), priv->box);
	gtk_widget_show (priv->box);
	
	priv->button = gtk_button_new_with_label ("");
	gtk_box_pack_start (GTK_BOX (priv->box), priv->button, FALSE, FALSE, 0);
	gtk_widget_show (priv->button);
}

GtkWidget*
gtk_mode_button_new (void)
{
	return g_object_new (GTK_TYPE_MODE_BUTTON, NULL);
}

gint
gtk_mode_button_get_selected (GtkModeButton *mode_button)
{
	return MODE_BUTTON_PRIVATE (mode_button)->selected;
}

void
gtk_mode_button_set_selected (GtkModeButton *mode_button, gint index)
{
	g_return_if_fail (index >= -1);
	
	GtkModeButtonPrivate *priv = MODE_BUTTON_PRIVATE (mode_button);
	
	g_return_if_fail (index < priv->n_children);
	
	gboolean selectable = FALSE;
	
	g_signal_emit (mode_button, mode_button_signals[SELECT], 0, index, &selectable);
	
	if (selectable)
	{
		priv->selected = index;
		gtk_widget_queue_draw (GTK_WIDGET (mode_button));
		g_signal_emit (mode_button, mode_button_signals[SWITCHED], 0, index);
	}
}

gint
gtk_mode_button_get_hovered (GtkModeButton *mode_button)
{
	return MODE_BUTTON_PRIVATE (mode_button)->hovered;
}

static void
_gtk_mode_button_set_hovered (GtkModeButton *mode_button, gint index)
{
	g_return_if_fail (index >= -1);
	
	GtkModeButtonPrivate *priv = MODE_BUTTON_PRIVATE (mode_button);
	
	g_return_if_fail (index < priv->n_children);
	
	if (priv->hovered != index)
	{
		priv->hovered = index;
		gtk_widget_queue_draw (GTK_WIDGET (mode_button));
	}
}

static gboolean
_gtk_mode_button_select (GtkModeButton *mode_button, gint index)
{
	return TRUE;
}

gint
gtk_mode_button_append (GtkModeButton *mode_button, GtkWidget *widget)
{
	GtkModeButtonPrivate *priv = MODE_BUTTON_PRIVATE (mode_button);
	gtk_box_pack_start (GTK_BOX (priv->box), widget, TRUE, TRUE, 6);
	g_signal_emit (mode_button, mode_button_signals[ADDED], 0, priv->n_children);
	return priv->n_children++;
}

gint
gtk_mode_button_append_button (GtkModeButton *mode_button, GtkWidget *button)
{
    gint ret;
    GtkWidget *btnchild;

    g_return_val_if_fail (GTK_IS_BUTTON(button), -1);
    btnchild = gtk_bin_get_child (GTK_BIN (button));
    /* ref the child so it stays alive when remove from the button */
    g_object_ref(btnchild);
    gtk_container_remove (GTK_CONTAINER (button), btnchild);

    ret = gtk_mode_button_append (mode_button, btnchild);
    /* remove the ref, we took ownership when we added it to priv->box */
    g_object_unref(btnchild);
    return ret;
}


void
gtk_mode_button_remove (GtkModeButton *mode_button, gint index)
{
	GtkModeButtonPrivate *priv = MODE_BUTTON_PRIVATE (mode_button);
	GList *list = gtk_container_get_children (GTK_CONTAINER (priv->box));
	GtkWidget *target = g_list_nth_data (list, index + 1);
	
	g_list_free (list);
	
	if (target)
	{
		gtk_container_remove (GTK_CONTAINER (priv->box), target);
		priv->n_children--;
		
		if (priv->selected == index)
			priv->selected = -1;
		else if (priv->selected >= index)
			priv->selected--;
		
		if (priv->hovered >= index)
			priv->hovered--;
	}
}

static gboolean
_gtk_mode_button_expose_event (GtkWidget *widget, GdkEventExpose *event)
{
	GtkModeButtonPrivate *priv = MODE_BUTTON_PRIVATE (widget);
	GdkRectangle clip_region = {0, 0, 0, 0};
	gint inner_border = 1;
	gint selected = priv->selected;
	gint hovered = priv->hovered;
	guint n_children = priv->n_children;
	
	gtk_widget_style_get (priv->button, "inner-border", &inner_border, NULL);
	
	gdk_window_begin_paint_rect (event->window, &event->area);
	
	gtk_paint_box (priv->button->style,
	               event->window,
	               GTK_STATE_NORMAL,
	               GTK_SHADOW_IN,
	               &event->area,
	               priv->button,
	               "button",
	               event->area.x,
	               event->area.y,
	               event->area.width,
	               event->area.height);

	if (selected >= 0)
	{
		if (n_children > 1)
		{
			clip_region.width = event->area.width / n_children;
			if (selected == 0)
				clip_region.x = 0;
			else
				clip_region.x = clip_region.width * selected + 1;
		}
		else
		{
			clip_region.x = 0;
			clip_region.width = event->area.width;
		}

		clip_region.y = event->area.y;
		clip_region.height = event->area.height;

		gtk_paint_box (priv->button->style,
		               event->window,
		               GTK_STATE_SELECTED,
		               GTK_SHADOW_IN,
		               &clip_region,
		               priv->button,
		               "button",
		               event->area.x,
		               event->area.y,
		               event->area.width,
		               event->area.height);
	}
	
	if (hovered >= 0 && selected != hovered)
	{
		if (n_children > 1)
		{
			clip_region.width = event->area.width / n_children;
			if (hovered == 0)
				clip_region.x = 0;
			else
				clip_region.x = clip_region.width * hovered + 1;
		}
		else
		{
			clip_region.x = 0;
			clip_region.width = event->area.width;
		}
		
		clip_region.y = event->area.y;
		clip_region.height = event->area.height;
		
		gtk_paint_box (priv->button->style,
		               event->window,
		               GTK_STATE_PRELIGHT,
		               GTK_SHADOW_IN,
		               &clip_region,
		               priv->button,
		               "button",
		               event->area.x,
		               event->area.y,
		               event->area.width,
		               event->area.height);
	}
	
	gint i;
	
	for (i = 1; i < n_children; i++)
	{
		gint offset = (event->area.width / (n_children)) * i;
		gtk_paint_vline (priv->button->style,
		                 event->window,
		                 GTK_STATE_NORMAL,
		                 NULL,
		                 priv->button,
		                 "button",
		                 event->area.y + inner_border + 2,
		                 event->area.y + event->area.height - (inner_border * 2) - 2,
		                 event->area.x + offset + 1);
	}

	gtk_container_propagate_expose (GTK_CONTAINER (widget),
	                                gtk_bin_get_child (GTK_BIN (widget)),
	                                event);
	
	gdk_window_end_paint (event->window);
	
	return FALSE;
}

static void
_gtk_mode_button_show (GtkWidget *widget)
{
	/* hide the hidden button. this all stems from the fact that creating
	 * the button in init doesnt work the same as the prototype non-class
	 * version. */
	GTK_WIDGET_CLASS (gtk_mode_button_parent_class)->show (widget);
	
	/* TODO: Make sure this doesn't flicker.  We have to do it after the show
	 *   so that the window, styles, etc get setup on the button.  I don't
	 *   think it will flicker since the draw should be done later with a
	 *   queue draw. */
	gtk_widget_hide (MODE_BUTTON_PRIVATE (widget)->button);
}

static gboolean
_gtk_mode_button_motion_notify_event (GtkWidget *widget, GdkEventMotion *event)
{
	GtkModeButtonPrivate *priv = MODE_BUTTON_PRIVATE (widget);
	
	if (!priv->n_children)
		return  FALSE;
	
	gdouble child_size = widget->allocation.width / priv->n_children;
	gint n = -1;

	if (child_size > 0)
		n = event->x / child_size;

	if (n >= 0 && n < priv->n_children)
		_gtk_mode_button_set_hovered (GTK_MODE_BUTTON (widget), n);

	return FALSE;
}

static gboolean
_gtk_mode_button_leave_notify_event (GtkWidget *widget, GdkEventCrossing *event)
{
	if (event->mode == GDK_CROSSING_NORMAL)
	{
		MODE_BUTTON_PRIVATE (widget)->hovered = -1;
		gtk_widget_queue_draw (widget);
	}
	
	return FALSE;
}

static gboolean
_gtk_mode_button_button_press_event (GtkWidget *widget, GdkEventButton *event)
{
	GtkModeButtonPrivate *priv = MODE_BUTTON_PRIVATE (widget);
	
	if (priv->hovered >= 0 && priv->hovered != priv->selected)
		gtk_mode_button_set_selected (GTK_MODE_BUTTON (widget), priv->hovered);
	
	return FALSE;
}
