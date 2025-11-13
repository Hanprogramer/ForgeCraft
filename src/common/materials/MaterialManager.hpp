#pragma once
#include <cstdint>

namespace ForgeCraft {
	struct MaterialData {
		const std::string materialId;
		const std::vector<uint32_t> palleteColors;
	};

	struct PartData {
		const std::string partId;
		const std::vector<uint32_t> palleteColors;

		const std::string partIcon;
		const std::string partObject;
	};

	struct PermutationData {
		const std::string permutationId;
		const std::vector<std::pair<PartData&, MaterialData&>> partMaterials;
	};

	struct ToolData {
		const std::string toolId;
		const std::vector<PartData> parts;
	};

	/// <summary>
	/// Singleton for getting and registering custom materials
	/// </summary>
	class MaterialManager {
	public:
		std::map<std::string, MaterialData> materials;
		std::map<std::string, PartData> parts;
		std::map<std::string, ToolData> tools;

		static MaterialManager& getInstance() {
			static MaterialManager instance;
			return instance;
		}

		MaterialManager() {
			registerMaterials();
			registerParts();

			registerTool(ToolData{
				"pickaxe", std::vector<PartData>{
					parts.at("tool_handle"),
					parts.at("pickaxe_head")
				} });

			auto& tool = tools.at("pickaxe");
			for (auto& val : getAllPermutationsFor(tool)) {
				Log::Info("Perm: {}", val.permutationId);
			}
		}

		void registerParts() {
			registerPart(PartData("tool_handle", std::vector<uint32_t>{
				0x898989FFu,
					0x686868FFu,
					0x494949FFu,
					0x282828FFu
			}, "textures/items/tool_handle", "textures/items/tool_handle"));

			registerPart(PartData("pickaxe_head", std::vector<uint32_t>{
				0xffffffFFu,
					0xd8d8d8FFu,
					0xc1c1c1FFu,
					0x444444FFu,
					0x181818FFu
			}, "textures/items/pickaxe_head", "textures/items/pickaxe_head"));
		}

		void registerMaterials() {
			// Wooden
			registerMaterial(MaterialData{
				"wooden", std::vector<uint32_t>{
					0x896727FFu,
					0x684e1eFFu,
					0x493615FFu,
					0x281e0bFFu,
					0x281e0bFFu
				} });
			// Stone
			registerMaterial(MaterialData{
				"stone", std::vector<uint32_t>{
					0x898989FFu,
					0x686868FFu,
					0x494949FFu,
					0x282828FFu,
					0x282828FFu
				} });
			// Iron
			registerMaterial(MaterialData{
				"iron", std::vector<uint32_t>{
					0xffffffFFu,
					0xdfdfdfFFu,
					0xb5b5b5FFu,
					0x888888FFu,
					0x888888FFu
				} });
			// Gold
			registerMaterial(MaterialData{
				"gold", std::vector<uint32_t>{
					0xfdf55fFFu,
					0xfad64aFFu,
					0xb26411FFu,
					0x752802FFu,
					0x752802FFu
				} });
			// Diamond
			registerMaterial(MaterialData{
				"diamond", std::vector<uint32_t>{
					0xa1fbe8FFu,
					0x4aedd9FFu,
					0x11727aFFu,
					0x145e53FFu,
					0x145e53FFu
				} });
		}

		void unregisterMaterials() {
			materials.clear();
		}

		const MaterialData* getMaterialData(const std::string& materialId) const {
			// Get material data by ID
			auto it = materials.find(materialId);
			if (it != materials.end()) {
				return &it->second;
			}
			return nullptr;
		}

		bool registerMaterial(const MaterialData& material) {
			// Register a new material
			auto result = materials.emplace(material.materialId, material);
			return result.second; // true if inserted, false if already existed
		}
		bool registerPart(const PartData& part) {
			// Register a new part data
			auto result = parts.emplace(part.partId, part);
			return result.second; // true if inserted, false if already existed
		}
		bool registerTool(const ToolData& tool) {
			// Register a new tool data
			auto result = tools.emplace(tool.toolId, tool);
			return result.second; // true if inserted, false if already existed
		}

		std::vector<PermutationData> getAllPermutationsFor(std::string toolId) const {
			return getAllPermutationsFor(tools.at(toolId));
		}
		std::vector<PermutationData> getAllPermutationsFor(const ToolData& tool) const {
			Log::Info("Generating permutations for {}", tool.toolId);
			std::vector<PermutationData> permutations;

			// build once: deterministic list of material pointers
			std::vector<MaterialData*> materialPtrs;
			materialPtrs.reserve(materials.size());
			for (auto const& kv : materials) {
				materialPtrs.push_back(const_cast<MaterialData*>(&kv.second)); // if materials are non-const values, cast not needed
			}

			Log::Info("Starting generation for {}", tool.toolId);
			// working state: chosen material index for each part
			std::vector<size_t> chosen(tool.parts.size(), 0);

			// helper to build permutation id from refs
			auto buildId = [&](const std::vector<std::pair<PartData&, MaterialData&>>& refs) {
				std::ostringstream id;
				id << tool.toolId;
				for (auto& pr : refs) id << '_' << pr.second.materialId;
				return id.str();
				};

			// recursion
			std::function<void(size_t)> generate = [&](size_t partIndex) {
				Log::Info("[Permutation] {}", partIndex);
				if (partIndex == tool.parts.size()) {
					// convert chosen indices to reference pairs
					std::vector<std::pair<PartData&, MaterialData&>> refs;
					refs.reserve(tool.parts.size());
					for (size_t i = 0; i < tool.parts.size(); ++i) {
						PartData& partRef = const_cast<PartData&>(tool.parts[i]);
						MaterialData& matRef = *materialPtrs[chosen[i]];        // O(1) lookup, safe reference
						refs.emplace_back(partRef, matRef);
					}

					// push permutation
					permutations.emplace_back(PermutationData{ buildId(refs), std::move(refs) });
					return;
				}

				for (size_t mi = 0; mi < materials.size(); ++mi) {
					chosen[partIndex] = mi;
					generate(partIndex + 1);
				}
			};

			generate(0);
			return permutations;
		}
	};
}