#!/usr/bin/env python
#
# Copyright (C) 2008 Christian Hergert <chris@dronelabs.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 
# 02110-1301 USA

import gobject
import gtk

class ModeButton(gtk.EventBox):
    __gtype_name__ = 'GtkModeButton'
    __gsignals__ = {
        'added': (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (int,)),
        'removed': (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gtk.Widget,)),
        'changed': (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (int,)),
    }

    selected = -1
    hovered = -1

    box = None
    button = None
    n_children = 0

    def __init__(self):
        gtk.EventBox.__init__(self)
        self.add_events(gtk.gdk.POINTER_MOTION_MASK|gtk.gdk.BUTTON_PRESS_MASK)

        self.box = gtk.HBox(True, 6)
        self.box.set_border_width(6)
        self.add(self.box)
        self.box.show()

        self.button = gtk.Button("")
        self.box.pack_start(self.button, False, False, 0)

    def get_selected(self):
        return self.selected

    def set_selected(self, index):
        if self.selected == index:
            return

        children = self.box.get_children()

        if self.selected > -1:
            child = children[self.selected + 1] # hidden button is first
            child.set_state(gtk.STATE_NORMAL)

        self.selected = index

        if index > -1:
            child = children[(index + 1)]
            child.set_state(gtk.STATE_SELECTED)

        self.queue_draw()
        self.emit('changed', self.selected)

    def get_hovered(self):
        return self.hovered

    def set_hovered(self, index):
        if self.hovered == index:
            return
        self.hovered = index
        self.queue_draw()

    def append(self, widget):
        self.box.pack_start(widget, True, True, 6)
        self.emit('added', self.n_children)
        self.n_children += 1

    def remove(self, index):
        children = self.box.get_children()
        child = children[index]
        self.box.remove(child)
        self.n_children -= 1

        if self.selected == index:
            self.selected = -1
        elif self.selected >= index:
            self.selected -= 1

        if self.hovered >= index:
            self.hovered -= 1

        self.emit('removed', child)

    def do_expose_event(self, event):
        clip_region = gtk.gdk.Rectangle(0,0,0,0)
        selected = self.selected
        hovered = self.hovered
        n_children = self.n_children
        inner_border = self.button.style_get_property('inner-border') or 1

        #if not self.button.window:
            # FIXME: no access to depth?
        self.button.show()
        self.button.hide()

        event.window.begin_paint_rect(event.area)

        self.button.style.paint_box(event.window,
                                    gtk.STATE_NORMAL,
                                    gtk.SHADOW_IN,
                                    event.area,
                                    self.button,
                                    "button",
                                    event.area.x,
                                    event.area.y,
                                    event.area.width,
                                    event.area.height)

        if selected >= 0:
            if n_children > 1:
                clip_region.width = event.area.width / n_children
                clip_region.x = clip_region.width * selected + 1
            else:
                clip_region.x = 0
                clip_region.width = event.area.width

            clip_region.y = event.area.y
            clip_region.height = event.area.height

            self.button.style.paint_box(event.window,
                                        gtk.STATE_SELECTED,
                                        gtk.SHADOW_ETCHED_OUT,
                                        clip_region,
                                        self.button,
                                        "button",
                                        event.area.x,
                                        event.area.y,
                                        event.area.width,
                                        event.area.height)

        if hovered >= 0 and selected != hovered:
            if n_children > 1:
                clip_region.width = event.area.width / n_children
                if hovered == 0:
                    clip_region.x = 0
                else:
                    clip_region.x = clip_region.width * hovered + 1
            else:
                clip_region.x = 0
                clip_region.width = event.area.width

            clip_region.y = event.area.y
            clip_region.height = event.area.height

            self.button.style.paint_box(event.window,
                                        gtk.STATE_PRELIGHT,
                                        gtk.SHADOW_IN,
                                        clip_region,
                                        self.button,
                                        "button",
                                        event.area.x,
                                        event.area.y,
                                        event.area.width,
                                        event.area.height)

        for i in range(1, n_children):
            offset = (event.area.width / n_children) * i
            self.button.style.paint_vline(event.window,
                                          gtk.STATE_NORMAL,
                                          event.area,
                                          self.button,
                                          "button",
                                          event.area.y + inner_border + 1,
                                          event.area.y + event.area.height - (inner_border * 2) - 1,
                                          event.area.x + offset + 1)

        self.propagate_expose(self.child, event)

        event.window.end_paint()

    def do_motion_notify_event(self, event):
        if not self.n_children:
            return False

        child_size = self.allocation.width / self.n_children
        n = -1

        if child_size > 0:
            n = event.x / child_size

        if n >= 0 and n < self.n_children:
            self.set_hovered(int(n))

        return False

    def do_button_press_event(self, event):
        if self.hovered > -1 and self.hovered != self.selected:
            self.set_selected(self.hovered)
        return False

    def do_leave_notify_event(self, event):
        if event.mode == gtk.gdk.CROSSING_NORMAL:
            self.hovered = -1
            self.queue_draw()

if __name__ == '__main__':
    w = gtk.Window()
    w.connect('destroy', gtk.main_quit)
    w.set_border_width(12)
    m = ModeButton()
    m.show()
    w.add(m)
    w.show()

    l1 = gtk.Label("Left")
    m.append(l1)
    l1.show()

    l2 = gtk.Label("Right")
    m.append(l2)
    l2.show()

    gtk.main()
