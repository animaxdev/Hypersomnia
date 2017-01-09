#pragma once
#include "game/transcendental/entity_handle_declaration.h"
#include "augs/graphics/renderable_positioning_type.h"
#include "augs/math/rects.h"

class interpolation_system;

template<bool is_const, class entity_handle_type>
class basic_renderable_helpers {
public:
	ltrb get_aabb(const renderable_positioning_type type = renderable_positioning_type::CENTER) const;
	ltrb get_aabb(const components::transform transform, const renderable_positioning_type type = renderable_positioning_type::CENTER) const;
	ltrb get_aabb(const interpolation_system&, const renderable_positioning_type type = renderable_positioning_type::CENTER) const;
};

template<bool, class>
class renderable_helpers;

template<class entity_handle_type>
class renderable_helpers<false, entity_handle_type> : public basic_renderable_helpers<false, entity_handle_type> {
public:
};

template<class entity_handle_type>
class renderable_helpers<true, entity_handle_type> : public basic_renderable_helpers<true, entity_handle_type> {
};