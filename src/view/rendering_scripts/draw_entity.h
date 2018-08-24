#pragma once
#include "augs/ensure.h"
#include "augs/drawing/drawing.h"
#include "augs/drawing/sprite_draw.h"
#include "augs/build_settings/platform_defines.h"

#include "game/enums/render_layer.h"

#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/detail/entity_handle_mixins/for_each_slot_and_item.hpp"
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"

#include "game/components/polygon_component.h"
#include "game/components/sprite_component.h"
#include "game/detail/view_input/particle_effect_input.h"
#include "game/components/render_component.h"
#include "game/components/remnant_component.h"

#include "game/detail/physics/physics_scripts.h"
#include "game/detail/frame_calculation.h"

#include "view/viewables/all_viewables_declaration.h"
#include "view/viewables/images_in_atlas_map.h"
#include "view/audiovisual_state/systems/randomizing_system.h"
#include "view/rendering_scripts/draw_entity_input.h"

using entities_with_renderables = entity_types_having_any_of<
	invariants::sprite,
	invariants::torso,
	invariants::gun,
   	invariants::polygon,
	invariants::animation
>;

template <bool for_gui = false, class E, class T>
FORCE_INLINE void detail_specific_entity_drawer(
	const cref_typed_entity_handle<E> typed_handle,
	const specific_draw_input& in,
	T render_visitor,
	const transformr viewing_transform
) {
	/* Might or might not be used depending on if constexpr flow */
	(void)render_visitor;
	(void)viewing_transform;
	(void)in;

	if constexpr(typed_handle.template has<invariants::sprite>()) {
		const auto sprite = [&typed_handle]() {
			auto result = typed_handle.template get<invariants::sprite>();

			if constexpr(typed_handle.template has<components::overridden_size>()) {
				const auto& s = typed_handle.template get<components::overridden_size>().get();

				if (s.is_enabled) {
					result.size = s.value;
				}
			}

			return result;
		}();

		auto input = [&]() {
			auto result = in.make_input_for<invariants::sprite>();

			if (const auto flips = typed_handle.calculate_flip_flags()) { 
				result.flip = *flips;
			}

			result.renderable_transform = viewing_transform;
			result.global_time_seconds = in.global_time_seconds;

			{
				const auto& v = sprite.neon_intensity_vibration;

				if (v.is_enabled && sprite.vibrate_diffuse_as_well) {
					const auto id = typed_handle.get_id();
					const auto mult = in.randomizing.advance_and_get_neon_mult(id, v.value);

					result.colorize.multiply_alpha(mult);
				}
			}

			if constexpr(typed_handle.template has<components::sprite>()) {
				const auto& sprite_comp = typed_handle.template get<components::sprite>();
				result.global_time_seconds += sprite_comp.effect_offset_secs;

				if (sprite_comp.colorize != white) {
					result.colorize *= sprite_comp.colorize;
				}
			}

			return result;
		}();

		if constexpr(!for_gui) {
			if constexpr(typed_handle.template has<components::hand_fuse>()) {
				const auto& fuse = typed_handle.template get<components::hand_fuse>();
				const auto& fuse_def = typed_handle.template get<invariants::hand_fuse>();

				const auto& logicals = typed_handle.get_cosmos().get_logical_assets();

				const auto& animation = typed_handle.template get<components::animation>();

				if (fuse.defused()) {
					if (fuse_def.defused_image_id.is_set()) {
						auto defused = sprite;
						defused.image_id = fuse_def.defused_image_id;

						render_visitor(defused, in.manager, input);
					}

					return;
				}

				if (fuse.when_armed.was_set()) {
					if (const auto displayed_frame = ::find_frame(fuse_def.armed_animation_id, animation, logicals)) {
						const auto image_id = displayed_frame->image_id;

						auto animated = sprite;
						animated.image_id = image_id;
						animated.size = in.manager.at(image_id).get_original_size();

						render_visitor(animated, in.manager, input);
						return;
					}
				}
			}

			if constexpr(typed_handle.template has<invariants::animation>()) {
				const auto& animation_def = typed_handle.template get<invariants::animation>();
				const auto& animation = typed_handle.template get<components::animation>();

				const auto& logicals = typed_handle.get_cosmos().get_logical_assets();

				if (const auto displayed_frame = ::find_frame(animation_def, animation, logicals)) {
					const auto image_id = displayed_frame->image_id;

					auto animated = sprite;
					animated.image_id = image_id;
					animated.size = in.manager.at(image_id).get_original_size();

					render_visitor(animated, in.manager, input);
					return;
				}
			}

			if constexpr(typed_handle.template has<components::trace>()) {
				const auto& trace = typed_handle.template get<components::trace>();

				if (trace.enabled) {
					const auto tracified_size = vec2(sprite.size) * trace.last_size_mult;

					if (const auto center_offset = tracified_size * trace.last_center_offset_mult;
						center_offset.non_zero()
					) {
						const auto final_rotation = input.renderable_transform.rotation;
						input.renderable_transform.pos -= vec2(center_offset).rotate(final_rotation);
					}

					auto tracified_sprite = sprite;
					tracified_sprite.size = tracified_size;

					render_visitor(tracified_sprite, in.manager, input);
					return;
				}
			}

			if constexpr(typed_handle.template has<components::remnant>()) {
				const auto& remnant = typed_handle.template get<components::remnant>();

				auto remnanted_sprite = sprite;
				remnanted_sprite.size *= remnant.last_size_mult;

				render_visitor(remnanted_sprite, in.manager, input);
				return;
			}

			if constexpr(typed_handle.template has<components::gun>()) {
				const auto& gun = typed_handle.template get<components::gun>();
				const auto& gun_def = typed_handle.template get<invariants::gun>();

				const auto& cosm = typed_handle.get_cosmos();
				const auto& logicals = cosm.get_logical_assets();

				if (const auto shoot_animation = logicals.find(gun_def.shoot_animation)) {
					if (const auto* const frame = ::find_frame(gun, *shoot_animation, cosm)) {
						auto animated = sprite;
						animated.image_id = frame->image_id;
						animated.size = in.manager.at(frame->image_id).get_original_size();

						render_visitor(animated, in.manager, input);
						return;
					}
				}
			}
		}

		render_visitor(sprite, in.manager, input);
		return;
	}

	if constexpr(typed_handle.template has<invariants::polygon>()) {
		auto input = in.make_input_for<invariants::polygon>();

		input.renderable_transform = viewing_transform;
		input.global_time_seconds = in.global_time_seconds;

		const auto& polygon = typed_handle.template get<invariants::polygon>();
		render_visitor(*polygon, in.manager, input);
		return;
	}
}

template <class E, class T>
FORCE_INLINE void specific_entity_drawer(
	const cref_typed_entity_handle<E> typed_handle,
	const draw_renderable_input& in,
	T render_visitor
) {
	/* Might or might not be used depending on if constexpr flow */
	(void)render_visitor;
	(void)in;

	const auto viewing_transform = typed_handle.get_viewing_transform(in.interp);

	if (typed_handle.template has<invariants::item>()) {
		if (typed_handle.get_owning_transfer_capability().alive()) {
			/* Will be drawn when the capability itself is drawn. */
			return;
		}
	}

	if (const auto maybe_torso = typed_handle.template find<invariants::torso>()) {
		if (const auto movement = typed_handle.template find<components::movement>()) {
			const auto& cosm = typed_handle.get_cosmos();
			const auto& logicals = cosm.get_logical_assets();

			const auto velocity = typed_handle.get_effective_velocity();

			const auto face_degrees = viewing_transform.rotation;

			auto render_frame = [&in, &render_visitor, &typed_handle](
				const auto& frame,
				const transformr where
			) {
				invariants::sprite sprite;
				sprite.set(frame.frame.image_id, in.manager);
				const auto& original_sprite = typed_handle.template get<invariants::sprite>();
				sprite.color = original_sprite.color;

				auto input = in.make_input_for<invariants::sprite>();
				input.renderable_transform = where;

				input.flip.vertically = frame.flip;
				render_visitor(sprite, in.manager, input);
			};

			const auto wielded_items = typed_handle.get_wielded_items();
			const auto stance_id = ::calc_stance_id(cosm, wielded_items);
			const auto& stance = maybe_torso->stances[stance_id];

			auto four_ways = movement->four_ways_animation;

			if (movement->was_walk_effective) {
				four_ways.index = 0;
			}

			if (const auto stance_usage = calc_stance_usage(
				cosm,
				stance, 
				four_ways,
				wielded_items
			)) {
				const auto stance_offsets = [&stance_usage, &logicals]() {
					const auto stance_image_id = stance_usage.frame->image_id;

					auto result = logicals.get_offsets(stance_image_id).torso;

					if (stance_usage.flip) {
						result.flip_vertically();
					}

					return result;
				}();

				{
					const auto leg_animation_id = maybe_torso->calc_leg_anim(velocity, face_degrees);

					if (const auto animation = logicals.find(leg_animation_id);
						animation != nullptr
					) {
						const auto legs_degrees = velocity.degrees();

						const auto& legs = stance_offsets.legs;
						const auto leg_offset = transformr(legs.pos, legs.rotation);

						render_frame(
							::get_frame_and_flip(four_ways, *animation),
							{ (viewing_transform * leg_offset).pos, legs_degrees }
						);
					}
				}

				/* Draw heavy items first. */

				std::optional<entity_id> drawn_heavy_item;

				if (stance_id == item_holding_stance::HEAVY_LIKE) {
					const auto heavy_item = cosm[wielded_items[0]];

					typed_handle.for_each_attachment_recursive(
						[&](
							const auto attachment_entity,
							const auto attachment_offset
						) {
							if (heavy_item.get_id() == attachment_entity.get_id()) {
								attachment_entity.template dispatch_on_having_all<invariants::item>(
									[&](const auto typed_attachment_handle) {
										detail_specific_entity_drawer(
											typed_attachment_handle,
											in,
											render_visitor,
											viewing_transform * attachment_offset
										);
									}
								);

								drawn_heavy_item = heavy_item.get_id();
								return;
							}
						},
						[stance_offsets]() {
							return stance_offsets;
						},
						attachment_offset_settings::for_rendering()
					);
				}

				const bool only_secondary = typed_handle.only_secondary_holds_item();

				{
					/* Draw the actual torso */
					auto usage = stance_usage;

					if (only_secondary) {
						auto& f = usage.flip;
						f = !f;
					}

					render_frame(usage.get_with_flip(), { viewing_transform.pos, face_degrees });
				}

				typed_handle.for_each_attachment_recursive(
					[&](
						const auto attachment_entity,
						const auto attachment_offset
					) {
						if (drawn_heavy_item != std::nullopt && drawn_heavy_item.value() == attachment_entity.get_id()) {
							return;
						}

						attachment_entity.template dispatch_on_having_all<invariants::item>(
							[&](const auto typed_attachment_handle) {
								detail_specific_entity_drawer(
									typed_attachment_handle,
									in,
									render_visitor,
									viewing_transform * attachment_offset
								);
							}
						);
					},
					[stance_offsets]() {
						return stance_offsets;
					},
					attachment_offset_settings::for_rendering()
				);

				if constexpr(typed_handle.template has<components::head>()) {
					const auto& head = typed_handle.template get<components::head>();
					const auto& head_def = typed_handle.template get<invariants::head>();

					const auto target_image = stance_usage.is_shooting ? head_def.shooting_head_image : head_def.head_image;

					const auto& head_offsets = logicals.get_offsets(target_image);

					auto stance_offsets_for_head = stance_offsets;
					auto anchor_for_head = head_offsets.item.head_anchor;

					if (only_secondary) {
						stance_offsets_for_head.flip_vertically();
						anchor_for_head.flip_vertically();
					}

					const auto target_offset = ::get_anchored_offset(stance_offsets_for_head.head, anchor_for_head);
					const auto target_transform = viewing_transform * target_offset;

					invariants::sprite sprite;
					sprite.set(target_image, in.manager);

					auto input = in.make_input_for<invariants::sprite>();
					input.renderable_transform = target_transform;
					input.renderable_transform.rotation += head.shake_rotation_amount;

					render_visitor(sprite, in.manager, input);
				}
			}
		}

		return;
	}

	detail_specific_entity_drawer(
		typed_handle,
		in,
		render_visitor,
		viewing_transform
	);
}

template <class E>
FORCE_INLINE void specific_draw_entity(
	const cref_typed_entity_handle<E> typed_handle,
	const draw_renderable_input& in
) {
	auto callback = [](const auto& renderable, auto&&... args) {
		augs::draw(renderable, std::forward<decltype(args)>(args)...);
	};

	specific_entity_drawer(typed_handle, in, callback);
}

template <class E, class border_provider>
FORCE_INLINE void specific_draw_border(
	const cref_typed_entity_handle<E> typed_handle,
	const draw_renderable_input& in,
	const border_provider& borders
) {
	if (const auto border_info = borders(typed_handle)) {
		auto border_maker = [border_info](const auto& renderable, const auto& manager, auto& input) {
			const auto source_pos = input.renderable_transform.pos;
			input.colorize = *border_info;

			const vec2i offsets[4] = {
				vec2i(-1, 0), vec2i(1, 0), vec2i(0, 1), vec2i(0, -1)
			};

			for (const auto o : offsets) {
				input.renderable_transform.pos = source_pos + o;
				augs::draw(renderable, manager, input);
			}
		};

		specific_entity_drawer(typed_handle, in, border_maker);
	}
}

template <class E>
FORCE_INLINE void specific_draw_color_highlight(
	const cref_typed_entity_handle<E> typed_handle,
	const rgba color,
	const draw_renderable_input& in
) {
	auto highlight_maker = [color](auto renderable, const auto& manager, const auto& input) {
		renderable.set_color(color);
		augs::draw(renderable, manager, input);
	};

	specific_entity_drawer(typed_handle, in, highlight_maker);
}

template <class E>
FORCE_INLINE void specific_draw_neon_map(
	const cref_typed_entity_handle<E> typed_handle,
	const draw_renderable_input& in
) {
	if constexpr(typed_handle.template has<components::sprite>()) {
		if (typed_handle.template get<components::sprite>().disable_neon_map) {
			return;
		}
	}

	auto neon_maker  = [&](const auto& renderable, const auto& manager, auto& input) {
		input.use_neon_map = true;

		if constexpr(typed_handle.template has<components::sprite>()) {
			const auto colorize = typed_handle.template get<components::sprite>().colorize_neon;

			if (colorize != white) {
				input.colorize *= colorize;
			}
		}

		{
			const auto& v = renderable.neon_intensity_vibration;

			if (v.is_enabled) {
				const auto id = typed_handle.get_id();
				const auto mult = in.randomizing.advance_and_get_neon_mult(id, v.value);

				input.colorize.multiply_alpha(mult);
			}
		}

		augs::draw(renderable, manager, input);
	};

	specific_entity_drawer(typed_handle, in, neon_maker);
}

/* Dispatching helpers */

FORCE_INLINE void draw_entity(
	const const_entity_handle handle,
	const draw_renderable_input& in
) {
	handle.conditional_dispatch<entities_with_renderables>([&in](const auto typed_handle) {
		specific_draw_entity(typed_handle, in);
	});
}

FORCE_INLINE void draw_neon_map(
	const const_entity_handle handle,
	const draw_renderable_input& in
) {
	handle.conditional_dispatch<entities_with_renderables>([&in](const auto typed_handle) {
		specific_draw_neon_map(typed_handle, in);
	});
}

FORCE_INLINE void draw_color_highlight(
	const const_entity_handle handle,
	const rgba color,
	const draw_renderable_input& in
) {
	handle.conditional_dispatch<entities_with_renderables>([&in, color](const auto typed_handle) {
		specific_draw_color_highlight(typed_handle, color, in);
	});
}

template <class B>
FORCE_INLINE void draw_border(
	const const_entity_handle handle,
	const draw_renderable_input& in,
	B&& borders
) {
	handle.conditional_dispatch<entities_with_renderables>([&in, &borders](const auto typed_handle) {
		specific_draw_border(typed_handle, in, std::forward<B>(borders));
	});
}
