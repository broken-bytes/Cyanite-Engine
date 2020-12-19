#pragma once
#include "pch.hxx"

namespace Cyanite::GraphicsKit {
	class GpuWorkerThread {
	public:
		auto CommandList() const ->winrt::com_ptr<ID3D12GraphicsCommandList>;
		

	private:
		winrt::com_ptr<ID3D12GraphicsCommandList> _commands;
	};
}
