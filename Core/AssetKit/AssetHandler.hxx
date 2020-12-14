#pragma once
#include "pch.hxx"
#include <string>
#include <cstdint>
#include <vector>

namespace Cyanide::AssetKit::AssetHandler {

	constexpr wchar_t AssetPath[] = L"Assets";
	
	auto GetBasePath() -> std::wstring;
	auto GetAssetsPath() ->std::wstring;
	auto GetAssetPath(std::wstring file)->std::wstring;
	auto LoadAsset(std::wstring path)->std::vector<uint8_t>;
}
