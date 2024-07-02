#ifndef EVENT_SUPPLIER_HPP
#define EVENT_SUPPLIER_HPP
#include "Event.hpp"
#include <memory>
#include "SFML/Window/Event.hpp"
#include <cstring>

namespace Communication {
	class EventSupplier {

	public:
		virtual std::unique_ptr<Event> GetEvent();
	};
};






#endif // !EVENT_SUPPLIER_HPP
