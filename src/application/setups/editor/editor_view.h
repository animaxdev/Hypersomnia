#pragma once
#include <unordered_set>
#include <optional>

#include "augs/misc/time_utils.h"
#include "augs/math/camera_cone.h"

#include "game/transcendental/entity_id.h"

#include "augs/math/snapping_grid.h"

struct editor_folder_meta {
	// GEN INTROSPECTOR struct editor_folder_meta
	augs::date_time timestamp;
	// END GEN INTROSPECTOR
};

struct editor_view {
	// GEN INTROSPECTOR struct editor_view
	editor_folder_meta meta;
	snapping_grid grid;
	bool show_grid = true;

	std::unordered_set<entity_id> selected_entities;
	std::optional<camera_cone> panned_camera;
	// END GEN INTROSPECTOR

	void reset_zoom_at(vec2);
	void toggle_grid();
};
