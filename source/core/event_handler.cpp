#include "core/event_handler.h"

#include <cassert>
#include <algorithm>

namespace Simple {
    namespace Event {

        EventHandler::EventHandler() : owner_(std::this_thread::get_id()) {}

        void EventHandler::checkThread_() {
            // The first subscriber / poller owns the handler (it may be constructed on another thread)
            if (dispatch_depth_ == 0u && subscriptions_.empty() && added_.empty()) owner_ = std::this_thread::get_id();
            assert(owner_ == std::this_thread::get_id() && "EventHandler: subscribe/dispatch/poll from one thread only");
        }

        bool EventHandler::write(const Event &_event) {
            if (writing_stopped_) return false;
            std::lock_guard<std::mutex> lk(queue_mutex_);
            queue_.push_back(_event);
            return true;
        }

        bool EventHandler::write(const std::vector<Event> &_events) {
            if (writing_stopped_) return false;
            std::lock_guard<std::mutex> lk(queue_mutex_);
            queue_.insert(queue_.end(), _events.begin(), _events.end());
            return true;
        }

        void EventHandler::clearQueue() {
            std::lock_guard<std::mutex> lk(queue_mutex_);
            queue_.clear();
        }

        size_t EventHandler::queued() {
            std::lock_guard<std::mutex> lk(queue_mutex_);
            return queue_.size();
        }

        SubscriptionID EventHandler::subscribe(uint32_t _type, const HandleFunc &_func, uint32_t _uses) {
            checkThread_();
            Subscription __sub;
            __sub.id = next_id_++;
            if (next_id_ == 0u) next_id_ = 1u;
            __sub.type = _type;
            __sub.func = _func;
            __sub.uses_left = _uses;
            // During dispatching the vectors are being iterated: add afterwards
            if (dispatch_depth_ > 0u) added_.push_back(__sub);
            else subscriptions_[_type].push_back(__sub);
            return __sub.id;
        }

        bool EventHandler::unsubscribe(SubscriptionID _id) {
            checkThread_();
            if (_id == 0u) return false;
            for (size_t i = 0u; i < added_.size(); ++i)
                if (added_[i].id == _id) { added_.erase(added_.begin() + i); return true; }
            for (auto &type_subs : subscriptions_)
                for (Subscription &sub : type_subs.second)
                    if (sub.id == _id) {
                        // Marked (id 0) during dispatching, erased by 'flushChanges_'
                        if (dispatch_depth_ > 0u) { sub.id = 0u; removed_ = true; }
                        else type_subs.second.erase(type_subs.second.begin() + (&sub - type_subs.second.data()));
                        return true;
                    }
            return false;
        }

        void EventHandler::unsubscribeAll(uint32_t _type) {
            checkThread_();
            for (size_t i = added_.size(); i-- > 0u; )
                if (added_[i].type == _type) added_.erase(added_.begin() + i);
            auto __it = subscriptions_.find(_type);
            if (__it == subscriptions_.end()) return;
            if (dispatch_depth_ > 0u) {
                for (Subscription &sub : __it->second) sub.id = 0u;
                removed_ = true;
            } else
                subscriptions_.erase(__it);
        }

        void EventHandler::flushChanges_() {
            if (removed_) {
                for (auto &type_subs : subscriptions_) {
                    std::vector<Subscription> &__subs = type_subs.second;
                    __subs.erase(std::remove_if(__subs.begin(), __subs.end(), [](const Subscription &_s) { return _s.id == 0u; }),
                                 __subs.end());
                }
                removed_ = false;
            }
            for (const Subscription &sub : added_) subscriptions_[sub.type].push_back(sub);
            added_.clear();
        }

        void EventHandler::dispatch(const Event &_event) {
            checkThread_();
            auto __it = subscriptions_.find(_event.type);
            if (__it == subscriptions_.end()) return;
            ++dispatch_depth_;
            // Note: subscriptions added by the handlers are appended after dispatching, so the vector is not reallocated here
            std::vector<Subscription> &__subs = __it->second;
            for (size_t i = 0u; i < __subs.size(); ++i) {
                if (__subs[i].id == 0u) continue;
                if (__subs[i].uses_left > 0u && --__subs[i].uses_left == 0u) {
                    // The last use: unsubscribe first, so a handler that throws doesn't stay
                    HandleFunc __func = __subs[i].func;
                    __subs[i].id = 0u;
                    removed_ = true;
                    __func(*this, _event);
                } else
                    __subs[i].func(*this, _event);
            }
            if (--dispatch_depth_ == 0u) flushChanges_();
        }

        uint32_t EventHandler::poll(uint32_t _max_rounds) {
            checkThread_();
            uint32_t __count = 0u;
            for (uint32_t round = 0u; round < _max_rounds; ++round) {
                processing_.clear();
                {
                    std::lock_guard<std::mutex> lk(queue_mutex_);
                    processing_.swap(queue_);
                }
                if (processing_.empty()) break;
                // Events written by the handlers go to 'queue_' and are dispatched in the next round
                for (size_t i = 0u; i < processing_.size(); ++i) dispatch(processing_[i]);
                __count += (uint32_t)processing_.size();
            }
            processing_.clear();
            return __count;
        }
    }; // Event END
}; // Simple END
