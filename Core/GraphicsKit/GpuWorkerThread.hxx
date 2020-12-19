#pragma once
#include "pch.hxx"

namespace Cyanite::GraphicsKit {
	class Gpu;

	class GpuWorkerThread {
	public:
		GpuWorkerThread(Gpu device);
		auto CommandList() const ->winrt::com_ptr<ID3D12GraphicsCommandList>;

		auto Submit() -> void;

	private:
		winrt::com_ptr<ID3D12GraphicsCommandList> _commands;
	};
}
