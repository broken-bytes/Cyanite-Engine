#include "pch.hxx"
#include "GPUBuffer.hxx"
auto Cyanite::GraphicsKit::GPUBuffer::BindDevice(
	winrt::com_ptr<ID3D12Device> device
) -> void {
	_device = device;
}
auto Cyanite::GraphicsKit::GPUBuffer::UploadResourceData(winrt::com_ptr<ID3D12Resource> resource) -> void {}

auto Cyanite::GraphicsKit::GPUBuffer::UploadTextureData(
	std::vector<Cyanite::GraphicsKit::Components::Texture> textures) -> void {}
