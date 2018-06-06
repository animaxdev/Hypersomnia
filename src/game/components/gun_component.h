#pragma once
#include <cstddef>
#include "augs/pad_bytes.h"

#include "augs/misc/timing/stepped_timing.h"
#include "augs/misc/minmax.h"

#include "game/enums/gun_action_type.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/step_declaration.h"
#include "game/transcendental/entity_flavour_id.h"

#include "game/components/render_component.h"

#include "game/assets/ids/asset_ids.h"
#include "game/assets/recoil_player.h"

#include "game/detail/view_input/sound_effect_input.h"
#include "game/detail/view_input/particle_effect_input.h"

namespace augs {
	struct introspection_access;
}

namespace components {
	struct gun {
		// GEN INTROSPECTOR struct components::gun
		augs::stepped_timestamp when_last_fired;

		bool is_trigger_pressed = false;
		bool is_cocking_handle_being_pulled = false;

		pad_bytes<2> pad;

		float current_heat = 0.f;

		augs::stepped_timestamp when_began_pulling_cocking_handle;

		recoil_player_instance recoil;
		// END GEN INTROSPECTOR

		void set_cocking_handle_pulling(
			const bool enabled,
			const augs::stepped_timestamp now
		);
	};
}

namespace invariants {
	struct gun {
		// GEN INTROSPECTOR struct invariants::gun
		float shot_cooldown_ms = 100.f;

		gun_action_type action_mode = gun_action_type::INVALID;
		unsigned num_last_bullets_to_trigger_low_ammo_cue = 0;

		augs::minmax<float> muzzle_velocity = { 2000.f, 2000.f };

		float damage_multiplier = 1.f;

		augs::minmax<float> shell_velocity = { 300.f, 1700.f };
		augs::minmax<float> shell_angular_velocity = { 2.f, 14.f };

		float shell_spread_degrees = 20.f;

		components::transform shell_spawn_offset;

		float gunshot_adds_heat = 0.05f;
		float maximum_heat = 1.f;

		sound_effect_input muzzle_shot_sound;
		sound_effect_input low_ammo_cue_sound;

		sound_effect_input firing_engine_sound;
		particle_effect_input muzzle_particles;

		real32 kickback_towards_wielder = 0.f;
		real32 recoil_multiplier = 1.f;

		float cocking_handle_pull_duration_ms = 500.f;

		constrained_entity_flavour_id<invariants::missile, components::sender> magic_missile_flavour;
		recoil_player_instance_def recoil;

		assets::plain_animation_id shoot_animation;
		// END GEN INTROSPECTOR
	};
}

template <class T>
auto get_bullet_spawn_offset(
	const T& gun_handle
) {
	const auto& cosmos = gun_handle.get_cosmos();

	if (const auto* const sprite = gun_handle.template find<invariants::sprite>()) {
		const auto reference_id = sprite->image_id;
		const auto& offsets = cosmos.get_logical_assets().get_offsets(reference_id);

		return offsets.gun.bullet_spawn;
	}

	return vec2i();
}

template <class T>
vec2 calc_muzzle_position(
	const T& gun_handle,
	const transformr& gun_transform
) {
	const auto bullet_spawn_offset = get_bullet_spawn_offset(gun_handle);

	if (const auto logical_width = gun_handle.find_logical_width()) {
		return (
			gun_transform 
			* transformr(bullet_spawn_offset + vec2(*logical_width / 2, 0))
		).pos;
	}

	return {};
}

template <class T>
vec2 calc_barrel_center(
	const T& gun_handle,
	const transformr& gun_transform
) {
	const auto bullet_spawn_offset = get_bullet_spawn_offset(gun_handle);

	return (gun_transform * transformr(vec2(0, bullet_spawn_offset.y))).pos;
}
