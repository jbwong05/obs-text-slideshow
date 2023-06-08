#include "slide.h"

#define S_SLIDE_DIRECTION "slide_direction"
#define S_SLIDE_LEFT "left"
#define S_SLIDE_RIGHT "right"
#define S_SLIDE_UP "up"
#define S_SLIDE_DOWN "down"

#define T_TR_SLIDE T_TR_("Slide")
#define TR_SLIDE "slide"
#define SLIDE_LOCALE(text) T_TR_("Slide." text)

#define DIRECTION "direction"

obs_source_t *slide_transition_source_create(obs_data_t *settings)
{
	const char *direction =
		obs_data_get_string(settings, S_SLIDE_DIRECTION);
	obs_data_t *data = obs_data_create();
	obs_data_set_string(data, DIRECTION, direction);
	obs_source_t *source =
		obs_source_create_private(SLIDE_TRANSITION_ID, NULL, data);
	obs_data_release(data);
	return source;
}

void slide_transition_setup_properties(transition *transition,
				       obs_properties_t *props, bool visible)
{
	obs_property_t *p = obs_properties_add_list(props, S_SLIDE_DIRECTION,
						    SLIDE_LOCALE("Direction"),
						    OBS_COMBO_TYPE_LIST,
						    OBS_COMBO_FORMAT_STRING);
	obs_property_list_add_string(p, SLIDE_LOCALE("Left"), S_SLIDE_LEFT);
	obs_property_list_add_string(p, SLIDE_LOCALE("Right"), S_SLIDE_RIGHT);
	obs_property_list_add_string(p, SLIDE_LOCALE("Up"), S_SLIDE_UP);
	obs_property_list_add_string(p, SLIDE_LOCALE("Down"), S_SLIDE_DOWN);

	obs_property_set_visible(p, visible);

	transition->property_names->insert(S_SLIDE_DIRECTION);
}

transition *create_slide_transition()
{
	transition *slide_transition =
		(transition *)bzalloc(sizeof(transition));
	slide_transition->id = SLIDE_TRANSITION_ID;
	slide_transition->prop_name = T_TR_SLIDE;
	slide_transition->prop_val = TR_SLIDE;
	slide_transition->property_names = new unordered_set<const char *>;

	return slide_transition;
}

void destroy_slide_transition(transition *transition)
{
	bfree(transition);
}
