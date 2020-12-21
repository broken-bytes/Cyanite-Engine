#include "pch.hxx"
#include "View.hxx"

namespace Cyanite::GraphicsKit::Components {

	View::View(std::optional<View> parent, Rect<uint32_t> rect) {
		_width = rect.Width();
		_height = rect.Height();

		if(parent.has_value()) {
			_parent = std::make_shared<View>(parent.value());
			CreateWindowW()
		}
		
	}

	auto View::Draw(uint64_t frame) -> void {
		_swapChain->Present(0, 0);
	}
}
