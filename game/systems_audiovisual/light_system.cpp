#include <array>
#include "light_system.h"
#include "game/view/viewing_step.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"
#include "game/view/viewing_session.h"
#include "game/components/light_component.h"
#include "game/components/polygon_component.h"

#include "game/resources/manager.h"
#include "game/messages/visibility_information.h"

#include "game/systems_stateless/visibility_system.h"
#include "game/systems_stateless/render_system.h"

#include "game/enums/filters.h"

void light_system::reserve_caches_for_entities(const size_t n) {
	per_entity_cache.resize(n);
}

light_system::cache::cache() {
	std::fill(all_variation_values.begin(), all_variation_values.end(), 0.f);
}

void light_system::advance_attenuation_variations(
	const cosmos& cosmos,
	const augs::delta dt
) {
	cosmos.for_each(
		processing_subjects::WITH_LIGHT,
		[&](const auto it) {
			const auto& light = it.get<components::light>();
			auto& cache = per_entity_cache[it.get_id().pool.indirection_index];

			const auto delta = dt.in_seconds();

			light.constant.variation.update_value(rng, cache.all_variation_values[0], delta);
			light.linear.variation.update_value(rng, cache.all_variation_values[1], delta);
			light.quadratic.variation.update_value(rng, cache.all_variation_values[2], delta);

			light.wall_constant.variation.update_value(rng, cache.all_variation_values[3], delta);
			light.wall_linear.variation.update_value(rng, cache.all_variation_values[4], delta);
			light.wall_quadratic.variation.update_value(rng, cache.all_variation_values[5], delta);

			light.position_variations[0].update_value(rng, cache.all_variation_values[6], delta);
			light.position_variations[1].update_value(rng, cache.all_variation_values[7], delta);
		}
	);
}

void light_system::render_all_lights(
	augs::renderer& output, 
	const std::array<float, 16> projection_matrix, 
	const viewing_step step, 
	std::function<void()> neon_callback
) const {
	const auto& cosmos = step.cosm;
	const float global_time_seconds = static_cast<float>(step.get_interpolated_total_time_passed_in_seconds());

	ensure_eq(0, output.get_triangle_count());

	output.light_fbo.use();
	glClearColor(0.1f, 0.2f, 0.2f, 1.0f);
	output.clear_current_fbo();
	glClearColor(0.f, 0.f, 0.f, 0.f);

	auto& light_program = *get_resource_manager().find(assets::program_id::LIGHT);
	auto& default_program = *get_resource_manager().find(assets::program_id::DEFAULT);
	light_program.use();

	const auto light_pos_uniform = glGetUniformLocation(light_program.id, "light_pos");
	const auto light_max_distance_uniform = glGetUniformLocation(light_program.id, "max_distance");
	const auto light_attenuation_uniform = glGetUniformLocation(light_program.id, "light_attenuation");
	const auto light_multiply_color_uniform = glGetUniformLocation(light_program.id, "multiply_color");
	const auto projection_matrix_uniform = glGetUniformLocation(light_program.id, "projection_matrix");
	const auto& interp = step.session.systems_audiovisual.get<interpolation_system>();
	const auto& particles = step.session.systems_audiovisual.get<particles_simulation_system>();
	
	const auto& visible_per_layer = step.visible.per_layer;

	std::vector<messages::visibility_information_request> requests;
	std::vector<messages::visibility_information_response> responses;

	glUniformMatrix4fv(projection_matrix_uniform, 1, GL_FALSE, projection_matrix.data());

	cosmos.for_each(
		processing_subjects::WITH_LIGHT,
		[&](const auto light_entity) {
			const auto& cache = per_entity_cache[light_entity.get_id().pool.indirection_index];
			const auto light_displacement = vec2(cache.all_variation_values[6], cache.all_variation_values[7]);

			messages::visibility_information_request request;
			request.eye_transform = light_entity.get_viewing_transform(interp);
			request.eye_transform.pos += light_displacement;
			request.filter = filters::line_of_sight_query();
			request.square_side = light_entity.get<components::light>().max_distance.base_value;
			request.subject = light_entity;

			requests.push_back(request);
		}
	);

	{
		std::vector<messages::line_of_sight_response> dummy;
		visibility_system().respond_to_visibility_information_requests(cosmos, {}, requests, dummy, responses);
	}

	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE); glerr;
	for (size_t i = 0; i < responses.size(); ++i) {
		const auto& r = responses[i];
		const auto& light_entity = cosmos[requests[i].subject];
		const auto& light = light_entity.get<components::light>();
		const auto world_light_pos = requests[i].eye_transform.pos;

		for (size_t t = 0; t < r.get_num_triangles(); ++t) {
			const auto world_light_tri = r.get_world_triangle(t, world_light_pos);
			augs::vertex_triangle renderable_light_tri;

			renderable_light_tri.vertices[0].pos = step.camera[world_light_tri[0]];
			renderable_light_tri.vertices[1].pos = step.camera[world_light_tri[1]];
			renderable_light_tri.vertices[2].pos = step.camera[world_light_tri[2]];

			auto considered_color = light.color;
			
			if (considered_color == black) {
				considered_color.set_hsv({ fmod(global_time_seconds / 16.f, 1.f), 1.0, 1.0 });
			}

			renderable_light_tri.vertices[0].color = considered_color;
			renderable_light_tri.vertices[1].color = considered_color;
			renderable_light_tri.vertices[2].color = considered_color;

			output.push_triangle(renderable_light_tri);
		}

		//for (size_t d = 0; d < r.get_num_discontinuities(); ++d) {
		//	const auto world_discontinuity = *r.get_discontinuity(d);
		//	
		//	if (!world_discontinuity.is_boundary) {
		//		vertex_triangle renderable_light_tri;
		//
		//		const float distance_from_light = (requests[i].eye_transform.pos - world_discontinuity.points.first).length();
		//		const float angle = 80.f / ((distance_from_light+0.1f)/50.f);
		//		
		//		//(requests[i].eye_transform.pos - world_discontinuity.points.first).length();
		//
		//		if (world_discontinuity.winding == world_discontinuity.RIGHT) {
		//			renderable_light_tri.vertices[0].pos = world_discontinuity.points.first + camera_offset;
		//			renderable_light_tri.vertices[1].pos = world_discontinuity.points.second + camera_offset;
		//			renderable_light_tri.vertices[2].pos = vec2(world_discontinuity.points.second).rotate(-angle, world_discontinuity.points.first) + camera_offset;
		//		}
		//		else {
		//			renderable_light_tri.vertices[0].pos = world_discontinuity.points.first + camera_offset;
		//			renderable_light_tri.vertices[1].pos = world_discontinuity.points.second + camera_offset;
		//			renderable_light_tri.vertices[2].pos = vec2(world_discontinuity.points.second).rotate(angle, world_discontinuity.points.first) + camera_offset;
		//		}
		//
		//		renderable_light_tri.vertices[0].color = light.color;
		//		renderable_light_tri.vertices[1].color = light.color;
		//		renderable_light_tri.vertices[2].color = light.color;
		//
		//		output.push_triangle(renderable_light_tri);
		//	}
		//}

		const auto& cache = per_entity_cache[light_entity.get_id().pool.indirection_index];

		const auto light_frag_pos = step.camera.get_screen_space_revert_y(world_light_pos);

		glUniform2f(light_pos_uniform, light_frag_pos.x, light_frag_pos.y);

		glUniform1f(light_max_distance_uniform, light.max_distance.base_value);
		
		glUniform3f(light_attenuation_uniform,
			cache.all_variation_values[0] + light.constant.base_value,
			cache.all_variation_values[1] + light.linear.base_value,
			cache.all_variation_values[2] + light.quadratic.base_value
		);

		glUniform3f(light_multiply_color_uniform,
			1.f,
			1.f,
			1.f);

		output.call_triangles();
		output.clear_triangles();
		
		light_program.use();

		glUniform1f(light_max_distance_uniform, light.wall_max_distance.base_value);
		
		glUniform3f(light_attenuation_uniform,
			cache.all_variation_values[3] + light.wall_constant.base_value,
			cache.all_variation_values[4] + light.wall_linear.base_value,
			cache.all_variation_values[5] + light.wall_quadratic.base_value
		);
		
		glUniform3f(light_multiply_color_uniform,
			light.color.r/255.f,
			light.color.g/255.f,
			light.color.b/255.f);
		
		render_system().draw_entities(interp, global_time_seconds,output.triangles, cosmos, visible_per_layer[render_layer::DYNAMIC_BODY], step.camera, renderable_drawing_type::NORMAL);

		output.call_triangles();
		output.clear_triangles();

		glUniform3f(light_multiply_color_uniform,
			1.f,
			1.f,
			1.f);
	}

	default_program.use();

	render_system().draw_entities(interp, global_time_seconds,output.triangles, cosmos, visible_per_layer[render_layer::DYNAMIC_BODY], step.camera, renderable_drawing_type::NEON_MAPS);
	render_system().draw_entities(interp, global_time_seconds,output.triangles, cosmos, visible_per_layer[render_layer::SMALL_DYNAMIC_BODY], step.camera, renderable_drawing_type::NEON_MAPS);
	render_system().draw_entities(interp, global_time_seconds,output.triangles, cosmos, visible_per_layer[render_layer::FLYING_BULLETS], step.camera, renderable_drawing_type::NEON_MAPS);
	render_system().draw_entities(interp, global_time_seconds,output.triangles, cosmos, visible_per_layer[render_layer::CAR_INTERIOR], step.camera, renderable_drawing_type::NEON_MAPS);
	render_system().draw_entities(interp, global_time_seconds,output.triangles, cosmos, visible_per_layer[render_layer::CAR_WHEEL], step.camera, renderable_drawing_type::NEON_MAPS);
	render_system().draw_entities(interp, global_time_seconds,output.triangles, cosmos, visible_per_layer[render_layer::EFFECTS], step.camera, renderable_drawing_type::NEON_MAPS);
	render_system().draw_entities(interp, global_time_seconds,output.triangles, cosmos, visible_per_layer[render_layer::ON_GROUND], step.camera, renderable_drawing_type::NEON_MAPS);
	render_system().draw_entities(interp, global_time_seconds,output.triangles, cosmos, visible_per_layer[render_layer::ON_TILED_FLOOR], step.camera, renderable_drawing_type::NEON_MAPS);

	{
		particles.draw(
			output.triangles,
			render_layer::EFFECTS,
			step.camera,
			renderable_drawing_type::NEON_MAPS
		);
	}

	neon_callback();

	output.call_triangles();
	output.clear_triangles();

	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE); glerr;

	augs::graphics::fbo::use_default();

	output.set_active_texture(2);
	output.bind_texture(output.light_fbo);
	output.set_active_texture(0);
}