#pragma once

#include <obs-module.h>

struct gdiplus_props {
    obs_data_t *curr_font;
	const char *text;
	const char *align;
	const char *valign;
	uint32_t color;
	uint32_t opacity;
	bool gradient;
	uint32_t gradient_color;
	uint32_t gradient_opacity;
	double gradient_dir;
	bool veritcal;
	bool outline;
	uint32_t outline_color;
	uint32_t outline_opacity;
	uint32_t outline_size;
	bool use_file;
	const char *file;
	bool chatlog_mode;
	long long chatlog_lines;
	bool extents;
	bool extents_wrap;
	uint32_t extents_cx;
	uint32_t extents_cy;
	long long transform;
	bool antialiasing;
	uint32_t bkcolor;
	uint32_t bkopacity;
};