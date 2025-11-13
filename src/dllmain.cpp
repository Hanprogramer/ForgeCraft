#include "dllmain.hpp"
#include <amethyst/runtime/events/RegisterEvents.hpp>
#include "common/materials/MaterialManager.hpp"
#include "common/items/ModItems.hpp"
#include "common/blocks/ModBlocks.hpp"


void OnStartJoinGame(OnStartJoinGameEvent& ev) {
}

ModFunction void Initialize(AmethystContext& ctx, const Amethyst::Mod& mod)
{
	Amethyst::InitializeAmethystMod(ctx, mod);
	Amethyst::GetEventBus().AddListener<RegisterItemsEvent>(&ForgeCraft::ModItems::RegisterItems);
	Amethyst::GetEventBus().AddListener<RegisterItemsEvent>(&ModBlocks::RegisterBlockItems);
	Amethyst::GetEventBus().AddListener<RegisterBlocksEvent>(&ModBlocks::RegisterModBlocks);
	Amethyst::GetEventBus().AddListener<OnStartJoinGameEvent>(&OnStartJoinGame);

	ForgeCraft::ModItems::RegisterHooks();
}
