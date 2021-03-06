#pragma once
#include "augs/math/rects.h"
#include "rect.h"
#include "augs/misc/timing/delta.h"
#include "augs/window_framework/event.h"

namespace augs {
	namespace gui {
		template <class gui_element_variant_id>
		struct middlescrolling {
			vec2 size = vec2(25, 25);
			vec2i middlescroll_icon_position;
			gui_element_variant_id subject;
			float speed_mult = 1.f;

			template<class C>
			void advance_elements(const C context, const delta& dt) {
				if (context.alive(subject)) {
					context(subject, [&](const auto& r) {
						r->set_scroll(r->get_scroll() + static_cast<vec2>(context.get_input_state().mouse.pos - middlescroll_icon_position) * float(speed_mult*dt.in_milliseconds()));
					});
				}
			}

			template<class C>
			bool handle_new_raw_state(const C context, const event::change& state) {
				if (context.alive(subject)) {
					if (state.was_pressed(event::keys::key::MMOUSE)) {
						subject = gui_element_variant_id();
					}

					return true;
				}

				return false;
			}
		};
	}
}
