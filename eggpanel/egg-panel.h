/* egg-panel.h
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

#ifndef __EGG_PANEL_H__
#define __EGG_PANEL_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define EGG_TYPE_PANEL            (egg_panel_get_type())
#define EGG_PANEL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_PANEL, EggPanel))
#define EGG_PANEL_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_PANEL, EggPanel const))
#define EGG_PANEL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  EGG_TYPE_PANEL, EggPanelClass))
#define EGG_IS_PANEL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_PANEL))
#define EGG_IS_PANEL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  EGG_TYPE_PANEL))
#define EGG_PANEL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  EGG_TYPE_PANEL, EggPanelClass))

typedef struct _EggPanel        EggPanel;
typedef struct _EggPanelClass   EggPanelClass;
typedef struct _EggPanelPrivate EggPanelPrivate;

struct _EggPanel
{
	GtkVBox parent;

	/*< private >*/
	EggPanelPrivate *priv;
};

struct _EggPanelClass
{
	GtkVBoxClass parent_class;
};

GType      egg_panel_get_type  (void) G_GNUC_CONST;
GtkWidget* egg_panel_new       (void);
void       egg_panel_set_title (EggPanel    *panel,
                                const gchar *title);

G_END_DECLS

#endif /* __EGG_PANEL_H__ */
