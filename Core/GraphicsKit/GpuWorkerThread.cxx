#include "pch.hxx"
#include "GpuWorkerThread.hxx"
#include "Gpu.hxx"

namespace Cyanite::GraphicsKit {
	GpuWorkerThread::GpuWorkerThread(Gpu device) {
		_commands = device
	}
	auto GpuWorkerThread::CommandList() const -> winrt::com_ptr<ID3D12GraphicsCommandList> {}
	auto GpuWorkerThread::Submit(
		winrt::com_ptr<ID3D12CommandQueue> queue
	) -> void {}
}
