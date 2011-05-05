/* gb-animation.c
 *
 * Copyright (C) 2010 Christian Hergert <chris@dronelabs.com>
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

#include <gobject/gvaluecollector.h>
#include <gtk/gtk.h>
#include <string.h>

#include "gb-animation.h"
#include "gb-frame-source.h"


#define TIMEVAL_TO_MSEC(t) (((t).tv_sec * 1000UL) + ((t).tv_usec / 1000UL))
#define LAST_FUNDAMENTAL 64
#define TWEEN(type)                                         \
    static void                                             \
    tween_##type (const GValue *begin,                      \
                  const GValue *end,                        \
                  GValue *value,                            \
                  gdouble offset)                           \
    {                                                       \
    	g##type x = g_value_get_##type(begin);              \
    	g##type y = g_value_get_##type(end);                \
    	g_value_set_##type(value, x + ((y - x) * offset));  \
    }


G_DEFINE_TYPE(GbAnimation, gb_animation, G_TYPE_INITIALLY_UNOWNED)


typedef gdouble (*AlphaFunc) (gdouble       offset);
typedef void    (*TweenFunc) (const GValue *begin,
                              const GValue *end,
                              GValue       *value,
                              gdouble       offset);

typedef struct
{
	gboolean    is_child; /* Does GParamSpec belong to parent widget */
	GParamSpec *pspec;    /* GParamSpec of target property */
	GValue      begin;    /* Begin value in animation */
	GValue      end;      /* End value in animation */
} Tween;


struct _GbAnimationPrivate
{
	gpointer  target;        /* Target object to animate */
	guint64   begin_msec;    /* Time in which animation started */
	guint     duration_msec; /* Duration of animation */
	guint     mode;          /* Tween mode */
	guint     tween_handler; /* GSource performing tweens */
	GArray   *tweens;        /* Array of tweens to perform */
	guint     frame_rate;    /* The frame-rate to use */
	guint     frame_count;   /* Counter for debugging frames rendered */
};


enum
{
	PROP_0,
	PROP_DURATION,
	PROP_FRAME_RATE,
	PROP_MODE,
	PROP_TARGET,
};


enum
{
	TICK,
	LAST_SIGNAL
};


/*
 * Globals.
 */
static AlphaFunc alpha_funcs[GB_ANIMATION_LAST] = { NULL };
static TweenFunc tween_funcs[LAST_FUNDAMENTAL] = { NULL };
static guint     signals[LAST_SIGNAL] = { 0 };
static gboolean  debug = FALSE;


/*
 * Tweeners for basic types.
 */
TWEEN(int);
TWEEN(uint);
TWEEN(long);
TWEEN(ulong);
TWEEN(float);
TWEEN(double);


/**
 * gb_animation_alpha_ease_in_cubic:
 * @offset: (in): The position within the animation; 0.0 to 1.0.
 *
 * An alpha function to transform the offset within the animation.
 * @GB_ANIMATION_CUBIC means the valu ewill be transformed into
 * cubic acceleration (x * x * x).
 */
static gdouble
gb_animation_alpha_ease_in_cubic (gdouble offset)
{
	return offset * offset * offset;
}


/**
 * gb_animation_alpha_linear:
 * @offset: (in): The position within the animation; 0.0 to 1.0.
 *
 * An alpha function to transform the offset within the animation.
 * @GB_ANIMATION_LINEAR means no tranformation will be made.
 *
 * Returns: @offset.
 * Side effects: None.
 */
static gdouble
gb_animation_alpha_linear (gdouble offset)
{
	return offset;
}


/**
 * gb_animation_alpha_ease_in_quad:
 * @offset: (in): The position within the animation; 0.0 to 1.0.
 *
 * An alpha function to transform the offset within the animation.
 * @GB_ANIMATION_EASE_IN_QUAD means that the value will be transformed
 * into a quadratic acceleration.
 *
 * Returns: A tranformation of @offset.
 * Side effects: None.
 */
static gdouble
gb_animation_alpha_ease_in_quad (gdouble offset)
{
	return offset * offset;
}


/**
 * gb_animation_alpha_ease_out_quad:
 * @offset: (in): The position within the animation; 0.0 to 1.0.
 *
 * An alpha function to transform the offset within the animation.
 * @GB_ANIMATION_EASE_OUT_QUAD means that the value will be transformed
 * into a quadratic deceleration.
 *
 * Returns: A tranformation of @offset.
 * Side effects: None.
 */
static gdouble
gb_animation_alpha_ease_out_quad (gdouble offset)
{
	return -1.0 * offset * (offset - 2.0);
}


/**
 * gb_animation_alpha_ease_in_out_quad:
 * @offset: (in): The position within the animation; 0.0 to 1.0.
 *
 * An alpha function to transform the offset within the animation.
 * @GB_ANIMATION_EASE_IN_OUT_QUAD means that the value will be transformed
 * into a quadratic acceleration for the first half, and quadratic
 * deceleration the second half.
 *
 * Returns: A tranformation of @offset.
 * Side effects: None.
 */
static gdouble
gb_animation_alpha_ease_in_out_quad (gdouble offset)
{
	offset *= 2.0;
	if (offset < 1.0) {
		return 0.5 * offset * offset;
	}
	offset -= 1.0;
	return -0.5 * (offset * (offset - 2.0) - 1.0);
}


/**
 * gb_animation_load_begin_values:
 * @animation: (in): A #GbAnimation.
 *
 * Load the begin values for all the properties we are about to
 * animate.
 *
 * Returns: None.
 * Side effects: None.
 */
static void
gb_animation_load_begin_values (GbAnimation *animation)
{
	GbAnimationPrivate *priv;
	GtkContainer *container;
	Tween *tween;
	gint i;

	g_return_if_fail(GB_IS_ANIMATION(animation));

	priv = animation->priv;

	for (i = 0; i < priv->tweens->len; i++) {
		tween = &g_array_index(priv->tweens, Tween, i);
		g_value_reset(&tween->begin);
		if (tween->is_child) {
			container = GTK_CONTAINER(gtk_widget_get_parent(priv->target));
			gtk_container_child_get_property(container, priv->target,
			                                 tween->pspec->name,
			                                 &tween->begin);
		} else {
			g_object_get_property(priv->target, tween->pspec->name,
			                      &tween->begin);
		}
	}
}


/**
 * gb_animation_unload_begin_values:
 * @animation: (in): A #GbAnimation.
 *
 * Unloads the begin values for the animation. This might be particularly
 * useful once we support pointer types.
 *
 * Returns: None.
 * Side effects: None.
 */
static void
gb_animation_unload_begin_values (GbAnimation *animation)
{
	GbAnimationPrivate *priv;
	Tween *tween;
	gint i;

	g_return_if_fail(GB_IS_ANIMATION(animation));

	priv = animation->priv;

	for (i = 0; i < priv->tweens->len; i++) {
		tween = &g_array_index(priv->tweens, Tween, i);
		g_value_reset(&tween->begin);
	}
}


/**
 * gb_animation_get_offset:
 * @animation: (in): A #GbAnimation.
 *
 * Retrieves the position within the animation from 0.0 to 1.0. This
 * value is calculated using the msec of the beginning of the animation
 * and the current time.
 *
 * Returns: The offset of the animation from 0.0 to 1.0.
 * Side effects: None.
 */
static gdouble
gb_animation_get_offset (GbAnimation *animation)
{
	GbAnimationPrivate *priv;
	GTimeVal now;
	guint64 msec;
	gdouble offset;

	g_return_val_if_fail(GB_IS_ANIMATION(animation), 0.0);

	priv = animation->priv;

	g_get_current_time(&now);
	msec = TIMEVAL_TO_MSEC(now);
	offset = (gdouble)(msec - priv->begin_msec)
	       / (gdouble)priv->duration_msec;
	return CLAMP(offset, 0.0, 1.0);
}


/**
 * gb_animation_update_property:
 * @animation: (in): A #GbAnimation.
 * @target: (in): A #GObject.
 * @tween: (in): a #Tween containing the property.
 * @value: (in) The new value for the property.
 *
 * Updates the value of a property on an object using @value.
 *
 * Returns: None.
 * Side effects: The property of @target is updated.
 */
static void
gb_animation_update_property (GbAnimation *animation,
                               gpointer      target,
                               Tween        *tween,
                               const GValue *value)
{
	g_object_set_property(target, tween->pspec->name, value);
}


/**
 * gb_animation_update_child_property:
 * @animation: (in): A #GbAnimation.
 * @target: (in): A #GObject.
 * @tween: (in): A #Tween containing the property.
 * @value: (in): The new value for the property.
 *
 * Updates the value of the parent widget of the target to @value.
 *
 * Returns: None.
 * Side effects: The property of @target<!-- -->'s parent widget is updated.
 */
static void
gb_animation_update_child_property (GbAnimation *animation,
                                     gpointer      target,
                                     Tween        *tween,
                                     const GValue *value)
{
	GtkWidget *parent = gtk_widget_get_parent(GTK_WIDGET(target));
	gtk_container_child_set_property(GTK_CONTAINER(parent), target,
	                                 tween->pspec->name, value);
}


/**
 * gb_animation_get_value_at_offset:
 * @animation: (in): A #GbAnimation.
 * @offset: (in): The offset in the animation from 0.0 to 1.0.
 * @tween: (in): A #Tween containing the property.
 * @value: (out): A #GValue in which to store the property.
 *
 * Retrieves a value for a particular position within the animation.
 *
 * Returns: None.
 * Side effects: None.
 */
static void
gb_animation_get_value_at_offset (GbAnimation *animation,
                                   gdouble       offset,
                                   Tween        *tween,
                                   GValue       *value)
{
	g_return_if_fail(GB_IS_ANIMATION(animation));
	g_return_if_fail(offset >= 0.0);
	g_return_if_fail(offset <= 1.0);
	g_return_if_fail(tween != NULL);
	g_return_if_fail(value != NULL);
	g_return_if_fail(value->g_type == tween->pspec->value_type);

	if (value->g_type < LAST_FUNDAMENTAL) {
		/*
		 * If you hit the following assertion, you need to add a function
		 * to create the new value at the given offset.
		 */
		g_assert(tween_funcs[value->g_type]);
		tween_funcs[value->g_type](&tween->begin, &tween->end, value, offset);
	} else {
		/*
		 * TODO: Support complex transitions.
		 */
		if (offset >= 1.0) {
			g_value_copy(&tween->end, value);
		}
	}
}


/**
 * gb_animation_tick:
 * @animation: (in): A #GbAnimation.
 *
 * Moves the object properties to the next position in the animation.
 *
 * Returns: %TRUE if the animation has not completed; otherwise %FALSE.
 * Side effects: None.
 */
static gboolean
gb_animation_tick (GbAnimation *animation)
{
	GbAnimationPrivate *priv;
	GdkWindow *window;
	gdouble offset;
	gdouble alpha;
	GValue value = { 0 };
	Tween *tween;
	gint i;

	g_return_val_if_fail(GB_IS_ANIMATION(animation), FALSE);

	priv = animation->priv;

	priv->frame_count++;
	offset = gb_animation_get_offset(animation);
	alpha = alpha_funcs[priv->mode](offset);

	/*
	 * Update property values.
	 */
	for (i = 0; i < priv->tweens->len; i++) {
		tween = &g_array_index(priv->tweens, Tween, i);
		g_value_init(&value, tween->pspec->value_type);
		gb_animation_get_value_at_offset(animation, alpha, tween, &value);
		if (!tween->is_child) {
			gb_animation_update_property(animation, priv->target,
			                              tween, &value);
		} else {
			gb_animation_update_child_property(animation, priv->target,
			                                    tween, &value);
		}
		g_value_unset(&value);
	}

	/*
	 * Notify anyone interested in the tick signal.
	 */
	g_signal_emit(animation, signals[TICK], 0);

	/*
	 * Flush any outstanding events to the graphics server (in the case of X).
	 */
	if (GTK_IS_WIDGET(priv->target)) {
		if ((window = gtk_widget_get_window(GTK_WIDGET(priv->target)))) {
			gdk_window_flush(window);
		}
	}

	return (offset < 1.0);
}


/**
 * gb_animation_timeout:
 * @data: (in): A #GbAnimation.
 *
 * Timeout from the main loop to move to the next step of the animation.
 *
 * Returns: %TRUE until the animation has completed; otherwise %FALSE.
 * Side effects: None.
 */
static gboolean
gb_animation_timeout (gpointer data)
{
	GbAnimation *animation = (GbAnimation *)data;
	gboolean ret;

	if (!(ret = gb_animation_tick(animation))) {
		gb_animation_stop(animation);
	}

	return ret;
}


/**
 * gb_animation_start:
 * @animation: (in): A #GbAnimation.
 *
 * Start the animation. When the animation stops, the internal reference will
 * be dropped and the animation may be finalized.
 *
 * Returns: None.
 * Side effects: None.
 */
void
gb_animation_start (GbAnimation *animation)
{
	GbAnimationPrivate *priv;
	GTimeVal now;

	g_return_if_fail(GB_IS_ANIMATION(animation));
	g_return_if_fail(!animation->priv->tween_handler);

	priv = animation->priv;

	g_get_current_time(&now);
	g_object_ref_sink(animation);
	gb_animation_load_begin_values(animation);

	priv->begin_msec = TIMEVAL_TO_MSEC(now);
	priv->tween_handler = gb_frame_source_add(priv->frame_rate,
	                                           gb_animation_timeout,
	                                           animation);
}


/**
 * gb_animation_stop:
 * @animation: (in): A #GbAnimation.
 *
 * Stops a running animation. The internal reference to the animation is
 * dropped and therefore may cause the object to finalize.
 *
 * Returns: None.
 * Side effects: None.
 */
void
gb_animation_stop (GbAnimation *animation)
{
	GbAnimationPrivate *priv;

	g_return_if_fail(GB_IS_ANIMATION(animation));

	priv = animation->priv;

   if (priv->tween_handler) {
      g_source_remove(priv->tween_handler);
      priv->tween_handler = 0;
      gb_animation_unload_begin_values(animation);
	   g_object_unref(animation);
   }
}


/**
 * gb_animation_add_property:
 * @animation: (in): A #GbAnimation.
 * @pspec: (in): A #ParamSpec of @target or a #GtkWidget<!-- -->'s parent.
 * @value: (in): The new value for the property at the end of the animation.
 *
 * Adds a new property to the set of properties to be animated during the
 * lifetime of the animation.
 *
 * Returns: None.
 * Side effects: None.
 */
void
gb_animation_add_property (GbAnimation *animation,
                            GParamSpec   *pspec,
                            const GValue *value)
{
	GbAnimationPrivate *priv;
	Tween tween = { 0 };
	GType type;

	g_return_if_fail(GB_IS_ANIMATION(animation));
	g_return_if_fail(pspec != NULL);
	g_return_if_fail(value != NULL);
	g_return_if_fail(value->g_type);
	g_return_if_fail(animation->priv->target);
	g_return_if_fail(!animation->priv->tween_handler);

	priv = animation->priv;

	type = G_TYPE_FROM_INSTANCE(priv->target);
	tween.is_child = !g_type_is_a(type, pspec->owner_type);
	if (tween.is_child) {
		if (!GTK_IS_WIDGET(priv->target)) {
			g_critical("Cannot locate property %s in class %s",
			           pspec->name, g_type_name(type));
			return;
		}
	}

	tween.pspec = g_param_spec_ref(pspec);
	g_value_init(&tween.begin, pspec->value_type);
	g_value_init(&tween.end, pspec->value_type);
	g_value_copy(value, &tween.end);
	g_array_append_val(priv->tweens, tween);
}


/**
 * gb_animation_dispose:
 * @object: (in): A #GbAnimation.
 *
 * Releases any object references the animation contains.
 *
 * Returns: None.
 * Side effects: None.
 */
static void
gb_animation_dispose (GObject *object)
{
	GbAnimationPrivate *priv = GB_ANIMATION(object)->priv;
	gpointer instance;

	if ((instance = priv->target)) {
		priv->target = NULL;
		g_object_unref(instance);
	}

	G_OBJECT_CLASS(gb_animation_parent_class)->dispose(object);
}


/**
 * gb_animation_finalize:
 * @object: (in): A #GbAnimation.
 *
 * Finalizes the object and releases any resources allocated.
 *
 * Returns: None.
 * Side effects: None.
 */
static void
gb_animation_finalize (GObject *object)
{
	GbAnimationPrivate *priv = GB_ANIMATION(object)->priv;
	Tween *tween;
	gint i;

	for (i = 0; i < priv->tweens->len; i++) {
		tween = &g_array_index(priv->tweens, Tween, i);
		g_value_unset(&tween->begin);
		g_value_unset(&tween->end);
		g_param_spec_unref(tween->pspec);
	}

	g_array_unref(priv->tweens);

	if (debug) {
		g_print("Rendered %d frames in %d msec animation.\n",
		        priv->frame_count, priv->duration_msec);
	}

	G_OBJECT_CLASS(gb_animation_parent_class)->finalize(object);
}


/**
 * gb_animation_set_property:
 * @object: (in): A #GObject.
 * @prop_id: (in): The property identifier.
 * @value: (in): The given property.
 * @pspec: (in): A #ParamSpec.
 *
 * Set a given #GObject property.
 */
static void
gb_animation_set_property (GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
	GbAnimation *animation = GB_ANIMATION(object);

	switch (prop_id) {
	case PROP_DURATION:
		animation->priv->duration_msec = g_value_get_uint(value);
		break;
	case PROP_FRAME_RATE:
		animation->priv->frame_rate = g_value_get_uint(value);
		break;
	case PROP_MODE:
		animation->priv->mode = g_value_get_enum(value);
		break;
	case PROP_TARGET:
		animation->priv->target = g_value_dup_object(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}


/**
 * gb_animation_class_init:
 * @klass: (in): A #GbAnimationClass.
 *
 * Initializes the GObjectClass.
 *
 * Returns: None.
 * Side effects: Properties, signals, and vtables are initialized.
 */
static void
gb_animation_class_init (GbAnimationClass *klass)
{
	GObjectClass *object_class;

	debug = !!g_getenv("GB_ANIMATION_DEBUG");

	object_class = G_OBJECT_CLASS(klass);
	object_class->dispose = gb_animation_dispose;
	object_class->finalize = gb_animation_finalize;
	object_class->set_property = gb_animation_set_property;
	g_type_class_add_private(object_class, sizeof(GbAnimationPrivate));

	g_object_class_install_property(object_class,
	                                PROP_DURATION,
	                                g_param_spec_uint("duration",
	                                                  "Duration",
	                                                  "The duration of the animation",
	                                                  0,
	                                                  G_MAXUINT,
	                                                  250,
	                                                  G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property(object_class,
	                                PROP_MODE,
	                                g_param_spec_enum("mode",
	                                                  "Mode",
	                                                  "The animation mode",
	                                                  GB_TYPE_ANIMATION_MODE,
	                                                  GB_ANIMATION_LINEAR,
	                                                  G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property(object_class,
	                                PROP_TARGET,
	                                g_param_spec_object("target",
	                                                    "Target",
	                                                    "The target of the animation",
	                                                    G_TYPE_OBJECT,
	                                                    G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property(object_class,
	                                PROP_FRAME_RATE,
	                                g_param_spec_uint("frame-rate",
	                                                  "frame-rate",
	                                                  "frame-rate",
	                                                  1,
	                                                  G_MAXUINT,
	                                                  60,
	                                                  G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

	signals[TICK] = g_signal_new("tick",
	                             GB_TYPE_ANIMATION,
	                             G_SIGNAL_RUN_FIRST,
	                             0,
	                             NULL,
	                             NULL,
	                             g_cclosure_marshal_VOID__VOID,
	                             G_TYPE_NONE,
	                             0);

#define SET_ALPHA(_T, _t) \
	alpha_funcs[GB_ANIMATION_##_T] = gb_animation_alpha_##_t

	SET_ALPHA(LINEAR, linear);
	SET_ALPHA(EASE_IN_QUAD, ease_in_quad);
	SET_ALPHA(EASE_OUT_QUAD, ease_out_quad);
	SET_ALPHA(EASE_IN_OUT_QUAD, ease_in_out_quad);
	SET_ALPHA(EASE_IN_CUBIC, ease_in_cubic);

#define SET_TWEEN(_T, _t) \
	G_STMT_START { \
		guint idx = G_TYPE_##_T; \
		tween_funcs[idx] = tween_##_t; \
	} G_STMT_END

	SET_TWEEN(INT, int);
	SET_TWEEN(UINT, uint);
	SET_TWEEN(LONG, long);
	SET_TWEEN(ULONG, ulong);
	SET_TWEEN(FLOAT, float);
	SET_TWEEN(DOUBLE, double);
}


/**
 * gb_animation_init:
 * @animation: (in): A #GbAnimation.
 *
 * Initializes the #GbAnimation instance.
 *
 * Returns: None.
 * Side effects: Everything.
 */
static void
gb_animation_init (GbAnimation *animation)
{
	GbAnimationPrivate *priv;

	priv = G_TYPE_INSTANCE_GET_PRIVATE(animation, GB_TYPE_ANIMATION,
	                                   GbAnimationPrivate);
	animation->priv = priv;

	priv->duration_msec = 250;
	priv->frame_rate = 60;
	priv->mode = GB_ANIMATION_LINEAR;
	priv->tweens = g_array_new(FALSE, FALSE, sizeof(Tween));
}


/**
 * gb_animation_mode_get_type:
 *
 * Retrieves the GType for #GbAnimationMode.
 *
 * Returns: A GType.
 * Side effects: GType registered on first call.
 */
GType
gb_animation_mode_get_type (void)
{
	static GType type_id = 0;
	static const GEnumValue values[] = {
		{ GB_ANIMATION_LINEAR, "GB_ANIMATION_LINEAR", "LINEAR" },
		{ GB_ANIMATION_EASE_IN_QUAD, "GB_ANIMATION_EASE_IN_QUAD", "EASE_IN_QUAD" },
		{ GB_ANIMATION_EASE_IN_OUT_QUAD, "GB_ANIMATION_EASE_IN_OUT_QUAD", "EASE_IN_OUT_QUAD" },
		{ GB_ANIMATION_EASE_OUT_QUAD, "GB_ANIMATION_EASE_OUT_QUAD", "EASE_OUT_QUAD" },
		{ GB_ANIMATION_EASE_IN_CUBIC, "GB_ANIMATION_EASE_IN_CUBIC", "EASE_IN_CUBIC" },
		{ 0 }
	};

	if (G_UNLIKELY(!type_id)) {
		type_id = g_enum_register_static("GbAnimationMode", values);
	}
	return type_id;
}

/**
 * gb_object_animatev:
 * Returns: (transfer none): A #GbAnimation.
 */
GbAnimation*
gb_object_animatev (gpointer          object,
                    GbAnimationMode  mode,
                    guint             duration_msec,
                    guint             frame_rate,
                    const gchar      *first_property,
                    va_list           args)
{
	GbAnimation *animation;
	GObjectClass *klass;
	GObjectClass *pklass;
	const gchar *name;
	GParamSpec *pspec;
	GtkWidget *parent;
	GValue value = { 0 };
	gchar *error = NULL;
	GType type;
	GType ptype;

	g_return_val_if_fail(first_property != NULL, NULL);
	g_return_val_if_fail(mode < GB_ANIMATION_LAST, NULL);

	name = first_property;
	type = G_TYPE_FROM_INSTANCE(object);
	klass = G_OBJECT_GET_CLASS(object);
	animation = g_object_new(GB_TYPE_ANIMATION,
	                         "duration", duration_msec,
	                         "frame-rate", frame_rate ? frame_rate : 60,
	                         "mode", mode,
	                         "target", object,
	                         NULL);

	do {
		/*
		 * First check for the property on the object. If that does not exist
		 * then check if the object has a parent and look at its child
		 * properties (if its a GtkWidget).
		 */
		if (!(pspec = g_object_class_find_property(klass, name))) {
			if (!g_type_is_a(type, GTK_TYPE_WIDGET)) {
				g_critical("Failed to find property %s in %s",
				           name, g_type_name(type));
				goto failure;
			}
			if (!(parent = gtk_widget_get_parent(object))) {
				g_critical("Failed to find property %s in %s",
				           name, g_type_name(type));
				goto failure;
			}
			pklass = G_OBJECT_GET_CLASS(parent);
			ptype = G_TYPE_FROM_INSTANCE(parent);
			if (!(pspec = gtk_container_class_find_child_property(pklass, name))) {
				g_critical("Failed to find property %s in %s or parent %s",
				           name, g_type_name(type), g_type_name(ptype));
				goto failure;
			}
		}

		g_value_init(&value, pspec->value_type);
		G_VALUE_COLLECT(&value, args, 0, &error);
		if (error != NULL) {
			g_critical("Failed to retrieve va_list value: %s", error);
			g_free(error);
			goto failure;
		}

		gb_animation_add_property(animation, pspec, &value);
		g_value_unset(&value);
	} while ((name = va_arg(args, const gchar *)));

	gb_animation_start(animation);

	return animation;

failure:
	g_object_ref_sink(animation);
	g_object_unref(animation);
	return NULL;
}

/**
 * gb_object_animate:
 * @object: (in): A #GObject.
 * @mode: (in): The animation mode.
 * @duration_msec: (in): The duration in milliseconds.
 * @first_property: (in): The first property to animate.
 *
 * Animates the properties of @object. The can be set in a similar
 * manner to g_object_set(). They will be animated from their current
 * value to the target value over the time period.
 *
 * Return value: (transfer none): A #GbAnimation.
 * Side effects: None.
 */
GbAnimation*
gb_object_animate (gpointer          object,
                   GbAnimationMode   mode,
                   guint             duration_msec,
                   const gchar      *first_property,
                   ...)
{
	GbAnimation *animation;
	va_list args;

	va_start(args, first_property);
	animation = gb_object_animatev(object, mode, duration_msec, 0,
	                              first_property, args);
	va_end(args);
	return animation;
}

/**
 * gb_object_animate_full:
 *
 * Return value: (transfer none): A #GbAnimation.
 */
GbAnimation*
gb_object_animate_full (gpointer          object,
                        GbAnimationMode  mode,
                        guint             duration_msec,
                        guint             frame_rate,
                        GDestroyNotify    notify,
                        gpointer          notify_data,
                        const gchar      *first_property,
                        ...)
{
	GbAnimation *animation;
	va_list args;

	va_start(args, first_property);
	animation = gb_object_animatev(object, mode, duration_msec,
	                               frame_rate, first_property, args);
	va_end(args);
	g_object_weak_ref(G_OBJECT(animation), (GWeakNotify)notify, notify_data);
	return animation;
}
