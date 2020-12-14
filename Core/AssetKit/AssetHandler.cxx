#include "pch.hxx"
#include "AssetHandler.hxx"

#include <cstdint>
#include <string>
#include <vector>

namespace Cyanide::AssetKit::AssetHandler {
	auto GetBasePath() -> std::wstring {
		auto* buffer = new wchar_t[4096];
		GetModuleFileNameW(nullptr, buffer, sizeof(buffer));
		std::wstring str = std::wstring(buffer);
		delete[] buffer;
		return str;
	}

	auto GetAssetsPath() -> std::wstring {
		return GetBasePath() + L"/" + AssetPath;
	}

	auto GetAssetPath(std::wstring file) -> std::wstring {
		return GetAssetsPath() + L"/" + GetAssetPath(file);
	}
	
	auto LoadAsset(std::wstring path) -> std::vector<uint8_t> {
		std::wstring str; 
		uint8_t* buffer = nullptr;
		std::ifstream file(path, std::ios::binary | std::ios::ate);
		std::streamsize size = file.tellg();
		buffer = new uint8_t[size];
		file.seekg(0, std::ios::beg);

		if (!file.read(reinterpret_cast<char*>(buffer), size))
		{
			return {};
		}
		std::vector<uint8_t> data(buffer, buffer + sizeof(buffer));
		delete[] buffer;
		return data;
	}
}
