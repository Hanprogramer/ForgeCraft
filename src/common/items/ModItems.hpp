#pragma once
#include <amethyst/runtime/events/RegisterEvents.hpp>

namespace ForgeCraft {
	class ModItems {
	public:
		static void RegisterItems(RegisterItemsEvent& ev);
		static void RegisterHooks();
	};
}