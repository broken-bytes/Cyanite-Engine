#pragma once
#include <functional>
#include <vector>

#ifdef _WIN32
typedef void* USBDeviceHandle;
typedef wchar_t* DevicePath;
#elif _OSX
#else
#endif


namespace BrokenBytes::ControllerKit::USB {
	enum class USBDeviceEvent {
		Connected,
		Removed
	};

	struct USBDevice {
		DevicePath Path;
		uint32_t VendorId;
		uint32_t ProductId;

		auto operator==(USBDevice& rhs) const -> bool {
			return this->Path == rhs.Path;
		}

		auto operator==(USBDevice rhs) const -> bool {
			return this->Path == rhs.Path;
		}

		auto operator==(USBDevice* rhs) const -> bool {
			return this->Path == rhs->Path;
		}
	};

	auto Init() -> void;
	auto EnumerateDevices() -> std::vector<USBDevice>;

}
