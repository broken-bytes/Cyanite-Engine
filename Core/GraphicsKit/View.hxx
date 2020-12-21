#pragma once
#include "Rect.hxx"

namespace Cyanite::GraphicsKit::Components {
	class View {
	public:
		View(std::optional<View> parent, Rect<uint32_t> rect);
		auto Draw(uint64_t frame) -> void;
		
	private:

		uint32_t _width;
		uint32_t _height;
		HWND _window;
		std::shared_ptr<View> _parent;

		winrt::com_ptr<ID3D12DescriptorHeap> _rtvHeap;
		winrt::com_ptr<IDXGISwapChain> _swapChain;
	};
}
