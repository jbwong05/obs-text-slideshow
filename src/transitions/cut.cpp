#include "cut.h"

#define T_TR_CUT T_TR_("Cut")
#define TR_CUT "cut"

obs_source_t *cut_transition_source_create() {
	return obs_source_create_private(CUT_TRANSITION_ID, NULL, NULL);
}

void cut_transition_setup_properties(obs_property_t *prop) {
    UNUSED_PARAMETER(prop);
}

transition *create_cut_transition() {
    transition *cut_transition = (transition *)bzalloc(sizeof(transition));
    cut_transition->id = CUT_TRANSITION_ID;
    cut_transition->prop_name = T_TR_CUT;
    cut_transition->prop_val = TR_CUT;
    cut_transition->properties = new unordered_set<obs_property_t *>;

    return cut_transition;
}

void destroy_cut_transition(transition *transition) {
    transition->properties->clear();
    delete transition->properties;
    bfree(transition);
}
