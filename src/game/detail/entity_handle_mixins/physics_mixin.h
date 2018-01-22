#pragma once
#include "augs/math/vec2.h"
#include "augs/build_settings/platform_defines.h"
#include "game/organization/all_components_declaration.h"
#include "game/detail/physics/owner_of_colliders.h"

struct rigid_body_cache_info;

template <bool is_const, class entity_handle_type>
class basic_physics_mixin {
public:
	entity_handle_type get_owner_friction_ground() const;
	
	auto& get_special_physics() const {
		const auto handle = *static_cast<const entity_handle_type*>(this);
		return handle.template get<components::rigid_body>().get_special();
	}

	entity_handle_type get_owner_of_colliders() const;
	owner_of_colliders calculate_owner_of_colliders() const;

	/* Assumes that the fixtures component is found. */
	real32 calculate_density(const owner_of_colliders calculated_owner) const;
};

template <bool, class>
class physics_mixin;

template <class entity_handle_type>
class physics_mixin<false, entity_handle_type> : public basic_physics_mixin<false, entity_handle_type> {
public:
	using base = basic_physics_mixin<false, entity_handle_type>;

	void infer_colliders() const;
};

template <class entity_handle_type>
class physics_mixin<true, entity_handle_type> : public basic_physics_mixin<true, entity_handle_type> {
};
