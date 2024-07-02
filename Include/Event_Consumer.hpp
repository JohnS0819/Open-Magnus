#ifndef EVENT_CONSUMER_HPP
#define EVENT_CONSUMER_HPP
#include "Event.hpp"

namespace Communication {
	class EventConsumer {

	public:
		virtual std::unique_ptr<Event> ProcessEvent(std::unique_ptr<Event>&);
	};







};
















#endif



