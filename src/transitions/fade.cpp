#include "fade.h"

#define T_TR_FADE T_TR_("Fade")
#define TR_FADE "fade"

obs_source_t *fade_transition_source_create(obs_data_t *settings)
{
	UNUSED_PARAMETER(settings);
	return obs_source_create_private(FADE_TRANSITION_ID, NULL, NULL);
}

void fade_transition_setup_properties(transition *transition,
				      obs_properties_t *props, bool visible)
{
	UNUSED_PARAMETER(transition);
	UNUSED_PARAMETER(props);
	UNUSED_PARAMETER(visible);
}

transition *create_fade_transition()
{
	transition *fade_transition = (transition *)bzalloc(sizeof(transition));
	fade_transition->id = FADE_TRANSITION_ID;
	fade_transition->prop_name = T_TR_FADE;
	fade_transition->prop_val = TR_FADE;
	fade_transition->property_names = new unordered_set<const char *>;

	return fade_transition;
}

void destroy_fade_transition(transition *transition)
{
	bfree(transition);
}
