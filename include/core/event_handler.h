#ifndef SVKFW_EVENT_HANDLER_H
#define SVKFW_EVENT_HANDLER_H

#include <cstdint>
#include <cstring>
#include <vector>
#include <unordered_map>
#include <functional>
#include <type_traits>
#include <mutex>
#include <thread>
#include <atomic>

#include "core/etcs_base.h"
#include "core/event_types.h"


namespace Simple {
    namespace Event {
// Note: the event types of the engine and their payloads are in 'core/event_types.h'


        // Note: an event is a type, an optional entity and a small payload (any trivially copyable struct up to 'PAYLOAD_SIZE')
        struct Event {
            static const uint32_t PAYLOAD_SIZE = 48u;

            uint32_t type = EV_TYPE_NONE;
            EntityID entity;
            alignas(8) uint8_t payload[PAYLOAD_SIZE];

            Event() { std::memset(payload, 0, sizeof(payload)); }
            explicit Event(uint32_t _type, EntityID _entity = EntityID::None) : type(_type), entity(_entity) {
                std::memset(payload, 0, sizeof(payload));
            }

            template <class T> static Event make(uint32_t _type, const T &_data, EntityID _entity = EntityID::None) {
                Event __ev(_type, _entity);
                __ev.set(_data);
                return __ev;
            }

            template <class T> void set(const T &_data) {
                static_assert(sizeof(T) <= PAYLOAD_SIZE, "Event: the payload is too big");
                static_assert(std::is_trivially_copyable<T>::value, "Event: the payload must be trivially copyable");
                std::memcpy(payload, &_data, sizeof(T));
            }

            template <class T> T get() const {
                static_assert(sizeof(T) <= PAYLOAD_SIZE, "Event: the payload is too big");
                static_assert(std::is_trivially_copyable<T>::value, "Event: the payload must be trivially copyable");
                T __data;
                std::memcpy(&__data, payload, sizeof(T));
                return __data;
            }
        }; // Event END


//  ============  Event Handler  ============  \\

        class EventHandler;

        // Note: '_handler' is the handler that dispatches (handlers may write new events or subscribe through it)
        typedef std::function<void(EventHandler &_handler, const Event &_event)> HandleFunc;
        typedef uint32_t SubscriptionID; // 0: none

        // Note: a queue of events with subscribers per event type.
        //       - 'write' is thread-safe: any thread (audio, asset loading, physics) can post events; it only holds a short lock.
        //       - 'poll' dispatches the queued events on the thread that calls it (the main thread, at a fixed point of the
        //         frame), without holding the lock: handlers can write events (handled by the same 'poll', in the next
        //         round, up to '_max_rounds'; the rest waits for the next 'poll'), subscribe and unsubscribe.
        //       - subscribing and dispatching belong to the polling thread (checked in debug builds).
        //       There is no dedicated event thread: game state changes happen at a known point of the frame, in order, without
        //       locks in every handler. Long work (loading) should run in a worker that writes an event when done.
        class EventHandler {
            struct Subscription {
                SubscriptionID id = 0u;
                uint32_t       type = 0u;
                HandleFunc     func;
                uint32_t       uses_left = 0u; // 0: unlimited
            };

            std::mutex         queue_mutex_;
            std::vector<Event> queue_;       // written by any thread
            std::vector<Event> processing_;  // swapped with 'queue_' by 'poll'
            std::atomic<bool>  writing_stopped_{false};

            std::unordered_map<uint32_t, std::vector<Subscription>> subscriptions_;
            std::vector<Subscription> added_;  // subscribed during dispatching: appended after it
            SubscriptionID next_id_ = 1u;
            uint32_t       dispatch_depth_ = 0u;
            bool           removed_ = false;   // some subscriptions were removed during dispatching
            std::thread::id owner_;

            void checkThread_();
            void flushChanges_();

        public:
            EventHandler();

        // Any thread

            // Queues the event; returns false if writing is stopped
            bool write(const Event &_event);
            bool write(uint32_t _type, EntityID _entity = EntityID::None) { return write(Event(_type, _entity)); }
            bool write(const std::vector<Event> &_events);

            void stopWriting()   { writing_stopped_ = true;  }
            void resumeWriting() { writing_stopped_ = false; }
            // Drops the queued events
            void clearQueue();
            size_t queued();

        // The polling thread

            // '_uses' > 0: the subscription ends after that many calls
            SubscriptionID subscribe(uint32_t _type, const HandleFunc &_func, uint32_t _uses = 0u);
            // Returns false if there is no such subscription (already ended)
            bool unsubscribe(SubscriptionID _id);
            void unsubscribeAll(uint32_t _type);

            // Calls the subscribers of the event now (bypasses the queue)
            void dispatch(const Event &_event);

            // Dispatches the queued events in the order they were written; returns the number of dispatched events
            uint32_t poll(uint32_t _max_rounds = 4u);
        }; // EventHandler END
    }; // Event END
}; // Simple END

#endif
