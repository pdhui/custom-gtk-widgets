// 
// TreeMap.cs
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
using System.Collections.Generic;

using Cairo;
using Gdk;
using GLib;
using Gtk;
using Pango;

namespace TreeMap
{
	public class TreeMap: Gtk.DrawingArea
	{
		const int ANIMATION_TIMEOUT        = 10;
		
		// Data and selection items
		Gtk.TreeModel      m_Model         = null;
		int                m_TextColumn    = -1;
		int                m_WeightColumn  = -1;
		string             m_Title         = String.Empty;
		int                m_titleOffset   = 0;
		int                m_titleHeight   = 36;
		int                m_titleLip      = 3;
		bool               m_pressed       = false;
		double             m_cursorX       = -1;
		double             m_cursorY       = -1;
		List<TreeMapItem>  m_rootItems     = null;
		
		// Formatting and color
		Gdk.Color          m_Background    = Gdk.Color.Zero;
		bool               m_BackgroundSet = false;
		Cairo.ImageSurface m_bgImage       = null;
		bool               m_ShowTitle     = true;
		
		// Animation handlers
		uint               m_showHandler   = 0;
		uint               m_hideHandler   = 0;
		
		public TreeMap ()
		{
			this.Events |= Gdk.EventMask.PointerMotionMask
			            |  Gdk.EventMask.EnterNotifyMask
			            |  Gdk.EventMask.LeaveNotifyMask
			            |  Gdk.EventMask.ButtonPressMask
			            |  Gdk.EventMask.ButtonReleaseMask;
			
			this.MotionNotifyEvent  += HandleMotionNotifyEvent;
			this.ButtonPressEvent   += HandleButtonPressEvent;
			this.ButtonReleaseEvent += HandleButtonReleaseEvent;
			this.EnterNotifyEvent   += HandleEnterNotifyEvent;
			this.LeaveNotifyEvent   += HandleLeaveNotifyEvent;
		}
		
		/// <summary>
		/// The data model for the TreeMap.
		/// </summary>
		public Gtk.TreeModel Model {
			get {
				return m_Model;
			}
			set {
				if (m_Model != null) {
					m_Model.RowDeleted  -= Model_RowDeleted;
					m_Model.RowInserted -= Model_RowInserted;
					m_Model.RowChanged  -= Model_RowChanged;
				}
				
				m_Model = value;
				
				if (m_Model != null) {
					m_Model.RowDeleted  += Model_RowDeleted;
					m_Model.RowInserted += Model_RowInserted;
					m_Model.RowChanged  += Model_RowChanged;
				}
				
				Invalidate ();
			}
		}
		
		/// <summary>
		/// The base background color for the treemap items.  Individual items
		/// will have a variant of the shade.
		/// </summary>
		public Gdk.Color Background {
			get {
				return m_Background;
			}
			set {
				m_Background = value;
				m_BackgroundSet = !value.Equals (Gdk.Color.Zero);
				Invalidate ();
			}
		}
		
		/// <summary>
		/// If the background has been explicitly set.
		/// </summary>
		public bool BackgroundSet {
			get { return m_BackgroundSet; }
		}
		
		/// <summary>
		/// Which column in the Gtk.TreeModel contains the text labels.
		/// </summary>
		public int TextColumn {
			get {
				return m_TextColumn;
			}
			set {
				m_TextColumn = value;
				Invalidate ();
			}
		}
		
		/// <summary>
		/// Which column in the Gtk.TreeModel contains the weight.
		/// </summary>
		public int WeightColumn {
			get {
				return m_WeightColumn;
			}
			set {
				m_WeightColumn = value;
				Invalidate ();
			}
		}
		
		/// <summary>
		/// If the treemap title should be displayed.  The title slides in
		/// and out from the top of the widget as an animation.  See the
		/// Title property.
		/// </summary>
		public bool ShowTitle {
			get {
				return m_ShowTitle;
			}
			set {
				m_ShowTitle = value;
				Invalidate ();
			}
		}
		
		/// <summary>
		/// The title of the treemap.
		/// </summary>
		public string Title {
			get {
				return m_Title;
			}
			set {
				m_Title = value;
				Invalidate ();
			}
		}
		
		protected override void OnRealized ()
		{
			base.OnRealized ();
			Invalidate ();
		}
		
		protected override void OnSizeAllocated (Gdk.Rectangle allocation)
		{
			base.OnSizeAllocated (allocation);
			Invalidate ();
		}
		
		protected override void OnStyleSet (Gtk.Style previous_style)
		{
			base.OnStyleSet (previous_style);
			Invalidate ();
		}

		protected override bool OnExposeEvent (Gdk.EventExpose e)
		{
			using (var c = Gdk.CairoHelper.Create (this.GdkWindow))
			{
				Gdk.CairoHelper.Rectangle (c, e.Area);
				c.Clip ();
				
				// Make sure we have an image to paint
				if (m_bgImage == null) {
					c.Fill ();
					return base.OnExposeEvent (e);
				}
				
				// Paint our background image (the actual treemap)
				c.SetSourceSurface (m_bgImage, 0, 0);
				c.Paint ();
				
				// Paint highlight if needed
				if (m_rootItems != null && m_cursorX >= 0 && m_cursorY >= 0) {
					foreach (TreeMapItem i in m_rootItems) {
						var item = i.FindByCoordinates (m_cursorX, m_cursorY);
						if (item != null) {
							c.Rectangle (item.X, item.Y, item.Width, item.Height);
							if (m_pressed)
								c.SetSourceRGBA (0, 0, 0, 0.2);
							else
								c.SetSourceRGBA (1, 1, 1, 0.2);
							c.Fill ();
							break;
						}
					}
				}
				
				// Paint the title if needed. it may be within an animation.
				if (m_ShowTitle)
					ExposeTitle (c);
			}
			
			return base.OnExposeEvent (e);
		}
		
		void Invalidate ()
		{
			UpdateStyle ();
			UpdateBgImage ();
		}
		
		void UpdateStyle ()
		{
			if (!m_BackgroundSet)
				m_Background = this.Style.Background (Gtk.StateType.Selected);
		}
		
		void UpdateBgImage ()
		{
			m_bgImage = null;
			
			if (TextColumn < 0 || WeightColumn < 0)
				return;
			
			m_bgImage = new Cairo.ImageSurface (Format.Rgb24, Allocation.Width, this.Allocation.Height);
			
			using (var c = new Cairo.Context (m_bgImage))
			{
				// Paint the default background (linear gradient)
				var g = new Cairo.LinearGradient (0, 0, 1, this.Allocation.Height);
				g.AddColorStop (0.2, CairoExtensions.GdkColorToCairoColor (m_Background));
				g.AddColorStop (0.9, CairoExtensions.GdkColorToCairoColor (this.Style.Background (Gtk.StateType.Normal)));
				c.Rectangle (0, 0, this.Allocation.Width, this.Allocation.Height);
				c.Pattern = g;
				c.Paint ();
				g.Dispose ();
				
				Gtk.TreeIter iter;
				
				List<TreeMapItem> rootItems = new List<TreeMapItem> ();
				
				if (m_Model.GetIterFirst (out iter)) {
					do {
						var item = new TreeMapItem (m_Model, ref iter, true, m_TextColumn, m_WeightColumn);
						rootItems.Add (item);
					} while (m_Model.IterNext (ref iter));
					
					double t = 0.0;
					
					rootItems.ForEach (delegate (TreeMapItem i) {
						t += Math.Abs (i.Weight);
					});
					
					double x = 0, y = 0, w = this.Allocation.Width, h = this.Allocation.Height;
					double myx = 0, myy = 0, myw = 0, myh = 0;
					
					rootItems.ForEach (delegate (TreeMapItem i) {
						var ratio = Math.Abs (i.Weight) / t;
						myx = x;
						myy = y;
						myw = w * ratio;
						myh = h;
						x += myw;
						
						i.SetArea (myx, myy, myw, myh);
						i.Render (c, m_Background);
					});
					
					// clear right away to lower refs
					if (this.m_rootItems != null)
						this.m_rootItems.Clear ();
					
					this.m_rootItems = rootItems;
				}
			}
		}
		
		void ExposeTitle (Cairo.Context c)
		{
			// The primary title background
			c.SetSourceRGBA (0, 0, 0, 0.3);
			c.Rectangle (0, m_titleOffset, this.Allocation.Width, m_titleHeight);
			c.Fill ();
			
			// The title shadow (lip)
			c.SetSourceRGBA (0, 0, 0, 0.1);
			c.Rectangle (0, m_titleOffset + m_titleHeight, this.Allocation.Width, m_titleLip);
			c.Fill ();
			
			// Paint the title text
			using (var p = Pango.CairoHelper.CreateLayout (c))
			{
				int w = 0, h = 0, pad = 0;
				
				p.SetText (m_Title);
				p.GetPixelSize (out w, out h);
				pad = (m_titleHeight - h) / 2;
				c.MoveTo (pad, m_titleOffset + pad);
				c.SetSourceRGB (1.0, 1.0, 1.0);
				Pango.CairoHelper.ShowLayout (c, p);
			}
		}
		
		void Model_RowInserted (object o, RowInsertedArgs e)
		{
			Invalidate ();
		}
		
		void Model_RowDeleted (object o, RowDeletedArgs e)
		{
			Invalidate ();
		}
		
		void Model_RowChanged (object o, RowChangedArgs e)
		{
			Invalidate ();
		}
		
		void HandleLeaveNotifyEvent (object o, LeaveNotifyEventArgs args)
		{
			m_cursorX = -1;
			m_cursorY = -1;
			this.QueueDraw ();
			
			if (m_hideHandler > 0)
				return;
			
			// Cancel any active show
			if (m_showHandler > 0) {
				GLib.Source.Remove (m_showHandler);
				m_showHandler = 0;
			}
			
			// Start our animation sequence and track the handler tag
			m_hideHandler = GLib.Timeout.Add (ANIMATION_TIMEOUT, delegate {
				m_titleOffset -= 2;
				if (m_titleOffset < -m_titleHeight)
					m_titleOffset = -m_titleHeight;
				this.QueueDrawArea (0, 0, this.Allocation.Width, m_titleHeight + m_titleLip);
				return m_titleOffset > -m_titleHeight;
			});
		}

		void HandleEnterNotifyEvent (object o, EnterNotifyEventArgs args)
		{
			this.QueueDraw ();
			
			if (m_showHandler > 0)
				return;
			
			// Cancel any active hide
			if (m_hideHandler > 0) {
				GLib.Source.Remove (m_hideHandler);
				m_hideHandler = 0;
			}
			
			// Start our animation sequence and track the handler tag
			m_showHandler = GLib.Timeout.Add (ANIMATION_TIMEOUT, delegate {
				m_titleOffset += 2;
				if (m_titleOffset > 0)
					m_titleOffset = 0;
				this.QueueDrawArea (0, 0, this.Allocation.Width, m_titleHeight + m_titleLip);
				return m_titleOffset < 0;
			});
		}

		void HandleButtonPressEvent (object o, ButtonPressEventArgs args)
		{
			if (args.Event.Button == 1) {
				m_pressed = true;
				this.QueueDraw ();
			}
		}
		
		void HandleButtonReleaseEvent (object o, ButtonReleaseEventArgs args)
		{
			m_pressed = false;
			this.QueueDraw ();
		}

		void HandleMotionNotifyEvent (object o, MotionNotifyEventArgs args)
		{
			m_cursorX = args.Event.X;
			m_cursorY = args.Event.Y;
			this.QueueDraw ();
		}
		
		class TreeMapItem
		{
			public Gtk.TreePath Path;
			public double X;
			public double Y;
			public double Width;
			public double Height;
			public double Weight;
			public List<TreeMapItem> Children;
			public string Title;
			public double ChildrenWeight;
			public bool Horiz;
			
			public TreeMapItem (Gtk.TreeModel m,
			                    ref Gtk.TreeIter iter,
			                    bool horiz,
			                    int textColumn,
			                    int weightColumn)
			{
				if (m == null)
					throw new ArgumentNullException ("m");
				
				if (iter.Equals (Gtk.TreeIter.Zero))
					throw new ArgumentException ("iter");
				
				if (textColumn < 0)
					throw new ArgumentException ("textColumn");
				
				if (weightColumn < 0)
					throw new ArgumentException ("weightColumn");
				
				Path = m.GetPath (iter);
				Title = (string)m.GetValue (iter, textColumn);
				Weight = (double)m.GetValue (iter, weightColumn);
				X = 0;
				Y = 0;
				Width = 0;
				Height = 0;
				Children = null;
				Horiz = horiz;
				
				Gtk.TreeIter citer;
				double c_total = 0.0;
				
				if (m.IterChildren (out citer, iter)) {
					do {
						AddChild (new TreeMapItem (m, ref citer, !horiz, textColumn, weightColumn));
					} while (m.IterNext (ref citer));
					
					// Get total weight
					Children.ForEach (delegate (TreeMapItem i) {
						c_total += Math.Abs (i.Weight);
					});
					
					this.ChildrenWeight = c_total;
				}
			}
			
			public TreeMapItem FindByCoordinates (double x, double y)
			{
				if (x >= X && x <= X + Width) {
					if (y >= Y && y <= Y + Height) {
						if (Children == null)
							return this;
						foreach (TreeMapItem item in Children) {
							TreeMapItem child;
							if (null != (child = item.FindByCoordinates (x, y)))
								return child;
						}
					}
				}
				
				return null;
			}
			
			public void AddChild (TreeMapItem item)
			{
				if (Children == null)
					Children = new List<TreeMapItem> ();
				Children.Add (item);
			}
			
			public void Render (Cairo.Context c, Gdk.Color @base)
			{
				// Fill the background
				var color = CairoExtensions.RandomShadeOfGdkColor (@base);
				c.Rectangle (X, Y, Width, Height);
				c.SetSourceRGB (color.R, color.G, color.B);
				c.Fill ();
				
				if (Children == null) {
					int w, h;
					
					// Render the item text
					using (var p = Pango.CairoHelper.CreateLayout (c))
					{
						p.SetText (Title);
						p.Width = (int)(Pango.Scale.PangoScale * Width);
						p.Ellipsize = Pango.EllipsizeMode.End;
						p.GetPixelSize (out w, out h);
						c.SetSourceRGB (1.0, 1.0, 1.0);
						if (w <= Width && h <= Height) {
							c.MoveTo (X + ((Width - w) / 2), Y + ((Height - h) / 2));
							Pango.CairoHelper.ShowLayout (c, p);
						}
					}
				}
				else {
					Children.ForEach (delegate (TreeMapItem i) {
						i.Render (c, @base);
					});
				}
				
				// Lose our reference to the title
				Title = String.Empty;
			}
			
			public void SetArea (double x, double y, double w, double h)
			{
				this.X = x;
				this.Y = y;
				this.Width = w;
				this.Height = h;
				
				double myx, myy, myw, myh;
				
				if (null != Children && 0.0 != ChildrenWeight) {
					Children.ForEach (delegate (TreeMapItem i) {
						var ratio = Math.Abs (i.Weight) / ChildrenWeight;
						myx = x;
						myy = y;
						
						if (!Horiz) {
							myw = w * ratio;
							myh = h;
							x += myw;
						}
						else {
							myw = w;
							myh = h * ratio;
							y += myh;
						}
						
						i.SetArea (myx, myy, myw, myh);
					});
				}
			}
		}
	}
}
