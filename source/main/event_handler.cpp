#include "main/event_handler.h"

namespace Simple {
    namespace Event {
        namespace Handle {

            HandleBase::HandleBase() {}

            HandleBase::HandleBase(const HandleBase &_handle_b) {}

            HandleBase::~HandleBase() {}

            void HandleBase::handle(EventHandler*, uint32_t) {}

            uint32_t HandleBase::usageCount() { return 0u; }

            HandleBase Empty_Handle;

            FuncHandle::FuncHandle() {}

            FuncHandle::FuncHandle(const FuncHandle &_func_handle) : func_handle{_func_handle.func_handle} {}

            FuncHandle::FuncHandle(void_func_void _func_handle) : func_handle{_func_handle} {}

            FuncHandle::~FuncHandle() {}

            void FuncHandle::set(void_func_void _func_handle) { func_handle = _func_handle; }

            FuncHandle::void_func_void FuncHandle::replace(void_func_void _func_handle) {
                void_func_void __prev_func = func_handle;
                func_handle = _func_handle;
                return __prev_func;
            }

            void FuncHandle::handle(EventHandler*, uint32_t _event_type) { func_handle(); }

            FuncEventHandle::FuncEventHandle() {}

            FuncEventHandle::FuncEventHandle(const FuncEventHandle &_func_handle) : func_handle{_func_handle.func_handle} {}

            FuncEventHandle::FuncEventHandle(void_func_event _func_handle) : func_handle{_func_handle} {}

            FuncEventHandle::~FuncEventHandle() {}

            void FuncEventHandle::set(void_func_event _func_handle) { func_handle = _func_handle; }

            FuncEventHandle::void_func_event FuncEventHandle::replace(void_func_event _func_handle) {
                void_func_event __prev_func = func_handle;
                func_handle = _func_handle;
                return __prev_func;
            }

            void FuncEventHandle::handle(EventHandler*, uint32_t _event_type) { func_handle(_event_type); }

            FuncExtHandle::FuncExtHandle() {}

            FuncExtHandle::FuncExtHandle(const FuncExtHandle &_func_handle) : func_handle{_func_handle.func_handle} {}

            FuncExtHandle::FuncExtHandle(void_func_handler_event _func_handle) : func_handle{_func_handle} {}

            FuncExtHandle::~FuncExtHandle() {}

            void FuncExtHandle::set(void_func_handler_event _func_handle) { func_handle = _func_handle; }

            FuncExtHandle::void_func_handler_event FuncExtHandle::replace(void_func_handler_event _func_handle) {
                void_func_handler_event __prev_func = func_handle;
                func_handle = _func_handle;
                return __prev_func;
            }

            void FuncExtHandle::handle(EventHandler* _this_ev_handler, uint32_t _event_type) {
                func_handle(_this_ev_handler, _event_type);
            }

            DelHandle Handle_Deleter;
        }; // Handle END


        uint32_t EventHandler::writeEvent(uint32_t _event) {
            std::lock_guard<std::mutex> lk(event_list_mutex);
            if (!stop_event_writing) {
                events.push_back(_event);
                return 0u;
            }
            return 1u;
        }

        uint32_t EventHandler::writeEventSeq(const std::vector<uint32_t> &_events) {
            std::lock_guard<std::mutex> lk(event_list_mutex);
            if (!stop_event_writing) {
                events.insert(events.end(), _events.begin(), _events.end());
                return 0u;
            }
            return 1u;
        }

        uint32_t EventHandler::setHandle(uint32_t _event, Handle::HandleBase* _handler) {
            std::lock_guard<std::mutex> lk(event_handling_mutex);
            if (!stop_event_handling) {
                event_handles[_event] = _handler;
                return 0u;
            }
            return 1u;
        }

        void EventHandler::stopEventHandling() {
            stop_event_handling = true;
        }

        void EventHandler::stopEventWriting() {
            stop_event_writing = true;
        }

        uint32_t EventHandler::handlerWriteEventSeq(const std::vector<uint32_t> &_events) {
            if (!stop_event_writing) {
                events.insert(events.end(), _events.begin(), _events.end());
                return 0u;
            }
            return 1u;
        }

        void EventHandler::pollEvents() {
            std::lock_guard<std::mutex> lk1(event_list_mutex);
            std::lock_guard<std::mutex> lk2(event_handling_mutex);

            while (!events.empty()) {
                uint32_t __curr_event = events.front();
                events.pop_front();

                if (event_handles.find(__curr_event) != event_handles.end())
                    event_handles[__curr_event]->handle(this, __curr_event);
                else
                    printf("%s %d\n", SVKFW_WRAPINFO("EventHandler", "unhandled event:"), __curr_event);
            }
        }

        void EventHandler::runEventHandling() {
            while (!stop_event_handling) {
                {
                // should be locked the entire time because the event may: stop event writing/clear event list
                std::lock_guard<std::mutex> lk1(event_list_mutex);

                if (!events.empty()) {
                    uint32_t __curr_event = events.front();
                    events.pop_front();

                    std::lock_guard<std::mutex> lk2(event_handling_mutex);
                    if (event_handles.find(__curr_event) != event_handles.end())
                        event_handles[__curr_event]->handle(this, __curr_event);
                    else
                        printf("%s %d\n", SVKFW_WRAPINFO("EventHandler", "unhandled event:"), __curr_event);
                }
                }
                std::this_thread::yield(); // let other threads write events (the list mutex is released here)
            }
        }

        void EventHandler::runEventHandlingLong() {
            bool has_event = false;
            uint32_t __curr_event = 0u;
            while (!stop_event_handling) {
                has_event = false;

                // Get event
                {
                    std::lock_guard<std::mutex> lk(event_list_mutex);
                    has_event = !events.empty();
                    if (has_event) {
                        __curr_event = events.front();
                        events.pop_front();
                    }
                }

                // Handle event
                if (has_event) {
                    std::lock_guard<std::mutex> lk1(event_list_mutex);
                    std::lock_guard<std::mutex> lk(event_handling_mutex);

                    if (event_handles.find(__curr_event) != event_handles.end())
                        event_handles[__curr_event]->handle(this, __curr_event);
                    else
                        printf("%s %d\n", SVKFW_WRAPINFO("EventHandler", "unhandled event:"), __curr_event);
                }
                else
                    std::this_thread::yield();
            }
        }

        void EventHandler::runEventHandlingAsync() {
            // std::async(runEventHandling, this);
        }

        EventHandler Event_Handler;
    }; // Event END
}; // Simple END
