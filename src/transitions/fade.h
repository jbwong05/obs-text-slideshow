#pragma once
#include "transition.h"

#define FADE_TRANSITION_ID "fade_transition"

obs_source_t *fade_transition_source_create();
void fade_transition_setup_properties(obs_property_t *);
transition *create_fade_transition();
void destroy_fade_transition(transition *);
