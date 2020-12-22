#pragma once
#include <cstdint>
#include <functional>
#include <optional>

namespace Cyanite::EventKit {
	enum class EventType {
		Generic,
		GameInput,
		WindowResize,
		WindowFullScreen
	};


	struct IEvent {
		EventType Type;
		void* Sender;
		void* Data;

		bool Handled;
	};

	template<
		typename S,
		typename D
	>
		struct Event : IEvent {
		Event(EventType type, S sender, D data) {
			this->Sender = reinterpret_cast<void*>(&sender);
			this->Data = reinterpret_cast<void*>(&data);
			this->Handled = false;
		}
	};
}
