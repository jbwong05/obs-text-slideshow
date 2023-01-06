#include "files.h"
#include "obs-module.h"
#include "plugin-macros.generated.h"
#include "utils.h"

#define CHUNK_LEN 256

void load_text_from_file(vector<char *> &texts, const char *file_path,
			 const char *delim)
{
	FILE *file = os_fopen(file_path, "rb"); /* should check the result */
	if (file == NULL) {
		blog(LOG_WARNING, "Failed to open file %s", file_path);
		return;
	}

	uint16_t header = 0;
	size_t num_read = fread(&header, 1, 2, file);
	if (num_read == 1 && (header == 0xFEFF || header == 0xFFFE)) {
		blog(LOG_WARNING, "UTF-16 not supported for file %s",
		     file_path);
		fclose(file);
		return;
	}

	fseek(file, 0, SEEK_SET);

	char chunk[CHUNK_LEN];
	memset(chunk, 0, CHUNK_LEN);
	size_t read = 0;
	vector<char *> tokens;
	bool append_first = false;
	read = fread(chunk, sizeof(char), CHUNK_LEN - 1, file);

	while (read) {

		chunk[read] = 0;

		tokens.clear();
		unsigned char token_flags =
			multichar_delim_strtok(chunk, delim, tokens);

		for (unsigned int i = 0; i < tokens.size(); i++) {
			const char *token = tokens[i];

			if (i == 0 && texts.size() > 0 && append_first &&
			    !(token_flags & START_WITH_DELIM)) {
				// Need to append to existing string
				size_t curr_index = texts.size() - 1;
				char *new_ptr =
					append_string(texts[curr_index], token);

				if (new_ptr == NULL) {
					fclose(file);
					return;
				}

				texts[curr_index] = new_ptr;

				// Account for delim split by chunk boundary
				vector<char *> temp_tokens;
				multichar_delim_strtok(new_ptr, delim,
						       temp_tokens);
				if (temp_tokens.size() > 1) {
					const char *first = temp_tokens[0];
					const char *second = temp_tokens[1];

					char *new_first_ptr = (char *)brealloc(
						(void *)texts[curr_index],
						strlen(first));

					if (new_first_ptr == NULL) {
						fclose(file);
						return;
					}

					texts[curr_index] = new_first_ptr;

					if (!append_new_text(second, texts)) {
						fclose(file);
						return;
					}
				}
			} else {
				if (!append_new_text(token, texts)) {
					fclose(file);
					return;
				}
			}
		}

		append_first = !(token_flags & END_WITH_DELIM);
		read = fread(chunk, sizeof(char), CHUNK_LEN - 1, file);
	}

	fclose(file);
}

void load_text_from_file(vector<char *> &texts, const char *file_path)
{
	FILE *file = os_fopen(file_path, "rb"); /* should check the result */
	if (file == NULL) {
		blog(LOG_WARNING, "Failed to open file %s", file_path);
		return;
	}

	uint16_t header = 0;
	size_t num_read = fread(&header, 1, 2, file);
	if (num_read == 1 && (header == 0xFEFF || header == 0xFFFE)) {
		blog(LOG_WARNING, "UTF-16 not supported for file %s",
		     file_path);
		fclose(file);
		return;
	}

	fseek(file, 0, SEEK_SET);

	char line[CHUNK_LEN];
	memset(line, 0, CHUNK_LEN);
	bool add_new_line = true;
	bool prev_new_line = false;

	while (fgets(line, sizeof(line), file)) {
		size_t curr_len = strlen(line);

		if ((curr_len == 2 && line[curr_len - 2] == '\r' &&
		     line[curr_len - 1] == '\n') ||
		    (curr_len == 1 && line[curr_len - 1] == '\n')) {

			add_new_line = true;

			if (!prev_new_line) {
				prev_new_line = true;
				continue;
			}
		}

		if (add_new_line) {
			// Need to add new string
			char *curr_text = (char *)bzalloc(curr_len + 1);

			if (curr_text == NULL) {
				fclose(file);
				return;
			}
#ifdef _WIN32
			strncpy_s(curr_text, curr_len + 1, line, curr_len);
#else
			memcpy(curr_text, line, curr_len);
#endif

			texts.push_back(curr_text);
			add_new_line = false;
			prev_new_line = false;

		} else {
			// Need to append to existing string
			size_t curr_index = texts.size() - 1;
			size_t existing_len = strlen(texts[curr_index]);
			char *new_ptr =
				(char *)brealloc((void *)texts[curr_index],
						 existing_len + curr_len + 1);

			if (new_ptr == NULL) {
				fclose(file);
				return;
			}

#ifdef _WIN32
			strncpy_s(new_ptr + existing_len, curr_len + 1, line,
				  curr_len);
#else
			memcpy(new_ptr + existing_len, line, curr_len);
#endif

			new_ptr[existing_len + curr_len] = 0;
			texts[curr_index] = new_ptr;
		}
	}

	remove_new_lines(0, texts);

	fclose(file);
}