#pragma once

#include <vector>
#include "obs-text-slideshow.h"

using std::vector;

void load_text_from_file(vector<char *> &texts, const char *file_path,
			 const char *delim);
void load_text_from_file(vector<char *> &texts, const char *file_path);