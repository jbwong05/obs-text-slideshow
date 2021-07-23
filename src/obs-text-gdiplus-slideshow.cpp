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
#include <obs-frontend-api.h>
#include "obs-text-slideshow.h"
#include "files.h"

// text gdiplus
#define S_FONT "font"
#define S_USE_FILE "read_from_file"
#define S_FILE "file"
#define S_TEXT "text"
#define S_COLOR "color"
#define S_GRADIENT "gradient"
#define S_GRADIENT_COLOR "gradient_color"
#define S_GRADIENT_DIR "gradient_dir"
#define S_GRADIENT_OPACITY "gradient_opacity"
#define S_ALIGN "align"
#define S_VALIGN "valign"
#define S_OPACITY "opacity"
#define S_BKCOLOR "bk_color"
#define S_BKOPACITY "bk_opacity"
#define S_VERTICAL "vertical"
#define S_OUTLINE "outline"
#define S_OUTLINE_SIZE "outline_size"
#define S_OUTLINE_COLOR "outline_color"
#define S_OUTLINE_OPACITY "outline_opacity"
#define S_CHATLOG_MODE "chatlog"
#define S_CHATLOG_LINES "chatlog_lines"
#define S_EXTENTS "extents"
#define S_EXTENTS_WRAP "extents_wrap"
#define S_EXTENTS_CX "extents_cx"
#define S_EXTENTS_CY "extents_cy"
#define S_TRANSFORM "transform"
#define S_ANTIALIASING "antialiasing"

#define S_ALIGN_LEFT "left"
#define S_ALIGN_CENTER "center"
#define S_ALIGN_RIGHT "right"

#define S_VALIGN_TOP "top"
#define S_VALIGN_CENTER S_ALIGN_CENTER
#define S_VALIGN_BOTTOM "bottom"

#define S_TRANSFORM_NONE 0
#define S_TRANSFORM_UPPERCASE 1
#define S_TRANSFORM_LOWERCASE 2
#define S_TRANSFORM_STARTCASE 3

#define S_ANTIALIASING_NONE 0
#define S_ANTIALIASING_STANDARD 1

// text gdiplus
#define T_(v) obs_module_text("Text." v)
#define T_FONT T_("Font")
#define T_TEXT T_("Text")
#define T_COLOR T_("Color")
#define T_GRADIENT T_("Gradient")
#define T_GRADIENT_COLOR T_("Gradient.Color")
#define T_GRADIENT_DIR T_("Gradient.Direction")
#define T_GRADIENT_OPACITY T_("Gradient.Opacity")
#define T_ALIGN T_("Alignment")
#define T_VALIGN T_("VerticalAlignment")
#define T_OPACITY T_("Opacity")
#define T_BKCOLOR T_("BkColor")
#define T_BKOPACITY T_("BkOpacity")
#define T_VERTICAL T_("Vertical")
#define T_OUTLINE T_("Outline")
#define T_OUTLINE_SIZE T_("Outline.Size")
#define T_OUTLINE_COLOR T_("Outline.Color")
#define T_OUTLINE_OPACITY T_("Outline.Opacity")
#define T_CHATLOG_MODE T_("ChatlogMode")
#define T_CHATLOG_LINES T_("ChatlogMode.Lines")
#define T_EXTENTS T_("UseCustomExtents")
#define T_EXTENTS_WRAP T_("UseCustomExtents.Wrap")
#define T_EXTENTS_CX T_("Width")
#define T_EXTENTS_CY T_("Height")
#define T_TRANSFORM T_("Transform")
#define T_ANTIALIASING T_("Antialiasing")

#define T_ALIGN_LEFT T_("Alignment.Left")
#define T_ALIGN_CENTER T_("Alignment.Center")
#define T_ALIGN_RIGHT T_("Alignment.Right")

#define T_VALIGN_TOP T_("VerticalAlignment.Top")
#define T_VALIGN_CENTER T_ALIGN_CENTER
#define T_VALIGN_BOTTOM T_("VerticalAlignment.Bottom")

#define T_TRANSFORM_NONE T_("Transform.None")
#define T_TRANSFORM_UPPERCASE T_("Transform.Uppercase")
#define T_TRANSFORM_LOWERCASE T_("Transform.Lowercase")
#define T_TRANSFORM_STARTCASE T_("Transform.Startcase")

static const char *gdiplus_getname(void *unused)
{
	UNUSED_PARAMETER(unused);
	return obs_module_text("TextGdiplusSlideshow");
}

#define obs_data_get_uint32 (uint32_t) obs_data_get_int
#define obs_data_set_uint32 obs_data_set_int

static obs_source_t *create_gdiplus(const char *text,
				    obs_data_t *text_ss_settings)
{
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
			    obs_data_get_uint32(text_ss_settings,
						S_GRADIENT_COLOR));
	obs_data_set_uint32(settings, S_GRADIENT_OPACITY,
			    obs_data_get_uint32(text_ss_settings,
						S_GRADIENT_OPACITY));
	obs_data_set_double(settings, S_GRADIENT_DIR,
			    obs_data_get_double(text_ss_settings,
						S_GRADIENT_DIR));
	obs_data_set_bool(settings, S_VERTICAL,
			  obs_data_get_bool(text_ss_settings, S_VERTICAL));
	obs_data_set_bool(settings, S_OUTLINE,
			  obs_data_get_bool(text_ss_settings, S_OUTLINE));
	obs_data_set_uint32(settings, S_OUTLINE_COLOR,
			    obs_data_get_uint32(text_ss_settings,
						S_OUTLINE_COLOR));
	obs_data_set_uint32(settings, S_OUTLINE_OPACITY,
			    obs_data_get_uint32(text_ss_settings,
						S_OUTLINE_OPACITY));
	obs_data_set_uint32(settings, S_OUTLINE_SIZE,
			    obs_data_get_uint32(text_ss_settings,
						S_OUTLINE_SIZE));
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
			    obs_data_get_uint32(text_ss_settings,
						S_EXTENTS_CX));
	obs_data_set_uint32(settings, S_EXTENTS_CY,
			    obs_data_get_uint32(text_ss_settings,
						S_EXTENTS_CY));
	obs_data_set_int(settings, S_TRANSFORM,
			 obs_data_get_int(text_ss_settings, S_TRANSFORM));
	obs_data_set_bool(settings, S_ANTIALIASING,
			  obs_data_get_bool(text_ss_settings, S_ANTIALIASING));

	obs_data_set_uint32(settings, S_BKCOLOR,
			    obs_data_get_uint32(text_ss_settings, S_BKCOLOR));
	obs_data_set_uint32(settings, S_BKOPACITY,
			    obs_data_get_uint32(text_ss_settings, S_BKOPACITY));

	source = obs_source_create_private("text_gdiplus", text, settings);

	obs_data_release(curr_font);
	obs_data_release(settings);

	return source;
}

static void update_gdiplus_alignment(obs_source_t *transition,
				     obs_data_t *text_ss_settings)
{
	const char *align_str = obs_data_get_string(text_ss_settings, S_ALIGN);
	if (strcmp(align_str, S_ALIGN_CENTER) == 0) {
		obs_transition_set_alignment(transition, OBS_ALIGN_CENTER);
	} else if (strcmp(align_str, S_ALIGN_RIGHT) == 0) {
		obs_transition_set_alignment(transition, OBS_ALIGN_RIGHT);
	} else {
		obs_transition_set_alignment(transition, OBS_ALIGN_LEFT);
	}
}

static void gdiplus_update(void *data, obs_data_t *settings)
{
	text_ss_update(data, settings, create_gdiplus,
		       update_gdiplus_alignment);
}

static void text_defaults(obs_data_t *settings)
{
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

static void gdiplus_defaults(obs_data_t *settings)
{
	ss_defaults(settings);
	text_defaults(settings);
}

static bool outline_changed(obs_properties_t *props, obs_property_t *p,
			    obs_data_t *s)
{
	bool outline = obs_data_get_bool(s, S_OUTLINE);

	set_vis(outline, S_OUTLINE_SIZE, true);
	set_vis(outline, S_OUTLINE_COLOR, true);
	set_vis(outline, S_OUTLINE_OPACITY, true);
	return true;
}

static bool chatlog_mode_changed(obs_properties_t *props, obs_property_t *p,
				 obs_data_t *s)
{
	bool chatlog_mode = obs_data_get_bool(s, S_CHATLOG_MODE);

	set_vis(chatlog_mode, S_CHATLOG_LINES, true);
	return true;
}

static bool gradient_changed(obs_properties_t *props, obs_property_t *p,
			     obs_data_t *s)
{
	bool gradient = obs_data_get_bool(s, S_GRADIENT);

	set_vis(gradient, S_GRADIENT_COLOR, true);
	set_vis(gradient, S_GRADIENT_OPACITY, true);
	set_vis(gradient, S_GRADIENT_DIR, true);
	return true;
}

static bool extents_modified(obs_properties_t *props, obs_property_t *p,
			     obs_data_t *s)
{
	bool use_extents = obs_data_get_bool(s, S_EXTENTS);

	set_vis(use_extents, S_EXTENTS_WRAP, true);
	set_vis(use_extents, S_EXTENTS_CX, true);
	set_vis(use_extents, S_EXTENTS_CY, true);
	return true;
}

#undef set_vis

static void text_properties(obs_properties_t *props)
{

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

static obs_properties_t *gdiplus_properties(void *data)
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
	obs_data_set_string(settings, S_FILE, new_path);
	obs_source_update(source, settings);
	obs_data_release(settings);

	UNUSED_PARAMETER(data);
}

static obs_missing_files_t *gdiplus_missing_files(void *data)
{
	struct text_slideshow *text_ss = (struct text_slideshow *)data;
	obs_missing_files_t *files = obs_missing_files_create();

	obs_source_t *source = text_ss->source;
	obs_data_t *settings = obs_source_get_settings(source);

	bool read = obs_data_get_bool(settings, S_USE_FILE);
	const char *path = obs_data_get_string(settings, S_FILE);

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

static bool enum_callback(void *param, obs_source_t *source)
{
	const char *id = obs_source_get_id(source);

	if (strcmp(id, "text-gdiplus-slideshow") == 0) {
		obs_data_t *settings = obs_source_get_settings(source);
		obs_source_update(source, settings);
		obs_data_release(settings);
	}

	return true;
}

static void obs_frontend_event_wrapper(enum obs_frontend_event event, void *ptr)
{
	if (event == OBS_FRONTEND_EVENT_FINISHED_LOADING) {
		obs_enum_sources(enum_callback, NULL);
	}
}

void load_text_gdiplus_slideshow()
{
	obs_source_info info = {};
	info.id = "text-gdiplus-slideshow";
	info.type = OBS_SOURCE_TYPE_INPUT;
	info.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW |
			    OBS_SOURCE_COMPOSITE |
			    OBS_SOURCE_CONTROLLABLE_MEDIA;
	info.get_properties = gdiplus_properties;
	info.icon_type = OBS_ICON_TYPE_SLIDESHOW;
	info.get_name = gdiplus_getname;
	info.create = text_ss_create;
	info.destroy = text_ss_destroy;
	info.get_width = text_ss_width;
	info.get_height = text_ss_height;
	info.get_defaults = gdiplus_defaults;
	info.update = gdiplus_update;
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
	info.missing_files = gdiplus_missing_files;

	obs_register_source(&info);
	obs_frontend_add_event_callback(obs_frontend_event_wrapper, NULL);
}