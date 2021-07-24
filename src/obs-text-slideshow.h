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

#pragma once

#include "obs-module.h"
#include <util/threading.h>
#include <util/platform.h>
#include <util/darray.h>
#include <util/dstr.h>
#include <string>
#include <vector>

using std::string;
using std::vector;

#define S_TR_SPEED "transition_speed"
#define S_CUSTOM_SIZE "use_custom_size"
#define S_SLIDE_TIME "slide_time"
#define S_TRANSITION "transition"
#define S_RANDOMIZE "randomize"
#define S_LOOP "loop"
#define S_HIDE "hide"
#define S_TEXTS "texts"
#define S_BEHAVIOR "playback_behavior"
#define S_BEHAVIOR_STOP_RESTART "stop_restart"
#define S_BEHAVIOR_PAUSE_UNPAUSE "pause_unpause"
#define S_BEHAVIOR_ALWAYS_PLAY "always_play"
#define S_MODE "slide_mode"
#define S_MODE_AUTO "mode_auto"
#define S_MODE_MANUAL "mode_manual"

#define S_READ_SINGLE_FILE "read_single_file"
#define S_TXT_FILE "txt_file"
#define S_READ_MULTIPLE_FILES "read_multiple_files"
#define S_FILES "files"

#define TR_CUT "cut"
#define TR_FADE "fade"
#define TR_SWIPE "swipe"
#define TR_SLIDE "slide"

#define T_SS_(text) obs_module_text("SlideShow." text)
#define T_TR_SPEED T_SS_("TransitionSpeed")
#define T_CUSTOM_SIZE T_SS_("CustomSize")
#define T_CUSTOM_SIZE_AUTO T_SS_("CustomSize.Auto")
#define T_SLIDE_TIME T_SS_("SlideTime")
#define T_TRANSITION T_SS_("Transition")
#define T_RANDOMIZE T_SS_("Randomize")
#define T_LOOP T_SS_("Loop")
#define T_HIDE T_SS_("HideWhenDone")
#define T_TEXTS T_SS_("Texts")
#define T_BEHAVIOR T_SS_("PlaybackBehavior")
#define T_BEHAVIOR_STOP_RESTART T_SS_("PlaybackBehavior.StopRestart")
#define T_BEHAVIOR_PAUSE_UNPAUSE T_SS_("PlaybackBehavior.PauseUnpause")
#define T_BEHAVIOR_ALWAYS_PLAY T_SS_("PlaybackBehavior.AlwaysPlay")
#define T_MODE T_SS_("SlideMode")
#define T_MODE_AUTO T_SS_("SlideMode.Auto")
#define T_MODE_MANUAL T_SS_("SlideMode.Manual")

#define T_USE_SINGLE_FILE T_SS_("ReadFromSingleFile")
#define T_FILE T_SS_("TextFile")
#define T_FILTER_TEXT_FILES T_SS_("Filter.TextFiles")
#define T_FILTER_ALL_FILES T_SS_("Filter.AllFiles")
#define T_USE_MULTIPLE_FILE T_SS_("ReadFromMultipleFiles")
#define T_FILES T_SS_("Files")

#define T_TR_(text) obs_module_text("SlideShow.Transition." text)
#define T_TR_CUT T_TR_("Cut")
#define T_TR_FADE T_TR_("Fade")
#define T_TR_SWIPE T_TR_("Swipe")
#define T_TR_SLIDE T_TR_("Slide")

#define set_vis(val, show)                          \
	do {                                        \
		p = obs_properties_get(props, val); \
		obs_property_set_visible(p, show);  \
	} while (false)

struct text_data {
	char *file_path;
	char *text;
	obs_source_t *source;
};

enum behavior {
	BEHAVIOR_STOP_RESTART,
	BEHAVIOR_PAUSE_UNPAUSE,
	BEHAVIOR_ALWAYS_PLAY,
};

struct text_slideshow {
	obs_source_t *source;
	obs_data_t *settings;

	bool randomize;
	bool loop;
	bool restart_on_activate;
	bool pause_on_deactivate;
	bool restart;
	bool manual;
	bool hide;
	bool use_cut;
	bool paused;
	bool stop;
	float slide_time;
	uint32_t tr_speed;
	const char *tr_name;
	obs_source_t *transition;

	float elapsed;
	size_t cur_item;

	uint32_t cx;
	uint32_t cy;

	bool dock_can_get_texts;
	pthread_cond_t dock_get_texts;
	pthread_mutex_t mutex;
	DARRAY(struct text_data) text_srcs;

	bool read_from_single_file = false;
	string file;
	bool read_from_multiple_files = false;

	enum behavior behavior;

	obs_hotkey_id play_pause_hotkey;
	obs_hotkey_id restart_hotkey;
	obs_hotkey_id stop_hotkey;
	obs_hotkey_id next_hotkey;
	obs_hotkey_id prev_hotkey;

	enum obs_media_state state;
};

typedef obs_source_t *(*text_source_create)(const char *path, const char *text,
					    obs_data_t *text_ss_settings);
typedef void (*set_text_alignment)(obs_source_t *transition,
				   obs_data_t *text_ss_settings);

void play_pause_hotkey(void *data, obs_hotkey_id id, obs_hotkey_t *hotkey,
		       bool pressed);
void restart_hotkey(void *data, obs_hotkey_id id, obs_hotkey_t *hotkey,
		    bool pressed);
void stop_hotkey(void *data, obs_hotkey_id id, obs_hotkey_t *hotkey,
		 bool pressed);
void next_slide_hotkey(void *data, obs_hotkey_id id, obs_hotkey_t *hotkey,
		       bool pressed);
void previous_slide_hotkey(void *data, obs_hotkey_id id, obs_hotkey_t *hotkey,
			   bool pressed);
void text_ss_destroy(void *data);
void *text_ss_create(obs_data_t *settings, obs_source_t *source);
void text_ss_update(void *data, obs_data_t *settings,
		    text_source_create text_creator,
		    set_text_alignment set_alignment);
void text_ss_activate(void *data);
void text_ss_deactivate(void *data);
void text_ss_video_render(void *data, gs_effect_t *effect);
void text_ss_video_tick(void *data, float seconds);
bool text_ss_audio_render(void *data, uint64_t *ts_out,
			  struct obs_source_audio_mix *audio_output,
			  uint32_t mixers, size_t channels, size_t sample_rate);
void text_ss_enum_sources(void *data, obs_source_enum_proc_t cb, void *param);
void text_ss_enum_all_sources(void *data, obs_source_enum_proc_t callback,
			      void *param);
uint32_t text_ss_width(void *data);
uint32_t text_ss_height(void *data);
void ss_defaults(obs_data_t *settings);
void ss_properites(void *data, obs_properties_t *props);
void text_ss_play_pause(void *data, bool pause);
void text_ss_restart(void *data);
void text_ss_stop(void *data);
void text_ss_next_slide(void *data);
void text_ss_previous_slide(void *data);
enum obs_media_state text_ss_get_state(void *data);