#pragma once

#include <vector>

using std::vector;

void load_text_from_file_end(vector<const char *> & texts, 
        const char *file_path);
void load_text_from_file(vector<const char *> & texts, const char *file_path,
        bool from_end = false);