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
#include "files.h"

#define S_FONT "font"
#define S_TEXT "text"
#define S_FROM_FILE "from_file"
#define S_ANTIALIASING "antialiasing"
#define S_LOG_MODE "log_mode"
#define S_LOG_LINES "log_lines"
#define S_TEXT_FILE "text_file"
#define S_COLOR_1 "color1"
#define S_COLOR_2 "color2"
#define S_OUTLINE "outline"
#define S_DROP_SHADOW "drop_shadow"
#define S_CUSTOM_WIDTH "custom_width"
#define S_WORD_WRAP "word_wrap"
#define S_FACE "face"
#define S_SIZE "size"
#define S_FLAGS "flags"
#define S_STYLE "style"

#define T_TXT_(text) obs_module_text("Text." text)
#define T_FONT T_TXT_("Font")
#define T_TEXT T_TXT_("Text")
#define T_FROM_FILE T_TXT_("ReadFromFile")
#define T_ANTIALIASING T_TXT_("Antialiasing")
#define T_LOG_MODE T_TXT_("ChatLogMode")
#define T_LOG_LINES T_TXT_("ChatLogLines")
#define T_TEXT_FILE T_TXT_("TextFile")
#define T_TEXT_FILE_FILTER T_TXT_("TextFileFilter")
#define T_COLOR_1 T_TXT_("Color1")
#define T_COLOR_2 T_TXT_("Color2")
#define T_OUTLINE T_TXT_("Outline")
#define T_DROP_SHADOW T_TXT_("DropShadow")
#define T_CUSTOM_WIDTH T_TXT_("CustomWidth")
#define T_WORD_WRAP T_TXT_("WordWrap")

#ifdef _WIN32
#define DEFAULT_FACE "Arial"
#elif __APPLE__
#define DEFAULT_FACE "Helvetica"
#else
#define DEFAULT_FACE "Sans Serif"
#endif

static const char *freetype2_getname(void *unused)
{
	UNUSED_PARAMETER(unused);
	return obs_module_text("TextFreetype2Slideshow");
}

static bool freetype2_get_chat_log_mode(obs_data_t *settings)
{
	return obs_data_get_bool(settings, S_LOG_MODE);
}

static obs_source_t *create_freetype2(const char *text,
				      obs_data_t *text_ss_settings)
{
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
	source = obs_source_create_private("text_ft2_source", text, settings);

	obs_data_release(curr_font);
	obs_data_release(settings);

	return source;
}

inline static void update_freetype2_alignment(obs_source_t *transition,
					      obs_data_t *text_ss_settings)
{
	obs_transition_set_alignment(transition, OBS_ALIGN_CENTER);
}

static void freetype2_update(void *data, obs_data_t *settings)
{
	text_ss_update(data, settings, freetype2_get_chat_log_mode,
		       create_freetype2, update_freetype2_alignment);
}

static void text_defaults(obs_data_t *settings)
{
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

static void freetype2_defaults(obs_data_t *settings)
{
	ss_defaults(settings);
	text_defaults(settings);
}

static void text_properties(obs_properties_t *props)
{
	// TODO:
	//	Scrolling. Can't think of a way to do it with the render
	//		targets currently being broken. (0.4.2)
	//	Better/pixel shader outline/drop shadow
	//	Some way to pull text files from network, I dunno

	obs_properties_add_font(props, S_FONT, T_FONT);

	obs_properties_add_bool(props, S_ANTIALIASING, T_ANTIALIASING);

	obs_properties_add_bool(props, S_LOG_MODE, T_LOG_MODE);

	obs_properties_add_int(props, S_LOG_LINES, T_LOG_LINES, 1, 1000, 1);

	obs_properties_add_color(props, S_COLOR_1, T_COLOR_1);

	obs_properties_add_color(props, S_COLOR_2, T_COLOR_2);

	obs_properties_add_bool(props, S_OUTLINE, T_OUTLINE);

	obs_properties_add_bool(props, S_DROP_SHADOW, T_DROP_SHADOW);

	obs_properties_add_int(props, S_CUSTOM_WIDTH, T_CUSTOM_WIDTH, 0, 4096,
			       1);

	obs_properties_add_bool(props, S_WORD_WRAP, T_WORD_WRAP);
}

static obs_properties_t *freetype2_properties(void *data)
{
	obs_properties_t *props = obs_properties_create();

	ss_properites(data, props);
	text_properties(props);

	return props;
}

static void missing_file_callback(void *src, const char *new_path, void *data)
{
	struct text_slideshow *text_ss = (struct text_slideshow *)src;

	obs_source_t *source = text_ss->source;
	obs_data_t *settings = obs_source_get_settings(source);
	obs_data_set_string(settings, S_TEXT_FILE, new_path);
	obs_source_update(source, settings);
	obs_data_release(settings);

	UNUSED_PARAMETER(data);
}

static obs_missing_files_t *freetype2_missing_files(void *data)
{
	struct text_slideshow *text_ss = (struct text_slideshow *)data;
	obs_missing_files_t *files = obs_missing_files_create();

	obs_source_t *source = text_ss->source;
	obs_data_t *settings = obs_source_get_settings(source);

	bool read = obs_data_get_bool(settings, S_FROM_FILE);
	const char *path = obs_data_get_string(settings, S_TEXT_FILE);

	if (read && strcmp(path, "") != 0) {
		if (!os_file_exists(path)) {
			obs_missing_file_t *file = obs_missing_file_create(
				path, missing_file_callback,
				OBS_MISSING_FILE_SOURCE, text_ss->source, NULL);

			obs_missing_files_add_file(files, file);
		}
	}

	obs_data_release(settings);

	return files;
}

void load_text_freetype2_slideshow()
{
	obs_source_info info = {};
	info.id = "text-freetype2-slideshow";
	info.type = OBS_SOURCE_TYPE_INPUT;
	info.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW |
			    OBS_SOURCE_COMPOSITE |
#ifdef _WIN32
			    OBS_SOURCE_DEPRECATED |
#endif
			    OBS_SOURCE_CONTROLLABLE_MEDIA;
	info.get_properties = freetype2_properties;
	info.icon_type = OBS_ICON_TYPE_SLIDESHOW;
	info.get_name = freetype2_getname;
	info.create = text_ss_create;
	info.destroy = text_ss_destroy;
	info.get_width = text_ss_width;
	info.get_height = text_ss_height;
	info.get_defaults = freetype2_defaults;
	info.update = freetype2_update;
	info.activate = text_ss_activate;
	info.deactivate = text_ss_deactivate;
	info.video_tick = text_ss_video_tick;
	info.video_render = text_ss_video_render;
	info.enum_active_sources = text_ss_enum_sources;
	info.audio_render = text_ss_audio_render;
	info.media_play_pause = text_ss_play_pause;
	info.media_restart = text_ss_restart;
	info.media_stop = text_ss_stop;
	info.media_next = text_ss_next_slide;
	info.media_previous = text_ss_previous_slide;
	info.media_get_state = text_ss_get_state;
	info.missing_files = freetype2_missing_files;

	obs_register_source(&info);
}