#pragma once
#include "Texture.hxx"

namespace Cyanite::GraphicsKit {
	class GPUBuffer
	{
	public:
		auto BindDevice(winrt::com_ptr<ID3D12Device> device) -> void;
		auto UploadResourceData(winrt::com_ptr<ID3D12Resource> resource) -> void;
		auto UploadTextureData(
			std::vector<Cyanite::GraphicsKit::Components::Texture> textures
		) -> void;

	private:
		winrt::com_ptr<ID3D12Device> _device;
	};
}

