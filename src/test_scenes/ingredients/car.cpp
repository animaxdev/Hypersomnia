#include "ingredients.h"
#include "augs/drawing/polygon.h"
#include "game/transcendental/cosmos.h"
#include "game/stateless_systems/particles_existence_system.h"
#include "game/stateless_systems/sound_existence_system.h"
#include "game/components/position_copying_component.h"

#include "game/components/crosshair_component.h"
#include "game/components/sprite_component.h"
#include "game/components/movement_component.h"
#include "game/components/rotation_copying_component.h"
#include "game/components/animation_component.h"
#include "game/components/rigid_body_component.h"
#include "game/components/car_component.h"
#include "game/components/rigid_body_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/type_component.h"
#include "game/components/particles_existence_component.h"
#include "game/components/sound_existence_component.h"
#include "game/components/polygon_component.h"
#include "game/components/shape_polygon_component.h"
#include "game/transcendental/cosmos.h"

#include "game/assets/all_logical_assets.h"

#include "game/enums/filters.h"

namespace test_types {
	void populate_car_types(const all_logical_assets& logicals, entity_types& types) {
		{
			auto& meta = get_test_type(types, test_scene_type::TRUCK_FRONT);
			
			definitions::render render_def;
			render_def.layer = render_layer::DYNAMIC_BODY;

			meta.set(render_def);

			definitions::polygon poly;
			poly.add_convex_polygons(logicals.at(assets::game_image_id::TRUCK_FRONT).shape.convex_polys);
			poly.texture_map_id = assets::game_image_id::TRUCK_FRONT;

			meta.set(poly);

			meta.add_shape_definition_from_renderable(logicals);
		}
		{
			auto& meta = get_test_type(types, test_scene_type::TRUCK_INTERIOR);

			definitions::render render_def;
			render_def.layer = render_layer::CAR_INTERIOR;

			meta.set(render_def);

			add_sprite(meta, logicals, assets::game_image_id::TRUCK_INSIDE);
						meta.add_shape_definition_from_renderable(logicals);

		}

		{
			auto& meta = get_test_type(types, test_scene_type::TRUCK_LEFT_WHEEL);

			definitions::render render_def;
			render_def.layer = render_layer::CAR_WHEEL;

			meta.set(render_def);
			definitions::sprite sprite_def;
			sprite_def.set(assets::game_image_id::BLANK, vec2 ( 40, 20 ), rgba(255, 255, 255, 0));
			meta.set(sprite_def);
			meta.add_shape_definition_from_renderable(logicals);
		}

		{
			auto& meta = get_test_type(types, test_scene_type::TRUCK_ENGINE_BODY);

			definitions::render render_def;
			render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

			meta.set(render_def);

			add_sprite(meta, logicals, assets::game_image_id::TRUCK_ENGINE);
			meta.add_shape_definition_from_renderable(logicals);
		}
	}
}

#if TODO
namespace prefabs {
	entity_handle create_car(const logic_step step, const components::transform& spawn_transform) {
		auto& world = step.get_cosmos();
		const auto& metas = step.get_logical_assets();

		auto front = create_test_scene_entity(world, test_scene_type::TRUCK_FRONT);
		auto interior = create_test_scene_entity(world, test_scene_type::TRUCK_INTERIOR);
		auto left_wheel = create_test_scene_entity(world, test_scene_type::TRUCK_LEFT_WHEEL);
		left_wheel.make_as_child_of(front);

		const auto si = world.get_si();

		const vec2 front_size = metas.at(assets::game_image_id::TRUCK_FRONT).get_size();
		const vec2 interior_size = metas.at(assets::game_image_id::TRUCK_INSIDE).get_size();

		{
			auto& car = front += components::car();
			components::rigid_body physics_definition(si, spawn_transform);
			components::fixtures colliders;

			car.interior = interior;
			car.left_wheel_trigger = left_wheel;
			car.input_acceleration.set(2500, 4500) /= 3;
			//car.acceleration_length = 4500 / 5.0;
			car.acceleration_length = 4500 / 6.2f;
			car.speed_for_pitch_unit = 2000.f;


			physics_definition.damping.linear = 0.4f;
			physics_definition.damping.angular = 2.f;

			components::fixtures group;

			group.filter = filters::dynamic_object();
			group.density = 0.6f;
			group.material = assets::physical_material_id::METAL;

			front += group;
			front += physics_definition;
			front.get<components::fixtures>().set_owner_body(front);
			//rigid_body.air_resistance = 0.2f;
		}
		
		{
			auto sprite = interior.get_def<definitions::sprite>();
			components::fixtures colliders;
			
			vec2 offset((front_size.x / 2 + sprite.get_size(/*metas*/).x / 2) * -1, 0);

			components::fixtures group;

			group.filter = filters::friction_ground();
			group.density = 0.6f;
			group.offsets_for_created_shapes[colliders_offset_type::SHAPE_OFFSET].pos = offset;
			group.friction_ground = true;
			group.material = assets::physical_material_id::METAL;

			interior  += group;

			interior.get<components::fixtures>().set_owner_body(front);
			interior.make_as_child_of(front);
		}

		{
			auto sprite = left_wheel.get_def<definitions::sprite>();
			components::fixtures colliders;

			vec2 offset((front_size.x / 2 + sprite.get_size(/*metas*/).x / 2 + 20) * -1, 0);

			components::fixtures group;

			group.filter = filters::trigger();
			group.density = 0.6f;
			group.disable_standard_collision_resolution = true;
			group.offsets_for_created_shapes[colliders_offset_type::SHAPE_OFFSET].pos = offset;
			group.material = assets::physical_material_id::METAL;

			left_wheel  += group;
			left_wheel.get<components::fixtures>().set_owner_body(front);
		}

		{
			for (int i = 0; i < 4; ++i) {
				components::transform this_engine_transform;
				const auto engine_physical = create_test_scene_entity(world, test_scene_type::TRUCK_ENGINE_BODY);
				engine_physical.make_as_child_of(front);

				{

					auto sprite = engine_physical.get_def<definitions::sprite>();

					components::transform offset;

					if (i == 0) {
						offset.pos.set((front_size.x / 2 + interior_size.x + sprite.get_size(/*metas*/).x / 2 - 5.f) * -1, (-interior_size.y / 2 + sprite.get_size(/*metas*/).y / 2));
					}
					if (i == 1) {
						offset.pos.set((front_size.x / 2 + interior_size.x + sprite.get_size(/*metas*/).x / 2 - 5.f) * -1, -(-interior_size.y / 2 + sprite.get_size(/*metas*/).y / 2));
					}
					if (i == 2) {
						offset.pos.set(-100, (interior_size.y / 2 + sprite.get_size(/*metas*/).x / 2) * -1);
						offset.rotation = -90;
					}
					if (i == 3) {
						offset.pos.set(-100, (interior_size.y / 2 + sprite.get_size(/*metas*/).x / 2) *  1);
						offset.rotation = 90;
					}
					
					components::fixtures group;

					group.filter = filters::see_through_dynamic_object();
					group.density = 1.0f;
					group.sensor = true;
					group.offsets_for_created_shapes[colliders_offset_type::SHAPE_OFFSET] = offset;
					group.material = assets::physical_material_id::METAL;

					engine_physical  += group;

					engine_physical.get<components::fixtures>().set_owner_body(front);
					engine_physical.add_standard_components(step);

					this_engine_transform = engine_physical.get_logic_transform();
				}

				const vec2 engine_size = metas.at(assets::game_image_id::TRUCK_ENGINE).get_size();

				{
					particles_existence_input input;
					
					input.effect.id = assets::particle_effect_id::ENGINE_PARTICLES;
					input.effect.modifier.colorize = cyan;
					input.effect.modifier.scale_amounts = 6.7f;
					input.effect.modifier.scale_lifetimes = 0.45f;
					input.delete_entity_after_effect_lifetime = false;

					auto place_of_birth = this_engine_transform;

					if (i == 0 || i == 1) {
						place_of_birth.rotation += 180;
					}

					const auto engine_particles = input.create_particle_effect_entity(
						step,
						place_of_birth,
						front
					);

					auto& existence = engine_particles.get<components::particles_existence>();
					existence.distribute_within_segment_of_length = engine_size.y;

					engine_particles.add_standard_components(step);

					if (i == 0) {
						front.get<components::car>().acceleration_engine[i].physical = engine_physical;
						front.get<components::car>().acceleration_engine[i].particles = engine_particles;
					}
					if (i == 1) {
						front.get<components::car>().acceleration_engine[i].physical = engine_physical;
						front.get<components::car>().acceleration_engine[i].particles = engine_particles;
					}
					if (i == 2) {
						front.get<components::car>().left_engine.physical = engine_physical;
						front.get<components::car>().left_engine.particles = engine_particles;
					}
					if (i == 3) {
						front.get<components::car>().right_engine.physical = engine_physical;
						front.get<components::car>().right_engine.particles = engine_particles;
					}

					components::particles_existence::deactivate(engine_particles);
				}
			}

			{
				sound_existence_input in;
				in.effect.id = assets::sound_buffer_id::ENGINE;
				in.effect.modifier.repetitions = -1;
				in.delete_entity_after_effect_lifetime = false;

				const auto engine_sound = in.create_sound_effect_entity(step, spawn_transform, front);
				engine_sound.add_standard_components(step);

				front.get<components::car>().engine_sound = engine_sound;
				components::sound_existence::deactivate(engine_sound);
			}
		}

		front.add_standard_components(step);
		left_wheel.add_standard_components(step);
		interior.add_standard_components(step);

		return front;
	}
}
#endif