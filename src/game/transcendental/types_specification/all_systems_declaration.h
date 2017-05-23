#pragma warning(disable : 4503)
#pragma once

namespace augs {
	template <class...>
	class storage_for_systems;
}

class relational_system;
class name_system;
class physics_system;
class tree_of_npo_system;
class processing_lists_system;

class interpolation_system;
class past_infection_system;
class light_system;
class particles_simulation_system;
class wandering_pixels_system;
class sound_system;
class gui_element_system;
class vertically_flying_number_system;
class pure_color_highlight_system;
class exploding_ring_system;
class thunder_system;

typedef augs::storage_for_systems <
	// It is critical that the relational system is the first on this list
	// so that it creates inferred state of relations before physics_system uses it for constructing
	// bodies, fixtures and joints.
	relational_system,
	name_system,
	physics_system,
	tree_of_npo_system,
	processing_lists_system
> storage_for_all_systems_inferred;

typedef augs::storage_for_systems <
	interpolation_system,
	past_infection_system,
	light_system,
	particles_simulation_system,
	wandering_pixels_system,
	sound_system,
	gui_element_system,
	vertically_flying_number_system,
	pure_color_highlight_system,
	exploding_ring_system,
	thunder_system
> storage_for_all_systems_audiovisual;
