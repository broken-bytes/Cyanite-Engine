#pragma once
#include "pch.hxx"
#include <string>
#include <cstdint>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

namespace Cyanite::AssetKit::AssetHandler {
	auto Initialize() -> void;
	auto GetBasePath() -> fs::path;
	auto GetAssetsPath() -> fs::path;
	auto GetAssetPath(std::wstring file)->fs::path;
	auto LoadAsset(std::wstring path)->std::vector<uint8_t>;
}
