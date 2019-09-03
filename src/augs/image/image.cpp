#include <cstring>

#define BUILD_IMAGE 1

#if BUILD_IMAGE
#define LODEPNG_NO_COMPILE_ALLOCATORS
#include "3rdparty/lodepng/lodepng.h"
#endif

#include "augs/ensure.h"

#include "augs/image/image.h"
#include "augs/filesystem/directory.h"
#include "augs/filesystem/file.h"
#include "augs/readwrite/byte_readwrite.h"
#include "augs/image/blit.h"
#include "augs/readwrite/byte_file.h"
#define STB_IMAGE_IMPLEMENTATION
#include "3rdparty/stb/stb_image.h"
#include "augs/readwrite/memory_stream.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "3rdparty/stb/stb_image_resize.h"

#if PLATFORM_UNIX
#include <arpa/inet.h>
#else
#include <winsock.h>
#undef min
#undef max
#endif

#if BUILD_IMAGE
/* 
	We write manual encoding/decoding wrappers to be able to pass our own rgba vectors,
	instead of vectors of unsigned chars.
*/

void* lodepng_malloc(size_t size)
{
  return malloc(size);
}

void* lodepng_realloc(void* ptr, size_t new_size)
{
  return realloc(ptr, new_size);
}

void lodepng_free(void* ptr)
{
  free(ptr);
}

unsigned decode_rgba(std::vector<rgba>& out, unsigned& w, unsigned& h, const unsigned char* in,
                size_t insize, LodePNGColorType colortype = LCT_RGBA, unsigned bitdepth = 8)
{
	using namespace lodepng;
  unsigned char* buffer;
  unsigned error = lodepng_decode_memory(&buffer, &w, &h, in, insize, colortype, bitdepth);
  if(buffer && !error)
  {
    State state;
    state.info_raw.colortype = colortype;
    state.info_raw.bitdepth = bitdepth;
    size_t buffersize = lodepng_get_raw_size(w, h, &state.info_raw);

	const auto elems_written = buffersize / sizeof(rgba);
	out.resize(out.size() + elems_written);
	std::memcpy(&out[out.size() - elems_written], &buffer[0], buffersize);
    lodepng_free(buffer);
  }
  return error;
}

unsigned decode_rgba(std::vector<rgba>& out, unsigned& w, unsigned& h, const std::vector<std::byte>& from) {
	return decode_rgba(out, w, h, reinterpret_cast<const unsigned char*>(from.data()), from.size());
}

unsigned encode_rgba(std::vector<std::byte>& out, const std::vector<rgba>& in_v, unsigned w, unsigned h,
                LodePNGColorType colortype = LCT_RGBA, unsigned bitdepth = 8)
{

	const auto in = std::addressof(in_v.data()->r);

  unsigned char* buffer;
  size_t buffersize;
  unsigned error = lodepng_encode_memory(&buffer, &buffersize, in, w, h, colortype, bitdepth);
  if(buffer)
  {
	const auto elems_written = buffersize;
	out.resize(out.size() + elems_written);
	std::memcpy(&out[out.size() - elems_written], &buffer[0], buffersize);
    lodepng_free(buffer);
  }
  return error;
}

#endif

template<class C>
static void Line(
	int x1, 
	int y1, 
	int x2, 
	int y2, 
	C callback
) {
	// Bresenham's line algorithm
	const bool steep = (std::abs(y2 - y1) > std::abs(x2 - x1));
	if (steep)
	{
		std::swap(x1, y1);
		std::swap(x2, y2);
	}

	if (x1 > x2)
	{
		std::swap(x1, x2);
		std::swap(y1, y2);
	}

	const int dx = x2 - x1;
	const int dy = std::abs(y2 - y1);

	float error = dx / 2.0f;
	const int ystep = (y1 < y2) ? 1 : -1;
	int y = (int)y1;

	const int maxX = (int)x2;

	for (int x = (int)x1; x<=maxX; x++)
	{
		if (steep)
		{
			callback(y, x);
		}
		else
		{
			callback(x, y);
		}

		error -= dy;
		if (error < 0)
		{
			y += ystep;
			error += dx;
		}
	}
}

auto& thread_local_file_buffer() {
	thread_local std::vector<std::byte> loaded_bytes;
	loaded_bytes.clear();
	return loaded_bytes;
}

namespace augs {
	static void throw_if_zero_size(const path_type& path, const vec2u size) {
		if (!size.x || !size.y) {
			throw image_loading_error("Failed to load image %x:\nWidth or height is zero!", path);
		}
	}

	image image::white_pixel() {
		auto out = image(vec2i(1, 1));
	
		out.fill(white);

		return out;
	}

	image::image(const vec2u new_size) {
		resize_fill(new_size);
	}
	
	image::image(
		const unsigned char* const ptr, 
		const vec2u new_size,
		const unsigned channels,
		const unsigned pitch 
	) {
		resize_no_fill(new_size);

		if (channels == 1) {
			for (unsigned j = 0; j < new_size.y; ++j) {
				for (unsigned i = 0; i < new_size.x; ++i) {
					pixel({ i, j }) = { 255, 255, 255, ptr[pitch * j + i] };
				}
			}
		}
		else if (channels == 4 && pitch == 0) {
			std::memcpy(v.data(), ptr, new_size.area() * 4);
		}
		else {
			ensure(false);
		}
	}

	void image::load_stbi_buffer(unsigned char* buf, const int w, const int h) {
		size.x = w;
		size.y = h;

		v.resize(size.area() * 4);
		std::memcpy(v.data(), buf, v.size());

		stbi_image_free(reinterpret_cast<void*>(buf));
	}

	vec2u image::get_size(const path_type& file_path) {
#if BUILD_IMAGE
		try {
			const auto extension = file_path.extension();

			auto do_open_file = [&]() {
				auto in = with_exceptions<std::ifstream>();
				in.open(file_path, std::ios::in | std::ios::binary);
				return in;
			};

			if (extension == ".png") {
				unsigned int width, height;

				auto in = do_open_file();
				in.seekg(16);
				in.read((char*)&width, 4);
				in.read((char*)&height, 4);

				width = ntohl(width);
				height = ntohl(height);

				return { width, height };
			}
			else if (extension == ".bin") {
				vec2u s;

				auto in = do_open_file();
				augs::read_bytes(in, s);
				return s;
			}
			else {
				int x;
				int y;
				int comp;

				const auto str_path = file_path.string();

				if (!stbi_info(str_path.c_str(), &x, &y, &comp)) {
					throw image_loading_error("Failed to read size of %x:\nstbi_info returned 0!", file_path);
				}

				return vec2u(x, y);
			}
		}
		catch (const image_loading_error& err) {
			throw;
		}
		catch (const std::exception& err) {
			throw image_loading_error("Failed to read size of %x:\n%x", file_path, err.what());
		}
#else
		return vec2u::zero;
#endif
	}

	vec2u image::get_size(const std::vector<std::byte>& bytes) {
		int x;
		int y;
		int comp;

		if (!stbi_info_from_memory(reinterpret_cast<const unsigned char*>(bytes.data()), static_cast<int>(bytes.size()), &x, &y, &comp)) {
			return vec2u(0, 0);
		}

		return vec2u(x, y);
	}

	vec2u image::get_png_size(const std::vector<std::byte>& bytes) {
#if BUILD_IMAGE
		const auto minimum_bytes = 16 + 4 + 4 + 1;

		if (bytes.size() < minimum_bytes) {
			throw image_loading_error("Failed to read size of byte vector. The number of bytes is %x, has to be at least %x.", bytes.size(), minimum_bytes);
		}

		uint32_t width;
		uint32_t height;

		std::memcpy(&width, &bytes[16], sizeof(uint32_t));
		std::memcpy(&height, &bytes[20], sizeof(uint32_t));

		width = ntohl(width);
		height = ntohl(height);

		return { width, height };
#else
		return vec2u::zero;
#endif
	}

	void image::from_file(const path_type& path) {
		const auto extension = path.extension();

		if (extension == ".png") {
			from_png(path);
		}
		else if (extension == ".bin") {
			from_binary_file(path);
		}
		else {
			int width;
			int height;
			int comp;

			const auto str_path = path.string();
			const auto result = stbi_load(str_path.c_str(), &width, &height, &comp, 4);

			if (result == nullptr) {
				throw image_loading_error(
					"Failed to load image %x (earlier loaded into memory):\nstbi returned NULL", path
				);
			}

			load_stbi_buffer(result, width, height);
		}

		throw_if_zero_size(path, size);
	}

	void image::from_png(const path_type& path) {
		v.clear();

		auto& loaded_bytes = thread_local_file_buffer();

		try {
			augs::file_to_bytes(path, loaded_bytes);
			from_image_bytes(loaded_bytes, path);
		}
		catch (const augs::file_open_error& err) {
			throw image_loading_error(
				"Failed to open the avatar file:\n%x\nEnsure that the file exists and the path is correct.\n\nDetails:\n%x", path, err.what()
			);
		}
	}

	void image::from_binary_file(const path_type& path) try {
		auto in = with_exceptions<std::ifstream>();
		in.open(path, std::ios::in | std::ios::binary);

		augs::read_bytes(in, size);
		augs::read_bytes(in, v);

		throw_if_zero_size(path, size);
	}
	catch (const augs::file_open_error& err) {
		throw image_loading_error(
			"Failed to load image %x:\n%x", path, err.what()
		);
	}

	void image::from_bytes(
		const std::vector<std::byte>& from, 
		const path_type& reported_path
	) {
		const auto extension = reported_path.extension();

		if (extension == ".png") {
			from_image_bytes(from, reported_path);
		}
		else if (extension == ".bin") {
			auto in = augs::cref_memory_stream(from);

			augs::read_bytes(in, size);
			augs::read_bytes(in, v);
		}
		else {
			int width;
			int height;
			int comp;

			const auto result = stbi_load_from_memory(reinterpret_cast<stbi_uc const*>(from.data()), static_cast<int>(from.size()), &width, &height, &comp, 4);

			if (result == nullptr) {
				throw image_loading_error(
					"Failed to load image %x (earlier loaded into memory):\nstbi returned NULL", reported_path
				);
			}

			load_stbi_buffer(result, width, height);
		}
	}

	void image::from_image_bytes(
		const std::vector<std::byte>& from, 
		const path_type& reported_path
	) {
		v.clear();

		unsigned width;
		unsigned height;

		if (const auto lodepng_result = decode_rgba(v, width, height, from)) {
			throw image_loading_error(
				"Failed to load image %x (earlier loaded into memory):\nlodepng returned %x", reported_path, lodepng_result
			);
		}

		size.x = width;
		size.y = height;

		throw_if_zero_size(reported_path, size);
	}

	void image::execute(const paint_command_variant& in) {
		std::visit([&](const auto& resolved) {
			execute(resolved);
		}, in);
	}

	void image::execute(
		const paint_circle_midpoint_command& in
	) {
		const auto side = in.radius * 2 + 1;

		image new_surface;
		auto& surface = v.empty() ? *this : new_surface;
		
		surface = augs::image(vec2u { std::max(size.x, side), std::max(size.y, side) });

		ensure(size.x >= side);
		ensure(size.y >= side);

		const auto angle_start = vec2::from_degrees(in.angle_start);
		const auto angle_end = vec2::from_degrees(in.angle_end);

		const auto pp = [&](
			const unsigned x, 
			const unsigned y
		) {
			const auto x_center = static_cast<signed>(x - size.x / 2);
			const auto y_center = static_cast<signed>(y - size.y / 2);

			const auto angle = vec2i(x_center, y_center);// .degrees();

			if (!in.constrain_angle || (angle_start.cross(vec2(angle)) >= 0.f && angle_end.cross(vec2(angle)) <= 0.f)) {
				const auto col = in.filling;
				//if (scale_alpha)
				//	col.a = (angle - angle_start) / (angle_end - angle_start) * 255;

				surface.pixel({ x, y } ) = col;
			}
		};

		for (unsigned i = 0; i < in.border_width; ++i) {
			int x = in.radius - i;
			int y = 0;
			
			int decisionOver2 = 1 - x;   // Decision criterion divided by 2 evaluated at x=r, y=0
			const auto x0 = size.x / 2;
			const auto y0 = size.y / 2;

			while (y <= x)
			{
				pp(x + x0, y + y0); // Octant 1
				pp(y + x0, x + y0); // Octant 2
				pp(-x + x0, y + y0); // Octant 4
				pp(-y + x0, x + y0); // Octant 3
				pp(-x + x0, -y + y0); // Octant 5
				pp(-y + x0, -x + y0); // Octant 6
				pp(x + x0, -y + y0); // Octant 7
				pp(y + x0, -x + y0); // Octant 8
				y++;

				if (decisionOver2 <= 0) {
					decisionOver2 += 2 * y + 1;   // Change in decision criterion for y -> y+1
				}
				else {
					decisionOver2 += 2 * (y - (--x)) + 1;   // Change for y -> y+1, x -> x-1
				}
			}
		}

		if (in.border_width > 1) {
			for (unsigned y = 0; y < surface.size.x; ++y) {
				for (unsigned x = 0; x < surface.size.x; ++x) {
					if (x > 0 && y > 0 && x < surface.size.x - 1 && y < surface.size.y - 1) {
						if (surface.pixel({ x, y }).a == 0 &&
							surface.pixel({x + 1, y}).a > 0 &&
							surface.pixel({x - 1, y}).a > 0 &&
							surface.pixel({x, y + 1}).a > 0 &&
							surface.pixel({x, y - 1}).a > 0)
							pp(x, y);
					}
				}
			}
		}

		if (&surface != this) {
			blit(
				*this,
				surface,
				{ 0u, 0u },
				false,
				true
			);
		}
	}

	void image::execute(
		const paint_circle_filled_command& in
	) {
		const auto side = in.radius * 2 + 1;

		if (v.empty()) {
			resize_fill({ side, side });
		}
		else {
			ensure(size.x >= side);
			ensure(size.y >= side);
		}

		for (unsigned y = 0; y < size.y; ++y) {
			for (unsigned x = 0; x < size.x; ++x) {
				const auto x_center = static_cast<signed>(x - size.x / 2);
				const auto y_center = static_cast<signed>(y - size.y / 2);

				if (x_center*x_center + y_center*y_center <= static_cast<int>(in.radius*in.radius)) {
					pixel({ x, y }) = in.filling;
				}
			}
		}

		//pixel(side / 2, 0) = rgba(0, 0, 0, 0);
		//pixel(side / 2, side -1) = rgba(0, 0, 0, 0);
		//pixel(0, side /2) = rgba(0, 0, 0, 0);
		//pixel(side -1, side /2) = rgba(0, 0, 0, 0);
	}

	void image::execute(
		const paint_line_command& in
	) {
		Line(in.from.x, in.from.y, in.to.x, in.to.y, [&](const int x, const int y) {
			const auto pos = vec2u(x, y);

			ensure(in_bounds(pos));
			pixel(pos) = in.filling;
		});
	}
	
	void image::swap_red_and_blue() {
		for (auto& p : v) {
			std::swap(p.r, p.b);
		}
	}
	
	void image::save(const path_type& path) const {
		const auto extension = path.extension();

		if (extension == ".png") {
			save_as_png(path);
		}
		else if (extension == ".bin") {
			save_as_binary_file(path);
		}
		else {
			LOG("Warning: %x has unknown extension: %x! Saving as binary.", path, extension);
			save_as_binary_file(path);
		}
	}

	std::vector<std::byte> image::to_image_bytes() const {
		std::vector<std::byte> saved_bytes;

		if (const auto lodepng_result = encode_rgba(saved_bytes, v, size.x, size.y)) {
			LOG("Failed to image to png bytes: lodepng returned %x.", lodepng_result);
		}

		return saved_bytes;
	}

	void image::save_as_png(const path_type& path) const {
		augs::create_directories_for(path);

		thread_local std::vector<std::byte> saved_bytes;
		saved_bytes.clear();

		if (const auto lodepng_result = encode_rgba(saved_bytes, v, size.x, size.y)) {
			LOG("Failed to save %x: lodepng returned %x. Ensure that the target directory exists.", path, lodepng_result);
		}
		else {
			augs::bytes_to_file(saved_bytes, path);
		}
	}

	void image::save_as_binary_file(const path_type& path) const {
		augs::create_directories_for(path);

		std::ofstream out(path, std::ios::out | std::ios::binary);
		augs::write_bytes(out, size);
		augs::write_bytes(out, v);
	}

	void image::fill(const rgba col) {
		for (auto& p : v) {
			p = col;
		}
	}

	image_view::image_view(rgba* v, vec2u size) : v(v), size(size) {}
	 

	void image_view::fill(const rgba fill_color) {
		for (auto y = 0u; y < size.y; ++y) {
			for (auto x = 0u; x < size.x; ++x) {
				pixel(vec2u{ y, x }) = fill_color;
			}
		}
	}

	image& image::desaturate() {
		for (unsigned y = 0; y < size.y; ++y) {
			for (unsigned x = 0; x < size.x; ++x) {
				pixel({ x, y }).desaturate();
			}
		}

		return *this;
	}

	void image::scale(const vec2u new_size, const scaling_method method) {
		if (method == scaling_method::STB) {
			image new_image;
			new_image.resize_no_fill(new_size);

			const auto in_ptr = v.data();
			const auto out_ptr = new_image.v.data();
			::stbir_resize_uint8(&in_ptr->r, size.x, size.y, 0, &out_ptr->r, new_size.x, new_size.y, 0, 4);

			*this = std::move(new_image);
		}
	}
}



