#pragma once
#include "pch.hxx"
#include "Event.hxx"

namespace Cyanite::EventKit {
	class EventQueue {
	public:
		static inline auto Push(IEvent event) {
			std::scoped_lock lock{ _mutex };
			_queue.emplace_back(event);
		}

		static inline auto Receive() ->std::vector<IEvent> {
			return _queue;
		}

	private:
		static std::vector<IEvent> _queue;
		static std::mutex _mutex;
	};
}
