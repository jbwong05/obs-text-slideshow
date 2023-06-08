#pragma once
#include "cut.h"
#include "fade.h"
#include "slide.h"
#include <vector>

using std::vector;

static vector<transition_vtable> transition_vtables = {
    {
        fade_transition_source_create,
        fade_transition_setup_properties,
        create_fade_transition,
        destroy_fade_transition
    },
	{
        cut_transition_source_create,
	    cut_transition_setup_properties,
        create_cut_transition,
	    destroy_cut_transition
    },
    {
        slide_transition_source_create,
        slide_transition_setup_properties,
        create_slide_transition,
        destroy_slide_transition
    }
};

vector<transition *> *load_transitions();
int match_transition(vector<transition *> *, const char *);
void destroy_transitions(vector<transition *> *);
bool transition_selected_callback(void *, obs_properties_t *, obs_property_t *, obs_data_t *);