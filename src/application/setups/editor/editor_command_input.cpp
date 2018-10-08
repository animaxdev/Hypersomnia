#include "application/intercosm.h"

#include "application/setups/editor/editor_command_input.h"
#include "application/setups/editor/editor_folder.h"

#include "application/setups/editor/gui/editor_entity_selector.h"
#include "application/setups/editor/gui/editor_entity_mover.h"

#include "application/setups/editor/gui/editor_fae_gui.h"
#include "application/setups/editor/editor_player.h"

editor_history& editor_command_input::get_history() const {
	return folder.history;
}

editor_player& editor_command_input::get_player() const {
	return folder.player;
}

augs::snapshotted_player_step_type editor_command_input::get_current_step() const {
	return get_player().get_current_step();
}

all_viewables_defs& editor_command_input::get_viewable_defs() const {
	return folder.commanded.work.viewables;
}

const all_logical_assets& editor_command_input::get_logical_assets() const {
	return get_cosmos().get_logical_assets();
}

cosmos& editor_command_input::get_cosmos() const {
	return folder.commanded.work.world;
}

void editor_command_input::interrupt_tweakers() const {
	fae_gui.interrupt_tweakers();
}

void editor_command_input::purge_selections() const {
	folder.commanded.view_ids.selected_entities.clear();
	selector.clear();
	mover.escape();
}

void editor_command_input::clear_selection_of(const entity_id id) const {
	erase_element(folder.commanded.view_ids.selected_entities, id);

	selector.clear_selection_of(id);
}
