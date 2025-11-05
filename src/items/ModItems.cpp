#include "ModItems.hpp"
#include <mc/src/common/world/item/registry/ItemRegistry.hpp>
#include <mc/src/common/world/item/Item.hpp>
#include <amethyst/runtime/ModContext.hpp>
#include <mc/src-client/common/client/game/MinecraftGame.hpp>
#include <mc/src-deps/core/resource/ResourceHelper.hpp>

void ModItems::RegisterItems(RegisterItemsEvent& ev)
{

	auto& sword = *ev.itemRegistry.registerItemShared<Item>("forgecraft:sword", ev.itemRegistry.getNextItemID());
	sword.setIconInfo("forgecraft:quartz", 0);
	sword.setMaxStackSize(1);
}

