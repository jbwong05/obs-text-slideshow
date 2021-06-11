#include "obs-text-slideshow.h"

void play_pause_hotkey(void *data, obs_hotkey_id id,
			      obs_hotkey_t *hotkey, bool pressed) {
	UNUSED_PARAMETER(id);
	UNUSED_PARAMETER(hotkey);

	struct text_slideshow *text_ss = data;

	if (pressed && obs_source_showing(text_ss->source))
		obs_source_media_play_pause(text_ss->source, !text_ss->paused);
}

void restart_hotkey(void *data, obs_hotkey_id id, obs_hotkey_t *hotkey,
			   bool pressed) {
	UNUSED_PARAMETER(id);
	UNUSED_PARAMETER(hotkey);

	struct text_slideshow *text_ss = data;

	if (pressed && obs_source_showing(text_ss->source))
		obs_source_media_restart(text_ss->source);
}

void stop_hotkey(void *data, obs_hotkey_id id, obs_hotkey_t *hotkey,
			bool pressed) {
	UNUSED_PARAMETER(id);
	UNUSED_PARAMETER(hotkey);

	struct text_slideshow *text_ss = data;

	if (pressed && obs_source_showing(text_ss->source))
		obs_source_media_stop(text_ss->source);
}

void next_slide_hotkey(void *data, obs_hotkey_id id,
			      obs_hotkey_t *hotkey, bool pressed) {
	UNUSED_PARAMETER(id);
	UNUSED_PARAMETER(hotkey);

	struct text_slideshow *text_ss = data;

	if (!text_ss->manual)
		return;

	if (pressed && obs_source_showing(text_ss->source))
		obs_source_media_next(text_ss->source);
}

void previous_slide_hotkey(void *data, obs_hotkey_id id,
				  obs_hotkey_t *hotkey, bool pressed) {
	UNUSED_PARAMETER(id);
	UNUSED_PARAMETER(hotkey);

	struct text_slideshow *text_ss = data;

	if (!text_ss->manual)
		return;

	if (pressed && obs_source_showing(text_ss->source))
		obs_source_media_previous(text_ss->source);
}

void free_text_srcs(struct darray *array) {
	DARRAY(struct text_data) text_srcs;
	text_srcs.da = *array;

	for (size_t i = 0; i < text_srcs.num; i++) {
		bfree(text_srcs.array[i].text);
		obs_source_release(text_srcs.array[i].source);
	}

	da_free(text_srcs);
}

void text_ss_destroy(void *data) {
	struct text_slideshow *text_ss = data;

	obs_source_release(text_ss->transition);
	free_text_srcs(&text_ss->text_srcs.da);
	pthread_mutex_destroy(&text_ss->mutex);
	bfree(text_ss);
}

void *text_ss_create(obs_data_t *settings, obs_source_t *source) {
	struct text_slideshow *text_ss = bzalloc(sizeof(*text_ss));

	text_ss->source = source;

	text_ss->manual = false;
	text_ss->paused = false;
	text_ss->stop = false;

	text_ss->play_pause_hotkey = obs_hotkey_register_source(
		source, "SlideShow.PlayPause",
		obs_module_text("SlideShow.PlayPause"), play_pause_hotkey, text_ss);

	text_ss->restart_hotkey = obs_hotkey_register_source(
		source, "SlideShow.Restart",
		obs_module_text("SlideShow.Restart"), restart_hotkey, text_ss);

	text_ss->stop_hotkey = obs_hotkey_register_source(
		source, "SlideShow.Stop", obs_module_text("SlideShow.Stop"),
		stop_hotkey, text_ss);

	text_ss->prev_hotkey = obs_hotkey_register_source(
		source, "SlideShow.NextSlide",
		obs_module_text("SlideShow.NextSlide"), next_slide_hotkey, text_ss);

	text_ss->prev_hotkey = obs_hotkey_register_source(
		source, "SlideShow.PreviousSlide",
		obs_module_text("SlideShow.PreviousSlide"),
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

static inline bool item_valid(struct text_slideshow *text_ss) {
	return text_ss->text_srcs.num && 
		text_ss->cur_item < text_ss->text_srcs.num;
}

void set_media_state(void *data, enum obs_media_state state) {
	struct text_slideshow *text_ss = data;
	text_ss->state = state;
}

void do_transition(void *data, bool to_null) {
	struct text_slideshow *text_ss = data;
	bool valid = item_valid(text_ss);

	if (valid && text_ss->use_cut) {
		obs_transition_set(text_ss->transition,
				   text_ss->text_srcs.array[text_ss->cur_item].source);

	} else if (valid && !to_null) {
		obs_transition_start(text_ss->transition, OBS_TRANSITION_MODE_AUTO,
				     text_ss->tr_speed,
				     text_ss->text_srcs.array[text_ss->cur_item].source);

	} else {
		obs_transition_start(text_ss->transition, OBS_TRANSITION_MODE_AUTO,
				     text_ss->tr_speed, NULL);
		set_media_state(text_ss, OBS_MEDIA_STATE_ENDED);
		obs_source_media_ended(text_ss->source);
	}
}

obs_source_t *get_source(struct darray *array, const char *text) {
	DARRAY(struct text_data) text_srcs;
	obs_source_t *source = NULL;

	text_srcs.da = *array;

	for (size_t i = 0; i < text_srcs.num; i++) {
		const char *curr_text = text_srcs.array[i].text;

		if (strcmp(text, curr_text) == 0) {
			source = text_srcs.array[i].source;
			obs_source_addref(source);
			break;
		}
	}

	return source;
}

void free_text_src(struct darray *array) {
	DARRAY(struct text_data) text_srcs;
	text_srcs.da = *array;

	for (size_t i = 0; i < text_srcs.num; i++) {
		bfree(text_srcs.array[i].text);
		obs_source_release(text_srcs.array[i].source);
	}

	da_free(text_srcs);
}

size_t random_text_src(struct text_slideshow *text_ss) {
	return (size_t)rand() % text_ss->text_srcs.num;
}

void text_ss_activate(void *data) {
	struct text_slideshow *text_ss = data;

	if (text_ss->behavior == BEHAVIOR_STOP_RESTART) {
		text_ss->restart_on_activate = true;
		text_ss->use_cut = true;
	} else if (text_ss->behavior == BEHAVIOR_PAUSE_UNPAUSE) {
		text_ss->pause_on_deactivate = false;
	}
}

void text_ss_deactivate(void *data) {
	struct text_slideshow *text_ss = data;

	if (text_ss->behavior == BEHAVIOR_PAUSE_UNPAUSE)
		text_ss->pause_on_deactivate = true;
}

obs_source_t *get_transition(struct text_slideshow *text_ss) {
	obs_source_t *tr;

	pthread_mutex_lock(&text_ss->mutex);
	tr = text_ss->transition;
	obs_source_addref(tr);
	pthread_mutex_unlock(&text_ss->mutex);

	return tr;
}

void text_ss_video_render(void *data, gs_effect_t *effect) {
	struct text_slideshow *text_ss = data;
	obs_source_t *transition = get_transition(text_ss);

	if (transition) {
		obs_source_video_render(transition);
		obs_source_release(transition);
	}

	UNUSED_PARAMETER(effect);
}

void text_ss_video_tick(void *data, float seconds) {
	struct text_slideshow *text_ss = data;

	if (!text_ss->transition || !text_ss->slide_time)
		return;

	if (text_ss->restart_on_activate && text_ss->use_cut) {
		text_ss->elapsed = 0.0f;
		text_ss->cur_item = text_ss->randomize ? random_text_src(text_ss) : 0;
		do_transition(text_ss, false);
		text_ss->restart_on_activate = false;
		text_ss->use_cut = false;
		text_ss->stop = false;
		return;
	}

	if (text_ss->pause_on_deactivate || text_ss->manual || text_ss->stop || 
			text_ss->paused)
		return;

	/* ----------------------------------------------------- */
	/* fade to transparency when the file list becomes empty */
	if (!text_ss->text_srcs.num) {
		obs_source_t *active_transition_source =
			obs_transition_get_active_source(text_ss->transition);

		if (active_transition_source) {
			obs_source_release(active_transition_source);
			do_transition(text_ss, true);
		}
	}

	/* ----------------------------------------------------- */
	/* do transition when slide time reached                 */
	text_ss->elapsed += seconds;

	if (text_ss->elapsed > text_ss->slide_time) {
		text_ss->elapsed -= text_ss->slide_time;

		if (!text_ss->loop && 
				text_ss->cur_item == text_ss->text_srcs.num - 1) {
			if (text_ss->hide)
				do_transition(text_ss, true);
			else
				do_transition(text_ss, false);

			return;
		}

		if (text_ss->randomize) {
			size_t next = text_ss->cur_item;
			if (text_ss->text_srcs.num > 1) {
				while (next == text_ss->cur_item)
					next = random_text_src(text_ss);
			}
			text_ss->cur_item = next;

		} else if (++text_ss->cur_item >= text_ss->text_srcs.num) {
			text_ss->cur_item = 0;
		}

		if (text_ss->text_srcs.num)
			do_transition(text_ss, false);
	}
}

static inline bool text_ss_audio_render_(obs_source_t *transition, 
					uint64_t *ts_out,
				    struct obs_source_audio_mix *audio_output,
				    uint32_t mixers, size_t channels,
				    size_t sample_rate) {
	struct obs_source_audio_mix child_audio;
	uint64_t source_ts;

	if (obs_source_audio_pending(transition))
		return false;

	source_ts = obs_source_get_audio_timestamp(transition);
	if (!source_ts)
		return false;

	obs_source_get_audio_mix(transition, &child_audio);
	for (size_t mix = 0; mix < MAX_AUDIO_MIXES; mix++) {
		if ((mixers & (1 << mix)) == 0)
			continue;

		for (size_t ch = 0; ch < channels; ch++) {
			float *out = audio_output->output[mix].data[ch];
			float *in = child_audio.output[mix].data[ch];

			memcpy(out, in,
			       AUDIO_OUTPUT_FRAMES * MAX_AUDIO_CHANNELS *
				       sizeof(float));
		}
	}

	*ts_out = source_ts;

	UNUSED_PARAMETER(sample_rate);
	return true;
}

bool text_ss_audio_render(void *data, uint64_t *ts_out,
			    struct obs_source_audio_mix *audio_output,
			    uint32_t mixers, size_t channels,
			    size_t sample_rate) {
	struct text_slideshow *text_ss = data;
	obs_source_t *transition = get_transition(text_ss);
	bool success;

	if (!transition)
		return false;

	success = text_ss_audio_render_(transition, ts_out, audio_output, mixers,
				   channels, sample_rate);

	obs_source_release(transition);
	return success;
}

void text_ss_enum_sources(void *data, 
		obs_source_enum_proc_t cb, void *param) {
	struct text_slideshow *text_ss = data;

	pthread_mutex_lock(&text_ss->mutex);
	if (text_ss->transition)
		cb(text_ss->source, text_ss->transition, param);
	pthread_mutex_unlock(&text_ss->mutex);
}

uint32_t text_ss_width(void *data) {
	struct text_slideshow *text_ss = data;
	return text_ss->transition ? text_ss->cx : 0;
}

uint32_t text_ss_height(void *data) {
	struct text_slideshow *text_ss = data;
	return text_ss->transition ? text_ss->cy : 0;
}

void ss_defaults(obs_data_t *settings) {
	obs_data_set_default_string(settings, S_TRANSITION, "fade");
	obs_data_set_default_int(settings, S_SLIDE_TIME, 8000);
	obs_data_set_default_int(settings, S_TR_SPEED, 700);
	obs_data_set_default_string(settings, S_CUSTOM_SIZE,
				    T_CUSTOM_SIZE_AUTO);
	obs_data_set_default_string(settings, S_BEHAVIOR,
				    S_BEHAVIOR_ALWAYS_PLAY);
	obs_data_set_default_string(settings, S_MODE, S_MODE_AUTO);
	obs_data_set_default_bool(settings, S_LOOP, true);
}

static const char *aspects[] = {"16:9", "16:10", "4:3", "1:1"};

#define NUM_ASPECTS (sizeof(aspects) / sizeof(const char *))

void ss_properites(obs_properties_t *props) {
	struct obs_video_info ovi;
	obs_property_t *p;
	int cx;
	int cy;

	/* ----------------- */

	obs_get_video_info(&ovi);
	cx = (int)ovi.base_width;
	cy = (int)ovi.base_height;

	/* ----------------- */

	obs_properties_add_editable_list(props, S_TEXTS, T_TEXTS,
					 OBS_EDITABLE_LIST_TYPE_STRINGS,
					 NULL, NULL);

	p = obs_properties_add_list(props, S_BEHAVIOR, T_BEHAVIOR,
				    OBS_COMBO_TYPE_LIST,
				    OBS_COMBO_FORMAT_STRING);
	obs_property_list_add_string(p, T_BEHAVIOR_ALWAYS_PLAY,
				     S_BEHAVIOR_ALWAYS_PLAY);
	obs_property_list_add_string(p, T_BEHAVIOR_STOP_RESTART,
				     S_BEHAVIOR_STOP_RESTART);
	obs_property_list_add_string(p, T_BEHAVIOR_PAUSE_UNPAUSE,
				     S_BEHAVIOR_PAUSE_UNPAUSE);

	p = obs_properties_add_list(props, S_MODE, T_MODE, OBS_COMBO_TYPE_LIST,
				    OBS_COMBO_FORMAT_STRING);
	obs_property_list_add_string(p, T_MODE_AUTO, S_MODE_AUTO);
	obs_property_list_add_string(p, T_MODE_MANUAL, S_MODE_MANUAL);

	p = obs_properties_add_list(props, S_TRANSITION, T_TRANSITION,
				    OBS_COMBO_TYPE_LIST,
				    OBS_COMBO_FORMAT_STRING);
	obs_property_list_add_string(p, T_TR_CUT, TR_CUT);
	obs_property_list_add_string(p, T_TR_FADE, TR_FADE);
	obs_property_list_add_string(p, T_TR_SWIPE, TR_SWIPE);
	obs_property_list_add_string(p, T_TR_SLIDE, TR_SLIDE);

	obs_properties_add_int(props, S_SLIDE_TIME, T_SLIDE_TIME, 50, 3600000,
			       50);
	obs_properties_add_int(props, S_TR_SPEED, T_TR_SPEED, 0, 3600000, 50);
	obs_properties_add_bool(props, S_LOOP, T_LOOP);
	obs_properties_add_bool(props, S_HIDE, T_HIDE);
	obs_properties_add_bool(props, S_RANDOMIZE, T_RANDOMIZE);

	p = obs_properties_add_list(props, S_CUSTOM_SIZE, T_CUSTOM_SIZE,
				    OBS_COMBO_TYPE_EDITABLE,
				    OBS_COMBO_FORMAT_STRING);

	obs_property_list_add_string(p, T_CUSTOM_SIZE_AUTO, T_CUSTOM_SIZE_AUTO);

	for (size_t i = 0; i < NUM_ASPECTS; i++)
		obs_property_list_add_string(p, aspects[i], aspects[i]);

	char str[32];
	snprintf(str, 32, "%dx%d", cx, cy);
	obs_property_list_add_string(p, str, str);
}

void text_ss_play_pause(void *data, bool pause) {
	struct text_slideshow *text_ss = data;

	if (text_ss->stop) {
		text_ss->stop = false;
		text_ss->paused = false;
		do_transition(text_ss, false);
	} else {
		text_ss->paused = pause;
		text_ss->manual = pause;
	}

	if (pause)
		set_media_state(text_ss, OBS_MEDIA_STATE_PAUSED);
	else
		set_media_state(text_ss, OBS_MEDIA_STATE_PLAYING);
}

void text_ss_restart(void *data) {
	struct text_slideshow *text_ss = data;

	text_ss->elapsed = 0.0f;
	text_ss->cur_item = 0;
	text_ss->stop = false;
	text_ss->paused = false;
	do_transition(text_ss, false);

	set_media_state(text_ss, OBS_MEDIA_STATE_PLAYING);
}

void text_ss_stop(void *data) {
	struct text_slideshow *text_ss = data;

	text_ss->elapsed = 0.0f;
	text_ss->cur_item = 0;

	do_transition(text_ss, true);
	text_ss->stop = true;
	text_ss->paused = false;

	set_media_state(text_ss, OBS_MEDIA_STATE_STOPPED);
}

void text_ss_next_slide(void *data) {
	struct text_slideshow *text_ss = data;

	if (!text_ss->text_srcs.num || 
			obs_transition_get_time(text_ss->transition) < 1.0f)
		return;

	if (++text_ss->cur_item >= text_ss->text_srcs.num)
		text_ss->cur_item = 0;

	do_transition(text_ss, false);
}

void text_ss_previous_slide(void *data) {
	struct text_slideshow *text_ss = data;

	if (!text_ss->text_srcs.num || 
			obs_transition_get_time(text_ss->transition) < 1.0f)
		return;

	if (text_ss->cur_item == 0)
		text_ss->cur_item = text_ss->text_srcs.num - 1;
	else
		--text_ss->cur_item;

	do_transition(text_ss, false);
}

enum obs_media_state ss_get_state(void *data) {
	struct text_slideshow *text_ss = data;
	return text_ss->state;
}
