#pragma once
#include <vector>
#include "augs/console_color.h"

#include "augs/misc/typesafe_sprintf.h"
#include "augs/build_settings/setting_enable_debug_log.h"
#include "augs/filesystem/path.h"

struct log_entry {
	console_color color;
	std::string text;
};

class program_log {
	static program_log global_instance;
	unsigned max_all_entries;

public:
	static auto& get_current() {
		return global_instance;
	}

	program_log(const unsigned max_all_entries);

	std::vector<log_entry> all_entries;

	void push_entry(const log_entry&);
	void save_complete_to(const augs::path_type& path) const;
};

template <typename... A>
void LOG(const std::string& f, A&&... a) {
	LOG(typesafe_sprintf(f, std::forward<A>(a)...));
}

template <>
void LOG(const std::string& f);

#define LOG_NVPS(...) { \
std::ostringstream sss;\
write_nvps(sss, #__VA_ARGS__, __VA_ARGS__);\
LOG("(%x)", sss.str());\
}

template <typename H1> 
std::ostream& write_nvps(
	std::ostream& out, 
	const char* const label, 
	H1&& value
) {
	return out << label << "=" << std::forward<H1>(value);
}

template<typename H1, typename... T> 
std::ostream& write_nvps(
	std::ostream& out, 
	const char* const label, 
	H1&& value, 
	T&&... rest
) {
	const char* const pcomma = strchr(label, ',');
	return write_nvps(
		out.write(label, pcomma - label) << "=" << std::forward<H1>(value) << ',',
		pcomma + 1,
		std::forward<T>(rest)...
	);
}

void CALL_SHELL(const std::string&);

#if ENABLE_DEBUG_LOG
#define DEBUG_LOG LOG
#else
#define DEBUG_LOG(...)
#endif