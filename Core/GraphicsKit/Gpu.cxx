#include "pch.hxx"
#include "Gpu.hxx"

#ifdef _WIN32
#include "DeviceHandler.hxx"
#endif
namespace Cyanite::GraphicsKit {
	Gpu::Gpu() {
		auto adapter = DeviceHandler::QueryAdapters();
		winrt::check_hresult(D3D12CreateDevice(
			adapter.get(),
			MIN_D3D_LVL,
			IID_PPV_ARGS(_device.put())
		));
	}

	Gpu::~Gpu() {
		_device = nullptr;
	}

}
