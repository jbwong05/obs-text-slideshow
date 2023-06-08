#pragma once
#include "transition.h"

#define SLIDE_TRANSITION_ID "slide_transition"

obs_source_t *slide_transition_source_create(obs_data_t *);
void slide_transition_setup_properties(transition *, obs_properties_t *, bool);
transition *create_slide_transition();
void destroy_slide_transition(transition *);
