#include "ModItems.hpp"
#include <mc/src/common/world/item/registry/ItemRegistry.hpp>
#include <mc/src/common/world/item/Item.hpp>
#include <amethyst/runtime/ModContext.hpp>
#include <mc/src-client/common/client/game/MinecraftGame.hpp>
#include <mc/src-deps/core/resource/ResourceHelper.hpp>

#include "common/materials/MaterialManager.hpp"
#include <mc/src/common/locale/I18n.hpp>
#include "../../client/util/TextureUtil.hpp"
#include <amethyst/runtime/utility/InlineHook.hpp>

namespace ForgeCraft {
	class ToolHandle : public Item {
	public:
		ToolHandle(std::string const& name, short id)
			: Item(name, id) {
		}
	};

	bool hasAddedOwnGenerators = false;

	SafetyHookInline _TextureAtlas_addRuntimeImageGenerator;

	std::vector<std::shared_ptr<RuntimeImageGeneratorInfo>> generators;
	void TextureAtlas_addRuntimeImageGenerator(TextureAtlas* self, std::weak_ptr<RuntimeImageGeneratorInfo> info) {
		// Add texture generators
		if (!hasAddedOwnGenerators) {
			hasAddedOwnGenerators = true;
			auto& matManager = ForgeCraft::MaterialManager::getInstance();

			// Loop through all possible materials
			for (const auto& [matId, material] : matManager.materials) {
				// Loop through all parts
				for (const auto& [partId, part] : matManager.parts) {
					generators.push_back(std::make_shared<RuntimeImageGeneratorInfo>(
						std::format("forgecraft:part_{}_{}", partId, matId),
						ResourceLocation(std::format("textures/items/{}_{}", partId, matId)),
						[matId, material, partId, part](AbstractTextureAccessor& accessor, cg::ImageBuffer& image) {
							auto loc2 = ResourceLocation(std::format("textures/items/{}", partId));
							auto& img_handle = accessor.getCachedImageOrLoadSync(loc2, true);
							image = TextureUtil::paletteSwap(
								img_handle,
								part.palleteColors,
								material.palleteColors
							);
						}
					));
				}
			}

			// Create finished tool textures
			for (const auto& [toolId, tool] : matManager.tools) {
				auto perms = matManager.getAllPermutationsFor(tool);
				for (const auto& perm : perms) {
					std::string iconId = std::format("forgecraft:tool_{}", perm.permutationId);
					generators.push_back(std::make_shared<RuntimeImageGeneratorInfo>(
						iconId,
						ResourceLocation(std::format("textures/items/tool_{}", perm.permutationId)),
						[toolId, tool, &matManager, perm](AbstractTextureAccessor& accessor, cg::ImageBuffer& image) {
							std::vector<cg::ImageBuffer> partImages;

							for (const auto& partData : perm.partMaterials) {
								// Get part images
								auto loc = ResourceLocation(partData.first.partIcon);
								auto& img_handle = accessor.getCachedImageOrLoadSync(loc, true);

								// Palette swap each part based on material
								auto& material = partData.second;
								auto& part = partData.first;
								img_handle = TextureUtil::paletteSwap(
									img_handle,
									part.palleteColors,
									material.palleteColors
								);
								partImages.push_back(img_handle);
							}

							// Combine part images into final tool image
							image = TextureUtil::combineImages(partImages);
						}
					));
				}
			}

			for (const std::weak_ptr<RuntimeImageGeneratorInfo>& ptr : generators) {
				self->addRuntimeImageGenerator(ptr);
			}
		}

		_TextureAtlas_addRuntimeImageGenerator.call<void, TextureAtlas*, std::weak_ptr<RuntimeImageGeneratorInfo>>(self, info);
	}


	void ModItems::RegisterItems(RegisterItemsEvent& ev)
	{
		auto& matManager = ForgeCraft::MaterialManager::getInstance();

		auto& i18n = getI18n();
		std::unordered_map<std::string, std::string> additionalTranslations = {
			{"item.forgecraft:part_pickaxe_head_diamond.name", "Beebop"}
		};
		i18n.appendAdditionalTranslations(additionalTranslations, "en-us");

		for (const auto& [matId, material] : matManager.materials) {
			for (const auto& [partId, part] : matManager.parts) {
				auto id = std::format("forgecraft:part_{}_{}", partId, matId);
				auto& item = *ev.itemRegistry.registerItemShared<ToolHandle>(id, ev.itemRegistry.getNextItemID());
				item.setIconInfo(id, 0);
				Log::Info("Item: {}", item.mFullName);
			}
		}

		for (const auto& [toolId, tool] : matManager.tools) {
			auto perms = matManager.getAllPermutationsFor(tool);
			for (const auto& perm : perms) {
				auto id = std::format("forgecraft:tool_{}", perm.permutationId);
				auto& item = *ev.itemRegistry.registerItemShared<ToolHandle>(id, ev.itemRegistry.getNextItemID());
				item.setIconInfo(id, 0);
				Log::Info("Item: {}", item.mFullName);
			}
		}
	}

	void ModItems::RegisterHooks()
	{
		Amethyst::HookManager& hooks = Amethyst::GetHookManager();
		HOOK(TextureAtlas, addRuntimeImageGenerator);
	}
}