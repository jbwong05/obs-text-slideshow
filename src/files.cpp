#include "files.h"
#include <util/platform.h>
#include "plugin-macros.generated.h"

#define CHUNK_LEN 256

FILE *os_fopen(const char *path, const char *mode) {
#ifdef _WIN32
	wchar_t *wpath = NULL;
	FILE *file = NULL;

	if (path) {
		os_utf8_to_wcs_ptr(path, 0, &wpath);
		file = os_wfopen(wpath, mode);
		bfree(wpath);
	}

	return file;
#else
	return path ? fopen(path, mode) : NULL;
#endif
}

static void load_text_from_file(vector<const char *> & texts, const char *file_path,
		bool from_end) {
    FILE* file = os_fopen(file_path, "rb"); /* should check the result */
	if(file == NULL) {
		blog(LOG_WARNING, "Failed to open file %s", file_path);
		return;
	}

	uint16_t header = 0;
	size_t num_read = fread(&header, 2, 1, file);
	if(num_read == 1 && (header == 0xFEFF || header == 0xFFFE)) {
		blog(LOG_WARNING, "UTF-16 not supported for file %s", file_path);
		fclose(file);
		return;
	}

	fseek(file, 0, SEEK_SET);

	unsigned int curr_index = 0;
    char line[CHUNK_LEN];
	memset(line, 0, CHUNK_LEN);
	bool add_new_line = true;

    while(fgets(line, sizeof(line), file)) {
		size_t curr_len = strlen(line);

		if(line[curr_len - 2] == '\r' && line[curr_len - 1] == '\n') {
			curr_len -= 2;
		} else if(line[curr_len - 1] == '\n') {
			curr_len--;
		}

		if(add_new_line) {
			// Need to add new string
			char *curr_text = (char *)bzalloc(curr_len + 1);

			if(curr_text == NULL) {
				fclose(file);
				return;
			}

			strncpy(curr_text, line, curr_len);

			if(from_end) {
				texts.insert(texts.begin(), curr_text);
			} else {
				texts.push_back(curr_text);
			}
			
		} else {
			// Need to append to existing string
			size_t existing_len = strlen(texts[curr_index]);
			char *new_ptr = (char *)brealloc((void *)texts[curr_index], 
					existing_len + curr_len + 1);
			
			if(new_ptr == NULL) {
				fclose(file);
				return;
			}

			strncpy(new_ptr + existing_len, line, curr_len);
			new_ptr[existing_len + curr_len] = 0;
			texts[curr_index] = new_ptr;
		}

		if(curr_len != CHUNK_LEN - 1 && !from_end) {
			curr_index++;
		}

		add_new_line = (line[curr_len] == '\r' && line[curr_len + 1] == '\n') 
			|| line[curr_len] == '\n';
    }

    fclose(file);
}

static void load_text_from_file_end(vector<const char *> & texts, 
        const char *file_path) {
	load_text_from_file(texts, file_path, true);
}

void read_file(struct text_slideshow *text_ss, 
		obs_data_t *settings, 
		get_chat_log_mode chat_log_mode_retriever, 
		vector<const char *> & texts) {

	const char *file_path = text_ss->file.c_str();

	if (!file_path || !*file_path || !os_file_exists(file_path)) {
		blog(LOG_WARNING,
				"FT2-text: Failed to open %s for "
				"reading",
				file_path);
	} else {
		if (!text_ss->file.empty()) {
			
			bool chat_log_mode = (*chat_log_mode_retriever)(settings);

			text_ss->file = file_path;
			if (chat_log_mode) {
				load_text_from_file_end(texts, file_path);
			} else {
				load_text_from_file(texts, file_path, false);
			}
		}
	}
}