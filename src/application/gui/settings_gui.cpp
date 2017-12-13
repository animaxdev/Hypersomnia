#include <cstring>

#include "augs/templates/introspect.h"
#include "augs/templates/enum_introspect.h"
#include "augs/templates/format_enum.h"

#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/imgui_utils.h"
#include "augs/audio/audio_context.h"

#include "augs/window_framework/platform_utils.h"
#include "augs/window_framework/window.h"

#include "view/necessary_resources.h"

#include "application/config_lua_table.h"
#include "application/gui/settings_gui.h"

static void ShowHelpMarker(const char* const desc) {
	ImGui::TextDisabled("(?)");

	if (ImGui::IsItemHovered()) {
		auto tooltip = augs::imgui::scoped_tooltip();

		ImGui::PushTextWrapPos(450.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
	}
}

static bool operator==(const ImVec2 a, const ImVec2 b) {
	return a.x == b.x && a.y == b.y;
};

void configuration_subscribers::sync_back_into(config_lua_table& into) const {
	window.sync_back_into(into.window);
}

void configuration_subscribers::apply(const config_lua_table& new_config) const {
	DEBUG_DRAWING = new_config.debug_drawing;
	
	const auto screen_size = window.get_screen_size();

	fbos.apply(screen_size, new_config.drawing);
	window.apply(new_config.window);
	audio_context.apply(new_config.audio);
}

#define CONFIG_NVP(x) format_field_name(std::string(#x)) + "##" + std::to_string(field_id++), config.x

void settings_gui_state::perform(
	sol::state& lua,
	const augs::path_type& config_path_for_saving,
	config_lua_table& config,
	config_lua_table& last_saved_config
) {
	if (!show) {
		return;
	}
	
	using namespace augs::imgui;

	{
		const auto screen_size = vec2(ImGui::GetIO().DisplaySize);
		const auto initial_settings_size = screen_size / 1.5;

		ImGui::SetNextWindowPos(ImVec2(screen_size / 2 - initial_settings_size / 2), ImGuiSetCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(initial_settings_size), ImGuiSetCond_FirstUseEver);
		ImGui::SetNextWindowSizeConstraints(ImVec2(initial_settings_size * 0.3), ImVec2(initial_settings_size * 2.0));
	}

	auto settings = scoped_window("Settings", &show);
	
	{
		auto child = scoped_child("settings view", ImVec2(0, -(ImGui::GetFrameHeightWithSpacing() + 4)));
		auto width = scoped_item_width(ImGui::GetWindowWidth() * 0.35f);

		int field_id = 0;

		{
			auto style = scoped_style_var(ImGuiStyleVar_FramePadding, []() { auto padding = ImGui::GetStyle().FramePadding; padding.x *= 4; return padding; }());

			{
				static auto labels = []() {
					static augs::enum_array<std::string, settings_pane> label_strs;
					augs::enum_array<const char*, settings_pane> c_strs;

					augs::for_each_enum_except_bounds([&c_strs](const settings_pane s) {
						label_strs[s] = format_enum(s);
						c_strs[s] = label_strs[s].c_str();
					});

					c_strs[settings_pane::GUI_STYLES] = "GUI styles";

					return c_strs;
				}();

				auto index = static_cast<int>(active_pane);
				ImGui::TabLabels(labels.data(), static_cast<int>(labels.size()), index, nullptr);
				active_pane = static_cast<settings_pane>(index);
			}
		}
		
		{
			auto scope = scoped_style_color(ImGuiCol_Separator, ImGui::GetStyle().Colors[ImGuiCol_Button]);
			ImGui::Separator();
		}

		auto revert = make_revert_button_lambda(config, last_saved_config);

		auto revertable_checkbox = [&](auto l, auto& f, auto&&... args) {
			checkbox(l, f, std::forward<decltype(args)>(args)...);
			revert(f);
		};

		auto revertable_slider = [&](auto l, auto& f, auto&&... args) {
			slider(l, f, std::forward<decltype(args)>(args)...);
			revert(f);
		};

		auto revertable_drag_rect_bounded_vec2 = [&](auto l, auto& f, auto&&... args) {
			drag_rect_bounded_vec2(l, f, std::forward<decltype(args)>(args)...);
			revert(f);
		};

		auto revertable_drag = [&](auto l, auto& f, auto&&... args) {
			drag(l, f, std::forward<decltype(args)>(args)...);
			revert(f);
		};

		auto revertable_color_edit = [&](auto l, auto& f, auto&&... args) {
			color_edit(l, f, std::forward<decltype(args)>(args)...);
			revert(f);
		};

		switch (active_pane) {
			case settings_pane::WINDOW: {
				enum_combo("Launch on game's startup", config.launch_mode);

				revertable_checkbox("Fullscreen", config.window.fullscreen);
				if (!config.window.fullscreen) {
					auto indent = scoped_indent();
					
					{
						const auto disp = augs::get_display();

						revertable_drag_rect_bounded_vec2("Window position", config.window.position, 0.3f, vec2i{ 0, 0 }, disp.get_size(), "%.0f");
						revertable_drag_rect_bounded_vec2("Windowed size", config.window.size, 0.3f, vec2i{ 0, 0 }, disp.get_size(), "%.0f");
					}

					revertable_checkbox(CONFIG_NVP(window.border));
				}

				{
					text("GUI cursor input source");

					auto indent = scoped_indent();

					int e = config.window.raw_mouse_input ? 1 : 0;
					ImGui::RadioButton("Raw (traps cursor inside the window)", &e, 1);
					
					if (ImGui::IsItemHovered()) {
						text_tooltip("Game draws its own cursor.\nWhen in GUI mode,\nforces the cursor inside the window.");
					}
#if TODO
					if (config.window.raw_mouse_input) {
						auto indent = scoped_indent();

						revertable_checkbox("But use system cursor for GUI", config.session.use_system_cursor_for_gui);
					}
#endif
					ImGui::RadioButton("System cursor", &e, 0);

					if (ImGui::IsItemHovered()) {
						text_tooltip("When in GUI mode,\nlets the cursor go outside the window.");
					}

					config.window.raw_mouse_input = e != 0;
				}

				input_text<100>(CONFIG_NVP(window.name)); revert(config.window.name);
				revertable_checkbox("Automatically hide settings in-game", config.session.automatically_hide_settings_ingame);

				break;
			}
			case settings_pane::GRAPHICS: {
				revertable_checkbox("Interpolate frames", config.interpolation.enabled);
				
				if (config.interpolation.enabled) {
					auto scope = scoped_indent();

					revertable_slider("Speed", config.interpolation.speed, 50.f, 1000.f);
				}

				revertable_checkbox("Highlight hovered world items", config.drawing.draw_aabb_highlighter);

				break;
			}
			case settings_pane::AUDIO: {
				revertable_slider("Music volume", config.audio_volume.music, 0.f, 1.f);
				revertable_slider("Sound effects volume", config.audio_volume.sound_effects, 0.f, 1.f);
				revertable_slider("GUI volume", config.audio_volume.gui, 0.f, 1.f);

				revertable_checkbox("Enable HRTF", config.audio.enable_hrtf);

				break;
			}
			case settings_pane::CONTROLS: {

				break;
			}
			case settings_pane::GAMEPLAY: {
				revertable_slider(CONFIG_NVP(camera.look_bound_expand), 0.2f, 2.f);
				
				revertable_checkbox(CONFIG_NVP(camera.enable_smoothing));

				if (config.camera.enable_smoothing) {
					auto indent = scoped_indent();

					revertable_slider(CONFIG_NVP(camera.smoothing.averages_per_sec), 0.f, 100.f); 
					revertable_slider(CONFIG_NVP(camera.smoothing.average_factor), 0.01f, 0.95f); 
				}

				revertable_checkbox("Draw weapon laser", config.drawing.draw_weapon_laser);
				revertable_checkbox("Draw crosshairs", config.drawing.draw_crosshairs);
				// revertable_checkbox("Draw gameplay GUI", config.drawing.draw_character_gui); revert(config.drawing.draw_character_gui);
				break;
			}
			case settings_pane::EDITOR: {
				if (auto node = scoped_tree_node("General")) {
					revertable_checkbox("Enable autosave", config.editor.autosave.enabled);
	
					if (config.editor.autosave.enabled) {
						auto scope = scoped_indent();
						text("Once per");
						ImGui::SameLine();
						revertable_drag("minutes", config.editor.autosave.once_every_min, 0.002f, 0.05f, 2000.f);
					}
					
					text("Remember last");
					ImGui::SameLine();

					revertable_drag("commands for undoing", config.editor.remember_last_n_commands, 1, 10, 2000);
				}	
				
				if (auto node = scoped_tree_node("Interface")) {
					revertable_drag("Camera panning speed", config.editor.camera_panning_speed, 0.001f, -10.f, 10.f);
				}

				if (auto node = scoped_tree_node("Appearance")) {
					revertable_color_edit("Controlled entity color", config.editor.controlled_entity_color);
					revertable_color_edit("Hovered entity color", config.editor.hovered_entity_color);
					revertable_color_edit("Selected entity color", config.editor.selected_entity_color);
					revertable_color_edit("Held entity color", config.editor.held_entity_color);

					revertable_color_edit("Rectangular selection color", config.editor.rectangular_selection_color);
					revertable_color_edit("Rectangular selection border color", config.editor.rectangular_selection_border_color);
				}

				break;
			}
			case settings_pane::GUI_STYLES: {
				text(
					"Note: this is the original ImGui style tweaker.\n"
					"It is used because imgui is being continuously improved,\n"
					"so keeping it up to date by ourselves would be pretty hard.\n\n"
					"To save your changes to the local configuration file,\n"
					"You need to push Save Ref and only then Save settings at the bottom."
				);
				
				ImGui::Separator();

				ImGuiStyle& style = config.gui_style;
				const ImGuiStyle& last_saved_style = last_saved_config.gui_style;

				// TODO: debug behaviour of this
				ImGui::ShowStyleEditor(&config.gui_style);

				break;
			}
			case settings_pane::DEBUG: {
				revertable_checkbox("Show developer console", config.session.show_developer_console);
				revertable_checkbox("Log keystrokes", config.window.log_keystrokes);
				revertable_slider("Camera query expansion", config.session.camera_query_expansion, -4.f, 0.9f);
				break;
			}
			default: {
				ensure(false && "Unknown settings pane type");
				break;
			}
		}
	}

	{
		const bool has_config_changed =
			!augs::equal_by_introspection(
				config,
				last_saved_config
			)
		;

		auto scope = scoped_child("save revert");

		ImGui::Separator();

		if (has_config_changed) {
			if (ImGui::Button("Save settings")) {
				augs::timer save_timer;
				last_saved_config = config;
				config.save(lua, config_path_for_saving);
				LOG("Saved new config in: %x ms", save_timer.get<std::chrono::milliseconds>());
			}

			ImGui::SameLine();

			if (ImGui::Button("Revert settings")) {
				config = last_saved_config;
				ImGui::GetStyle() = config.gui_style;
			}
		}
	}
}

#undef CONFIG_NVP
