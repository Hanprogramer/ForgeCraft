#pragma once
#include <mc/src-deps/coregraphics/ImageBuffer.hpp>
#include <mc/src-deps/coregraphics/TextureDescription.hpp>

namespace TextureUtil {

	static cg::ImageBuffer combineImage(cg::ImageBuffer& src, cg::ImageBuffer& dest) {
		// basic validity and format checks
		if (!dest.isValid() || !src.isValid()) {
			Log::Error("Invalid image: dest:{}, src:{}", dest.isValid(), src.isValid());
			return cg::ImageBuffer();
		}

		Log::Error("Texture format: src:{} & dst:{}", (unsigned int)dest.mImageDescription.mTextureFormat, (unsigned int)src.mImageDescription.mTextureFormat);
		if (dest.mImageDescription.mTextureFormat != src.mImageDescription.mTextureFormat) {
			Log::Error("Texture format mismatch {} & {}", (unsigned int)dest.mImageDescription.mTextureFormat, (unsigned int)src.mImageDescription.mTextureFormat);
			return cg::ImageBuffer();
		}

		if (dest.mImageDescription.mImageType != src.mImageDescription.mImageType) {
			Log::Error("Texture type mismatch {} & {}", (unsigned int)dest.mImageDescription.mImageType, (unsigned int)src.mImageDescription.mImageType);
			return cg::ImageBuffer();
		}

		const int width = dest.mImageDescription.mWidth;
		const int height = dest.mImageDescription.mHeight;
		const int stride = cg::ImageDescription::getStrideFromFormat(dest.mImageDescription.mTextureFormat);
		if (stride <= 0) {
			Log::Error("Stride <= 0");
			return cg::ImageBuffer();
		}

		const std::size_t rowBytes = static_cast<std::size_t>(width) * stride;
		const std::size_t planeSize = rowBytes * static_cast<std::size_t>(height);

		// allocate new blob and copy dest into it
		mce::Blob outBlob(planeSize);
		auto outPtr = outBlob.data();
		auto destPtr = dest.mStorage.data();
		auto srcPtr = src.mStorage.data();
		if (!outPtr || !destPtr || !srcPtr) {
			Log::Error("outPtr || destPtr || srcPtr is null");
			return cg::ImageBuffer();
		}

		// copy destination into output
		std::memcpy(outPtr, destPtr, planeSize);

		// If stride == 4 assume RGBA8 with alpha in byte 3 and copy only fully opaque pixels from src
		if (stride == 4) {
			const std::size_t pixelCount = static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
			const uint8_t* s = reinterpret_cast<const uint8_t*>(srcPtr);
			uint8_t* o = reinterpret_cast<uint8_t*>(outPtr);

			for (std::size_t i = 0; i < pixelCount; ++i) {
				const uint8_t alpha = s[3];
				if (alpha == 0xFF) {
					o[0] = s[0];
					o[1] = s[1];
					o[2] = s[2];
					o[3] = s[3];
				}
				s += 4;
				o += 4;
			}
		}
		else {
			// fallback: same-size full overwrite when format/stride is not RGBA8
			std::memcpy(outPtr, srcPtr, planeSize);
		}

		// build new ImageBuffer
		cg::ImageDescription outDesc = dest.mImageDescription;
		Log::Info("Texture merge successful");
		return cg::ImageBuffer(std::move(outBlob), std::move(outDesc));
	}

	static cg::ImageBuffer combineImages(std::vector<cg::ImageBuffer>& sources) {
		// Combine all images in sources into one final image 
		// by successive calls to combineImage
		if (sources.size() == 0) {
			Log::Error("combineImages: empty source list");
			return cg::ImageBuffer();
		}
		cg::ImageBuffer result = sources[0];
		for (std::size_t i = 1; i < sources.size(); ++i) {
			result = combineImage(sources[i], result);
			if (!result.isValid()) {
				Log::Error("combineImages: failed at index {}", i);
				return cg::ImageBuffer();
			}
		}
		return result;
	}


	// Helper: convert packed uint32_t (R | G<<8 | B<<16 | A<<24) to 4 byte array [R,G,B,A]
	static inline std::array<uint8_t, 4> unpackRGBA(uint32_t packed) {
		return std::array<uint8_t, 4>{
				static_cast<uint8_t>((packed >> 24) & 0xFFu),
				static_cast<uint8_t>((packed >> 16) & 0xFFu),
				static_cast<uint8_t>((packed >> 8) & 0xFFu),
				static_cast<uint8_t>(packed & 0xFFu)
		};
	}

	// Primary API: pointer-based color arrays
	// srcColors and dstColors point to contiguous arrays of 4-byte RGBA colors (R,G,B,A order), count elements.
	static cg::ImageBuffer paletteSwap(
		const cg::ImageBuffer& srcImage,
		const uint8_t* srcColors, // pointer to count * 4 bytes
		const uint8_t* dstColors, // pointer to count * 4 bytes
		std::size_t count
	) {
		if (!srcImage.isValid()) {
			Log::Error("paletteSwap: invalid source image");
			return cg::ImageBuffer();
		}

		if (!srcColors || !dstColors || count == 0) {
			Log::Error("paletteSwap: invalid color arrays");
			return cg::ImageBuffer();
		}

		const int width = srcImage.mImageDescription.mWidth;
		const int height = srcImage.mImageDescription.mHeight;
		const int stride = cg::ImageDescription::getStrideFromFormat(srcImage.mImageDescription.mTextureFormat);
		if (stride != 4) {
			Log::Error("paletteSwap: unsupported stride (need RGBA8 == 4)");
			return cg::ImageBuffer();
		}

		const std::size_t pixelCount = static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
		const std::size_t planeSize = pixelCount * 4u;

		const uint8_t* srcPtr = srcImage.mStorage.data();
		if (!srcPtr) {
			Log::Error("paletteSwap: source data null");
			return cg::ImageBuffer();
		}

		// allocate output and copy source into it
		mce::Blob outBlob(planeSize);
		auto outPtr = outBlob.data();
		if (!outPtr) {
			Log::Error("paletteSwap: allocation failed");
			return cg::ImageBuffer();
		}
		std::memcpy(outPtr, srcPtr, planeSize);

		// Build vectors of 4-byte entries for faster indexing
		std::vector<std::array<uint8_t, 4>> srcPal;
		std::vector<std::array<uint8_t, 4>> dstPal;
		srcPal.reserve(count);
		dstPal.reserve(count);
		for (std::size_t i = 0; i < count; ++i) {
			const uint8_t* s = srcColors + i * 4;
			const uint8_t* d = dstColors + i * 4;
			srcPal.push_back({ s[0], s[1], s[2], s[3] });
			dstPal.push_back({ d[0], d[1], d[2], d[3] });
		}

		// Per-pixel scan and replace when match found
		uint8_t* out = outPtr;
		for (std::size_t p = 0; p < pixelCount; ++p) {
			// compare to palette entries
			uint8_t r = out[0];
			uint8_t g = out[1];
			uint8_t b = out[2];
			uint8_t a = out[3];

			for (std::size_t k = 0; k < count; ++k) {
				const auto& sc = srcPal[k];
				if (r == sc[0] && g == sc[1] && b == sc[2] && a == sc[3]) {
					const auto& dc = dstPal[k];
					out[0] = dc[0];
					out[1] = dc[1];
					out[2] = dc[2];
					out[3] = dc[3];
					break; // one match only
				}
			}

			out += 4;
		}

		cg::ImageDescription outDesc = srcImage.mImageDescription;
		return cg::ImageBuffer(std::move(outBlob), std::move(outDesc));
	}

	// Convenience overload: accept vectors of packed uint32_t (R | G<<8 | B<<16 | A<<24)
	static cg::ImageBuffer paletteSwap(
		const cg::ImageBuffer& srcImage,
		const std::vector<uint32_t>& srcPacked,
		const std::vector<uint32_t>& dstPacked
	) {
		
		if(srcPacked.size() > dstPacked.size()) {
			Log::Error("paletteSwap: palette sizes mismatch {} > {}", srcPacked.size(), dstPacked.size());
			return cg::ImageBuffer();
		}
		const std::size_t count = srcPacked.size();
		if (count == 0) {
			Log::Info("paletteSwap: empty palette, returning copy");
			// return a copy of the source
			const int width = srcImage.mImageDescription.mWidth;
			const int height = srcImage.mImageDescription.mHeight;
			const int stride = cg::ImageDescription::getStrideFromFormat(srcImage.mImageDescription.mTextureFormat);
			const std::size_t planeSize = static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * static_cast<std::size_t>(stride);
			mce::Blob outBlob(planeSize);
			auto outPtr = outBlob.data();
			auto srcPtr = srcImage.mStorage.data();
			if (!outPtr || !srcPtr) return cg::ImageBuffer();
			std::memcpy(outPtr, srcPtr, planeSize);
			cg::ImageDescription outDesc = srcImage.mImageDescription;
			return cg::ImageBuffer(std::move(outBlob), std::move(outDesc));
		}

		// build contiguous byte arrays for src/dst
		std::vector<uint8_t> srcColors(count * 4);
		std::vector<uint8_t> dstColors(count * 4);
		for (std::size_t i = 0; i < count; ++i) {
			auto s = unpackRGBA(srcPacked[i]);
			auto d = unpackRGBA(dstPacked[i]);
			srcColors[i * 4 + 0] = s[0];
			srcColors[i * 4 + 1] = s[1];
			srcColors[i * 4 + 2] = s[2];
			srcColors[i * 4 + 3] = s[3];
			dstColors[i * 4 + 0] = d[0];
			dstColors[i * 4 + 1] = d[1];
			dstColors[i * 4 + 2] = d[2];
			dstColors[i * 4 + 3] = d[3];
		}

		return paletteSwap(srcImage, srcColors.data(), dstColors.data(), count);
	}
}