#ifndef EVENT_HPP
#define EVENT_HPP
#include <string>
namespace Communication {

    class Connection;
    class Listener;


    class EventData{
    public:
        virtual ~EventData(){}
    };

    class POD_EventData : public EventData {
    public:
        char* data = nullptr;
        template<typename T>
        POD_EventData(const T& target) {
            static_assert(std::is_standard_layout<T>::value);
            static_assert(std::is_trivial<T>::value);
            data = (char*)malloc(sizeof(T));
            memcpy(data, &target, sizeof(T));
        }

        POD_EventData(const char* target) {
            size_t len = strlen(target);
            data = (char*)malloc(len);
            memcpy(data, target, len);
        }

        ~POD_EventData() {
            if (data != nullptr) {
                free(data);
            }
        }


    };

    /*
    @brief Event Data which is in a packed form,
    */
    class PackedEventData {
    protected:

    public:



    };

    enum EventType {
        Return_Event = 1,
        Error_Event = 2,
        Log_Event = 4
    };

    class  Event {
    public:
    protected:

        const EventType ID_;

        EventData* Data_;

        Event(const EventType& ID) : ID_(ID) {}
    public:
        EventType get_ID() const {return ID_;}

        const EventData* get_Data() const { return Data_; }

        Event(Event&& event) : ID_(event.ID_), Data_(event.Data_) {}

        Event(EventData* data, EventType type) : ID_(type), Data_(data) {}

        virtual ~Event() {
            if (Data_ != nullptr) {
                delete Data_;
            }
            
        }

    };

    class Message : public Event {};

    /*class Connection {
    protected:
        Consumer* consumer_;
        Listener* supplier_;
    public:
        virtual Message pass(const Event& event) {
            return consumer_->handle(event);
        }

    };

    class Listener : public Consumer {
    protected:
        Connection& connection_;
    public:

        virtual Message handle(const Event& event) {
            return connection_.pass(event);
        }


    };*/



    
};
#endif // !EVENT_HPP