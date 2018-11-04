#pragma once
#include <optional>

#include "augs/math/vec2.h"
#include "augs/math/transform.h"

struct editor_history;
class editor_setup;

struct entity_mover_input {
	editor_setup& setup;
};

enum class mover_op_type {
	NONE,

	TRANSLATING,
	ROTATING,
	RESIZING
};

class editor_entity_mover {
	bool active = false;
	vec2 initial_world_cursor_pos;

	void transform_selection(
		entity_mover_input in,
		std::optional<vec2> rotation_center,
		std::optional<transformr> one_shot_delta
	);

	void start_transforming_selection(
		entity_mover_input in,
		std::optional<vec2> rotation_center
	);

public:
	bool escape();
	bool is_active(const editor_history&) const;
	mover_op_type get_current_op(const editor_history&) const;

	void start_moving_selection(entity_mover_input in);
	void start_rotating_selection(entity_mover_input in);
	void rotate_selection_once_by(entity_mover_input in, int degrees);

	void start_resizing_selection(entity_mover_input in, bool both_axes_simultaneously);

	vec2* current_mover_pos_delta(entity_mover_input in) const;
	float* current_mover_rot_delta(entity_mover_input in) const;

	bool do_mousemotion(entity_mover_input in, vec2 world_cursor_pos);
	bool do_left_press(entity_mover_input in);
};

