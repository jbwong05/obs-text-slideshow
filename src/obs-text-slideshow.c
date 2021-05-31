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
#include <util/threading.h>
#include <util/platform.h>
#include <util/darray.h>
#include <util/dstr.h>

#define S_TR_SPEED                     "transition_speed"
#define S_CUSTOM_SIZE                  "use_custom_size"
#define S_SLIDE_TIME                   "slide_time"
#define S_TRANSITION                   "transition"
#define S_RANDOMIZE                    "randomize"
#define S_LOOP                         "loop"
#define S_HIDE                         "hide"
#define S_FILES                        "files"
#define S_BEHAVIOR                     "playback_behavior"
#define S_BEHAVIOR_STOP_RESTART        "stop_restart"
#define S_BEHAVIOR_PAUSE_UNPAUSE       "pause_unpause"
#define S_BEHAVIOR_ALWAYS_PLAY         "always_play"
#define S_MODE                         "slide_mode"
#define S_MODE_AUTO                    "mode_auto"
#define S_MODE_MANUAL                  "mode_manual"

#define TR_CUT                         "cut"
#define TR_FADE                        "fade"
#define TR_SWIPE                       "swipe"
#define TR_SLIDE                       "slide"

#define T_(text) obs_module_text("TextSlideShow." text)
#define T_TR_SPEED                     T_("TransitionSpeed")
#define T_CUSTOM_SIZE                  T_("CustomSize")
#define T_CUSTOM_SIZE_AUTO             T_("CustomSize.Auto")
#define T_SLIDE_TIME                   T_("SlideTime")
#define T_TRANSITION                   T_("Transition")
#define T_RANDOMIZE                    T_("Randomize")
#define T_LOOP                         T_("Loop")
#define T_HIDE                         T_("HideWhenDone")
#define T_FILES                        T_("Files")
#define T_BEHAVIOR                     T_("PlaybackBehavior")
#define T_BEHAVIOR_STOP_RESTART        T_("PlaybackBehavior.StopRestart")
#define T_BEHAVIOR_PAUSE_UNPAUSE       T_("PlaybackBehavior.PauseUnpause")
#define T_BEHAVIOR_ALWAYS_PLAY         T_("PlaybackBehavior.AlwaysPlay")
#define T_MODE                         T_("SlideMode")
#define T_MODE_AUTO                    T_("SlideMode.Auto")
#define T_MODE_MANUAL                  T_("SlideMode.Manual")

#define T_TR_(text) obs_module_text("TextSlideShow.Transition." text)
#define T_TR_CUT                       T_TR_("Cut")
#define T_TR_FADE                      T_TR_("Fade")
#define T_TR_SWIPE                     T_TR_("Swipe")
#define T_TR_SLIDE                     T_TR_("Slide")

struct text_data {
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

	pthread_mutex_t mutex;
	DARRAY(struct text_data) text_srcs;

	enum behavior behavior;

	obs_hotkey_id play_pause_hotkey;
	obs_hotkey_id restart_hotkey;
	obs_hotkey_id stop_hotkey;
	obs_hotkey_id next_hotkey;
	obs_hotkey_id prev_hotkey;

	enum obs_media_state state;
};

static const char *text_ss_getname(void *unused) {
    UNUSED_PARAMETER(unused);
    return obs_module_text("TextSlideshow");
}

static void play_pause_hotkey(void *data, obs_hotkey_id id,
			      obs_hotkey_t *hotkey, bool pressed) {
	UNUSED_PARAMETER(id);
	UNUSED_PARAMETER(hotkey);

	struct text_slideshow *text_ss = data;

	if (pressed && obs_source_showing(text_ss->source))
		obs_source_media_play_pause(text_ss->source, !text_ss->paused);
}

static void restart_hotkey(void *data, obs_hotkey_id id, obs_hotkey_t *hotkey,
			   bool pressed) {
	UNUSED_PARAMETER(id);
	UNUSED_PARAMETER(hotkey);

	struct text_slideshow *text_ss = data;

	if (pressed && obs_source_showing(text_ss->source))
		obs_source_media_restart(text_ss->source);
}

static void stop_hotkey(void *data, obs_hotkey_id id, obs_hotkey_t *hotkey,
			bool pressed)
{
	UNUSED_PARAMETER(id);
	UNUSED_PARAMETER(hotkey);

	struct text_slideshow *text_ss = data;

	if (pressed && obs_source_showing(text_ss->source))
		obs_source_media_stop(text_ss->source);
}

static void next_slide_hotkey(void *data, obs_hotkey_id id,
			      obs_hotkey_t *hotkey, bool pressed) {
	UNUSED_PARAMETER(id);
	UNUSED_PARAMETER(hotkey);

	struct text_slideshow *text_ss = data;

	if (!text_ss->manual)
		return;

	if (pressed && obs_source_showing(text_ss->source))
		obs_source_media_next(text_ss->source);
}

static void previous_slide_hotkey(void *data, obs_hotkey_id id,
				  obs_hotkey_t *hotkey, bool pressed) {
	UNUSED_PARAMETER(id);
	UNUSED_PARAMETER(hotkey);

	struct text_slideshow *text_ss = data;

	if (!text_ss->manual)
		return;

	if (pressed && obs_source_showing(text_ss->source))
		obs_source_media_previous(text_ss->source);
}

static void free_text_srcs(struct darray *array) {
	DARRAY(struct text_data) text_srcs;
	text_srcs.da = *array;

	for (size_t i = 0; i < text_srcs.num; i++) {
		bfree(text_srcs.array[i].text);
		obs_source_release(text_srcs.array[i].source);
	}

	da_free(text_srcs);
}

static void text_ss_destroy(void *data) {
	struct text_slideshow *text_ss = data;

	obs_source_release(text_ss->transition);
	free_text_srcs(&text_ss->text_srcs.da);
	pthread_mutex_destroy(&text_ss->mutex);
	bfree(text_ss);
}

static void *text_ss_create(obs_data_t *settings, obs_source_t *source) {
	struct text_slideshow *text_ss = bzalloc(sizeof(*text_ss));

	text_ss->source = source;

	text_ss->manual = false;
	text_ss->paused = false;
	text_ss->stop = false;

	text_ss->play_pause_hotkey = obs_hotkey_register_source(
		source, "TextSlideShow.PlayPause",
		obs_module_text("TextSlideShow.PlayPause"), play_pause_hotkey, text_ss);

	text_ss->restart_hotkey = obs_hotkey_register_source(
		source, "TextSlideShow.Restart",
		obs_module_text("TextSlideShow.Restart"), restart_hotkey, text_ss);

	text_ss->stop_hotkey = obs_hotkey_register_source(
		source, "TextSlideShow.Stop", obs_module_text("TextSlideShow.Stop"),
		stop_hotkey, text_ss);

	text_ss->prev_hotkey = obs_hotkey_register_source(
		source, "TextSlideShow.NextSlide",
		obs_module_text("TextSlideShow.NextSlide"), next_slide_hotkey, text_ss);

	text_ss->prev_hotkey = obs_hotkey_register_source(
		source, "TextSlideShow.PreviousSlide",
		obs_module_text("TextSlideShow.PreviousSlide"),
		previous_slide_hotkey, text_ss);

	pthread_mutex_init_value(&text_ss->mutex);
	if (pthread_mutex_init(&text_ss->mutex, NULL) != 0) {
		text_ss_destroy(text_ss);
		return NULL;
	}

	obs_source_update(source, NULL);

	UNUSED_PARAMETER(settings);
	return text_ss;
}

struct obs_source_info text_slideshow_info = {
	.id = "text-slideshow",
	.type = OBS_SOURCE_TYPE_INPUT,
	.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW |
			OBS_SOURCE_COMPOSITE | OBS_SOURCE_CONTROLLABLE_MEDIA,
	.get_name = text_ss_getname,
	.create = text_ss_create,
	.destroy = text_ss_destroy,
	/*.update = ss_update,
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