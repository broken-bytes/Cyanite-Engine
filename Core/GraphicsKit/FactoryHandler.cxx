#include "pch.hxx"
#include <type_traits>
#include "FactoryHandler.hxx"

namespace Cyanite::GraphicsKit::FactoryHandler {
	winrt::com_ptr<IDXGIFactory7> Factory;
	
	auto CreateFactory() -> winrt::com_ptr<IDXGIFactory7> {
		uint64_t createFactoryFlags = 0;
#if defined(_DEBUG)
		createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

		if(Factory == nullptr) {
			winrt::check_hresult(
				CreateDXGIFactory2(
					createFactoryFlags,
					IID_PPV_ARGS(Factory.put())
				)
			);
		}
		return Factory;
	}
	
	auto DisposeFactory(winrt::com_ptr<IDXGIFactory7> factory) {
		factory->Release();
		factory = nullptr;
	}
}
