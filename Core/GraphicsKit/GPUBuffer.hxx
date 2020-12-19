#pragma once
#include "Texture.hxx"
#include "Gpu.hxx"

namespace Cyanite::GraphicsKit {
	class GPUBuffer
	{
	public:
		auto BindDevice(Gpu device) -> void;
		auto UploadResourceData(winrt::com_ptr<ID3D12Resource> resource) -> void;
		auto UploadTextureData(
			std::vector<Cyanite::GraphicsKit::Components::Texture> textures
		) -> void;

	private:
		Gpu _device;
	};
}

