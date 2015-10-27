#pragma once
#include "stdafx.h"
#include "bindings/bindings.h"

#include "misc/vector_wrapper.h"

#include "world_instance.h"
#include "game_framework.h"
#include "utilities/script.h"

#include <fstream>
#include <iostream>

int bitflag(lua_State* L) {
	int result = 1 << luabind::object_cast<int>(luabind::object(luabind::from_stack(L, 1)));
	lua_pushinteger(L, result);
	return 1;
}

namespace bindings {
	extern luabind::scope
		_audio(),

		_id_generator(),
		_minmax(),
		_vec2(),
		_value_animator(),
		_b2Filter(),
		_rgba(),
		_rect_ltrb(),
		_rect_xywh(),
		_glwindow(),
		_script(),
		_texture(),
		_animation(),
		_world(),
		_sprite(),
		_polygon(),
		_polygon_fader(),

		_timer(),

		_network_binding(),

		_particle(),
		_emission(),
		_particle_effect(),

		_message(),
		_intent_message(),
		_animate_message(),
		_particle_burst_message(),
		_collision_message(),
		_damage_message(),
		_destroy_message(),
		_shot_message(),

		_render_component(),
		_transform_component(),
		_visibility_component(),
		_pathfinding_component(),
		_animate_component(),
		_camera_component(),
		_chase_component(),
		_children_component(),
		_crosshair_component(),
		_damage_component(),
		_gun_component(),
		_input_component(),
		_lookat_component(),
		_movement_component(),
		_particle_emitter_component(),
		_physics_component(),
		_steering_component(),
		_behaviour_tree_component(),

		_entity(),
		_body_helper(),

		_opengl_binding(),
		_random_binding(),
		_all_systems(),

		_text(),

		_file_watcher(),

		_utilities()
		;
}

double get_meters_to_pixels() {
	return METERS_TO_PIXELS;
}

void set_meters_to_pixels(double val) {
	METERS_TO_PIXELS = val;
	PIXELS_TO_METERS = 1.0 / METERS_TO_PIXELS;
	METERS_TO_PIXELSf = float(val);
	PIXELS_TO_METERSf = 1.0f / METERS_TO_PIXELSf;
}

void framework::bind_whole_engine(augs::lua_state_wrapper& wrapper) {
	using namespace resources;
	using namespace helpers;

	auto& raw = wrapper.raw;
	luabind::open(raw);

	lua_register(raw, "bitflag", bitflag);
	luabind::module(raw)[
		luabind::class_<ptr_wrapper<float>>("float_ptr"),

			misc::vector_wrapper<float>::bind("float_vector"),

			bindings::_id_generator(),
			bindings::_minmax(),
			bindings::_vec2(),
			misc::vector_wrapper<vec2<>>::bind_vector("vec2_vector"),
			misc::vector_wrapper<int>::bind("int_vector"),
			bindings::_value_animator(),
			bindings::_b2Filter(),
			bindings::_rgba(),
			bindings::_rect_ltrb(),
			bindings::_rect_xywh(),
			bindings::_glwindow(),
			bindings::_script(),
			bindings::_texture(),
			bindings::_animation(),
			bindings::_world(),
			misc::vector_wrapper<entity_id>::bind_vector("entity_ptr_vector"),
			bindings::_sprite(),
			bindings::_polygon(),

			bindings::_timer(),

			bindings::_network_binding(),

			bindings::_particle(),
			bindings::_emission(),
			bindings::_particle_effect(),

			bindings::_message(),
			bindings::_intent_message(),
			bindings::_animate_message(),
			bindings::_particle_burst_message(),
			bindings::_collision_message(),
			bindings::_damage_message(),
			bindings::_destroy_message(),
			bindings::_shot_message(),

			bindings::_transform_component(),
			bindings::_render_component(),
			bindings::_visibility_component(),
			bindings::_pathfinding_component(),
			bindings::_animate_component(),
			bindings::_camera_component(),
			bindings::_chase_component(),
			bindings::_children_component(),
			bindings::_crosshair_component(),
			bindings::_damage_component(),
			bindings::_gun_component(),
			bindings::_input_component(),
			bindings::_lookat_component(),
			bindings::_movement_component(),
			bindings::_particle_emitter_component(),
			bindings::_physics_component(),
			bindings::_steering_component(),
			bindings::_behaviour_tree_component(),

			bindings::_entity(),
			bindings::_body_helper(),

			bindings::_opengl_binding(),

			luabind::def("clamp", &augs::get_clamp<float>),
			bindings::_random_binding(),

			luabind::def("open_editor", lua_state_wrapper::open_editor),
			luabind::def("get_meters_to_pixels", get_meters_to_pixels),
			luabind::def("set_meters_to_pixels", set_meters_to_pixels),
			luabind::def("get_executable_path", window::get_executable_path),
			luabind::def("remove_filename_from_path", window::remove_filename_from_path),

			luabind::class_<std::string>("std_string")
			.def("c_str", &std::string::c_str)
			,

			misc::vector_wrapper<std::string>::bind("string_vector"),
			bindings::_file_watcher(),

			bindings::_polygon_fader(),
			bindings::_all_systems(),

			world_instance::bind(),

			bindings::_text(),

			bindings::_utilities()
	];

	wrapper.global("THIS_LUA_STATE", wrapper);

	//luabind::bind_class_info(raw);
}