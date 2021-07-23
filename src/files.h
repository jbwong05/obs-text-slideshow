#pragma once

#include <vector>
#include "obs-module.h"
#include "obs-text-slideshow.h"

using std::vector;

void read_file(struct text_slideshow *text_ss, obs_data_t *settings,
	       vector<const char *> &texts);