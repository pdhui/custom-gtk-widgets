/*
 * Clutter.
 *
 * An OpenGL based 'interactive canvas' library.
 *
 * Authored By Neil Roberts  <neil@linux.intel.com>
 *
 * Copyright (C) 2009  Intel Corporation.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __GB_TIMEOUT_INTERVAL_H__
#define __GB_TIMEOUT_INTERVAL_H__

#include <glib.h>

G_BEGIN_DECLS

typedef struct _GbTimeoutInterval GbTimeoutInterval;

struct _GbTimeoutInterval
{
  GTimeVal start_time;
  guint frame_count, fps;
};

void _gb_timeout_interval_init (GbTimeoutInterval *interval,
                                 guint fps);

gboolean _gb_timeout_interval_prepare (const GTimeVal *current_time,
                                        GbTimeoutInterval *interval,
                                        gint *delay);

gboolean _gb_timeout_interval_dispatch (GbTimeoutInterval *interval,
                                         GSourceFunc     callback,
                                         gpointer        user_data);

gint _gb_timeout_interval_compare_expiration (const GbTimeoutInterval *a,
                                               const GbTimeoutInterval *b);

G_END_DECLS

#endif /* __GB_TIMEOUT_INTERVAL_H__ */
