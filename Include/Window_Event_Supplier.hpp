#ifndef WINDOW_EVENT_SUPPLIER_HPP
#define WINDOW_EVENT_SUPPLIER_HPP
#include "Event_Supplier.hpp"
#include "Renderer.hpp"
#include "Event_Channel.hpp"

namespace Rendering {
	class sfEventData : Communication::POD_EventData {
	public:
		sfEventData(sf::Event&);

		const sf::Event& getEvent() const;
	};

	class WindowEventSupplier : public Communication::EventSupplier {
	protected:
		Render_Window* WindowSource_;



	public:
		virtual std::unique_ptr<Communication::Event> GetEvent();
		std::unique_ptr<Communication::Event> GetMouseEvent();

	};

};


#endif // !WINDOW_SUPPLIER_HPP
