#include "utils.h"
#include "obs-module.h"
#include "string.h"
#include "plugin-macros.generated.h"

unsigned char multichar_delim_strtok(char *str, const char *delim,
				     vector<char *> &output)
{
	unsigned char return_flags = 0;

	if (!str) {
		return_flags |= STRTOK_ERROR;
		return return_flags;
	}

	// Add '\n' to start of delim
	size_t delim_len = strlen(delim);
	char *extended_delim = (char *)bzalloc(1 + delim_len + 1);
	extended_delim[0] = '\n';
#ifdef _WIN32
	strncpy_s(extended_delim + 1, delim_len, delim, delim_len);
#else
	memcpy(extended_delim + 1, delim, delim_len);
#endif

	size_t len = strlen(str);
	size_t increment = 0;
	bool add_next = true;

	char *curr = str;
	char *ret = strstr(curr, extended_delim);
	while (ret) {

		// what we want to check for
		// \n*\n or \n*\r\n or \r\n*\n or \r\n*\r\n

		if (ret + delim_len + 1 > str + len &&
		    ret[delim_len + 1] == '\n') {
			// '\n*\n' case
			increment = delim_len + 2;

		} else if (ret + delim_len + 2 < str + len &&
			   ret[delim_len + 1] == '\r' &&
			   ret[delim_len + 2] == '\n') {
			// '\n*\r\n' case
			increment = delim_len + 3;

		} else {
			// '\n*' false delim
			if (ret == str) {
				// very start
				output.push_back(str);
				add_next = false;
			}

			curr = ret + delim_len + 1;
			ret = strstr(curr, extended_delim);
			continue;
		}

		if (ret != str) {
			// delim is not at the very start
			*(ret - 1) = 0;

			if (add_next) {
				output.push_back(curr);
			} else {
				add_next = true;
			}
		} else {
			return_flags |= START_WITH_DELIM;
		}

		curr = ret + increment;

		ret = strstr(curr, extended_delim);
	}

	if (curr < str + len) {
		output.push_back(curr);
	} else {
		return_flags |= END_WITH_DELIM;
	}

	bfree(extended_delim);
	return return_flags;
}

void remove_ending_new_line(char *text)
{
	size_t len = strlen(text);

	if (len >= 2 && text[len - 2] == '\r' && text[len - 1] == '\n') {
		text[len - 2] = 0;
		text[len - 1] = 0;
	} else if (len >= 1 && text[len - 1] == '\n') {
		text[len - 1] = 0;
	}
}

void remove_new_lines(size_t start, vector<char *> &texts)
{
	// Remove trailing new lines
	for (size_t i = start; i < texts.size(); i++) {
		remove_ending_new_line(texts[i]);
	}
}

bool append_new_text(const char *token, vector<char *> &texts)
{
	// Need to add new string
	size_t token_len = strlen(token);
	char *curr_text = (char *)bzalloc(token_len + 1);

	if (curr_text == NULL) {
		return false;
	}
#ifdef _WIN32
	strncpy_s(curr_text, token_len + 1, token, token_len);
#else
	memcpy(curr_text, token, token_len);
#endif
	texts.push_back(curr_text);

	return true;
}

char *append_string(const char *original_str, const char *new_str)
{
	size_t existing_len = strlen(original_str);
	size_t new_len = strlen(new_str);
	char *new_ptr = (char *)brealloc((void *)original_str,
					 existing_len + new_len + 1);

	if (new_ptr == NULL) {
		return NULL;
	}

#ifdef _WIN32
	strncpy_s(new_ptr + existing_len, new_len + 1, new_str, new_len);
#else
	memcpy(new_ptr + existing_len, new_str, new_len);
#endif

	new_ptr[existing_len + new_len] = 0;
	return new_ptr;
}