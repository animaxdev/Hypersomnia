#pragma once
#include "augs/gui/formatted_string.h"
#include "augs/gui/text/caret.h"
#include "augs/gui/text/printer.h"
#include "augs/misc/action_list/action_list.h"
#include "augs/misc/action_list/standard_actions.h"

namespace augs {
	class action;
}

using act = std::unique_ptr<augs::action>;

struct appearing_text {
	rgba_channel alpha = 0;

	std::wstring text;

	std::array<std::wstring, 2> target_text;

	std::wstring get_total_target_text() const {
		return target_text[0] + target_text[1];
	}

	bool caret_active = false;
	bool should_disappear = true;
	float population_variation = 0.4f;
	float population_interval = 150.f;

	void push_disappearance(augs::action_list& into, const bool blocking = true) {
		auto push = [&](act a) {
			blocking ? into.push_blocking(std::move(a)) : into.push_non_blocking(std::move(a));
		};

		push(act(new augs::delay_action(1000.f)));
		push(act(new augs::tween_value_action<rgba_channel>(alpha, 0, 2000.f)));
		push(act(new augs::delay_action(500.f)));
	}

	void push_actions(augs::action_list& into) {
		static size_t rng = 0;

		auto push = [&](act a) {
			into.push_blocking(std::move(a));
		};

		push(act(new augs::set_value_action<rgba_channel>(alpha, 255)));
		push(act(new augs::set_value_action<std::wstring>(text, std::wstring())));
		push(act(new augs::set_value_action<bool>(caret_active, true)));

		push(act(new augs::populate_with_delays<std::wstring>(text, target_text[0], population_interval * target_text[0].length(), population_variation, rng++)));

		if (target_text[1].size() > 0) {
			push(act(new augs::delay_action(1000.f)));
			push(act(new augs::populate_with_delays<std::wstring>(text, target_text[1], population_interval * target_text[1].length(), population_variation, rng++)));
		}

		push(act(new augs::delay_action(1000.f)));
		push(act(new augs::set_value_action<bool>(caret_active, false)));

		if (should_disappear) {
			push_disappearance(into);
		}
	}

	bool should_draw() const {
		return caret_active || (text.size() > 0 && alpha > 0);
	}

	void draw(
		const augs::drawer_with_default output,
		vec2 target_pos,
		augs::gui::text::style style
	) const {
		using namespace augs::gui::text;

		if (!should_draw()) {
			return;
		}

		const auto formatted = formatted_string(text, style).set_alpha(alpha);

		caret_info caret(style);
		caret.pos = text.size();

		print_stroked(
			output,
			target_pos,
			formatted,
			caret
		);
	}
};