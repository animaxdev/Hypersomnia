#include "fixed_delta_timer.h"
#include <algorithm>
#include "augs/ensure.h"
#include <cmath>
#undef min
/* credits goes to http://www.unagames.com/blog/daniele/2010/06/fixed-time-step-implementation-box2d */

namespace augs {
	fixed_delta_timer::fixed_delta_timer(const unsigned max_steps_to_perform) : max_steps_to_perform(max_steps_to_perform) {}

	unsigned fixed_delta_timer::count_logic_steps_to_perform(const fixed_delta& basic_delta) {
		accumulator += ticks.extract<std::chrono::milliseconds>() * time_multiplier;

		const unsigned steps = static_cast<unsigned>(std::floor(accumulator / basic_delta.delta_ms));

		if (steps > 0) 
			accumulator -= steps * basic_delta.delta_ms;

		ensure(
			"Accumulator must have a value lesser than the fixed time step" &&
			accumulator < basic_delta.delta_ms
			);

		return std::min(steps, max_steps_to_perform);
	}

	float fixed_delta_timer::fraction_of_step_until_next_step(const fixed_delta& basic_delta) const {
		return accumulator / basic_delta.delta_ms;
	}

	void fixed_delta_timer::set_stepping_speed_multiplier(const float tm) {
		time_multiplier = tm;
	}

	float fixed_delta_timer::get_stepping_speed_multiplier() const {
		return time_multiplier;
	}
}
