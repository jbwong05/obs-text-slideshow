#pragma once

#include <obs-module.h>

struct freetype2_props {
	obs_data_t *font;
	bool drop_shadow;
	bool outline;
	bool word_wrap;
	long long color_1;
	long long color_2;
	long long custom_width;
	bool from_file;
	bool log_mode;
	long long log_lines;
	bool antialiasing;
	const char *text_file;
	const char *text;
};