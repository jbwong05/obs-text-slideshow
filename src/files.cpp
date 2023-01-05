#include "files.h"
#include "string.h"
#include <util/platform.h>
#include "plugin-macros.generated.h"

#define TABLE_SIZE 256
#define CHUNK_LEN 256
#ifdef _WIN32
// Format is \r\n*\r\n\0 where '*' is the delim
#define NEW_LINE "\r\n"
#define DELIM_LEN 6
#else
// Format is \n*\n\0 where '*' is the delim
#define NEW_LINE "\n"
#define DELIM_LEN 4
#endif

static bool multichar_delim_strtok(char *str, const char *delim,
				   vector<char *> &output)
{
	if (!str) {
		return false;
	}

	int len = strlen(str);

	char *curr = str;
	char *ret = strstr(curr, delim);
	while (ret) {

		if (ret != str) {
			*(ret - 1) = 0;
		}

		output.push_back(curr);
		curr = ret + DELIM_LEN;

		ret = strstr(curr, delim);
	}

	if (curr - str < len) {
		output.push_back(curr);
		return true;
	} else {
		output.push_back(NULL);
		return false;
	}
}

static void remove_ending_new_line(char *text)
{
	size_t len = strlen(text);

	if (len >= 2 && text[len - 2] == '\r' && text[len - 1] == '\n') {
		text[len - 2] = 0;
		text[len - 1] = 0;
	} else if (len >= 1 && text[len - 1] == '\n') {
		text[len - 1] = 0;
	}
}

static void remove_new_lines(size_t start, vector<char *> &texts)
{
	// Remove trailing new lines
	for (size_t i = start; i < texts.size(); i++) {
		remove_ending_new_line(texts[i]);
	}
}

static void load_text_from_file(vector<char *> &texts, const char *file_path,
				const char *delim_char)
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
	char delim[DELIM_LEN];
	memset(delim, 0, DELIM_LEN);

#ifdef _WIN32
	// Format is \r\n*\0\0\0 where '*' is the delim
	delim[0] = '\r';
	delim[1] = '\n';
	delim[2] = delim_char[0];
#else
	// Format is \n*\0\0 where '*' is the delim
	delim[0] = '\n';
	delim[1] = delim_char[0];
#endif

	bool append = false;
	size_t read = 0;

	vector<char *> tokens;
	read = fread(chunk, sizeof(char), CHUNK_LEN - 1, file);
	while (read) {

		chunk[read] = 0;

		tokens.clear();
		bool append_next_iteration =
			multichar_delim_strtok(chunk, delim, tokens);

		for (unsigned int i = 0; i < tokens.size(); i++) {
			const char *token = tokens[i];

			if (!token) {
				token = NEW_LINE;
			}
			blog(LOG_INFO, "TOKEN: %s", token);

			size_t token_len = strlen(token);

			if (i == 0 && append) {
				// Need to append to existing string
				size_t curr_index = texts.size() - 1;
				size_t existing_len = strlen(texts[curr_index]);
				char *new_ptr = (char *)brealloc(
					(void *)texts[curr_index],
					existing_len + token_len + 1);

				if (new_ptr == NULL) {
					fclose(file);
					return;
				}

#ifdef _WIN32
				strncpy_s(new_ptr + existing_len, token_len + 1,
					  token, token_len);
#else
				memcpy(new_ptr + existing_len, token,
				       token_len);
#endif

				new_ptr[existing_len + token_len] = 0;
				texts[curr_index] = new_ptr;
			} else {
				// Need to add new string
				char *curr_text =
					(char *)bzalloc(token_len + 1);

				if (curr_text == NULL) {
					fclose(file);
					return;
				}
#ifdef _WIN32
				strncpy_s(curr_text, token_len + 1, token,
					  token_len);
#else
				memcpy(curr_text, token, token_len);
#endif
				texts.push_back(curr_text);
			}
		}

		append = append_next_iteration;

		read = fread(chunk, sizeof(char), CHUNK_LEN - 1, file);
	}

	fclose(file);
}

static void load_text_from_file(vector<char *> &texts, const char *file_path)
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

void read_file(struct text_slideshow *text_ss, vector<char *> &texts)
{

	const char *file_path = text_ss->file.c_str();

	if (!file_path || !*file_path || !os_file_exists(file_path)) {
		blog(LOG_WARNING,
		     "Failed to open %s for "
		     "reading",
		     file_path);
	} else {
		if (!text_ss->file.empty()) {

			if (text_ss->custom_delim) {
				load_text_from_file(texts, file_path,
						    text_ss->custom_delim);
			} else {
				load_text_from_file(texts, file_path);
			}
		}
	}
}