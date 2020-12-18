#include "pch.hxx"
#include "AssetHandler.hxx"

#include <filesystem>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

namespace Cyanite::AssetKit::AssetHandler {
	constexpr wchar_t AssetPath[] = L"Assets";

	std::wstring BasePath;
	
	auto Initialize() -> void {
		auto* buff = new wchar_t[4096];
		if (GetModuleFileNameW(nullptr, buff, 4096) == 0)
		{
			int ret = GetLastError();
			fprintf(stderr, "GetModuleFileName failed, error = %d\n", ret);
		}

		std::wstring path(buff);
		delete[] buff;
		path = path.substr(0, path.find_last_of(L'\\'));
		BasePath = path;
	}
	
	auto GetBasePath() -> fs::path {
		return BasePath;
	}

	auto GetAssetsPath() -> fs::path {
		return GetBasePath().append("Assets");
	}

	auto GetAssetPath(std::wstring file) -> fs::path {
		return GetAssetsPath().append(file);
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
