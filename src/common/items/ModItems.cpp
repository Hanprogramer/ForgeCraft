#include "ModItems.hpp"
#include <mc/src/common/world/item/registry/ItemRegistry.hpp>
#include <mc/src/common/world/item/Item.hpp>
#include <amethyst/runtime/ModContext.hpp>
#include <mc/src-client/common/client/game/MinecraftGame.hpp>
#include <mc/src-deps/core/resource/ResourceHelper.hpp>

class Sword : public Item {
public:
	Sword(std::string const& name, short id)
		: Item(name, id) {}

	virtual ResolvedItemIconInfo getIconInfo(const ItemStackBase& stack, int unk1, bool unk2) const override {
		return ResolvedItemIconInfo("test_icon:test_icon", 0, ItemIconInfoType::Icon);
	}
};

void ModItems::RegisterItems(RegisterItemsEvent& ev)
{
	auto& sword = *ev.itemRegistry.registerItemShared<Sword>("forgecraft:sword", ev.itemRegistry.getNextItemID());
	sword.setMaxStackSize(1);
}

