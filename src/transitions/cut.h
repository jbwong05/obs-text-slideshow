#pragma once
#include "transition.h"

#define CUT_TRANSITION_ID "cut_transition"

obs_source_t *cut_transition_source_create();
void cut_transition_setup_properties(obs_property_t *);
transition *create_cut_transition();
void destroy_cut_transition(transition *);
