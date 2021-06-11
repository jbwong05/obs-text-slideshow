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

#define S_FONT                         "font"
#define S_TEXT                         "text"
#define S_FROM_FILE                    "from_file"
#define S_ANTIALIASING                 "antialiasing"
#define S_LOG_MODE                     "log_mode"
#define S_LOG_LINES                    "log_lines"
#define S_TEXT_FILE                    "text_file"
#define S_COLOR_1                      "color1"
#define S_COLOR_2                      "color2"
#define S_OUTLINE                      "outline"
#define S_DROP_SHADOW                  "drop_shadow"
#define S_CUSTOM_WIDTH                 "custom_width"
#define S_WORD_WRAP                    "word_wrap"
#define S_FACE                         "face"
#define S_SIZE                         "size"
#define S_FLAGS                        "flags"
#define S_STYLE                        "style"

#define T_TXT_(text) obs_module_text("Text." text)
#define T_FONT                         T_TXT_("Font")
#define T_TEXT                         T_TXT_("Text")
#define T_FROM_FILE                    T_TXT_("ReadFromFile")
#define T_ANTIALIASING                 T_TXT_("Antialiasing")
#define T_LOG_MODE                     T_TXT_("ChatLogMode")
#define T_LOG_LINES                    T_TXT_("ChatLogLines")
#define T_TEXT_FILE                    T_TXT_("TextFile")
#define T_TEXT_FILE_FILTER             T_TXT_("TextFileFilter")
#define T_COLOR_1                      T_TXT_("Color1")
#define T_COLOR_2                      T_TXT_("Color2")
#define T_OUTLINE                      T_TXT_("Outline")
#define T_DROP_SHADOW                  T_TXT_("DropShadow")
#define T_CUSTOM_WIDTH                 T_TXT_("CustomWidth")
#define T_WORD_WRAP                    T_TXT_("WordWrap")

#ifdef _WIN32
#define DEFAULT_FACE "Arial"
#elif __APPLE__
#define DEFAULT_FACE "Helvetica"
#else
#define DEFAULT_FACE "Sans Serif"
#endif

static const char *freetype2_getname(void *unused) {
    UNUSED_PARAMETER(unused);
    return obs_module_text("TextFreetype2Slideshow");
}

static obs_source_t *create_freetype2(const char *text, obs_data_t *text_ss_settings) {
	obs_data_t *settings = obs_data_create();
	obs_source_t *source;
	obs_data_t *curr_font = obs_data_get_obj(text_ss_settings, S_FONT);

	obs_data_set_obj(settings, S_FONT, curr_font);
	obs_data_set_bool(settings, S_DROP_SHADOW, 
		obs_data_get_bool(text_ss_settings, S_DROP_SHADOW));
	obs_data_set_bool(settings, S_OUTLINE, 
		obs_data_get_bool(text_ss_settings, S_OUTLINE));
	obs_data_set_bool(settings, S_WORD_WRAP, 
		obs_data_get_bool(text_ss_settings, S_WORD_WRAP));
	obs_data_set_int(settings, S_COLOR_1, 
		obs_data_get_int(text_ss_settings, S_COLOR_1));
	obs_data_set_int(settings, S_COLOR_2, 
		obs_data_get_int(text_ss_settings, S_COLOR_2));
	obs_data_set_int(settings, S_CUSTOM_WIDTH, 
		obs_data_get_int(text_ss_settings, S_CUSTOM_WIDTH));
	obs_data_set_bool(settings, S_FROM_FILE, false);
	obs_data_set_bool(settings, S_LOG_MODE, 
		obs_data_get_bool(text_ss_settings, S_LOG_MODE));
	obs_data_set_int(settings, S_LOG_LINES, 
		obs_data_get_int(text_ss_settings, S_LOG_LINES));
	obs_data_set_bool(settings, S_ANTIALIASING, 
		obs_data_get_bool(text_ss_settings, S_ANTIALIASING));
	obs_data_set_string(settings, S_TEXT_FILE, "");
	obs_data_set_string(settings, S_TEXT, text);
	source = obs_source_create_private("text_ft2_source", NULL, settings);

	obs_data_release(curr_font);
	obs_data_release(settings);

	return source;
}

static void add_freetype2_src(struct text_slideshow *text_ss, struct darray *array,
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
		new_source = create_freetype2(text, settings);

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

static void freetype2_update(void *data, obs_data_t *settings) {
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
		add_freetype2_src(text_ss, &new_text_srcs.da, curr_text, &cx, &cy, 
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

	// Currently only supporting text freetype2
	const uint16_t font_size = 256;

	obs_data_set_default_string(font_obj, S_FACE, DEFAULT_FACE);
	obs_data_set_default_int(font_obj, S_SIZE, font_size);
	obs_data_set_default_int(font_obj, S_FLAGS, 0);
	obs_data_set_default_string(font_obj, S_STYLE, "");
	obs_data_set_default_obj(settings, S_FONT, font_obj);

	obs_data_set_default_bool(settings, S_ANTIALIASING, true);
	obs_data_set_default_bool(settings, S_WORD_WRAP, false);
	obs_data_set_default_bool(settings, S_OUTLINE, false);
	obs_data_set_default_bool(settings, S_DROP_SHADOW, false);

	obs_data_set_default_int(settings, S_LOG_LINES, 6);

	obs_data_set_default_int(settings, S_COLOR_1, 0xFFFFFFFF);
	obs_data_set_default_int(settings, S_COLOR_2, 0xFFFFFFFF);

	obs_data_release(font_obj);
}

static void freetype2_defaults(obs_data_t *settings) {
	ss_defaults(settings);
	text_defaults(settings);
}

static void text_properties(obs_properties_t *props) {
	// TODO:
	//	Scrolling. Can't think of a way to do it with the render
	//		targets currently being broken. (0.4.2)
	//	Better/pixel shader outline/drop shadow
	//	Some way to pull text files from network, I dunno

	obs_properties_add_font(props, S_FONT, T_FONT);

	obs_properties_add_bool(props, S_ANTIALIASING,
				T_ANTIALIASING);

	obs_properties_add_bool(props, S_LOG_MODE,
				T_LOG_MODE);

	obs_properties_add_int(props, S_LOG_LINES,
			       T_LOG_LINES, 1, 1000, 1);

	obs_properties_add_color(props, S_COLOR_1, T_COLOR_1);

	obs_properties_add_color(props, S_COLOR_2, T_COLOR_2);

	obs_properties_add_bool(props, S_OUTLINE, T_OUTLINE);

	obs_properties_add_bool(props, S_DROP_SHADOW,
				T_DROP_SHADOW);

	obs_properties_add_int(props, S_CUSTOM_WIDTH,
			       T_CUSTOM_WIDTH, 0, 4096, 1);

	obs_properties_add_bool(props, S_WORD_WRAP,
				T_WORD_WRAP);
}

static obs_properties_t *freetype2_properties(void *data) {
	obs_properties_t *props = obs_properties_create();
	struct text_slideshow *text_ss = data;
	
	ss_properites(props);
	text_properties(props);

	return props;
}

struct obs_source_info text_freetype2_slideshow_info = {
	.id = "text-freetype2-slideshow",
	.type = OBS_SOURCE_TYPE_INPUT,
	.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW |
			OBS_SOURCE_COMPOSITE | 
#ifdef _WIN32
			OBS_SOURCE_DEPRECATED |
#endif
			OBS_SOURCE_CONTROLLABLE_MEDIA,
	.get_name = freetype2_getname,
	.create = text_ss_create,
	.destroy = text_ss_destroy,
	.update = freetype2_update,
	.activate = text_ss_activate,
	.deactivate = text_ss_deactivate,
	.video_render = text_ss_video_render,
	.video_tick = text_ss_video_tick,
	.audio_render = text_ss_audio_render,
	.enum_active_sources = text_ss_enum_sources,
	.get_width = text_ss_width,
	.get_height = text_ss_height,
	.get_defaults = freetype2_defaults,
	.get_properties = freetype2_properties,
	.icon_type = OBS_ICON_TYPE_SLIDESHOW,
	.media_play_pause = text_ss_play_pause,
	.media_restart = text_ss_restart,
	.media_stop = text_ss_stop,
	.media_next = text_ss_next_slide,
	.media_previous = text_ss_previous_slide,
	.media_get_state = ss_get_state,
};