/*
obs-text-slideshow
Copyright (C) 2021 Joshua Wong jbwong05@gmail.com

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#include <obs-module.h>
#include "obs-text-slideshow.h"

// text gdiplus
#define S_FONT                          "font"
#define S_USE_FILE                      "read_from_file"
#define S_FILE                          "file"
#define S_TEXT                          "text"
#define S_COLOR                         "color"
#define S_GRADIENT                      "gradient"
#define S_GRADIENT_COLOR                "gradient_color"
#define S_GRADIENT_DIR                  "gradient_dir"
#define S_GRADIENT_OPACITY              "gradient_opacity"
#define S_ALIGN                         "align"
#define S_VALIGN                        "valign"
#define S_OPACITY                       "opacity"
#define S_BKCOLOR                       "bk_color"
#define S_BKOPACITY                     "bk_opacity"
#define S_VERTICAL                      "vertical"
#define S_OUTLINE                       "outline"
#define S_OUTLINE_SIZE                  "outline_size"
#define S_OUTLINE_COLOR                 "outline_color"
#define S_OUTLINE_OPACITY               "outline_opacity"
#define S_CHATLOG_MODE                  "chatlog"
#define S_CHATLOG_LINES                 "chatlog_lines"
#define S_EXTENTS                       "extents"
#define S_EXTENTS_WRAP                  "extents_wrap"
#define S_EXTENTS_CX                    "extents_cx"
#define S_EXTENTS_CY                    "extents_cy"
#define S_TRANSFORM                     "transform"
#define S_ANTIALIASING                  "antialiasing"

#define S_ALIGN_LEFT                    "left"
#define S_ALIGN_CENTER                  "center"
#define S_ALIGN_RIGHT                   "right"

#define S_VALIGN_TOP                    "top"
#define S_VALIGN_CENTER                 S_ALIGN_CENTER
#define S_VALIGN_BOTTOM                 "bottom"

#define S_TRANSFORM_NONE                0
#define S_TRANSFORM_UPPERCASE           1
#define S_TRANSFORM_LOWERCASE           2
#define S_TRANSFORM_STARTCASE           3

#define S_ANTIALIASING_NONE             0
#define S_ANTIALIASING_STANDARD         1

// text gdiplus
#define T_(v)                           obs_module_text("Text." v)
#define T_FONT                          T_("Font")
#define T_TEXT                          T_("Text")
#define T_COLOR                         T_("Color")
#define T_GRADIENT                      T_("Gradient")
#define T_GRADIENT_COLOR                T_("Gradient.Color")
#define T_GRADIENT_DIR                  T_("Gradient.Direction")
#define T_GRADIENT_OPACITY              T_("Gradient.Opacity")
#define T_ALIGN                         T_("Alignment")
#define T_VALIGN                        T_("VerticalAlignment")
#define T_OPACITY                       T_("Opacity")
#define T_BKCOLOR                       T_("BkColor")
#define T_BKOPACITY                     T_("BkOpacity")
#define T_VERTICAL                      T_("Vertical")
#define T_OUTLINE                       T_("Outline")
#define T_OUTLINE_SIZE                  T_("Outline.Size")
#define T_OUTLINE_COLOR                 T_("Outline.Color")
#define T_OUTLINE_OPACITY               T_("Outline.Opacity")
#define T_CHATLOG_MODE                  T_("ChatlogMode")
#define T_CHATLOG_LINES                 T_("ChatlogMode.Lines")
#define T_EXTENTS                       T_("UseCustomExtents")
#define T_EXTENTS_WRAP                  T_("UseCustomExtents.Wrap")
#define T_EXTENTS_CX                    T_("Width")
#define T_EXTENTS_CY                    T_("Height")
#define T_TRANSFORM                     T_("Transform")
#define T_ANTIALIASING                  T_("Antialiasing")

#define T_FILTER_TEXT_FILES             T_("Filter.TextFiles")
#define T_FILTER_ALL_FILES              T_("Filter.AllFiles")

#define T_ALIGN_LEFT                    T_("Alignment.Left")
#define T_ALIGN_CENTER                  T_("Alignment.Center")
#define T_ALIGN_RIGHT                   T_("Alignment.Right")

#define T_VALIGN_TOP                    T_("VerticalAlignment.Top")
#define T_VALIGN_CENTER                 T_ALIGN_CENTER
#define T_VALIGN_BOTTOM                 T_("VerticalAlignment.Bottom")

#define T_TRANSFORM_NONE                T_("Transform.None")
#define T_TRANSFORM_UPPERCASE           T_("Transform.Uppercase")
#define T_TRANSFORM_LOWERCASE           T_("Transform.Lowercase")
#define T_TRANSFORM_STARTCASE           T_("Transform.Startcase")

static const char *gdiplus_getname(void *unused) {
    UNUSED_PARAMETER(unused);
    return obs_module_text("TextGdiplusSlideshow");
}

#define obs_data_get_uint32 (uint32_t) obs_data_get_int
#define obs_data_set_uint32 obs_data_set_int

static obs_source_t *create_gdiplus(const char *text, obs_data_t *text_ss_settings) {
	obs_data_t *settings = obs_data_create();
	obs_source_t *source;
	obs_data_t *curr_font = obs_data_get_obj(text_ss_settings, S_FONT);

	obs_data_set_string(settings, S_TEXT, text);
	obs_data_set_obj(settings, S_FONT, curr_font);
	obs_data_set_string(settings, S_ALIGN, 
		obs_data_get_string(text_ss_settings, S_ALIGN));
	obs_data_set_string(settings, S_VALIGN, 
		obs_data_get_string(text_ss_settings, S_VALIGN));
	obs_data_set_uint32(settings, S_COLOR, 
		obs_data_get_uint32(text_ss_settings, S_COLOR));
	obs_data_set_uint32(settings, S_OPACITY, 
		obs_data_get_uint32(text_ss_settings, S_OPACITY));
	obs_data_set_bool(settings, S_GRADIENT, 
		obs_data_get_bool(text_ss_settings, S_GRADIENT));
	obs_data_set_uint32(settings, S_GRADIENT_COLOR, 
		obs_data_get_uint32(text_ss_settings, S_GRADIENT_COLOR));
	obs_data_set_uint32(settings, S_GRADIENT_OPACITY, 
		obs_data_get_uint32(text_ss_settings, S_GRADIENT_OPACITY));
	obs_data_set_double(settings, S_GRADIENT_DIR, 
		obs_data_get_double(text_ss_settings, S_GRADIENT_DIR));
	obs_data_set_bool(settings, S_VERTICAL, 
		obs_data_get_bool(text_ss_settings, S_VERTICAL));
	obs_data_set_bool(settings, S_OUTLINE, 
		obs_data_get_bool(text_ss_settings, S_OUTLINE));
	obs_data_set_uint32(settings, S_OUTLINE_COLOR, 
		obs_data_get_uint32(text_ss_settings, S_OUTLINE_COLOR));
	obs_data_set_uint32(settings, S_OUTLINE_OPACITY, 
		obs_data_get_uint32(text_ss_settings, S_OUTLINE_OPACITY));
	obs_data_set_uint32(settings, S_OUTLINE_SIZE, 
		obs_data_get_uint32(text_ss_settings, S_OUTLINE_SIZE));
	obs_data_set_bool(settings, S_USE_FILE, false);
	obs_data_set_string(settings, S_FILE, "");
	obs_data_set_bool(settings, S_CHATLOG_MODE, 
		obs_data_get_bool(text_ss_settings, S_CHATLOG_MODE));
	obs_data_set_int(settings, S_CHATLOG_LINES, 
		obs_data_get_int(text_ss_settings, S_CHATLOG_LINES));
	obs_data_set_bool(settings, S_EXTENTS, 
		obs_data_get_bool(text_ss_settings, S_EXTENTS));
	obs_data_set_bool(settings, S_EXTENTS_WRAP, 
		obs_data_get_bool(text_ss_settings, S_EXTENTS_WRAP));
	obs_data_set_uint32(settings, S_EXTENTS_CX, 
		obs_data_get_uint32(text_ss_settings, S_EXTENTS_CX));
	obs_data_set_uint32(settings, S_EXTENTS_CY, 
		obs_data_get_uint32(text_ss_settings, S_EXTENTS_CY));
	obs_data_set_int(settings, S_TRANSFORM, 
		obs_data_get_int(text_ss_settings, S_TRANSFORM));
	obs_data_set_bool(settings, S_ANTIALIASING, 
		obs_data_get_bool(text_ss_settings, S_ANTIALIASING));

	obs_data_set_uint32(settings, S_BKCOLOR, 
		obs_data_get_uint32(text_ss_settings, S_BKCOLOR));
	obs_data_set_uint32(settings, S_BKOPACITY, 
		obs_data_get_uint32(text_ss_settings, S_BKOPACITY));

	source = obs_source_create_private("text_gdiplus", NULL, settings);

	obs_data_release(curr_font);
	obs_data_release(settings);

	return source;
}

static void add_gdiplus_src(struct text_slideshow *text_ss, struct darray *array,
		     const char *text, uint32_t *cx, uint32_t *cy, 
			 obs_data_t *settings) {
	DARRAY(struct text_data) new_text_data;
	struct text_data data;
	obs_source_t *new_source;

	new_text_data.da = *array;

	pthread_mutex_lock(&text_ss->mutex);
	new_source = get_source(&text_ss->text_srcs.da, text);
	pthread_mutex_unlock(&text_ss->mutex);

	if (!new_source)
		new_source = get_source(&new_text_data.da, text);
	if (new_source)
		obs_source_update(new_source, settings);
	if (!new_source)
		new_source = create_gdiplus(text, settings);

	if (new_source) {
		uint32_t new_cx = obs_source_get_width(new_source);
		uint32_t new_cy = obs_source_get_height(new_source);

		data.text = bstrdup(text);
		data.source = new_source;
		da_push_back(new_text_data, &data);

		if (new_cx > *cx)
			*cx = new_cx;
		if (new_cy > *cy)
			*cy = new_cy;

		void *source_data = obs_obj_get_data(new_source);
	}

	*array = new_text_data.da;
}

static void gdiplus_update(void *data, obs_data_t *settings) {
	DARRAY(struct text_data) new_text_srcs;
	DARRAY(struct text_data) old_text_srcs;
	obs_source_t *new_tr = NULL;
	obs_source_t *old_tr = NULL;
	struct text_slideshow *text_ss = data;
	obs_data_array_t *array;
	const char *tr_name;
	uint32_t new_duration;
	uint32_t new_speed;
	uint32_t cx = 0;
	uint32_t cy = 0;
	size_t count;
	const char *behavior;
	const char *mode;

	/* ------------------------------------- */
	/* get settings data */

	da_init(new_text_srcs);

	behavior = obs_data_get_string(settings, S_BEHAVIOR);

	if (astrcmpi(behavior, S_BEHAVIOR_PAUSE_UNPAUSE) == 0)
		text_ss->behavior = BEHAVIOR_PAUSE_UNPAUSE;
	else if (astrcmpi(behavior, S_BEHAVIOR_ALWAYS_PLAY) == 0)
		text_ss->behavior = BEHAVIOR_ALWAYS_PLAY;
	else /* S_BEHAVIOR_STOP_RESTART */
		text_ss->behavior = BEHAVIOR_STOP_RESTART;

	mode = obs_data_get_string(settings, S_MODE);

	text_ss->manual = (astrcmpi(mode, S_MODE_MANUAL) == 0);

	tr_name = obs_data_get_string(settings, S_TRANSITION);
	if (astrcmpi(tr_name, TR_CUT) == 0)
		tr_name = "cut_transition";
	else if (astrcmpi(tr_name, TR_SWIPE) == 0)
		tr_name = "swipe_transition";
	else if (astrcmpi(tr_name, TR_SLIDE) == 0)
		tr_name = "slide_transition";
	else
		tr_name = "fade_transition";

	text_ss->randomize = obs_data_get_bool(settings, S_RANDOMIZE);
	text_ss->loop = obs_data_get_bool(settings, S_LOOP);
	text_ss->hide = obs_data_get_bool(settings, S_HIDE);

	if (!text_ss->tr_name || strcmp(tr_name, text_ss->tr_name) != 0)
		new_tr = obs_source_create_private(tr_name, NULL, NULL);

	new_duration = (uint32_t)obs_data_get_int(settings, S_SLIDE_TIME);
	new_speed = (uint32_t)obs_data_get_int(settings, S_TR_SPEED);

	array = obs_data_get_array(settings, S_TEXTS);
	count = obs_data_array_count(array);

	/* ------------------------------------- */
	/* create new list of sources */

	// image-slideshow recreates private sources every update
	// can also simply update existing source settings if this method is too 
	// slow
	for (size_t i = 0; i < count; i++) {
		obs_data_t *item = obs_data_array_item(array, i);
		const char *curr_text = obs_data_get_string(item, "value");
		add_gdiplus_src(text_ss, &new_text_srcs.da, curr_text, &cx, &cy, 
			settings);
		obs_data_release(item);
	}

	/* ------------------------------------- */
	/* update settings data */

	pthread_mutex_lock(&text_ss->mutex);

	old_text_srcs.da = text_ss->text_srcs.da;
	text_ss->text_srcs.da = new_text_srcs.da;
	if (new_tr) {
		old_tr = text_ss->transition;
		text_ss->transition = new_tr;
	}

	if (strcmp(tr_name, "cut_transition") != 0) {
		if (new_duration < 100)
			new_duration = 100;

		new_duration += new_speed;
	} else {
		if (new_duration < 50)
			new_duration = 50;
	}

	text_ss->tr_speed = new_speed;
	text_ss->tr_name = tr_name;
	text_ss->slide_time = (float)new_duration / 1000.0f;

	pthread_mutex_unlock(&text_ss->mutex);

	/* ------------------------------------- */
	/* clean up and restart transition */

	if (old_tr)
		obs_source_release(old_tr);
	free_text_srcs(&old_text_srcs.da);

	/* ------------------------- */

	const char *res_str = obs_data_get_string(settings, S_CUSTOM_SIZE);
	bool aspect_only = false, use_auto = true;
	int cx_in = 0, cy_in = 0;

	if (strcmp(res_str, T_CUSTOM_SIZE_AUTO) != 0) {
		int ret = sscanf(res_str, "%dx%d", &cx_in, &cy_in);
		if (ret == 2) {
			aspect_only = false;
			use_auto = false;
		} else {
			ret = sscanf(res_str, "%d:%d", &cx_in, &cy_in);
			if (ret == 2) {
				aspect_only = true;
				use_auto = false;
			}
		}
	}

	if (!use_auto) {
		double cx_f = (double)cx;
		double cy_f = (double)cy;

		double old_aspect = cx_f / cy_f;
		double new_aspect = (double)cx_in / (double)cy_in;

		if (aspect_only) {
			if (fabs(old_aspect - new_aspect) > EPSILON) {
				if (new_aspect > old_aspect)
					cx = (uint32_t)(cy_f * new_aspect);
				else
					cy = (uint32_t)(cx_f / new_aspect);
			}
		} else {
			cx = (uint32_t)cx_in;
			cy = (uint32_t)cy_in;
		}
	}

	/* ------------------------- */

	text_ss->cx = cx;
	text_ss->cy = cy;
	text_ss->cur_item = 0;
	text_ss->elapsed = 0.0f;
	obs_transition_set_size(text_ss->transition, cx, cy);
	obs_transition_set_alignment(text_ss->transition, OBS_ALIGN_CENTER);
	obs_transition_set_scale_type(text_ss->transition,
				      OBS_TRANSITION_SCALE_ASPECT);

	if (text_ss->randomize && text_ss->text_srcs.num)
		text_ss->cur_item = random_text_src(text_ss);
	if (new_tr)
		obs_source_add_active_child(text_ss->source, new_tr);
	if (text_ss->text_srcs.num) {
		do_transition(text_ss, false);

		if (text_ss->manual)
			set_media_state(text_ss, OBS_MEDIA_STATE_PAUSED);
		else
			set_media_state(text_ss, OBS_MEDIA_STATE_PLAYING);

		obs_source_media_started(text_ss->source);
	}

	obs_data_array_release(array);
}

static void text_defaults(obs_data_t *settings) {
	obs_data_t *font_obj = obs_data_create();
	obs_data_set_default_string(font_obj, "face", "Arial");

	// Only non-obsolete v2 atm
	obs_data_set_default_int(font_obj, "size", 256);

	obs_data_set_default_obj(settings, S_FONT, font_obj);
	obs_data_set_default_string(settings, S_ALIGN, S_ALIGN_LEFT);
	obs_data_set_default_string(settings, S_VALIGN, S_VALIGN_TOP);
	obs_data_set_default_int(settings, S_COLOR, 0xFFFFFF);
	obs_data_set_default_int(settings, S_OPACITY, 100);
	obs_data_set_default_int(settings, S_GRADIENT_COLOR, 0xFFFFFF);
	obs_data_set_default_int(settings, S_GRADIENT_OPACITY, 100);
	obs_data_set_default_double(settings, S_GRADIENT_DIR, 90.0);
	obs_data_set_default_int(settings, S_BKCOLOR, 0x000000);
	obs_data_set_default_int(settings, S_BKOPACITY, 0);
	obs_data_set_default_int(settings, S_OUTLINE_SIZE, 2);
	obs_data_set_default_int(settings, S_OUTLINE_COLOR, 0xFFFFFF);
	obs_data_set_default_int(settings, S_OUTLINE_OPACITY, 100);
	obs_data_set_default_int(settings, S_CHATLOG_LINES, 6);
	obs_data_set_default_bool(settings, S_EXTENTS_WRAP, true);
	obs_data_set_default_int(settings, S_EXTENTS_CX, 100);
	obs_data_set_default_int(settings, S_EXTENTS_CY, 100);
	obs_data_set_default_int(settings, S_TRANSFORM, S_TRANSFORM_NONE);
	obs_data_set_default_bool(settings, S_ANTIALIASING, true);

	obs_data_release(font_obj);
}

static void gdiplus_defaults(obs_data_t *settings) {
	ss_defaults(settings);
	text_defaults(settings);
}

#define set_vis(var, val, show)                           \
	do {                                              \
		p = obs_properties_get(props, val);       \
		obs_property_set_visible(p, var == show); \
	} while (false)

static bool use_file_changed(obs_properties_t *props, obs_property_t *p,
			     obs_data_t *s) {
	bool use_file = obs_data_get_bool(s, S_USE_FILE);

	set_vis(use_file, S_TEXT, false);
	set_vis(use_file, S_FILE, true);
	return true;
}

static bool outline_changed(obs_properties_t *props, obs_property_t *p,
			    obs_data_t *s) {
	bool outline = obs_data_get_bool(s, S_OUTLINE);

	set_vis(outline, S_OUTLINE_SIZE, true);
	set_vis(outline, S_OUTLINE_COLOR, true);
	set_vis(outline, S_OUTLINE_OPACITY, true);
	return true;
}

static bool chatlog_mode_changed(obs_properties_t *props, obs_property_t *p,
				 obs_data_t *s) {
	bool chatlog_mode = obs_data_get_bool(s, S_CHATLOG_MODE);

	set_vis(chatlog_mode, S_CHATLOG_LINES, true);
	return true;
}

static bool gradient_changed(obs_properties_t *props, obs_property_t *p,
			     obs_data_t *s) {
	bool gradient = obs_data_get_bool(s, S_GRADIENT);

	set_vis(gradient, S_GRADIENT_COLOR, true);
	set_vis(gradient, S_GRADIENT_OPACITY, true);
	set_vis(gradient, S_GRADIENT_DIR, true);
	return true;
}

static bool extents_modified(obs_properties_t *props, obs_property_t *p,
			     obs_data_t *s) {
	bool use_extents = obs_data_get_bool(s, S_EXTENTS);

	set_vis(use_extents, S_EXTENTS_WRAP, true);
	set_vis(use_extents, S_EXTENTS_CX, true);
	set_vis(use_extents, S_EXTENTS_CY, true);
	return true;
}

#undef set_vis

static void text_properties(obs_properties_t *props) {

	obs_property_t *p;

	obs_properties_add_font(props, S_FONT, T_FONT);

	obs_properties_add_bool(props, S_ANTIALIASING, T_ANTIALIASING);

	p = obs_properties_add_list(props, S_TRANSFORM, T_TRANSFORM,
				    OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(p, T_TRANSFORM_NONE, S_TRANSFORM_NONE);
	obs_property_list_add_int(p, T_TRANSFORM_UPPERCASE,
				  S_TRANSFORM_UPPERCASE);
	obs_property_list_add_int(p, T_TRANSFORM_LOWERCASE,
				  S_TRANSFORM_LOWERCASE);
	obs_property_list_add_int(p, T_TRANSFORM_STARTCASE,
				  S_TRANSFORM_STARTCASE);

	obs_properties_add_bool(props, S_VERTICAL, T_VERTICAL);

	obs_properties_add_color(props, S_COLOR, T_COLOR);
	p = obs_properties_add_int_slider(props, S_OPACITY, T_OPACITY, 0, 100,
					  1);
	obs_property_int_set_suffix(p, "%");

	p = obs_properties_add_bool(props, S_GRADIENT, T_GRADIENT);
	obs_property_set_modified_callback(p, gradient_changed);

	obs_properties_add_color(props, S_GRADIENT_COLOR, T_GRADIENT_COLOR);
	p = obs_properties_add_int_slider(props, S_GRADIENT_OPACITY,
					  T_GRADIENT_OPACITY, 0, 100, 1);
	obs_property_int_set_suffix(p, "%");
	obs_properties_add_float_slider(props, S_GRADIENT_DIR, T_GRADIENT_DIR,
					0, 360, 0.1);

	obs_properties_add_color(props, S_BKCOLOR, T_BKCOLOR);
	p = obs_properties_add_int_slider(props, S_BKOPACITY, T_BKOPACITY, 0,
					  100, 1);
	obs_property_int_set_suffix(p, "%");

	p = obs_properties_add_list(props, S_ALIGN, T_ALIGN,
				    OBS_COMBO_TYPE_LIST,
				    OBS_COMBO_FORMAT_STRING);
	obs_property_list_add_string(p, T_ALIGN_LEFT, S_ALIGN_LEFT);
	obs_property_list_add_string(p, T_ALIGN_CENTER, S_ALIGN_CENTER);
	obs_property_list_add_string(p, T_ALIGN_RIGHT, S_ALIGN_RIGHT);

	p = obs_properties_add_list(props, S_VALIGN, T_VALIGN,
				    OBS_COMBO_TYPE_LIST,
				    OBS_COMBO_FORMAT_STRING);
	obs_property_list_add_string(p, T_VALIGN_TOP, S_VALIGN_TOP);
	obs_property_list_add_string(p, T_VALIGN_CENTER, S_VALIGN_CENTER);
	obs_property_list_add_string(p, T_VALIGN_BOTTOM, S_VALIGN_BOTTOM);

	p = obs_properties_add_bool(props, S_OUTLINE, T_OUTLINE);
	obs_property_set_modified_callback(p, outline_changed);

	obs_properties_add_int(props, S_OUTLINE_SIZE, T_OUTLINE_SIZE, 1, 20, 1);
	obs_properties_add_color(props, S_OUTLINE_COLOR, T_OUTLINE_COLOR);
	p = obs_properties_add_int_slider(props, S_OUTLINE_OPACITY,
					  T_OUTLINE_OPACITY, 0, 100, 1);
	obs_property_int_set_suffix(p, "%");

	p = obs_properties_add_bool(props, S_CHATLOG_MODE, T_CHATLOG_MODE);
	obs_property_set_modified_callback(p, chatlog_mode_changed);

	obs_properties_add_int(props, S_CHATLOG_LINES, T_CHATLOG_LINES, 1, 1000,
			       1);

	p = obs_properties_add_bool(props, S_EXTENTS, T_EXTENTS);
	obs_property_set_modified_callback(p, extents_modified);

	obs_properties_add_int(props, S_EXTENTS_CX, T_EXTENTS_CX, 32, 8000, 1);
	obs_properties_add_int(props, S_EXTENTS_CY, T_EXTENTS_CY, 32, 8000, 1);
	obs_properties_add_bool(props, S_EXTENTS_WRAP, T_EXTENTS_WRAP);
}

static obs_properties_t *gdiplus_properties(void *data) {
	obs_properties_t *props = obs_properties_create();
	struct text_slideshow *text_ss = data;
	
	ss_properites(props);
	text_properties(props);

	return props;
}

struct obs_source_info text_gdiplus_slideshow_info = {
	.id = "text-gdiplus-slideshow",
	.type = OBS_SOURCE_TYPE_INPUT,
	.output_flags = 
			OBS_SOURCE_VIDEO | 
			OBS_SOURCE_CUSTOM_DRAW |
			OBS_SOURCE_COMPOSITE | 
			OBS_SOURCE_CONTROLLABLE_MEDIA,
	.get_name = gdiplus_getname,
	.create = text_ss_create,
	.destroy = text_ss_destroy,
	.update = gdiplus_update,
	.activate = text_ss_activate,
	.deactivate = text_ss_deactivate,
	.video_render = text_ss_video_render,
	.video_tick = text_ss_video_tick,
	.audio_render = text_ss_audio_render,
	.enum_active_sources = text_ss_enum_sources,
	.get_width = text_ss_width,
	.get_height = text_ss_height,
	.get_defaults = gdiplus_defaults,
	.get_properties = gdiplus_properties,
	.icon_type = OBS_ICON_TYPE_SLIDESHOW,
	.media_play_pause = text_ss_play_pause,
	.media_restart = text_ss_restart,
	.media_stop = text_ss_stop,
	.media_next = text_ss_next_slide,
	.media_previous = text_ss_previous_slide,
	.media_get_state = ss_get_state,
};