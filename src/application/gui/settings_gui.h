#pragma once
#include "augs/filesystem/path.h"

struct config_lua_table;

enum class settings_pane {
	// GEN INTROSPECTOR enum class settings_pane
	WINDOW,
	GRAPHICS,
	AUDIO,
	CONTROLS,
	GAMEPLAY,
	NETWORK,
	EDITOR,
	GUI_STYLES,
	ADVANCED,

	COUNT
	// END GEN INTROSPECTOR
};

class settings_gui_state {
	settings_pane active_pane = settings_pane::WINDOW;

public:
	bool show = false;
	
	void perform(
		sol::state& lua,
		const augs::path_type& path_for_saving,
		config_lua_table& into,
		config_lua_table& last_saved,
		vec2i screen_size
	);
};

namespace augs {
	class window;
	class audio_context;
}

struct all_necessary_fbos;
struct all_necessary_shaders;

struct all_necessary_sounds;
struct necessary_image_definitions_map;

struct configuration_subscribers {
	augs::window& window;
	all_necessary_fbos& fbos;
	augs::audio_context& audio_context;

#if TODO
	augs::renderer& renderer;
	all_necessary_shaders& shaders;
	const all_necessary_sounds& sounds;
	const necessary_image_definitions_map& images;
#endif

	void apply(const config_lua_table&) const;
	void sync_back_into(config_lua_table&) const;
};