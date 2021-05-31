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

static const char *text_ss_getname(void *unused) {
    UNUSED_PARAMETER(unused);
    return obs_module_text("TextSlideshow");
}



struct obs_source_info text_slideshow_info = {
	.id = "text-slideshow",
	.type = OBS_SOURCE_TYPE_INPUT,
	.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW |
			OBS_SOURCE_COMPOSITE | OBS_SOURCE_CONTROLLABLE_MEDIA,
	.get_name = text_ss_getname,
	/*.create = ss_create,
	.destroy = ss_destroy,
	.update = ss_update,
	.activate = ss_activate,
	.deactivate = ss_deactivate,
	.video_render = ss_video_render,
	.video_tick = ss_video_tick,
	.audio_render = ss_audio_render,
	.enum_active_sources = ss_enum_sources,
	.get_width = ss_width,
	.get_height = ss_height,
	.get_defaults = ss_defaults,
	.get_properties = ss_properties,
	.missing_files = ss_missingfiles,
	.icon_type = OBS_ICON_TYPE_SLIDESHOW,
	.media_play_pause = ss_play_pause,
	.media_restart = ss_restart,
	.media_stop = ss_stop,
	.media_next = ss_next_slide,
	.media_previous = ss_previous_slide,
	.media_get_state = ss_get_state,*/
};