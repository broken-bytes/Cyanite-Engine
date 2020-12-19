#pragma once
#include "pch.hxx"

namespace Cyanite::SystemKit::SystemInfo {

	/// <summary>
	/// Gets the number of logical cpu cores for this system
	/// </summary>
	/// <returns></returns>
	inline auto CpuCount() {
		return std::thread::hardware_concurrency();
	}

	/// <summary>
	/// Get the Ram in MB
	/// </summary>
	/// <returns></returns>
	inline auto Ram(){
		uint64_t ram;
		winrt::check_bool(GetPhysicallyInstalledSystemMemory(&ram));
		return ram = 1024;
	}
}
