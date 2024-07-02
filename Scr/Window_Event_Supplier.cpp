#include "Window_Event_Supplier.hpp"

Rendering::sfEventData::sfEventData(sf::Event& event_) : Communication::POD_EventData(event_) {}

const sf::Event& Rendering::sfEventData::getEvent() const {
	return reinterpret_cast<const sf::Event&>(POD_EventData::data);
}
std::unique_ptr<Communication::Event> Rendering::WindowEventSupplier::GetEvent() {
	sf::Event event_;
	if (!WindowSource_->pollEvent(event_)) {
		std::make_unique<Communication::Event>(Rendering::sfEventData(event_),Communication::EventType::Return_Event);
	}
}

