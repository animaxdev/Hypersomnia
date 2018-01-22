#include "ingredients.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

#include "game/enums/item_category.h"
#include "game/assets/all_logical_assets.h"

#include "game/detail/inventory/perform_transfer.h"
#include "game/components/container_component.h"
#include "game/components/item_component.h"

#include "game/transcendental/entity_handle.h"

namespace ingredients {
	void add_backpack_container(entity_handle e) {
		auto& container = e += components::container();
		auto& item = make_item(e);
		
		inventory_slot slot_def;
		slot_def.space_available = to_space_units("20");

		container.slots[slot_function::ITEM_DEPOSIT] = slot_def;

		item.dual_wield_accuracy_loss_multiplier = 1;
		item.dual_wield_accuracy_loss_percentage = 50;
		item.space_occupied_per_charge = to_space_units("1");
		item.categories_for_slot_compatibility.set(item_category::SHOULDER_CONTAINER);
	}
}

namespace test_types {
	void populate_backpack_types(const all_logical_assets& logicals, entity_types& types) {
		{
			auto& meta = get_test_type(types, test_scene_type::SAMPLE_BACKPACK);

			definitions::render render_def;
			render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

			meta.set(render_def);

			test_types::add_sprite(meta, logicals, assets::game_image_id::BACKPACK, white);
			meta.add_shape_definition_from_renderable(logicals);
			test_types::add_see_through_dynamic_body(meta);
		}
	}
}

namespace prefabs {
	entity_handle create_sample_backpack(const logic_step step, vec2 pos) {
		auto& world = step.get_cosmos();
		const auto backpack = create_test_scene_entity(world, test_scene_type::SAMPLE_BACKPACK);
		const auto& metas = step.get_logical_assets();
		
		ingredients::add_backpack_container(backpack);

		backpack.set_logic_transform(step, pos);
		backpack.add_standard_components(step);
		return backpack;
	}
}