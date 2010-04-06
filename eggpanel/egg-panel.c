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

static gboolean egg_panel_toplevel_configure (GtkWidget         *toplevel,
                                              GdkEventConfigure *event,
                                              EggPanel          *panel);
static gboolean egg_panel_toplevel_focus_in  (GtkWidget         *toplevel,
                                              GdkEventFocus     *event,
                                              EggPanel          *panel);
static gboolean egg_panel_toplevel_focus_out (GtkWidget         *toplevel,
                                              GdkEventFocus     *event,
                                              EggPanel          *panel);

enum
{
	STATE_INITIAL,
	STATE_DOCKED,
	STATE_DRAGGING,
	STATE_SNAPPING,
	STATE_FOCUSING,
	STATE_BLURING,
	STATE_EXPANDING,
	STATE_CONTRACTING,
	STATE_LAST
};

struct _EggPanelPrivate
{
	gint       state;
	EggPanel  *group;
	GtkWidget *toplevel;
	GtkWidget *child;
	GtkWidget *header;
	GtkWidget *title;
	GtkWidget *arrow;
};

static const gchar* states[] = {
	"INITIAL",
	"DOCKED",
	"DRAGGING",
	"SNAPPING",
	"FOCUSING",
	"BLURING",
	"EXPANDING",
	"CONTRACTING",
};

static void
egg_panel_transition (EggPanel *panel,
                      guint     state)
{
	EggPanelPrivate *priv;
	gint current;

	g_return_if_fail(EGG_IS_PANEL(panel));
	g_return_if_fail(state < STATE_LAST);

	priv = panel->priv;
	current = priv->state;

	#define ASSERT_INVALID_TRANSITION G_STMT_START {            \
	    g_error("EggPanel: invalid state transition: %s => %s", \
	            states[current], states[state]);                \
	} G_STMT_END

	#define SET_STATE(s) G_STMT_START {                         \
	    g_message("EggPanel: state transition %s => %s",        \
	              states[current], states[state]);              \
	    priv->state = (s);                                      \
	} G_STMT_END

	switch (current) {
	case STATE_INITIAL:
		switch (state) {
		case STATE_DOCKED: {
			if (!priv->group) {
				GtkWidget *child;

				priv->toplevel = gtk_window_new(GTK_WINDOW_TOPLEVEL);
				gtk_window_set_decorated(GTK_WINDOW(priv->toplevel), FALSE);
				gtk_window_set_skip_pager_hint(GTK_WINDOW(priv->toplevel), TRUE);
				gtk_window_set_skip_taskbar_hint(GTK_WINDOW(priv->toplevel), TRUE);
				gtk_window_set_default_size(GTK_WINDOW(priv->toplevel), 1, 1);
				child = gtk_vbox_new(FALSE, 0);
				gtk_container_add(GTK_CONTAINER(priv->toplevel), child);
				gtk_box_pack_start(GTK_BOX(child), GTK_WIDGET(panel),
				                   TRUE, TRUE, 0);
				gtk_widget_show(child);
				g_signal_connect(priv->toplevel,
				                 "configure-event",
				                 G_CALLBACK(egg_panel_toplevel_configure),
				                 panel);
				g_signal_connect(priv->toplevel,
				                 "focus-in-event",
				                 G_CALLBACK(egg_panel_toplevel_focus_in),
				                 panel);
				g_signal_connect(priv->toplevel,
				                 "focus-out-event",
				                 G_CALLBACK(egg_panel_toplevel_focus_out),
				                 panel);
			}
			break;
		}
		default:
			ASSERT_INVALID_TRANSITION;
		}
		break;
	case STATE_DOCKED:
		switch (state) {
		case STATE_DRAGGING: {
			/*
			 * TODO: If not the owner of current toplevel, we need to
			 *       remove and add to its own toplevel.
			 */
			break;
		}
		case STATE_BLURING: {
			gtk_window_set_opacity(GTK_WINDOW(priv->toplevel), .5);
			/*
			 * Don't allow state to change.
			 */
			return;
		}
		case STATE_FOCUSING: {
			gtk_window_set_opacity(GTK_WINDOW(priv->toplevel), 1.);
			/*
			 * Don't allow state to change.
			 */
			return;
		}
		default:
			ASSERT_INVALID_TRANSITION;
		}
		break;
	case STATE_DRAGGING:
		switch (state) {
		case STATE_BLURING:
			gtk_window_set_opacity(GTK_WINDOW(priv->toplevel), 1.);
			/*
			 * Don't allow state to change.
			 */
			return;
		case STATE_DOCKED:
			break;
		default:
			ASSERT_INVALID_TRANSITION;
		}
		break;
	case STATE_SNAPPING:
		switch (state) {
		default:
			ASSERT_INVALID_TRANSITION;
		}
		break;
	case STATE_FOCUSING:
		switch (state) {
		case STATE_DOCKED:
			if (priv->toplevel) {
				gtk_window_set_opacity(GTK_WINDOW(priv->toplevel), 1.);
			}
			egg_panel_transition(panel, STATE_DOCKED);
			break;
		default:
			ASSERT_INVALID_TRANSITION;
		}
		break;
	case STATE_BLURING:
		switch (state) {
		case STATE_DOCKED:
			if (priv->toplevel) {
				gtk_window_set_opacity(GTK_WINDOW(priv->toplevel), .5);
			}
			egg_panel_transition(panel, STATE_DOCKED);
			break;
		default:
			ASSERT_INVALID_TRANSITION;
		}
		break;
	case STATE_EXPANDING:
		switch (state) {
		default:
			ASSERT_INVALID_TRANSITION;
		}
		break;
	case STATE_CONTRACTING:
		switch (state) {
		default:
			ASSERT_INVALID_TRANSITION;
		}
		break;
	default:
		g_assert_not_reached();
	}

	SET_STATE(state);
}

static void
egg_panel_show (GtkWidget *widget)
{
	EggPanelPrivate *priv = EGG_PANEL(widget)->priv;

	if (priv->state != STATE_DOCKED) {
		egg_panel_transition(EGG_PANEL(widget), STATE_DOCKED);
	}

	if (priv->toplevel) {
		gtk_widget_show(priv->toplevel);
	}

	GTK_WIDGET_CLASS(egg_panel_parent_class)->show(widget);
}

static void
egg_panel_add (GtkContainer *container,
               GtkWidget    *child)
{
	EGG_PANEL(container)->priv->child = child;
	gtk_box_pack_start(GTK_BOX(container), child, TRUE, TRUE, 0);
}

static gboolean
egg_panel_toplevel_focus_out (GtkWidget     *widget,
                              GdkEventFocus *event,
                              EggPanel      *panel)
{
	egg_panel_transition(panel, STATE_BLURING);
	return FALSE;
}

static gboolean
egg_panel_ebox_expose (GtkWidget      *ebox,
                       GdkEventExpose *event,
                       EggPanel       *panel)
{
	EggPanelPrivate *priv = panel->priv;
	GdkRectangle alloc;
	cairo_pattern_t *pt;
	cairo_t *cr;
	double r1, r2, g1, g2, b1, b2;
	GtkStyle *style;
	GtkStateType state;

	/*
	 * Determine state from button press.
	 */
	state = (priv->state == STATE_DRAGGING) ? GTK_STATE_SELECTED : GTK_STATE_NORMAL;

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

static gboolean
egg_panel_ebox_button_press (GtkWidget      *ebox,
                             GdkEventButton *event,
                             EggPanel       *panel)
{
	GtkAllocation alloc;
	GtkWidget *arrow;
	GtkArrowType type;

	if (event->button == 1) {
		gtk_widget_get_allocation(panel->priv->arrow, &alloc);

		if (event->x >= alloc.x && event->x <= (alloc.x + alloc.width)) {
			if (event->y >= alloc.y && event->y <= (alloc.y + alloc.height)) {
				GtkWidget *child;

				/*
				 * Handle clicks within the arrow area.
				 */
				arrow = panel->priv->arrow;
				g_object_get(arrow, "arrow-type", &type, NULL);
				type = (type == GTK_ARROW_DOWN) ? GTK_ARROW_RIGHT : GTK_ARROW_DOWN;
				g_object_set(arrow, "arrow-type", type, NULL);

				/*
				 * TODO: Animate Expanding/Collapsing.
				 */
				child = panel->priv->child;
				gtk_widget_set_visible(child, (type == GTK_ARROW_DOWN));

				return FALSE;
			}
		}

		egg_panel_transition(panel, STATE_DRAGGING);
		gtk_window_begin_move_drag(GTK_WINDOW(panel->priv->toplevel),
		                           event->button,
		                           event->x_root, event->y_root,
		                           event->time);
	}

	return FALSE;
}

static gboolean
egg_panel_toplevel_configure (GtkWidget         *toplevel,
                              GdkEventConfigure *event,
                              EggPanel          *panel)
{
	if (panel->priv->state == STATE_DRAGGING) {
		egg_panel_transition(panel, STATE_DOCKED);
	}

	return FALSE;
}

static gboolean
egg_panel_toplevel_focus_in (GtkWidget     *toplevel,
                             GdkEventFocus *event,
                             EggPanel      *panel)
{
	/*
	 * Check if we just came out of a drag event.
	 */
	if (panel->priv->state == STATE_DRAGGING) {
		egg_panel_transition(panel, STATE_DOCKED);
	}
	egg_panel_transition(panel, STATE_FOCUSING);

	return FALSE;
}

static void
egg_panel_finalize (GObject *object)
{
	G_OBJECT_CLASS(egg_panel_parent_class)->finalize(object);
}

static void
egg_panel_class_init (EggPanelClass *klass)
{
	GObjectClass *object_class;
	GtkWidgetClass *widget_class;
	GtkContainerClass *container_class;

	object_class = G_OBJECT_CLASS(klass);
	object_class->finalize = egg_panel_finalize;
	g_type_class_add_private(object_class, sizeof(EggPanelPrivate));

	widget_class = GTK_WIDGET_CLASS(klass);
	widget_class->show = egg_panel_show;

	container_class = GTK_CONTAINER_CLASS(klass);
	container_class->add = egg_panel_add;
}

static void
egg_panel_init (EggPanel *panel)
{
	GtkWidget *ebox;

	panel->priv = G_TYPE_INSTANCE_GET_PRIVATE(panel,
	                                          EGG_TYPE_PANEL,
	                                          EggPanelPrivate);

	/*
	 * Set default state.
	 */
	panel->priv->state = STATE_INITIAL;

	/*
	 * Create required children widgets.
	 */
	ebox = gtk_event_box_new();
	panel->priv->header = gtk_hbox_new(FALSE, 0);
	panel->priv->title = gtk_label_new(NULL);
	panel->priv->arrow = gtk_arrow_new(GTK_ARROW_DOWN, GTK_SHADOW_NONE);

	/*
	 * Modify widget properties.
	 */
	gtk_misc_set_padding(GTK_MISC(panel->priv->title), 6, 3);
	gtk_misc_set_alignment(GTK_MISC(panel->priv->title), 0., .5);
	gtk_widget_set_app_paintable(ebox, TRUE);
	gtk_misc_set_padding(GTK_MISC(panel->priv->arrow), 1, 0);

	/*
	 * Pack children widgets.
	 */
	gtk_box_pack_start(GTK_BOX(panel), ebox, FALSE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(ebox), panel->priv->header);
	gtk_box_pack_start(GTK_BOX(panel->priv->header), panel->priv->arrow,
	                   FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(panel->priv->header), panel->priv->title,
	                   TRUE, TRUE, 0);

	/*
	 * Attach signal handlers.
	 */
	g_signal_connect(ebox, "expose-event",
	                 G_CALLBACK(egg_panel_ebox_expose), panel);
	g_signal_connect(ebox, "button-press-event",
	                 G_CALLBACK(egg_panel_ebox_button_press), panel);

	/*
	 * Show chlidren widgets.
	 */
	gtk_widget_show(panel->priv->title);
	gtk_widget_show(panel->priv->header);
	gtk_widget_show(panel->priv->arrow);
	gtk_widget_show(ebox);
	
}

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
 * egg_panel_set_title:
 * @panel: An #EggPanel.
 * @title: The panel title.
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
	gchar *markup;

	g_return_if_fail(EGG_IS_PANEL(panel));
	g_return_if_fail(title != NULL);

	markup = g_strdup_printf("<span size=\"smaller\" weight=\"bold\">%s</span>",
	                         title);
	gtk_label_set_markup(GTK_LABEL(panel->priv->title), markup);
	g_free(markup);
}
