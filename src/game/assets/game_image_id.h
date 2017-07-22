#pragma once
#include "game/container_sizes.h"

namespace assets {
	enum class game_image_id {
		// GEN INTROSPECTOR enum class assets::game_image_id
		INVALID,
		BLANK,

		DROP_HAND_ICON,

		GUI_CURSOR,
		GUI_CURSOR_RESIZE_NWSE,
		GUI_CURSOR_HOVER,
		GUI_CURSOR_TEXT_INPUT,
		GUI_CURSOR_ADD,
		GUI_CURSOR_MINUS,
		GUI_CURSOR_ERROR,

		MENU_GAME_LOGO,

		MENU_BUTTON_INSIDE,

		MENU_BUTTON_LT,
		MENU_BUTTON_RT,
		MENU_BUTTON_RB,
		MENU_BUTTON_LB,

		MENU_BUTTON_L,
		MENU_BUTTON_T,
		MENU_BUTTON_R,
		MENU_BUTTON_B,

		MENU_BUTTON_LB_COMPLEMENT,

		MENU_BUTTON_LT_BORDER,
		MENU_BUTTON_RT_BORDER,
		MENU_BUTTON_RB_BORDER,
		MENU_BUTTON_LB_BORDER,

		MENU_BUTTON_L_BORDER,
		MENU_BUTTON_T_BORDER,
		MENU_BUTTON_R_BORDER,
		MENU_BUTTON_B_BORDER,

		MENU_BUTTON_LB_COMPLEMENT_BORDER,

		MENU_BUTTON_LT_INTERNAL_BORDER,
		MENU_BUTTON_RT_INTERNAL_BORDER,
		MENU_BUTTON_RB_INTERNAL_BORDER,
		MENU_BUTTON_LB_INTERNAL_BORDER,

		HOTBAR_BUTTON_INSIDE,

		HOTBAR_BUTTON_LT,
		HOTBAR_BUTTON_RT,
		HOTBAR_BUTTON_RB,
		HOTBAR_BUTTON_LB,

		HOTBAR_BUTTON_L,
		HOTBAR_BUTTON_T,
		HOTBAR_BUTTON_R,
		HOTBAR_BUTTON_B,

		HOTBAR_BUTTON_LB_COMPLEMENT,

		HOTBAR_BUTTON_LT_BORDER,
		HOTBAR_BUTTON_RT_BORDER,
		HOTBAR_BUTTON_RB_BORDER,
		HOTBAR_BUTTON_LB_BORDER,

		HOTBAR_BUTTON_L_BORDER,
		HOTBAR_BUTTON_T_BORDER,
		HOTBAR_BUTTON_R_BORDER,
		HOTBAR_BUTTON_B_BORDER,

		HOTBAR_BUTTON_LB_COMPLEMENT_BORDER,

		HOTBAR_BUTTON_LT_INTERNAL_BORDER,
		HOTBAR_BUTTON_RT_INTERNAL_BORDER,
		HOTBAR_BUTTON_RB_INTERNAL_BORDER,
		HOTBAR_BUTTON_LB_INTERNAL_BORDER,

		ACTION_BUTTON_FILLED,
		ACTION_BUTTON_BORDER,

		LASER,
		LASER_GLOW_EDGE,

		CIRCULAR_BAR_MEDIUM,
		CIRCULAR_BAR_SMALL,

		CONTAINER_OPEN_ICON,
		CONTAINER_CLOSED_ICON,

		ATTACHMENT_CIRCLE_FILLED,
		ATTACHMENT_CIRCLE_BORDER,
		PRIMARY_HAND_ICON,
		SECONDARY_HAND_ICON,
		SHOULDER_SLOT_ICON,
		ARMOR_SLOT_ICON,
		CHAMBER_SLOT_ICON,
		DETACHABLE_MAGAZINE_SLOT_ICON,
		GUN_MUZZLE_SLOT_ICON,

		BLINK_1,
		BLINK_2,
		BLINK_3,
		BLINK_4,
		BLINK_5,
		BLINK_6,
		BLINK_7,

		WANDERING_CROSS,
		SPELL_BORDER,
		
		CAST_HIGHLIGHT,
		
		REQUISITE_COUNT,

#if BUILD_TEST_SCENES
		CRATE,
		CAR_FRONT,

		TRUCK_INSIDE,
		TRUCK_FRONT,

		JMIX114,
		
		TEST_CROSSHAIR,

		STANDARD_HEAD,

		SMOKE_1,
		SMOKE_2,
		SMOKE_3,
		SMOKE_4,
		SMOKE_5,
		SMOKE_6,

		PIXEL_THUNDER_1,
		PIXEL_THUNDER_2,
		PIXEL_THUNDER_3,
		PIXEL_THUNDER_4,
		PIXEL_THUNDER_5,

		ASSAULT_RIFLE,
		BILMER2000,
		KEK9,
		SN69,
		SUBMACHINE,
		URBAN_CYAN_MACHETE,
		ROCKET_LAUNCHER,

		TEST_BACKGROUND,

		SAMPLE_MAGAZINE,
		SMALL_MAGAZINE,
		SAMPLE_SUPPRESSOR,
		ROUND_TRACE,
		ENERGY_BALL,
		PINK_CHARGE,
		PINK_SHELL,
		CYAN_CHARGE,
		CYAN_SHELL,
		RED_CHARGE,
		RED_SHELL,
		GREEN_CHARGE,
		GREEN_SHELL,
		BACKPACK,

		CATHEDRAL_TILE_1,
		CATHEDRAL_TILE_2,
		CATHEDRAL_TILE_3,
		CATHEDRAL_TILE_4,
		CATHEDRAL_TILE_5,
		CATHEDRAL_TILE_6,
		CATHEDRAL_TILE_7,
		CATHEDRAL_TILE_8,
		CATHEDRAL_TILE_9,
		CATHEDRAL_TILE_10,
		CATHEDRAL_TILE_11,
		CATHEDRAL_TILE_12,
		CATHEDRAL_TILE_13,
		CATHEDRAL_TILE_14,
		CATHEDRAL_TILE_15,
		CATHEDRAL_TILE_16,
		CATHEDRAL_TILE_17,
		CATHEDRAL_TILE_18,
		CATHEDRAL_TILE_19,
		CATHEDRAL_TILE_20,
		CATHEDRAL_TILE_21,
		CATHEDRAL_TILE_22,
		CATHEDRAL_TILE_23,
		CATHEDRAL_TILE_24,
		CATHEDRAL_TILE_25,
		CATHEDRAL_TILE_26,
		CATHEDRAL_TILE_27,
		CATHEDRAL_TILE_28,
		CATHEDRAL_TILE_29,
		CATHEDRAL_TILE_30,
		CATHEDRAL_TILE_31,
		CATHEDRAL_TILE_32,
		CATHEDRAL_TILE_33,
		CATHEDRAL_TILE_34,
		CATHEDRAL_TILE_35,
		CATHEDRAL_TILE_36,
		CATHEDRAL_TILE_37,
		CATHEDRAL_TILE_38,
		CATHEDRAL_TILE_39,
		CATHEDRAL_TILE_40,
		CATHEDRAL_TILE_41,
		CATHEDRAL_TILE_42,
		CATHEDRAL_TILE_43,
		CATHEDRAL_TILE_44,
		CATHEDRAL_TILE_45,
		CATHEDRAL_TILE_46,
		CATHEDRAL_TILE_47,
		CATHEDRAL_TILE_48,
		CATHEDRAL_TILE_49,

		HAVE_A_PLEASANT,
		AWAKENING,
		METROPOLIS,

		BRICK_WALL,
		ROAD,
		ROAD_FRONT_DIRT,

		CAST_BLINK_1,
		CAST_BLINK_2,
		CAST_BLINK_3,
		CAST_BLINK_4,
		CAST_BLINK_5,
		CAST_BLINK_6,
		CAST_BLINK_7,
		CAST_BLINK_8,
		CAST_BLINK_9,
		CAST_BLINK_10,
		CAST_BLINK_11,
		CAST_BLINK_12,
		CAST_BLINK_13,
		CAST_BLINK_14,
		CAST_BLINK_15,
		CAST_BLINK_16,
		CAST_BLINK_17,
		CAST_BLINK_18,
		CAST_BLINK_19,

		TRUCK_ENGINE,

		HEALTH_ICON,
		PERSONAL_ELECTRICITY_ICON,
		CONSCIOUSNESS_ICON,

		AMPLIFIER_ARM,

		SPELL_HASTE_ICON,
		SPELL_GREATER_HASTE_ICON,
		SPELL_ELECTRIC_SHIELD_ICON,
		SPELL_ELECTRIC_MISSILE_ICON,
		SPELL_ELECTRIC_TRIAD_ICON,
		SPELL_FURY_OF_THE_AEONS_ICON,
		SPELL_ULTIMATE_WRATH_OF_THE_AEONS_ICON,
		SPELL_EXALTATION_ICON,
		SPELL_ECHOES_OF_THE_HIGHER_REALMS_ICON,

		PERK_HASTE_ICON,
		PERK_ELECTRIC_SHIELD_ICON,

		FORCE_GRENADE,
		PED_GRENADE,
		INTERFERENCE_GRENADE,

		FORCE_GRENADE_RELEASED,
		PED_GRENADE_RELEASED,
		INTERFERENCE_GRENADE_RELEASED,
		FORCE_ROCKET,
#endif
		COUNT = MAX_GAME_IMAGE_COUNT + 1
		// END GEN INTROSPECTOR
	};
}