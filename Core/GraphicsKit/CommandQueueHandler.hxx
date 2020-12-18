#pragma once
#include "pch.hxx"

namespace Cyanite::GraphicsKit {
	class CommandQueueHandler {
	public:
		/// <summary>
		/// Binds the Handler to a device
		/// </summary>
		/// <param name="device"></param>
		/// <returns></returns>
		auto BindDevice(winrt::com_ptr<ID3D12Device8> device) -> void;

		/// <summary>
		/// Creates a new CommandQueue
		/// </summary>
		/// <returns>A new command queue</returns>
		auto CreateCommandQueue()->winrt::com_ptr<ID3D12CommandQueue>;
		/// <summary>
		/// Disposes a command queue
		/// </summary>
		/// <param name="queue"></param>
		/// <returns></returns>
		auto DisposeCommandQueue(winrt::com_ptr<ID3D12CommandQueue> queue);


	private:
		winrt::com_ptr<ID3D12Device> _device;
	};
}
