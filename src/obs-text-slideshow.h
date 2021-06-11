#include "obs-module.h"
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
#define S_TEXTS                        "texts"
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

#define T_SS_(text) obs_module_text("SlideShow." text)
#define T_TR_SPEED                     T_SS_("TransitionSpeed")
#define T_CUSTOM_SIZE                  T_SS_("CustomSize")
#define T_CUSTOM_SIZE_AUTO             T_SS_("CustomSize.Auto")
#define T_SLIDE_TIME                   T_SS_("SlideTime")
#define T_TRANSITION                   T_SS_("Transition")
#define T_RANDOMIZE                    T_SS_("Randomize")
#define T_LOOP                         T_SS_("Loop")
#define T_HIDE                         T_SS_("HideWhenDone")
#define T_TEXTS                        T_SS_("Texts")
#define T_BEHAVIOR                     T_SS_("PlaybackBehavior")
#define T_BEHAVIOR_STOP_RESTART        T_SS_("PlaybackBehavior.StopRestart")
#define T_BEHAVIOR_PAUSE_UNPAUSE       T_SS_("PlaybackBehavior.PauseUnpause")
#define T_BEHAVIOR_ALWAYS_PLAY         T_SS_("PlaybackBehavior.AlwaysPlay")
#define T_MODE                         T_SS_("SlideMode")
#define T_MODE_AUTO                    T_SS_("SlideMode.Auto")
#define T_MODE_MANUAL                  T_SS_("SlideMode.Manual")

#define T_TR_(text) obs_module_text("SlideShow.Transition." text)
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

void play_pause_hotkey(void *data, obs_hotkey_id id,
	obs_hotkey_t *hotkey, bool pressed);
void restart_hotkey(void *data, obs_hotkey_id id, obs_hotkey_t *hotkey,
	bool pressed);
void stop_hotkey(void *data, obs_hotkey_id id, obs_hotkey_t *hotkey,
    bool pressed);
void next_slide_hotkey(void *data, obs_hotkey_id id,
	obs_hotkey_t *hotkey, bool pressed);
void previous_slide_hotkey(void *data, obs_hotkey_id id,
	obs_hotkey_t *hotkey, bool pressed);
void free_text_srcs(struct darray *array);
void text_ss_destroy(void *data);
void *text_ss_create(obs_data_t *settings, obs_source_t *source);
void set_media_state(void *data, enum obs_media_state state);
void do_transition(void *data, bool to_null);
obs_source_t *get_source(struct darray *array, const char *text);
void free_text_src(struct darray *array);
size_t random_text_src(struct text_slideshow *text_ss);
void text_ss_activate(void *data);
void text_ss_deactivate(void *data);
obs_source_t *get_transition(struct text_slideshow *text_ss);
void text_ss_video_render(void *data, gs_effect_t *effect);
void text_ss_video_tick(void *data, float seconds);
bool text_ss_audio_render(void *data, uint64_t *ts_out,
	struct obs_source_audio_mix *audio_output, uint32_t mixers, 
    size_t channels, size_t sample_rate);
void text_ss_enum_sources(void *data, 
	obs_source_enum_proc_t cb, void *param);
uint32_t text_ss_width(void *data);
uint32_t text_ss_height(void *data);
void ss_defaults(obs_data_t *settings);
void ss_properites(obs_properties_t *props);
void text_ss_play_pause(void *data, bool pause);
void text_ss_restart(void *data);
void text_ss_stop(void *data);
void text_ss_next_slide(void *data);
void text_ss_previous_slide(void *data);
enum obs_media_state ss_get_state(void *data);