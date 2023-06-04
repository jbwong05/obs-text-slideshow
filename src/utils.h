#pragma once

#include <cstdlib>
#include <vector>

using std::vector;

#define STRTOK_ERROR 0x01
#define START_WITH_DELIM 0x02
#define END_WITH_DELIM 0X04

#define set_vis(val, show)                          \
	do {                                        \
		p = obs_properties_get(props, val); \
		obs_property_set_visible(p, show);  \
	} while (false)

unsigned char multichar_delim_strtok(char *str, const char *delim,
				     vector<char *> &output);
void remove_ending_new_line(char *text);
void remove_new_lines(size_t start, vector<char *> &texts);
bool append_new_text(const char *token, vector<char *> &texts);
char *append_string(const char *original_str, const char *new_str);