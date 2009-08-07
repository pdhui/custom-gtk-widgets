#!/usr/bin/env python

import cairo
import gobject
import gtk

rcdata = """
style "dark-view"
{
    bg[NORMAL] = "#272727"
    bg[PRELIGHT] = "#232323"
    bg[ACTIVE] = "#212121"
    bg[SELECTED] = "#ffffff"

    text[NORMAL] = "#ffffff"
    text[PRELIGHT] = "#ffffff"
    text[ACTIVE] = "#ffffff"
    text[SELECTED] = "#ffffff"

    fg[NORMAL] = "#ffffff"
    fg[PRELIGHT] = "#ffffff"
    fg[ACTIVE] = "#ffffff"
    fg[SELECTED] = "#ffffff"

    base[NORMAL] = "#272727"
    base[PRELIGHT] = "#232323"
    base[ACTIVE] = "#212121"
    base[SELECTED] = "#212121"
}

class "GtkTreeView" style "dark-view"
class "GtkButton" style "dark-view"
class "GtkLabel" style "dark-view"
class "GtkComboBox" style "dark-view"
"""

def rounded_rectangle (cr, x, y, width, height, x_radius, y_radius):
    x1 = x
    x2 = x1 + width
    y1 = y
    y2 = y1 + height
 
    x_radius = min(x_radius, width / 2.0)
    y_radius = min(y_radius, width / 2.0)
 
    xr1 = x_radius
    xr2 = x_radius / 2.0
    yr1 = y_radius
    yr2 = y_radius / 2.0
 
    cr.move_to(x1 + xr1, y1)
    cr.line_to(x2 - xr1, y1)
    cr.curve_to(x2 - xr2, y1, x2, y1 + yr2, x2, y1 + yr1)
    cr.line_to(x2, y2 - yr1)
    cr.curve_to(x2, y2 - yr2, x2 - xr2, y2, x2 - xr1, y2)
    cr.line_to(x1 + xr1, y2)
    cr.curve_to(x1 + xr2, y2, x1, y2 - yr2, x1, y2 - yr1)
    cr.line_to(x1, y1 + yr1)
    cr.curve_to(x1, y1 + yr2, x1 + xr2, y1, x1 + xr1, y1)
    cr.close_path()

class HudWindow(gtk.Window):
    def __init__(self):
        gtk.Window.__init__(self)
        self.set_border_width(24)
        self.set_app_paintable(True)
        self.set_decorated(False)
        self.set_position(gtk.WIN_POS_CENTER_ALWAYS)

        # setup RGBA colormaps for translucency
        screen = self.get_screen()
        colormap = screen.get_rgba_colormap()
        if colormap:
            gtk.widget_set_default_colormap(colormap)

        # parse the rcdata
        gtk.rc_parse_string(rcdata)

        self.connect('expose-event', self.hud_expose)

    def hud_expose(self, _, event):
        c = event.window.cairo_create()
        w = self.allocation.width
        h = self.allocation.height

        # clip render area
        c.rectangle(event.area)
        c.clip()

        # clear the background
        c.set_operator(cairo.OPERATOR_CLEAR)
        c.paint()

        # slightly translucent background
        c.set_operator(cairo.OPERATOR_OVER)
        rounded_rectangle(c, 10, 10, w - 20, h - 20, 10, 10)
        c.set_source_rgba(0, 0, 0, 0.80)
        c.fill()

if __name__ == '__main__':
    w = HudWindow()
    w.set_default_size(640, 200)
    w.connect('delete-event', gtk.main_quit)
    w.show()

    v = gtk.VBox(False, 12)
    w.add(v)
    v.show()

    s = gtk.ScrolledWindow()
    s.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
    v.add(s)
    s.show()

    t = gtk.TreeView()
    t.props.headers_visible = False
    s.add(t)
    t.show()

    m = gtk.ListStore(str)
    m.append(row=('',))
    t.set_model(m)

    cl = gtk.TreeViewColumn()
    cl.set_title('Task')
    cell = gtk.CellRendererText()
    cell.props.editable = True
    def edited(c, p, new_text):
        i = m.get_iter(p)
        m.set(i, 0, new_text)
    cell.connect('edited', edited)
    cl.pack_start(cell, True)
    cl.add_attribute(cell, 'text', 0)
    t.append_column(cl)

    wh = gtk.HBox(False, 6)
    v.pack_start(wh, False, True, 0)
    wh.show()

    where = gtk.Label('Where:')
    wh.pack_start(where, False, True, 0)
    where.show()

    combo = gtk.combo_box_new_text()
    combo_cell = combo.get_cells()[0]
    combo_cell.props.foreground = '#ffffff'
    combo.append_text('Inbox')
    combo.set_size_request(200, -1)
    combo.set_active(0)
    wh.pack_start(combo, False, True, 0)
    combo.show()

    h = gtk.HButtonBox()
    h.set_spacing(6)
    h.set_layout(gtk.BUTTONBOX_END)
    wh.pack_start(h, True, True, 0)
    h.show()

    c = gtk.Button(stock=gtk.STOCK_CANCEL)
    h.pack_end(c, False, True, 0)
    c.connect('clicked', gtk.main_quit)
    c.show()

    a = gtk.Button(stock=gtk.STOCK_SAVE)
    h.pack_end(a, False, True, 0)
    a.connect('clicked', gtk.main_quit)
    a.show()

    t.grab_focus()
    gtk.main()
