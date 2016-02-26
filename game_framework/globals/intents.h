#pragma once

enum intent_type {
	NONE,

	THROW_PRIMARY_ITEM,
	THROW_SECONDARY_ITEM,

	HOLSTER_PRIMARY_ITEM,
	HOLSTER_SECONDARY_ITEM,

	START_PICKING_UP_ITEMS,

	USE_BUTTON,
	// contextual
	PRESS_WORLD_TRIGGER,
	RELEASE_CAR,

	SPACE_BUTTON,
	// contextual
	HAND_BRAKE,

	MOVE_FORWARD,
	MOVE_BACKWARD,
	MOVE_LEFT,
	MOVE_RIGHT,

	MOVE_CROSSHAIR,
	CROSSHAIR_PRIMARY_ACTION,
	CROSSHAIR_SECONDARY_ACTION,

	PRESS_GUN_TRIGGER,
	RELOAD,

	SWITCH_LOOK,
	SWITCH_TO_GUI,
	SWITCH_CHARACTER,
	ZOOM_CAMERA,
};