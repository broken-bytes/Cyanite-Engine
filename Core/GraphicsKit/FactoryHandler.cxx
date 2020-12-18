#include "pch.hxx"
#include <type_traits>
#include "FactoryHandler.hxx"

namespace Cyanite::GraphicsKit::FactoryHandler {
	winrt::com_ptr<IDXGIFactory7> Factory;
	
	auto CreateFactory() -> winrt::com_ptr<IDXGIFactory7> {
		if(Factory == nullptr) {
			winrt::check_hresult(
				CreateDXGIFactory2(
					DXGI_CREATE_FACTORY_DEBUG,
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
