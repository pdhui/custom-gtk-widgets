// 
// Main.cs
//  
// Author:
//       Christian Hergert <chris@dronelabs.com>
// 
// Copyright (c) 2009 Christian Hergert
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

using System;

using Gtk;
using TreeMap;

namespace Example
{
	class MainClass
	{
		public static void Main (string[] args)
		{
			Application.Init ();
			
			var window = new Gtk.Window (Gtk.WindowType.Toplevel) {
				Title       = "Treemap Example",
				BorderWidth = 0,//12,
			};
			window.SetDefaultSize (640, 480);
			window.DeleteEvent += delegate {
				Gtk.Application.Quit ();
			};
			window.Show ();
			
			var vbox = new Gtk.VBox (false, 6);
			window.Add (vbox);
			vbox.Show ();
			
			var treemap = new TreeMap.TreeMap () {
				Model        = BuildModel (),
				TextColumn   = 0,
				WeightColumn = 1,
				Title        = "Treemap Example",
			};
			vbox.PackStart (treemap, true, true, 0);
			treemap.Show ();
			
			var buttonbox = new Gtk.HButtonBox ();
			buttonbox.BorderWidth = 12;
			buttonbox.Layout = Gtk.ButtonBoxStyle.End;
			vbox.PackStart (buttonbox, false, true, 0);
			buttonbox.Show ();
			
			var close = new Gtk.Button (Gtk.Stock.Close);
			close.CanDefault = true;
			close.Clicked += delegate { Gtk.Application.Quit (); };
			buttonbox.PackStart (close, false, true, 0);
			window.Default = close;
			close.Show ();
			
			Application.Run ();
		}
		
		static Gtk.TreeModel BuildModel ()
		{
			var model = new Gtk.TreeStore (typeof (string), typeof (double));
			
			var i = model.AppendValues ("Item 1", 20.0);
			model.AppendValues (i, "Item 1.1", 123.0);
			model.AppendValues (i, "Item 1.2", 103.0);
			model.AppendValues (i, "Item 1.3", 113.0);
			
			model.AppendValues ("Item 2", 15.0);
			model.AppendValues ("Item 3", 35.0);
			model.AppendValues ("Item 4", 5.0);
			
			return model;
		}
	}
}
