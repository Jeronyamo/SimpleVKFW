#ifndef SVKFW_EVENT_HANDLER_H
#define SVKFW_EVENT_HANDLER_H

#include <list>
#include <map>
#include <vector>
#include <unordered_map>

#include <mutex>
#include <future>
#include <atomic>
#include <thread>


#include "common/terminal.h"


namespace Simple {
    namespace Event {
// EventType enum

        typedef enum EventType : uint32_t {
            EV_TYPE_NONE = 0,
            // EV_TYPE_PROGRAM_INIT = 1,
            // EV_TYPE_PROGRAM_END = 2,
            // EV_TYPE_PROGRAM_ERROR = 3,
            // and what exactly should it all do? Too broad

            EV_TYPE_EV_HANDLER_CLEAR_LIST = 11,
            EV_TYPE_EV_HANDLER_STOP_WRITING = 12,
            EV_TYPE_EV_HANDLER_STOP_HANDLING = 13,

            EV_TYPE_DELETE_EV_HANDLES = 20,
            EV_TYPE_DELETE_EV_HANDLES_UNBIND = 21,

        } EventType; // EventType END


//  ============  EventHandle type base class and some derivatives  ============  \\

        class EventHandler;

        namespace Handle {
            struct HandleBase {
                HandleBase();
                HandleBase(const HandleBase &_handle_b);
                virtual ~HandleBase();

                virtual void handle(EventHandler*, uint32_t);

                // This function is called after handle().
                // Returned value: 0 - endless, >0 - number of uses. Default: 0.
                // When returns 1, EventHandler unbinds this handler for this event.
                virtual uint32_t usageCount();
            }; // HandlerBase END
            extern HandleBase Empty_Handle;


    // EventHandle type for functions

            struct FuncHandle : HandleBase {
                using void_func_void = void (*)(void);
                void_func_void func_handle = nullptr;

                FuncHandle();
                FuncHandle(const FuncHandle &_func_handle);
                FuncHandle(void_func_void _func_handle);
                virtual ~FuncHandle();


                void set(void_func_void _func_handle);

                void_func_void replace(void_func_void _func_handle);

                void handle(EventHandler*, uint32_t _event_type) override;
            }; // FuncHandle END

            struct FuncEventHandle : HandleBase {
                using void_func_event = void (*)(uint32_t);
                void_func_event func_handle = nullptr;

                FuncEventHandle();
                FuncEventHandle(const FuncEventHandle &_func_handle);
                FuncEventHandle(void_func_event _func_handle);
                virtual ~FuncEventHandle();


                void set(void_func_event _func_handle);

                void_func_event replace(void_func_event _func_handle);

                void handle(EventHandler*, uint32_t _event_type) override;
            }; // FuncEventHandle END

            struct FuncExtHandle : HandleBase {
                using void_func_handler_event = void (*)(EventHandler*, uint32_t);
                void_func_handler_event func_handle = nullptr;

                FuncExtHandle();
                FuncExtHandle(const FuncExtHandle &_func_handle);
                FuncExtHandle(void_func_handler_event _func_handle);
                virtual ~FuncExtHandle();


                void set(void_func_handler_event _func_handle);

                void_func_handler_event replace(void_func_handler_event _func_handle);

                void handle(EventHandler* _this_ev_handler, uint32_t _event_type) override;
            }; // FuncExtHandle END


    // EventHandle for deleting EventHandle objects created by "new"

            struct DelHandle : HandleBase {
                std::map<HandleBase*, std::vector<uint32_t>> handles_events;
                std::mutex handles_events_mutex;


            };
            extern DelHandle Handle_Deleter;


    // EventHandle 

    // EventHandle decorator(?): Adds finite "lifetime" for EventHandle object 

            template <class EvHandle>
            struct CountableWrap : EvHandle {
                uint32_t usage_count = 0u;

                CountableWrap() : EvHandle{} {}
                CountableWrap(const EvHandle &_ev_handle, uint32_t _usage_c)
                                : EvHandle{_ev_handle}, usage_count{_usage_c} {}

                uint32_t usageCount() override { return usage_count > 1u ? usage_count-- : usage_count; }
            }; // CountableWrap END
        }; // Handle END



//  ============  Event Handler  ============  \\

        class EventHandler {
            std::list<uint32_t> events;
            std::mutex event_list_mutex;

            std::unordered_map<uint32_t, Handle::HandleBase*> event_handles{ {0u, &Handle::Empty_Handle} };
            std::mutex event_handling_mutex;

            std::atomic<bool> stop_event_handling{false};
            std::atomic<bool> stop_event_writing{false};

        public:
        // methods used from outside

            uint32_t writeEvent(uint32_t _event);
            // inline std::future<uint32_t> writeEventAsync(uint32_t _event) {
            //     return std::async(writeEvent, this, _event);
            // }

            uint32_t writeEventSeq(const std::vector<uint32_t> &_events);
            // inline std::future<uint32_t> writeEventSeqAsync(const std::vector<uint32_t> &_events) {
            //     return std::async(writeEventSeq, this, _events);
            // }


            uint32_t setHandle(uint32_t _event, Handle::HandleBase* _handler = &Handle::Empty_Handle);
            // inline std::future<uint32_t> setHandleAsync(uint32_t _event, Handle::HandleBase* _handler = &Handle::Empty_Handle) {
            //     return std::async(setHandle, this, _event, _handler);
            // }

            void stopEventHandling();

            void stopEventWriting();

        // methods used in handlers (they are guaranteed to have access to EventHandler members (mutexes already locked))

            uint32_t handlerWriteEventSeq(const std::vector<uint32_t> &_events);

        // poll events

            void pollEvents();

        // handling loops

            void runEventHandling();

            // Note: the event list is not locked while waiting for the next event, but it is locked while handling,
            //       because handlers may call 'handlerWriteEventSeq()' (which expects the list mutex to be locked).
            void runEventHandlingLong();
            void runEventHandlingAsync();
        }; // EventHandler END
        extern EventHandler Event_Handler;
    }; // Event END
}; // Simple END

#endif