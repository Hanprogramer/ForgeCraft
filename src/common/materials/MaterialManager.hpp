#pragma once
#include <cstdint>

namespace ForgeCraft {
	struct PalleteSwapData {
		const uint8_t* srcColors; // pointer to count * 4 bytes
		const uint8_t* dstColors; // pointer to count * 4 bytes
	};

	static const std::vector<uint32_t> pallete_src_tool_handle = {
		// All 4 colors of the handle
		0x898989FFu,
		0x686868FFu,
		0x494949FFu,
		0x282828FFu
	};

	static const std::vector<uint32_t> pallete_dst_tool_handle_wood = {
		// All 4 colors of the handle - wood
		0x896727FFu,
		0x684e1eFFu,
		0x493615FFu,
		0x281e0bFFu
	};

	class MaterialManager {

	};
}