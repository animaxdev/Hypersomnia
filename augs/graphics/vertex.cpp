#include "vertex.h"
#include "augs/texture_atlas/texture_atlas.h"

namespace augs {
	vertex::vertex(vec2 pos, vec2 texcoord, rgba color, augs::texture_atlas_entry& tex) :
		pos(pos), texcoord(texcoord), color(color) {
			tex.get_atlas_space_uv(this->texcoord);
	}

	void vertex::set_texcoord(vec2 coord, const augs::texture_atlas_entry& tex) {
		texcoord = coord;
		tex.get_atlas_space_uv(this->texcoord);
	}
}

vertex_triangle_buffer_reference::vertex_triangle_buffer_reference(augs::vertex_triangle_buffer& target_buffer) : target_buffer(target_buffer) {

}
