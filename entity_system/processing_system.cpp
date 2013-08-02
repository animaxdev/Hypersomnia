#pragma once
#include "processing_system.h"
#include "entity.h"
#include <algorithm>

namespace augmentations {
	namespace entity_system {
		void processing_system::add(entity* e) {
			targets.push_back(e);
		}
		
		void processing_system::remove(entity* e) {
			targets.erase(std::remove(targets.begin(), targets.end(), e), targets.end());
		}
	}
}