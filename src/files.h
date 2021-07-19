#pragma once

#include "sys/stat.h"
#include <vector>

using std::vector;

time_t get_modified_timestamp(const char *filename);
void load_text_from_file_end(vector<const char *> & texts, 
        const char *file_path);
void load_text_from_file(vector<const char *> & texts, const char *file_path,
        bool from_end = false);