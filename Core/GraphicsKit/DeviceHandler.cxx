#include "pch.hxx"
#include "DeviceHandler.hxx"

#include "GraphicsHandler.hxx"

namespace Cyanide::GraphicsKit::DeviceHandler {
	auto QueryAdapters()-> winrt::com_ptr<IDXGIAdapter4>
	{
		winrt::com_ptr<IDXGIAdapter4> adapter;
		// You begin DXCore adapter enumeration by creating an adapter factory.
		winrt::com_ptr<IDXGIFactory7> factory;
		winrt::check_hresult(
			CreateDXGIFactory2(
				DXGI_CREATE_FACTORY_DEBUG,
				IID_PPV_ARGS(factory.put())
			)
		);

		for (
			uint8_t index = 0;
			index != factory->EnumAdapterByGpuPreference(
				index,
				DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
				IID_PPV_ARGS(adapter.put())
			);
			++index
			) {
			DXGI_ADAPTER_DESC3 desc;
			adapter->GetDesc3(&desc);
			if(desc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE) {
				continue;
			}
			// Check to see whether the adapter supports Direct3D 12, but don't create the
			// actual device yet.
			//
			winrt::check_hresult(D3D12CreateDevice(adapter.get(), MIN_D3D_LVL, _uuidof(ID3D12Device), nullptr));
			break;
		}
		return adapter;
	}
}
