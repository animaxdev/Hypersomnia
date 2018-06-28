#define TRACE_PARAMETERS 0
#define TRACE_CONSTRUCTORS_DESTRUCTORS 0
#define Y_IS_Z 1

#if BUILD_OPENAL
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <AL/efx.h>
#endif

#include "augs/math/vec2.h"
#include "augs/math/si_scaling.h"

#include "augs/audio/sound_source.h"
#include "augs/audio/sound_buffer.h"

#include "augs/audio/OpenAL_error.h"

#if TRACE_CONSTRUCTORS_DESTRUCTORS
int g_num_sources = 0;
#endif

namespace augs {
	sound_source::sound_source() {
#if BUILD_OPENAL
		alGenSources(1, &id);
		const ALenum err { alGetError() };

		if (err == AL_OUT_OF_MEMORY) {
			throw too_many_sound_sources_error();
		}
#endif

#if TRACE_CONSTRUCTORS_DESTRUCTORS
		++g_num_sources;
		LOG("alGenSources: %x (now %x sources)", id, g_num_sources);
#endif
		AL_CHECK(alSourcef(id, AL_PITCH, 1));
		AL_CHECK(alSourcef(id, AL_GAIN, 1));
		AL_CHECK(alSourcei(id, AL_LOOPING, AL_FALSE));

		initialized = true;
	}

	sound_source::~sound_source() {
		destroy();
	}
	
	sound_source::sound_source(sound_source&& b) :
		initialized(b.initialized),
		id(b.id),
		attached_buffer(b.attached_buffer)
	{
		b.initialized = false;
	}

	sound_source& sound_source::operator=(sound_source&& b) {
		destroy();

		initialized = b.initialized;
		id = b.id;
		attached_buffer = b.attached_buffer;

		b.initialized = false;

		return *this;
	}

	void sound_source::destroy() {
		if (initialized) {
			stop();
#if TRACE_CONSTRUCTORS_DESTRUCTORS
			--g_num_sources;
			LOG("alDeleteSources: %x (now %x sources)", id, g_num_sources);
#endif
			AL_CHECK(alDeleteSources(1, &id));
			initialized = false;
			attached_buffer = nullptr;
		}
	}

	ALuint sound_source::get_id() const {
		return id;
	}

	sound_source::operator ALuint() const {
		return get_id();
	}

	void sound_source::play() const {
		AL_CHECK(alSourcePlay(id));
	}
	
	void sound_source::seek_to(const float seconds) const {
		(void)seconds;
		AL_CHECK(alSourcef(id, AL_SEC_OFFSET, seconds));
	}
	
	float sound_source::get_time_in_seconds() const {
		float seconds = 0.f;
		AL_CHECK(alGetSourcef(id, AL_SEC_OFFSET, &seconds));
		return seconds;
	}

	void sound_source::stop() const {
		AL_CHECK(alSourceStop(id));
	}
	
	void sound_source::set_looping(const bool loop) const {
		(void)loop;
		AL_CHECK(alSourcei(id, AL_LOOPING, loop));
#if TRACE_PARAMETERS
		LOG_NVPS(loop);
#endif
	}

	void sound_source::set_distance_model(const distance_model model) {
#if BUILD_OPENAL
		ALint resolved;

		switch (model) {
			case distance_model::INVERSE_DISTANCE: resolved = AL_INVERSE_DISTANCE; break;
			case distance_model::INVERSE_DISTANCE_CLAMPED: resolved = AL_INVERSE_DISTANCE_CLAMPED; break;
			case distance_model::LINEAR_DISTANCE: resolved = AL_LINEAR_DISTANCE; break;
			case distance_model::LINEAR_DISTANCE_CLAMPED: resolved = AL_LINEAR_DISTANCE_CLAMPED; break;
			case distance_model::EXPONENT_DISTANCE: resolved = AL_EXPONENT_DISTANCE; break;
			case distance_model::EXPONENT_DISTANCE_CLAMPED: resolved = AL_EXPONENT_DISTANCE_CLAMPED; break;
			default: resolved = AL_NONE; break;
		}

		AL_CHECK(alSourcei(id, AL_DISTANCE_MODEL, resolved));
#else
		(void)model;
#endif
	}

	void sound_source::set_rolloff_factor(const float factor) const {
		(void)factor;
		AL_CHECK(alSourcef(id, AL_ROLLOFF_FACTOR, factor));
	}

	void sound_source::set_doppler_factor(const float factor) const {
		(void)factor;
		AL_CHECK(alSourcef(id, AL_DOPPLER_FACTOR, factor));
	}

	void sound_source::set_spatialize(const bool f) const {
		AL_CHECK(alSourcei(id, AL_SOURCE_SPATIALIZE_SOFT, f ? AL_TRUE : AL_FALSE));
	}

	void sound_source::set_pitch(const float pitch) const {
		(void)pitch;
		AL_CHECK(alSourcef(id, AL_PITCH, pitch));
#if TRACE_PARAMETERS
		LOG_NVPS(pitch);
#endif
	}

	void sound_source::set_gain(const float gain) const {
		(void)gain;
		AL_CHECK(alSourcef(id, AL_GAIN, gain));
#if TRACE_PARAMETERS
		LOG_NVPS(gain);
#endif
	}

	void sound_source::set_air_absorption_factor(const float absorption) const {
		(void)absorption;
		AL_CHECK(alSourcef(id, AL_AIR_ABSORPTION_FACTOR, absorption));
#if TRACE_PARAMETERS
		LOG_NVPS(absorption);
#endif
	}

	void sound_source::set_velocity(const si_scaling si, vec2 v) const {
		v = si.get_meters(v);
#if Y_IS_Z
		AL_CHECK(alSource3f(id, AL_VELOCITY, v.x, 0.f, v.y));
#else
		AL_CHECK(alSource3f(id, AL_VELOCITY, v.x, v.y, 0.f));
#endif
#if TRACE_PARAMETERS
		LOG_NVPS(v);
#endif
	}

	void sound_source::set_position(const si_scaling si, vec2 pos) const {
		pos = si.get_meters(pos);

#if Y_IS_Z
		AL_CHECK(alSource3f(id, AL_POSITION, pos.x, 0.f, pos.y));
#else
		AL_CHECK(alSource3f(id, AL_POSITION, pos.x, pos.y, 0.f));
#endif
#if TRACE_PARAMETERS
		LOG_NVPS(pos);
#endif
	}

	void sound_source::set_max_distance(const si_scaling si, const float distance) const {
		const auto passed_distance = si.get_meters(distance);

		(void)passed_distance;
		AL_CHECK(alSourcef(id, AL_MAX_DISTANCE, passed_distance));
#if TRACE_PARAMETERS
		LOG_NVPS(passed_distance);
#endif
	}

	void sound_source::set_relative(const bool f) const {
		AL_CHECK(alSourcei(id, AL_SOURCE_RELATIVE, f ? 1 : 0));
		(void)f;
	}

	void sound_source::set_relative_and_zero_vel_pos() const {
		set_relative(true);
		AL_CHECK(alSource3f(id, AL_POSITION, 0.0f, 0.0f, 0.0f));
		AL_CHECK(alSource3f(id, AL_VELOCITY, 0.0f, 0.0f, 0.0f));
	}

	void sound_source::set_reference_distance(const si_scaling si, const float distance) const {
		const auto passed_distance = si.get_meters(distance);

		(void)passed_distance;
		AL_CHECK(alSourcef(id, AL_REFERENCE_DISTANCE, passed_distance));
#if TRACE_PARAMETERS
		LOG_NVPS(passed_distance);
#endif
	}

	void sound_source::set_direct_channels(const bool flag) const {
		(void)flag;
		AL_CHECK(alSourcei(id, AL_DIRECT_CHANNELS_SOFT, flag ? 1 : 0));

#if TRACE_PARAMETERS
		LOG_NVPS(flag);
#endif
	}

	float sound_source::get_gain() const {
		float gain = 0.f;
		AL_CHECK(alGetSourcef(id, AL_GAIN, &gain));
		return gain;
	}

	float sound_source::get_pitch() const {
		float pitch = 0.f;
		AL_CHECK(alGetSourcef(id, AL_PITCH, &pitch));
		return pitch;
	}

	bool sound_source::is_playing() const {
#if BUILD_OPENAL
		ALenum state = 0xdeadbeef;
		AL_CHECK(alGetSourcei(id, AL_SOURCE_STATE, &state));
		return state == AL_PLAYING;
#else
		return false;
#endif
	}

	void sound_source::bind_buffer(const single_sound_buffer& buf) {
		std::optional<float> previous_seconds;

		{
			const auto buf_addr = std::addressof(buf);

			if (attached_buffer == buf_addr) {
				return;
			}

			if (attached_buffer != nullptr) {
				previous_seconds = get_time_in_seconds();
			}

			attached_buffer = &buf;
		}

		if (previous_seconds) {
			stop();
		}

		AL_CHECK(alSourcei(id, AL_BUFFER, buf.get_id()));
#if TRACE_PARAMETERS
		LOG_NVPS(buf.get_id());
#endif

		if (previous_seconds) {
			play();
			seek_to(*previous_seconds);
		}
	}

	void sound_source::bind_buffer(
		const sound_buffer& source_buffer, 
		const std::size_t variation_index,
		const bool direct
	) {
		bind_buffer(source_buffer.get_buffer(variation_index, direct));
		set_direct_channels(direct);
	}

	void sound_source::unbind_buffer() {
		attached_buffer = nullptr;
		AL_CHECK(alSourcei(id, AL_BUFFER, 0));
	}
	
	const single_sound_buffer* sound_source::get_bound_buffer() const {
		return attached_buffer;
	}

	void set_listener_position(const si_scaling si, vec2 pos) {
		pos = si.get_meters(pos);

#if Y_IS_Z
		AL_CHECK(alListener3f(AL_POSITION, pos.x, 0.f, pos.y));
#else
		AL_CHECK(alListener3f(AL_POSITION, pos.x, pos.y, 0.f));
#endif
#if TRACE_PARAMETERS
		LOG_NVPS(pos);
#endif
	}

	void set_listener_velocity(const si_scaling si, vec2 v) {
		v = si.get_meters(v);

#if Y_IS_Z
		AL_CHECK(alListener3f(AL_VELOCITY, v.x, 0.f, v.y));
#else
		AL_CHECK(alListener3f(AL_VELOCITY, v.x, v.y, 0.f));
#endif

#if TRACE_PARAMETERS
		LOG_NVPS(v);
#endif
	}

	void set_listener_orientation(const std::array<float, 6> data) {
		(void)data;
		AL_CHECK(alListenerfv(AL_ORIENTATION, data.data()));
	}
}
