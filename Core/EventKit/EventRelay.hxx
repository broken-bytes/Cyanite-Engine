#pragma once
#include "pch.hxx"
#include "Event.hxx"
#include "EventQueue.hxx"

namespace Cyanite::EventKit {

	class EventRelay {
	public:
		EventRelay(std::function<void(IEvent)> callback) {
			_callback = callback;
		}

		auto PullEvents() -> void {
			for (auto& item : EventQueue::Receive()) {
				_callback(item);
			}
		}
		
	private:
		std::thread _pullThread;
		std::function<void(IEvent)> _callback;
	};
}