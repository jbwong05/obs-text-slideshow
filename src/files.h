#pragma once

#include <vector>
#include "obs-module.h"
#include "obs-text-slideshow.h"

using std::vector;

void read_file(struct text_slideshow *text_ss, obs_data_t *settings,
	       get_chat_log_mode chat_log_mode_retriever,
	       vector<const char *> &texts);