#pragma once
#include <vector>
#include <variant>

#include "augs/pad_bytes.h"
#include "augs/templates/exception_templates.h"

#include "augs/math/vec2.h"
#include "augs/graphics/rgba.h"

#include "augs/filesystem/path.h"

namespace augs {
	struct image_loading_error : error_with_typesafe_sprintf {
		using error_with_typesafe_sprintf::error_with_typesafe_sprintf;
	};

	struct paint_circle_midpoint_command {
		// GEN INTROSPECTOR struct augs::paint_circle_midpoint_command
		unsigned radius = 0u;
		unsigned border_width = 1;
		bool scale_alpha = false;
		bool constrain_angle = false;
		pad_bytes<2> pad;
		float angle_start = 0.f;
		float angle_end = 0.f;
		rgba filling = white;
		// END GEN INTROSPECTOR

		static const char* get_custom_type_name() {
			return "circle_midpoint";
		}
	};

	struct paint_circle_filled_command {
		// GEN INTROSPECTOR struct augs::paint_circle_filled_command
		unsigned radius = 0u;
		rgba filling = white;
		// END GEN INTROSPECTOR

		static const char* get_custom_type_name() {
			return "circle_filled";
		}
	};

	struct paint_line_command {
		// GEN INTROSPECTOR struct augs::paint_line_command
		vec2u from;
		vec2u to;
		rgba filling = white;
		// END GEN INTROSPECTOR

		static const char* get_custom_type_name() {
			return "line";
		}
	};

	using paint_command_variant = std::variant<
		paint_circle_midpoint_command,
		paint_circle_filled_command,
		paint_line_command
	>;

	class image {
		std::vector<rgba> v;
		vec2u size;
	public:

		static vec2u get_size(const path_type& file_path);
		
		image(const vec2u image_size = {});
		
		image(
			const rgba_channel* const ptr,
			const unsigned channels,
			const unsigned pitch,
			const vec2u size
		);
		
		image(const path_type& file_path);

		void fill(const rgba fill_color);

		void execute(const paint_command_variant&);

		void execute(const paint_circle_midpoint_command&);
		void execute(const paint_circle_filled_command&);
		void execute(const paint_line_command&);
		
		void swap_red_and_blue();

		void resize_no_fill(const vec2u new_size) {
			size = new_size;
			v.resize(new_size.area());
		}

		void resize_fill(const vec2u new_size, const rgba col = rgba(0, 0, 0, 0)) {
			size = new_size;
			v.resize(new_size.area(), col);
		}

		void clear() {
			v.clear();
			size = {};
		}

		void from_file(const path_type& path);
		void from_png(const path_type& path);

		void from_png(
			const std::vector<std::byte>& from, 
			const path_type& reported_path
		);

		void from_binary_file(const path_type& path);

		void save_as_png(const path_type& path) const;
		void save_as_binary_file(const path_type& path) const;

		void save(const augs::path_type& path) const;
		
		/* 
			These are performance-critical functions,
			so let them be inlined even in builds without link-time optimization enabled.
		*/

		const rgba_channel* get_data() const {
			return reinterpret_cast<const rgba_channel*>(v.data());
		}

		vec2u get_size() const {
			return size;
		}

		unsigned get_rows() const {
			return size.y;
		}

		unsigned get_columns() const {
			return size.x;
		}

		rgba& pixel(const unsigned pos) {
			return v[pos];
		}

		const rgba& pixel(const unsigned pos) const {
			return v[pos];
		}

		rgba& pixel(const vec2u pos) {
			return v[pos.y * size.x + pos.x];
		}

		const rgba& pixel(const vec2u pos) const {
			return v[pos.y * size.x + pos.x];
		}

		auto to_pixel_coord(const unsigned pos) const {
			const auto y = pos / size.x;
			const auto x = pos - y * size.x;

			return vec2u(x, y);
		}

		bool in_bounds(const vec2u p) const {
			return p.x < size.x && p.y < size.y;
		}

		image& desaturate();

		auto begin() {
			return v.begin();
		}

		auto begin() const {
			return v.begin();
		}

		auto end() {
			return v.end();
		}

		auto end() const {
			return v.end();
		}
	};

	class image_view {
		rgba* const v;
		const vec2u size;

	public:
		image_view(rgba* v, vec2u size);

		rgba& pixel(const vec2u pos) {
			return v[pos.y * size.x + pos.x];
		}

		const rgba& pixel(const vec2u pos) const {
			return v[pos.y * size.x + pos.x];
		}

		void fill(const rgba fill_color);
	};
}