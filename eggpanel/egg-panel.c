/* egg-panel.c
 *
 * Copyright (C) 2010 Christian Hergert <chris@dronelabs.com>
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "egg-panel.h"

/**
 * SECTION:egg-panel
 * @title: EggPanel
 * @short_description: 
 *
 * Section overview.
 */

G_DEFINE_TYPE(EggPanel, egg_panel, GTK_TYPE_VBOX)

struct _EggPanelPrivate
{
	GtkWidget *toplevel;
	EggPanel  *group;
	gboolean   has_child;
	gboolean   pressed;

	GtkWidget *header;
	GtkWidget *title;
	GtkWidget *ebox;
};

static GtkWidget* egg_panel_get_toplevel (EggPanel *panel);

/**
 * egg_panel_new:
 *
 * Creates a new instance of #EggPanel.
 *
 * Returns: the newly created instance of #EggPanel.
 * Side effects: None.
 */
GtkWidget*
egg_panel_new (void)
{
	return g_object_new(EGG_TYPE_PANEL, NULL);
}

/**
 * egg_pannel_set_title:
 * @panel: An #EggPanel.
 * @title: The title.
 *
 * Sets the title of the panel.
 *
 * Returns: None.
 * Side effects: None.
 */
void
egg_panel_set_title (EggPanel    *panel,
                     const gchar *title)
{
	EggPanelPrivate *priv;
	gchar *markup;

	g_return_if_fail(EGG_IS_PANEL(panel));

	priv = panel->priv;

	markup = g_strdup_printf("<span size=\"smaller\" weight=\"bold\">%s</span>",
	                         title);
	gtk_label_set_markup(GTK_LABEL(priv->title), markup);
	g_free(markup);
}

static inline EggPanel*
egg_panel_get_group (EggPanel *panel)
{
	EggPanelPrivate *priv = panel->priv;
	return (priv->group == NULL) ? panel : priv->group;
}

static gboolean
egg_panel_toplevel_expose (GtkWidget      *widget,
                           GdkEventExpose *event,
                           EggPanel       *panel)
{
	GdkRectangle alloc;
	gint x, y;

	gtk_widget_get_allocation(widget, &alloc);
	x = alloc.width - 17;
	y = alloc.height - 17;

	/*
	 * Expose child.
	 */
	gtk_container_propagate_expose(GTK_CONTAINER(widget),
	                               gtk_bin_get_child(GTK_BIN(widget)),
	                               event);

	/*
	 * Expose resize grip.
	 */
	gtk_paint_resize_grip(gtk_widget_get_style(widget),
	                      event->window,
	                      GTK_STATE_ACTIVE,
	                      &alloc,
	                      widget,
	                      "",
	                      GDK_WINDOW_EDGE_SOUTH_EAST,
	                      x, y, 15, 15);

	return FALSE;
}

static gboolean
egg_panel_toplevel_configure (GtkWidget         *ebox,
                              GdkEventConfigure *event,
                              EggPanel          *panel)
{
	EggPanelPrivate *priv = panel->priv;
	GtkWidget *toplevel;

	priv->pressed = FALSE;
	toplevel = egg_panel_get_toplevel(panel);

	gtk_window_set_opacity(GTK_WINDOW(toplevel), 1.);

	gtk_widget_queue_draw(ebox);

	return FALSE;
}

static inline GtkWidget*
egg_panel_get_toplevel (EggPanel *panel)
{
	EggPanel *group;
	GtkWidget *toplevel;
	GtkWidget *box;

	/*
	 * Get the panel group.
	 */
	group = egg_panel_get_group(panel);
	g_assert(group);

	/*
	 * If there is not yet a toplevel window, go ahead and
	 * create it.
	 */
	toplevel = group->priv->toplevel;
	if (!toplevel) {
		group->priv->toplevel = toplevel = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_decorated(GTK_WINDOW(toplevel), FALSE);
		gtk_window_set_skip_taskbar_hint(GTK_WINDOW(toplevel), TRUE);
		gtk_window_set_skip_pager_hint(GTK_WINDOW(toplevel), TRUE);
		gtk_widget_set_app_paintable(toplevel, TRUE);
		box = gtk_vbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(toplevel), box);
		gtk_widget_show(box);

		/*
		 * Attach window signals.
		 *
		 * TODO: Remove callback when group removed from toplevel.
		 */
		g_signal_connect(toplevel,
		                 "expose-event",
		                 G_CALLBACK(egg_panel_toplevel_expose),
		                 group);
		g_signal_connect(toplevel,
		                 "configure-event",
		                 G_CALLBACK(egg_panel_toplevel_configure),
		                 group);
	}

	g_assert(toplevel);
	return toplevel;
}

static void
egg_panel_add (GtkContainer *panel,
               GtkWidget    *child)
{
	EggPanelPrivate *priv;

	g_return_if_fail(EGG_IS_PANEL(panel));

	priv = EGG_PANEL(panel)->priv;

	if (priv->has_child) {
		g_warning("EggPanel only supports a single child.");
		return;
	}

	gtk_box_pack_start(GTK_BOX(panel), child, TRUE, TRUE, 0);
	priv->has_child = TRUE;
}

static gboolean
egg_panel_ebox_expose (GtkWidget      *ebox,
                       GdkEventExpose *event,
                       EggPanel       *panel)
{
	GdkRectangle alloc;
	cairo_pattern_t *pt;
	cairo_t *cr;
	double r1, r2, g1, g2, b1, b2;
	GtkStyle *style;
	GtkStateType state;

	/*
	 * Determine state from button press.
	 */
	state = panel->priv->pressed ? GTK_STATE_SELECTED : GTK_STATE_NORMAL;

	/*
	 * Create cairo context and clip drawing area to event region.
	 */
	cr = gdk_cairo_create(event->window);
	gdk_cairo_rectangle(cr, &event->area);
	cairo_clip(cr);

	/*
	 * Get gradient colors.
	 */
	style = gtk_widget_get_style(ebox);
	r1 = style->bg[state].red / 65535.;
	g1 = style->bg[state].green / 65535.;
	b1 = style->bg[state].blue / 65535.;
	r2 = style->dark[state].red / 65535.;
	g2 = style->dark[state].green / 65535.;
	b2 = style->dark[state].blue / 65535.;

	/*
	 * Draw the background gradient.
	 */
	gtk_widget_get_allocation(ebox, &alloc);
	pt = cairo_pattern_create_linear(0., 0., 0., alloc.height);
	cairo_pattern_add_color_stop_rgb(pt, 0., r1, g1, b1);
	cairo_pattern_add_color_stop_rgb(pt, 1., r2, g2, b2);
	cairo_rectangle(cr, 0., 0., alloc.width, alloc.height);
	cairo_set_source(cr, pt);
	cairo_fill(cr);

	/*
	 * Cleanup resources.
	 */
	cairo_pattern_destroy(pt);
	cairo_destroy(cr);

	/*
	 * Expose the child.
	 */
	gtk_container_propagate_expose(GTK_CONTAINER(ebox),
	                               panel->priv->header,
	                               event);

	return FALSE;
}

static void
egg_panel_ebox_button_press (GtkWidget      *ebox,
                             GdkEventButton *event,
                             EggPanel       *panel)
{
	EggPanelPrivate *priv = panel->priv;
	GtkWidget *toplevel;
	EggPanel *group;

	priv->pressed = TRUE;
	group = egg_panel_get_group(panel);
	toplevel = egg_panel_get_toplevel(panel);

	/*
	 * Start a move-drag on the toplevel window since we are trying to move
	 * the entire thing.
	 */
	if (group == panel) {
		gtk_window_set_opacity(GTK_WINDOW(toplevel), .5);
		gtk_window_begin_move_drag(GTK_WINDOW(toplevel),
		                           event->button,
		                           event->x_root,
		                           event->y_root,
		                           event->time);
	}

	gtk_widget_queue_draw(ebox);
}

static void
egg_panel_ebox_button_release (GtkWidget      *ebox,
                               GdkEventButton *event,
                               EggPanel       *panel)
{
	EggPanelPrivate *priv = panel->priv;

	priv->pressed = FALSE;

	gtk_widget_queue_draw(ebox);
}

static void
egg_panel_show (GtkWidget *panel)
{
	GtkWidget *toplevel;
	GtkWidget *box;

	/*
	 * Make sure the toplevel window is visible.
	 */
	toplevel = egg_panel_get_toplevel(EGG_PANEL(panel));
	gtk_widget_show_all(toplevel);

	/*
	 * If we haven't yet been added to the toplevel, add ourselves.
	 */
	if (!gtk_widget_get_parent(panel)) {
		box = gtk_bin_get_child(GTK_BIN(toplevel));
		gtk_box_pack_start(GTK_BOX(box), panel, FALSE, TRUE, 0);
	}

	/*
	 * Show ourself.
	 */
	GTK_WIDGET_CLASS(egg_panel_parent_class)->show(panel);
}

static void
egg_panel_finalize (GObject *object)
{
	G_OBJECT_CLASS(egg_panel_parent_class)->finalize(object);
}

static void
egg_panel_dispose (GObject *object)
{
	G_OBJECT_CLASS(egg_panel_parent_class)->dispose(object);
}

static void
egg_panel_class_init (EggPanelClass *klass)
{
	GObjectClass *object_class;
	GtkWidgetClass *widget_class;
	GtkContainerClass *container_class;

	object_class = G_OBJECT_CLASS(klass);
	object_class->finalize = egg_panel_finalize;
	object_class->dispose = egg_panel_dispose;
	g_type_class_add_private(object_class, sizeof(EggPanelPrivate));

	widget_class = GTK_WIDGET_CLASS(klass);
	widget_class->show = egg_panel_show;

	container_class = GTK_CONTAINER_CLASS(klass);
	container_class->add = egg_panel_add;
}

static void
egg_panel_init (EggPanel *panel)
{
	panel->priv = G_TYPE_INSTANCE_GET_PRIVATE(panel, EGG_TYPE_PANEL,
	                                          EggPanelPrivate);

	/*
	 * Create the event box for the header background.
	 */
	panel->priv->ebox = gtk_event_box_new();
	gtk_widget_set_app_paintable(panel->priv->ebox, TRUE);
	gtk_box_pack_start(GTK_BOX(panel), panel->priv->ebox, FALSE, TRUE, 0);
	gtk_widget_show(panel->priv->ebox);

	/*
	 * Create the header box.
	 */
	panel->priv->header = gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(panel->priv->ebox), panel->priv->header);
	gtk_widget_show(panel->priv->header);

	/*
	 * Create the title label.
	 */
	panel->priv->title = gtk_label_new(NULL);
	gtk_misc_set_alignment(GTK_MISC(panel->priv->title), 0., .5);
	gtk_misc_set_padding(GTK_MISC(panel->priv->title), 3, 1);
	gtk_box_pack_start(GTK_BOX(panel->priv->header),
	                   panel->priv->title, TRUE, TRUE, 0);
	gtk_widget_show(panel->priv->title);

	/*
	 * Attach signals.
	 */
	g_signal_connect(panel->priv->ebox,
	                 "expose-event",
	                 G_CALLBACK(egg_panel_ebox_expose),
	                 panel);
	g_signal_connect(panel->priv->ebox,
	                 "button-press-event",
	                 G_CALLBACK(egg_panel_ebox_button_press),
	                 panel);
	g_signal_connect(panel->priv->ebox,
	                 "button-release-event",
	                 G_CALLBACK(egg_panel_ebox_button_release),
	                 panel);
}
