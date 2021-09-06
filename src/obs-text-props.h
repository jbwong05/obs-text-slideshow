#pragma once

#include <obs-module.h>

typedef void (*switch_text_settings_cb)(struct text_slideshow *text_ss,
					obs_data_t *settings);