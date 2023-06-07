#pragma once
#include "obs-module.h"
#include <unordered_set>

using std::unordered_set;

#define S_TRANSITION "transition"
#define T_TR_(text) obs_module_text("SlideShow.Transition." text)

typedef obs_source_t *(*transition_source_create)();
typedef void (*transition_setup_properties)(obs_property_t *prop);
typedef struct transition *(*create_transition)();
typedef void (*destroy_transition)(struct transition *transition);

typedef struct transition_vtable {
	transition_source_create create_transition_source;
	transition_setup_properties setup_transition_properties;
    create_transition constructor;
	destroy_transition destructor;
} transition_vtable;

typedef struct transition {
	const char *id;
	const char *prop_name;
	const char *prop_val;
	unordered_set<const char *> *property_names;
} transition;
