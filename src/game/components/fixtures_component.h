#pragma once
#include <optional>
#include <Box2D/Dynamics/b2Fixture.h>

#include "augs/pad_bytes.h"

#include "augs/math/vec2.h"
#include "augs/math/rects.h"

#include "augs/misc/enum/enum_array.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/component_synchronizer.h"

#include "game/enums/colliders_offset_type.h"
#include "game/assets/ids/physical_material_id.h"

#include "game/components/transform_component.h"

namespace components {
	struct specific_body_owner {
		// GEN INTROSPECTOR struct components::specific_body_owner
		owner_of_colliders owner;
		// END GEN INTROSPECTOR
	};
}

namespace definitions {
	struct fixtures {
		// GEN INTROSPECTOR struct definitions::fixtures
		bool friction_ground = false;
		bool disable_standard_collision_resolution = false;
		bool driver_shoot_through = false;
		bool destructible = false;
		bool sensor = false;
		pad_bytes<3> pad;

		assets::physical_material_id material = assets::physical_material_id::INVALID;

		float collision_sound_gain_mult = 1.f;

		float density = 1.f;
		float friction = 0.f;
		float restitution = 0.f;

		b2Filter filter;
		// END GEN INTROSPECTOR

		/*
			Interface for when we want to calculate these depending on a cached value
		*/

		bool is_friction_ground() const {
			return friction_ground;
		}	

		bool is_destructible() const {
			return destructible;
		}

		bool can_driver_shoot_through() const {
			return driver_shoot_through;
		}

		bool standard_collision_resolution_disabled() const {
			return disable_standard_collision_resolution;
		}
	};
}
