#include "cut.h"

#define T_TR_CUT T_TR_("Cut")
#define TR_CUT "cut"

obs_source_t *cut_transition_source_create(obs_data_t *settings)
{
	UNUSED_PARAMETER(settings);
	return obs_source_create_private(CUT_TRANSITION_ID, NULL, NULL);
}

void cut_transition_setup_properties(transition *transition,
				     obs_properties_t *props, bool visisble)
{
	UNUSED_PARAMETER(transition);
	UNUSED_PARAMETER(props);
	UNUSED_PARAMETER(visisble);
}

transition *create_cut_transition()
{
	transition *cut_transition = (transition *)bzalloc(sizeof(transition));
	cut_transition->id = CUT_TRANSITION_ID;
	cut_transition->prop_name = T_TR_CUT;
	cut_transition->prop_val = TR_CUT;
	cut_transition->property_names = new unordered_set<const char *>;

	return cut_transition;
}

void destroy_cut_transition(transition *transition)
{
	bfree(transition);
}
