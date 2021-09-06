#pragma once

#include <obs-module.h>
#include "obs-text-props.h"

#define S_FONT "font"
#define S_TEXT "text"
#define S_FROM_FILE "from_file"
#define S_ANTIALIASING "antialiasing"
#define S_LOG_MODE "log_mode"
#define S_LOG_LINES "log_lines"
#define S_TEXT_FILE "text_file"
#define S_COLOR_1 "color1"
#define S_COLOR_2 "color2"
#define S_OUTLINE "outline"
#define S_DROP_SHADOW "drop_shadow"
#define S_CUSTOM_WIDTH "custom_width"
#define S_WORD_WRAP "word_wrap"
#define S_FACE "face"
#define S_SIZE "size"
#define S_FLAGS "flags"
#define S_STYLE "style"

#define T_TXT_(text) obs_module_text("Text." text)
#define T_FONT T_TXT_("Font")
#define T_TEXT T_TXT_("Text")
#define T_FROM_FILE T_TXT_("ReadFromFile")
#define T_ANTIALIASING T_TXT_("Antialiasing")
#define T_LOG_MODE T_TXT_("ChatLogMode")
#define T_LOG_LINES T_TXT_("ChatLogLines")
#define T_TEXT_FILE T_TXT_("TextFile")
#define T_TEXT_FILE_FILTER T_TXT_("TextFileFilter")
#define T_COLOR_1 T_TXT_("Color1")
#define T_COLOR_2 T_TXT_("Color2")
#define T_OUTLINE T_TXT_("Outline")
#define T_DROP_SHADOW T_TXT_("DropShadow")
#define T_CUSTOM_WIDTH T_TXT_("CustomWidth")
#define T_WORD_WRAP T_TXT_("WordWrap")

#ifdef _WIN32
#define DEFAULT_FACE "Arial"
#elif __APPLE__
#define DEFAULT_FACE "Helvetica"
#else
#define DEFAULT_FACE "Sans Serif"
#endif

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