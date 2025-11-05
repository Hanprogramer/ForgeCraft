#include "dllmain.hpp"
#include <amethyst/runtime/events/RegisterEvents.hpp>
#include <amethyst/runtime/events/GameEvents.hpp>

#include <mc/src-client/common/client/game/MinecraftGame.hpp>
#include <mc/src-deps/core/resource/ResourceHelper.hpp>
#include <mc/src-deps/coregraphics/ImageBuffer.hpp>

#include <common/items/ModItems.hpp>
#include <common/blocks/ModBlocks.hpp>
#include <client/util/TextureUtil.hpp>

#include "common/materials/MaterialManager.hpp"

void generateTextures(AbstractTextureAccessor& accessor, cg::ImageBuffer& image) {
	Log::Info("Generating texture");
	auto loc = ResourceLocation("textures/items/pickaxe_head");
	auto& img_head = accessor.getCachedImageOrLoadSync(loc, true);

	auto loc2 = ResourceLocation("textures/items/tool_handle");
	auto& img_handle = accessor.getCachedImageOrLoadSync(loc2, true);
	img_handle = TextureUtil::paletteSwap(
		img_handle,
		ForgeCraft::pallete_src_tool_handle,
		ForgeCraft::pallete_dst_tool_handle_wood
	);

	auto newImage = TextureUtil::combineImage(img_handle, img_head);
	image = newImage;
}

bool hasAddedOwnGenerators = false;
std::shared_ptr<RuntimeImageGeneratorInfo> shared;

SafetyInlineHook _TextureAtlas_addRuntimeImageGenerator;

void TextureAtlas_addRuntimeImageGenerator(TextureAtlas* self, std::weak_ptr<RuntimeImageGeneratorInfo> info) {
	// auto val = info.lock();
	// auto newInfo = RuntimeImageGeneratorInfo(val->unk0, val->loc, generateTextures);
	// auto shared = std::make_shared<RuntimeImageGeneratorInfo>(newInfo);
	// auto newWeak = std::weak_ptr(shared);

	// Log::Info("Adding runtime image generator: {} -> {}", newInfo.unk0, newInfo.loc.mPath);

	if (!hasAddedOwnGenerators) {
		hasAddedOwnGenerators = true;

		shared = std::make_shared<RuntimeImageGeneratorInfo>(
			"test_icon:test_icon",
			ResourceLocation("textures/items/test_icon"),
			generateTextures
		);

		std::weak_ptr<RuntimeImageGeneratorInfo> newWeak = shared;
		self->addRuntimeImageGenerator(newWeak);
	}

	_TextureAtlas_addRuntimeImageGenerator.call<void, TextureAtlas*, std::weak_ptr<RuntimeImageGeneratorInfo>>(self, info);
}


const char materials[][10] = {
	"iron",
	"gold",
	"copper",
	"emerald",
	"obsidian",
	"redstone",
	"amethyst"
};

const char parts[][20] = {
	"sword_blade",
	"sword_longblade",
	"sword_shortblade",
	"pickaxe_head",
	"axe_head",
	"shovel_head",
	"hoe_head",
	"binding",
	"sword_hilt"
};

void OnStartJoinGame(OnStartJoinGameEvent& ev) {
}



//SafetyHookInline __finishReloadingResources;
//static __int64 _finishReloadingResources(__int64** a1, __int64 a2) {
//	auto result = __finishReloadingResources.call<__int64, __int64**, __int64>(a1, a2);
//
//	Log::Info("Finished loading packs. Now registering atlas generators");
//
//
//
//	return result;
//}

ModFunction void Initialize(AmethystContext& ctx, const Amethyst::Mod& mod)
{
	Amethyst::InitializeAmethystMod(ctx, mod);
	Amethyst::GetEventBus().AddListener<RegisterItemsEvent>(&ModItems::RegisterItems);
	Amethyst::GetEventBus().AddListener<RegisterItemsEvent>(&ModBlocks::RegisterBlockItems);
	Amethyst::GetEventBus().AddListener<RegisterBlocksEvent>(&ModBlocks::RegisterModBlocks);
	Amethyst::GetEventBus().AddListener<OnStartJoinGameEvent>(&OnStartJoinGame);

	Amethyst::HookManager& hooks = Amethyst::GetHookManager();

	/*hooks.CreateHookAbsolute(__finishReloadingResources,
		SigScan("48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 4C 8B E2 ? ? ? 48 89 54 24 ? 33 F6"),
		&_finishReloadingResources);*/

	 HOOK(TextureAtlas, addRuntimeImageGenerator);
}
