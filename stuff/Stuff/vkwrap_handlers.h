#ifndef SVKFW_VKWRAP_HANDLERS_H
#define SVKFW_VKWRAP_HANDLERS_H

#include "common/terminal.h"
#include "glfwrap.h"

#include <vector>


namespace Simple {

//  ===================================================  \\
                Vulkan framework - handlers
//  ===================================================  \\


//  ============  Destroyable objects  ============  \\



    namespace VKFWInstance {
        struct BaseHandler {
        // Data:

            VkInstance instance = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkInstance _instance, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : instance{_instance}, destroy_callback{_destroy_callback} {}
            BaseHandler(const BaseHandler &_handler)
                            : instance{_handler.instance}, destroy_callback{_handler.destroy_callback} {}
            BaseHandler(BaseHandler &&_handler)
                            : instance{_handler.instance}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                instance = _handler.instance;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                instance = _handler.instance;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkInstance* ptr() const { return &instance; }


        // Destroy Vk object:

            void destroyVkObject() {
                vkDestroyInstance(instance, destroy_callback);
            }


        // Clear state:

            void clearHandler() {
                instance = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseHandler END


        struct Handler : BaseHandler {
        // Constructors:

            Handler() {}
            Handler(VkInstance _instance, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseHandler{_instance, _destroy_callback} {}
            Handler(const BaseHandler &_handler) : BaseHandler{_handler} {}
            Handler(BaseHandler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }
            Handler(const Handler &_handler) : BaseHandler{_handler} {}
            Handler(Handler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }

            ~Handler() {
                if (instance != VK_NULL_HANDLE)
                    destroyVkObject();
            }


        // Assignment:

            Handler& operator=(const Handler &_handler) {
                instance = _handler.instance;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                instance = _handler.instance;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }
        }; // Handler END
    }; // VKFWInstance END


    namespace VKFWDevice {
        struct BaseHandler {
        // Data:

            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : device{_device}, destroy_callback{_destroy_callback} {}
            BaseHandler(const BaseHandler &_handler)
                            : device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseHandler(BaseHandler &&_handler)
                            : device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkDevice* ptr() const { return &device; }


        // Destroy Vk object:

            void destroyVkObject() {
                vkDestroyDevice(device, destroy_callback);
            }


        // Clear state:

            void clearHandler() {
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseHandler END


        struct Handler : BaseHandler {
        // Constructors:

            Handler() {}
            Handler(VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseHandler{_device, _destroy_callback} {}
            Handler(const BaseHandler &_handler) : BaseHandler{_handler} {}
            Handler(BaseHandler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }
            Handler(const Handler &_handler) : BaseHandler{_handler} {}
            Handler(Handler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }

            ~Handler() {
                if (device != VK_NULL_HANDLE)
                    destroyVkObject();
            }


        // Assignment:

            Handler& operator=(const Handler &_handler) {
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }
        }; // Handler END
    }; // VKFWDevice END


    namespace VKFWFence {
        struct BaseHandler {
        // Data:

            VkFence fence = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkFence _fence, VkDevice _device = VK_NULL_HANDLE, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : fence{_fence}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseHandler(const BaseHandler &_handler)
                            : fence{_handler.fence}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseHandler(BaseHandler &&_handler)
                            : fence{_handler.fence}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                fence = _handler.fence;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                fence = _handler.fence;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkFence* ptr() const { return &fence; }


        // Destroy Vk object:

            void destroyVkObject() {
                vkDestroyFence(device, fence, destroy_callback);
            }


        // Clear state:

            void clearHandler() {
                fence = VK_NULL_HANDLE;
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseHandler END


        struct Handler : BaseHandler {
        // Constructors:

            Handler() {}
            Handler(VkFence _fence, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseHandler{_fence, _device, _destroy_callback} {}
            Handler(const BaseHandler &_handler) : BaseHandler{_handler} {}
            Handler(BaseHandler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }
            Handler(const Handler &_handler) : BaseHandler{_handler} {}
            Handler(Handler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }

            ~Handler() {
                if (fence != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
                    destroyVkObject();
            }


        // Assignment:

            Handler& operator=(const Handler &_handler) {
                fence = _handler.fence;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                fence = _handler.fence;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }
        }; // Handler END


        struct BaseVecHandler {
        // Data:

            std::vector<VkFence> fences;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseVecHandler() {}
            BaseVecHandler(const std::vector<VkFence> &_fences, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : fences{_fences}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseVecHandler(const BaseHandler &_handler)
                            : fences{_handler.fence}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseVecHandler(BaseHandler &&_handler)
                            : fences{_handler.fence}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            BaseVecHandler(const BaseVecHandler &_handler)
                            : fences{_handler.fences}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseVecHandler(BaseVecHandler &&_handler)
                            : fences{_handler.fences}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearVecHandler();
            }
            virtual ~BaseVecHandler() {
                clearVecHandler();
            }


        // Assignment:

            BaseVecHandler& operator=(const BaseHandler &_handler) {
                fences = { _handler.fence };
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseVecHandler& operator=(BaseHandler &&_handler) {
                fences = { _handler.fence };
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }

            BaseVecHandler& operator=(const BaseVecHandler &_handler) {
                fences = _handler.fences;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseVecHandler& operator=(BaseVecHandler &&_handler) {
                fences = _handler.fences;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearVecHandler();
                return *this;
            }


        // Access by index:

            VkFence operator[](uint32_t _index) const { return fences[_index]; }
            BaseHandler getBaseHandler(uint32_t _index) const { return { fences[_index], device, destroy_callback }; }


        // Pointer and count:

            const VkFence* ptr() const { return fences.data(); }
            uint32_t count() const { return fences.size(); }


        // Destroy all Vk objects in vector:

            void destroyAllVkObjects() {
                for (auto fence : fences)
                    vkDestroyFence(device, fence, destroy_callback);
            }


        // Clear state:

            void clearVecHandler() {
                fences.clear();
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseVecHandler END


        struct VecHandler : BaseVecHandler {
        // Constructors:

            VecHandler() {}
            VecHandler(const std::vector<VkFence> &_fences, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseVecHandler{fences, device, destroy_callback} {}
            VecHandler(const BaseHandler &_handler) : BaseVecHandler{_handler} {}
            VecHandler(BaseHandler &&_handler) : BaseVecHandler{_handler} {
                _handler.clearHandler();
            }
            VecHandler(const BaseVecHandler &_handler) : BaseVecHandler{_handler} {}
            VecHandler(BaseVecHandler &&_handler) : BaseVecHandler{_handler} {
                _handler.clearVecHandler();
            }
            VecHandler(const VecHandler &_handler) : BaseVecHandler{_handler} {}
            VecHandler(VecHandler &&_handler) : BaseVecHandler{_handler} {
                _handler.clearVecHandler();
            }

            ~VecHandler() {
                if (!fences.empty() && device != VK_NULL_HANDLE)
                    destroyAllVkObjects();
            }


        // Assignment:

            VecHandler& operator=(const VecHandler &_handler) {
                fences = _handler.fences;
                        device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            VecHandler& operator=(VecHandler &&_handler) {
                fences = _handler.fences;
                        device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearVecHandler();
                return *this;
            }
        }; // VecHandler END
    }; // VKFWFence END


    namespace VKFWSemaphore {
        struct BaseHandler {
        // Data:

            VkSemaphore semaphore = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkSemaphore _semaphore, VkDevice _device = VK_NULL_HANDLE, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : semaphore{_semaphore}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseHandler(const BaseHandler &_handler)
                            : semaphore{_handler.semaphore}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseHandler(BaseHandler &&_handler)
                            : semaphore{_handler.semaphore}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                semaphore = _handler.semaphore;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                semaphore = _handler.semaphore;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkSemaphore* ptr() const { return &semaphore; }


        // Destroy Vk object:

            void destroyVkObject() {
                vkDestroySemaphore(device, semaphore, destroy_callback);
            }


        // Clear state:

            void clearHandler() {
                semaphore = VK_NULL_HANDLE;
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseHandler END


        struct Handler : BaseHandler {
        // Constructors:

            Handler() {}
            Handler(VkSemaphore _semaphore, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseHandler{_semaphore, _device, _destroy_callback} {}
            Handler(const BaseHandler &_handler) : BaseHandler{_handler} {}
            Handler(BaseHandler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }
            Handler(const Handler &_handler) : BaseHandler{_handler} {}
            Handler(Handler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }

            ~Handler() {
                if (semaphore != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
                    destroyVkObject();
            }


        // Assignment:

            Handler& operator=(const Handler &_handler) {
                semaphore = _handler.semaphore;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                semaphore = _handler.semaphore;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }
        }; // Handler END


        struct BaseVecHandler {
        // Data:

            std::vector<VkSemaphore> semaphores;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseVecHandler() {}
            BaseVecHandler(const std::vector<VkSemaphore> &_semaphores, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : semaphores{_semaphores}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseVecHandler(const BaseHandler &_handler)
                            : semaphores{_handler.semaphore}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseVecHandler(BaseHandler &&_handler)
                            : semaphores{_handler.semaphore}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            BaseVecHandler(const BaseVecHandler &_handler)
                            : semaphores{_handler.semaphores}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseVecHandler(BaseVecHandler &&_handler)
                            : semaphores{_handler.semaphores}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearVecHandler();
            }
            virtual ~BaseVecHandler() {
                clearVecHandler();
            }


        // Assignment:

            BaseVecHandler& operator=(const BaseHandler &_handler) {
                semaphores = { _handler.semaphore };
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseVecHandler& operator=(BaseHandler &&_handler) {
                semaphores = { _handler.semaphore };
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }

            BaseVecHandler& operator=(const BaseVecHandler &_handler) {
                semaphores = _handler.semaphores;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseVecHandler& operator=(BaseVecHandler &&_handler) {
                semaphores = _handler.semaphores;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearVecHandler();
                return *this;
            }


        // Access by index:

            VkSemaphore operator[](uint32_t _index) const { return semaphores[_index]; }
            BaseHandler getBaseHandler(uint32_t _index) const { return { semaphores[_index], device, destroy_callback }; }


        // Pointer and count:

            const VkSemaphore* ptr() const { return semaphores.data(); }
            uint32_t count() const { return semaphores.size(); }


        // Destroy all Vk objects in vector:

            void destroyAllVkObjects() {
                for (auto semaphore : semaphores)
                    vkDestroySemaphore(device, semaphore, destroy_callback);
            }


        // Clear state:

            void clearVecHandler() {
                semaphores.clear();
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseVecHandler END


        struct VecHandler : BaseVecHandler {
        // Constructors:

            VecHandler() {}
            VecHandler(const std::vector<VkSemaphore> &_semaphores, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseVecHandler{semaphores, device, destroy_callback} {}
            VecHandler(const BaseHandler &_handler) : BaseVecHandler{_handler} {}
            VecHandler(BaseHandler &&_handler) : BaseVecHandler{_handler} {
                _handler.clearHandler();
            }
            VecHandler(const BaseVecHandler &_handler) : BaseVecHandler{_handler} {}
            VecHandler(BaseVecHandler &&_handler) : BaseVecHandler{_handler} {
                _handler.clearVecHandler();
            }
            VecHandler(const VecHandler &_handler) : BaseVecHandler{_handler} {}
            VecHandler(VecHandler &&_handler) : BaseVecHandler{_handler} {
                _handler.clearVecHandler();
            }

            ~VecHandler() {
                if (!semaphores.empty() && device != VK_NULL_HANDLE)
                    destroyAllVkObjects();
            }


        // Assignment:

            VecHandler& operator=(const VecHandler &_handler) {
                semaphores = _handler.semaphores;
                        device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            VecHandler& operator=(VecHandler &&_handler) {
                semaphores = _handler.semaphores;
                        device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearVecHandler();
                return *this;
            }
        }; // VecHandler END
    }; // VKFWSemaphore END


    namespace VKFWEvent {
        struct BaseHandler {
        // Data:

            VkEvent event = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkEvent _event, VkDevice _device = VK_NULL_HANDLE, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : event{_event}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseHandler(const BaseHandler &_handler)
                            : event{_handler.event}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseHandler(BaseHandler &&_handler)
                            : event{_handler.event}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                event = _handler.event;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                event = _handler.event;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkEvent* ptr() const { return &event; }


        // Destroy Vk object:

            void destroyVkObject() {
                vkDestroyEvent(device, event, destroy_callback);
            }


        // Clear state:

            void clearHandler() {
                event = VK_NULL_HANDLE;
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseHandler END


        struct Handler : BaseHandler {
        // Constructors:

            Handler() {}
            Handler(VkEvent _event, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseHandler{_event, _device, _destroy_callback} {}
            Handler(const BaseHandler &_handler) : BaseHandler{_handler} {}
            Handler(BaseHandler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }
            Handler(const Handler &_handler) : BaseHandler{_handler} {}
            Handler(Handler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }

            ~Handler() {
                if (event != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
                    destroyVkObject();
            }


        // Assignment:

            Handler& operator=(const Handler &_handler) {
                event = _handler.event;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                event = _handler.event;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }
        }; // Handler END
    }; // VKFWEvent END


    namespace VKFWQueryPool {
        struct BaseHandler {
        // Data:

            VkQueryPool query_pool = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkQueryPool _query_pool, VkDevice _device = VK_NULL_HANDLE, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : query_pool{_query_pool}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseHandler(const BaseHandler &_handler)
                            : query_pool{_handler.query_pool}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseHandler(BaseHandler &&_handler)
                            : query_pool{_handler.query_pool}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                query_pool = _handler.query_pool;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                query_pool = _handler.query_pool;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkQueryPool* ptr() const { return &query_pool; }


        // Destroy Vk object:

            void destroyVkObject() {
                vkDestroyQueryPool(device, query_pool, destroy_callback);
            }


        // Clear state:

            void clearHandler() {
                query_pool = VK_NULL_HANDLE;
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseHandler END


        struct Handler : BaseHandler {
        // Constructors:

            Handler() {}
            Handler(VkQueryPool _query_pool, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseHandler{_query_pool, _device, _destroy_callback} {}
            Handler(const BaseHandler &_handler) : BaseHandler{_handler} {}
            Handler(BaseHandler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }
            Handler(const Handler &_handler) : BaseHandler{_handler} {}
            Handler(Handler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }

            ~Handler() {
                if (query_pool != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
                    destroyVkObject();
            }


        // Assignment:

            Handler& operator=(const Handler &_handler) {
                query_pool = _handler.query_pool;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                query_pool = _handler.query_pool;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }
        }; // Handler END
    }; // VKFWQueryPool END


    namespace VKFWBuffer {
        struct BaseHandler {
        // Data:

            VkBuffer buffer = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkBuffer _buffer, VkDevice _device = VK_NULL_HANDLE, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : buffer{_buffer}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseHandler(const BaseHandler &_handler)
                            : buffer{_handler.buffer}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseHandler(BaseHandler &&_handler)
                            : buffer{_handler.buffer}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                buffer = _handler.buffer;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                buffer = _handler.buffer;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkBuffer* ptr() const { return &buffer; }


        // Destroy Vk object:

            void destroyVkObject() {
                vkDestroyBuffer(device, buffer, destroy_callback);
            }


        // Clear state:

            void clearHandler() {
                buffer = VK_NULL_HANDLE;
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseHandler END


        struct Handler : BaseHandler {
        // Constructors:

            Handler() {}
            Handler(VkBuffer _buffer, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseHandler{_buffer, _device, _destroy_callback} {}
            Handler(const BaseHandler &_handler) : BaseHandler{_handler} {}
            Handler(BaseHandler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }
            Handler(const Handler &_handler) : BaseHandler{_handler} {}
            Handler(Handler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }

            ~Handler() {
                if (buffer != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
                    destroyVkObject();
            }


        // Assignment:

            Handler& operator=(const Handler &_handler) {
                buffer = _handler.buffer;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                buffer = _handler.buffer;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }
        }; // Handler END
    }; // VKFWBuffer END


    namespace VKFWBufferView {
        struct BaseHandler {
        // Data:

            VkBufferView buffer_view = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkBufferView _buffer_view, VkDevice _device = VK_NULL_HANDLE, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : buffer_view{_buffer_view}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseHandler(const BaseHandler &_handler)
                            : buffer_view{_handler.buffer_view}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseHandler(BaseHandler &&_handler)
                            : buffer_view{_handler.buffer_view}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                buffer_view = _handler.buffer_view;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                buffer_view = _handler.buffer_view;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkBufferView* ptr() const { return &buffer_view; }


        // Destroy Vk object:

            void destroyVkObject() {
                vkDestroyBufferView(device, buffer_view, destroy_callback);
            }


        // Clear state:

            void clearHandler() {
                buffer_view = VK_NULL_HANDLE;
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseHandler END


        struct Handler : BaseHandler {
        // Constructors:

            Handler() {}
            Handler(VkBufferView _buffer_view, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseHandler{_buffer_view, _device, _destroy_callback} {}
            Handler(const BaseHandler &_handler) : BaseHandler{_handler} {}
            Handler(BaseHandler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }
            Handler(const Handler &_handler) : BaseHandler{_handler} {}
            Handler(Handler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }

            ~Handler() {
                if (buffer_view != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
                    destroyVkObject();
            }


        // Assignment:

            Handler& operator=(const Handler &_handler) {
                buffer_view = _handler.buffer_view;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                buffer_view = _handler.buffer_view;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }
        }; // Handler END
    }; // VKFWBufferView END


    namespace VKFWImage {
        struct BaseHandler {
        // Data:

            VkImage image = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkImage _image, VkDevice _device = VK_NULL_HANDLE, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : image{_image}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseHandler(const BaseHandler &_handler)
                            : image{_handler.image}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseHandler(BaseHandler &&_handler)
                            : image{_handler.image}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                image = _handler.image;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                image = _handler.image;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkImage* ptr() const { return &image; }


        // Destroy Vk object:

            void destroyVkObject() {
                vkDestroyImage(device, image, destroy_callback);
            }


        // Clear state:

            void clearHandler() {
                image = VK_NULL_HANDLE;
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseHandler END


        struct Handler : BaseHandler {
        // Constructors:

            Handler() {}
            Handler(VkImage _image, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseHandler{_image, _device, _destroy_callback} {}
            Handler(const BaseHandler &_handler) : BaseHandler{_handler} {}
            Handler(BaseHandler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }
            Handler(const Handler &_handler) : BaseHandler{_handler} {}
            Handler(Handler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }

            ~Handler() {
                if (image != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
                    destroyVkObject();
            }


        // Assignment:

            Handler& operator=(const Handler &_handler) {
                image = _handler.image;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                image = _handler.image;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }
        }; // Handler END


        struct BaseVecHandler {
        // Data:

            std::vector<VkImage> images;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseVecHandler() {}
            BaseVecHandler(const std::vector<VkImage> &_images, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : images{_images}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseVecHandler(const BaseHandler &_handler)
                            : images{_handler.image}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseVecHandler(BaseHandler &&_handler)
                            : images{_handler.image}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            BaseVecHandler(const BaseVecHandler &_handler)
                            : images{_handler.images}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseVecHandler(BaseVecHandler &&_handler)
                            : images{_handler.images}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearVecHandler();
            }
            virtual ~BaseVecHandler() {
                clearVecHandler();
            }


        // Assignment:

            BaseVecHandler& operator=(const BaseHandler &_handler) {
                images = { _handler.image };
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseVecHandler& operator=(BaseHandler &&_handler) {
                images = { _handler.image };
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }

            BaseVecHandler& operator=(const BaseVecHandler &_handler) {
                images = _handler.images;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseVecHandler& operator=(BaseVecHandler &&_handler) {
                images = _handler.images;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearVecHandler();
                return *this;
            }


        // Access by index:

            VkImage operator[](uint32_t _index) const { return images[_index]; }
            BaseHandler getBaseHandler(uint32_t _index) const { return { images[_index], device, destroy_callback }; }


        // Pointer and count:

            const VkImage* ptr() const { return images.data(); }
            uint32_t count() const { return images.size(); }


        // Destroy all Vk objects in vector:

            void destroyAllVkObjects() {
                for (auto image : images)
                    vkDestroyImage(device, image, destroy_callback);
            }


        // Clear state:

            void clearVecHandler() {
                images.clear();
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseVecHandler END


        struct VecHandler : BaseVecHandler {
        // Constructors:

            VecHandler() {}
            VecHandler(const std::vector<VkImage> &_images, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseVecHandler{images, device, destroy_callback} {}
            VecHandler(const BaseHandler &_handler) : BaseVecHandler{_handler} {}
            VecHandler(BaseHandler &&_handler) : BaseVecHandler{_handler} {
                _handler.clearHandler();
            }
            VecHandler(const BaseVecHandler &_handler) : BaseVecHandler{_handler} {}
            VecHandler(BaseVecHandler &&_handler) : BaseVecHandler{_handler} {
                _handler.clearVecHandler();
            }
            VecHandler(const VecHandler &_handler) : BaseVecHandler{_handler} {}
            VecHandler(VecHandler &&_handler) : BaseVecHandler{_handler} {
                _handler.clearVecHandler();
            }

            ~VecHandler() {
                if (!images.empty() && device != VK_NULL_HANDLE)
                    destroyAllVkObjects();
            }


        // Assignment:

            VecHandler& operator=(const VecHandler &_handler) {
                images = _handler.images;
                        device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            VecHandler& operator=(VecHandler &&_handler) {
                images = _handler.images;
                        device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearVecHandler();
                return *this;
            }
        }; // VecHandler END
    }; // VKFWImage END


    namespace VKFWImageView {
        struct BaseHandler {
        // Data:

            VkImageView image_view = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkImageView _image_view, VkDevice _device = VK_NULL_HANDLE, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : image_view{_image_view}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseHandler(const BaseHandler &_handler)
                            : image_view{_handler.image_view}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseHandler(BaseHandler &&_handler)
                            : image_view{_handler.image_view}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                image_view = _handler.image_view;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                image_view = _handler.image_view;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkImageView* ptr() const { return &image_view; }


        // Destroy Vk object:

            void destroyVkObject() {
                vkDestroyImageView(device, image_view, destroy_callback);
            }


        // Clear state:

            void clearHandler() {
                image_view = VK_NULL_HANDLE;
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseHandler END


        struct Handler : BaseHandler {
        // Constructors:

            Handler() {}
            Handler(VkImageView _image_view, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseHandler{_image_view, _device, _destroy_callback} {}
            Handler(const BaseHandler &_handler) : BaseHandler{_handler} {}
            Handler(BaseHandler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }
            Handler(const Handler &_handler) : BaseHandler{_handler} {}
            Handler(Handler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }

            ~Handler() {
                if (image_view != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
                    destroyVkObject();
            }


        // Assignment:

            Handler& operator=(const Handler &_handler) {
                image_view = _handler.image_view;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                image_view = _handler.image_view;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }
        }; // Handler END


        struct BaseVecHandler {
        // Data:

            std::vector<VkImageView> image_views;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseVecHandler() {}
            BaseVecHandler(const std::vector<VkImageView> &_image_views, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : image_views{_image_views}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseVecHandler(const BaseHandler &_handler)
                            : image_views{_handler.image_view}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseVecHandler(BaseHandler &&_handler)
                            : image_views{_handler.image_view}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            BaseVecHandler(const BaseVecHandler &_handler)
                            : image_views{_handler.image_views}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseVecHandler(BaseVecHandler &&_handler)
                            : image_views{_handler.image_views}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearVecHandler();
            }
            virtual ~BaseVecHandler() {
                clearVecHandler();
            }


        // Assignment:

            BaseVecHandler& operator=(const BaseHandler &_handler) {
                image_views = { _handler.image_view };
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseVecHandler& operator=(BaseHandler &&_handler) {
                image_views = { _handler.image_view };
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }

            BaseVecHandler& operator=(const BaseVecHandler &_handler) {
                image_views = _handler.image_views;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseVecHandler& operator=(BaseVecHandler &&_handler) {
                image_views = _handler.image_views;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearVecHandler();
                return *this;
            }


        // Access by index:

            VkImageView operator[](uint32_t _index) const { return image_views[_index]; }
            BaseHandler getBaseHandler(uint32_t _index) const { return { image_views[_index], device, destroy_callback }; }


        // Pointer and count:

            const VkImageView* ptr() const { return image_views.data(); }
            uint32_t count() const { return image_views.size(); }


        // Destroy all Vk objects in vector:

            void destroyAllVkObjects() {
                for (auto image_view : image_views)
                    vkDestroyImageView(device, image_view, destroy_callback);
            }


        // Clear state:

            void clearVecHandler() {
                image_views.clear();
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseVecHandler END


        struct VecHandler : BaseVecHandler {
        // Constructors:

            VecHandler() {}
            VecHandler(const std::vector<VkImageView> &_image_views, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseVecHandler{image_views, device, destroy_callback} {}
            VecHandler(const BaseHandler &_handler) : BaseVecHandler{_handler} {}
            VecHandler(BaseHandler &&_handler) : BaseVecHandler{_handler} {
                _handler.clearHandler();
            }
            VecHandler(const BaseVecHandler &_handler) : BaseVecHandler{_handler} {}
            VecHandler(BaseVecHandler &&_handler) : BaseVecHandler{_handler} {
                _handler.clearVecHandler();
            }
            VecHandler(const VecHandler &_handler) : BaseVecHandler{_handler} {}
            VecHandler(VecHandler &&_handler) : BaseVecHandler{_handler} {
                _handler.clearVecHandler();
            }

            ~VecHandler() {
                if (!image_views.empty() && device != VK_NULL_HANDLE)
                    destroyAllVkObjects();
            }


        // Assignment:

            VecHandler& operator=(const VecHandler &_handler) {
                image_views = _handler.image_views;
                        device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            VecHandler& operator=(VecHandler &&_handler) {
                image_views = _handler.image_views;
                        device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearVecHandler();
                return *this;
            }
        }; // VecHandler END
    }; // VKFWImageView END


    namespace VKFWShaderModule {
        struct BaseHandler {
        // Data:

            VkShaderModule shader_module = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkShaderModule _shader_module, VkDevice _device = VK_NULL_HANDLE, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : shader_module{_shader_module}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseHandler(const BaseHandler &_handler)
                            : shader_module{_handler.shader_module}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseHandler(BaseHandler &&_handler)
                            : shader_module{_handler.shader_module}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                shader_module = _handler.shader_module;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                shader_module = _handler.shader_module;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkShaderModule* ptr() const { return &shader_module; }


        // Destroy Vk object:

            void destroyVkObject() {
                vkDestroyShaderModule(device, shader_module, destroy_callback);
            }


        // Clear state:

            void clearHandler() {
                shader_module = VK_NULL_HANDLE;
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseHandler END


        struct Handler : BaseHandler {
        // Constructors:

            Handler() {}
            Handler(VkShaderModule _shader_module, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseHandler{_shader_module, _device, _destroy_callback} {}
            Handler(const BaseHandler &_handler) : BaseHandler{_handler} {}
            Handler(BaseHandler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }
            Handler(const Handler &_handler) : BaseHandler{_handler} {}
            Handler(Handler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }

            ~Handler() {
                if (shader_module != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
                    destroyVkObject();
            }


        // Assignment:

            Handler& operator=(const Handler &_handler) {
                shader_module = _handler.shader_module;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                shader_module = _handler.shader_module;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }
        }; // Handler END


        struct BaseVecHandler {
        // Data:

            std::vector<VkShaderModule> shader_modules;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseVecHandler() {}
            BaseVecHandler(const std::vector<VkShaderModule> &_shader_modules, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : shader_modules{_shader_modules}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseVecHandler(const BaseHandler &_handler)
                            : shader_modules{_handler.shader_module}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseVecHandler(BaseHandler &&_handler)
                            : shader_modules{_handler.shader_module}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            BaseVecHandler(const BaseVecHandler &_handler)
                            : shader_modules{_handler.shader_modules}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseVecHandler(BaseVecHandler &&_handler)
                            : shader_modules{_handler.shader_modules}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearVecHandler();
            }
            virtual ~BaseVecHandler() {
                clearVecHandler();
            }


        // Assignment:

            BaseVecHandler& operator=(const BaseHandler &_handler) {
                shader_modules = { _handler.shader_module };
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseVecHandler& operator=(BaseHandler &&_handler) {
                shader_modules = { _handler.shader_module };
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }

            BaseVecHandler& operator=(const BaseVecHandler &_handler) {
                shader_modules = _handler.shader_modules;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseVecHandler& operator=(BaseVecHandler &&_handler) {
                shader_modules = _handler.shader_modules;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearVecHandler();
                return *this;
            }


        // Access by index:

            VkShaderModule operator[](uint32_t _index) const { return shader_modules[_index]; }
            BaseHandler getBaseHandler(uint32_t _index) const { return { shader_modules[_index], device, destroy_callback }; }


        // Pointer and count:

            const VkShaderModule* ptr() const { return shader_modules.data(); }
            uint32_t count() const { return shader_modules.size(); }


        // Destroy all Vk objects in vector:

            void destroyAllVkObjects() {
                for (auto shader_module : shader_modules)
                    vkDestroyShaderModule(device, shader_module, destroy_callback);
            }


        // Clear state:

            void clearVecHandler() {
                shader_modules.clear();
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseVecHandler END


        struct VecHandler : BaseVecHandler {
        // Constructors:

            VecHandler() {}
            VecHandler(const std::vector<VkShaderModule> &_shader_modules, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseVecHandler{shader_modules, device, destroy_callback} {}
            VecHandler(const BaseHandler &_handler) : BaseVecHandler{_handler} {}
            VecHandler(BaseHandler &&_handler) : BaseVecHandler{_handler} {
                _handler.clearHandler();
            }
            VecHandler(const BaseVecHandler &_handler) : BaseVecHandler{_handler} {}
            VecHandler(BaseVecHandler &&_handler) : BaseVecHandler{_handler} {
                _handler.clearVecHandler();
            }
            VecHandler(const VecHandler &_handler) : BaseVecHandler{_handler} {}
            VecHandler(VecHandler &&_handler) : BaseVecHandler{_handler} {
                _handler.clearVecHandler();
            }

            ~VecHandler() {
                if (!shader_modules.empty() && device != VK_NULL_HANDLE)
                    destroyAllVkObjects();
            }


        // Assignment:

            VecHandler& operator=(const VecHandler &_handler) {
                shader_modules = _handler.shader_modules;
                        device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            VecHandler& operator=(VecHandler &&_handler) {
                shader_modules = _handler.shader_modules;
                        device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearVecHandler();
                return *this;
            }
        }; // VecHandler END
    }; // VKFWShaderModule END


    namespace VKFWPipelineCache {
        struct BaseHandler {
        // Data:

            VkPipelineCache pipeline_cache = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkPipelineCache _pipeline_cache, VkDevice _device = VK_NULL_HANDLE, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : pipeline_cache{_pipeline_cache}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseHandler(const BaseHandler &_handler)
                            : pipeline_cache{_handler.pipeline_cache}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseHandler(BaseHandler &&_handler)
                            : pipeline_cache{_handler.pipeline_cache}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                pipeline_cache = _handler.pipeline_cache;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                pipeline_cache = _handler.pipeline_cache;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkPipelineCache* ptr() const { return &pipeline_cache; }


        // Destroy Vk object:

            void destroyVkObject() {
                vkDestroyPipelineCache(device, pipeline_cache, destroy_callback);
            }


        // Clear state:

            void clearHandler() {
                pipeline_cache = VK_NULL_HANDLE;
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseHandler END


        struct Handler : BaseHandler {
        // Constructors:

            Handler() {}
            Handler(VkPipelineCache _pipeline_cache, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseHandler{_pipeline_cache, _device, _destroy_callback} {}
            Handler(const BaseHandler &_handler) : BaseHandler{_handler} {}
            Handler(BaseHandler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }
            Handler(const Handler &_handler) : BaseHandler{_handler} {}
            Handler(Handler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }

            ~Handler() {
                if (pipeline_cache != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
                    destroyVkObject();
            }


        // Assignment:

            Handler& operator=(const Handler &_handler) {
                pipeline_cache = _handler.pipeline_cache;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                pipeline_cache = _handler.pipeline_cache;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }
        }; // Handler END
    }; // VKFWPipelineCache END


    namespace VKFWPipeline {
        struct BaseHandler {
        // Data:

            VkPipeline pipeline = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkPipeline _pipeline, VkDevice _device = VK_NULL_HANDLE, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : pipeline{_pipeline}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseHandler(const BaseHandler &_handler)
                            : pipeline{_handler.pipeline}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseHandler(BaseHandler &&_handler)
                            : pipeline{_handler.pipeline}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                pipeline = _handler.pipeline;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                pipeline = _handler.pipeline;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkPipeline* ptr() const { return &pipeline; }


        // Destroy Vk object:

            void destroyVkObject() {
                vkDestroyPipeline(device, pipeline, destroy_callback);
            }


        // Clear state:

            void clearHandler() {
                pipeline = VK_NULL_HANDLE;
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseHandler END


        struct Handler : BaseHandler {
        // Constructors:

            Handler() {}
            Handler(VkPipeline _pipeline, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseHandler{_pipeline, _device, _destroy_callback} {}
            Handler(const BaseHandler &_handler) : BaseHandler{_handler} {}
            Handler(BaseHandler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }
            Handler(const Handler &_handler) : BaseHandler{_handler} {}
            Handler(Handler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }

            ~Handler() {
                if (pipeline != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
                    destroyVkObject();
            }


        // Assignment:

            Handler& operator=(const Handler &_handler) {
                pipeline = _handler.pipeline;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                pipeline = _handler.pipeline;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }
        }; // Handler END
    }; // VKFWPipeline END


    namespace VKFWPipelineLayout {
        struct BaseHandler {
        // Data:

            VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkPipelineLayout _pipeline_layout, VkDevice _device = VK_NULL_HANDLE, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : pipeline_layout{_pipeline_layout}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseHandler(const BaseHandler &_handler)
                            : pipeline_layout{_handler.pipeline_layout}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseHandler(BaseHandler &&_handler)
                            : pipeline_layout{_handler.pipeline_layout}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                pipeline_layout = _handler.pipeline_layout;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                pipeline_layout = _handler.pipeline_layout;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkPipelineLayout* ptr() const { return &pipeline_layout; }


        // Destroy Vk object:

            void destroyVkObject() {
                vkDestroyPipelineLayout(device, pipeline_layout, destroy_callback);
            }


        // Clear state:

            void clearHandler() {
                pipeline_layout = VK_NULL_HANDLE;
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseHandler END


        struct Handler : BaseHandler {
        // Constructors:

            Handler() {}
            Handler(VkPipelineLayout _pipeline_layout, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseHandler{_pipeline_layout, _device, _destroy_callback} {}
            Handler(const BaseHandler &_handler) : BaseHandler{_handler} {}
            Handler(BaseHandler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }
            Handler(const Handler &_handler) : BaseHandler{_handler} {}
            Handler(Handler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }

            ~Handler() {
                if (pipeline_layout != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
                    destroyVkObject();
            }


        // Assignment:

            Handler& operator=(const Handler &_handler) {
                pipeline_layout = _handler.pipeline_layout;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                pipeline_layout = _handler.pipeline_layout;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }
        }; // Handler END
    }; // VKFWPipelineLayout END


    namespace VKFWSampler {
        struct BaseHandler {
        // Data:

            VkSampler sampler = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkSampler _sampler, VkDevice _device = VK_NULL_HANDLE, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : sampler{_sampler}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseHandler(const BaseHandler &_handler)
                            : sampler{_handler.sampler}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseHandler(BaseHandler &&_handler)
                            : sampler{_handler.sampler}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                sampler = _handler.sampler;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                sampler = _handler.sampler;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkSampler* ptr() const { return &sampler; }


        // Destroy Vk object:

            void destroyVkObject() {
                vkDestroySampler(device, sampler, destroy_callback);
            }


        // Clear state:

            void clearHandler() {
                sampler = VK_NULL_HANDLE;
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseHandler END


        struct Handler : BaseHandler {
        // Constructors:

            Handler() {}
            Handler(VkSampler _sampler, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseHandler{_sampler, _device, _destroy_callback} {}
            Handler(const BaseHandler &_handler) : BaseHandler{_handler} {}
            Handler(BaseHandler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }
            Handler(const Handler &_handler) : BaseHandler{_handler} {}
            Handler(Handler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }

            ~Handler() {
                if (sampler != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
                    destroyVkObject();
            }


        // Assignment:

            Handler& operator=(const Handler &_handler) {
                sampler = _handler.sampler;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                sampler = _handler.sampler;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }
        }; // Handler END
    }; // VKFWSampler END


    namespace VKFWDescriptorSetLayout {
        struct BaseHandler {
        // Data:

            VkDescriptorSetLayout descriptor_set_layout = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkDescriptorSetLayout _descriptor_set_layout, VkDevice _device = VK_NULL_HANDLE, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : descriptor_set_layout{_descriptor_set_layout}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseHandler(const BaseHandler &_handler)
                            : descriptor_set_layout{_handler.descriptor_set_layout}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseHandler(BaseHandler &&_handler)
                            : descriptor_set_layout{_handler.descriptor_set_layout}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                descriptor_set_layout = _handler.descriptor_set_layout;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                descriptor_set_layout = _handler.descriptor_set_layout;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkDescriptorSetLayout* ptr() const { return &descriptor_set_layout; }


        // Destroy Vk object:

            void destroyVkObject() {
                vkDestroyDescriptorSetLayout(device, descriptor_set_layout, destroy_callback);
            }


        // Clear state:

            void clearHandler() {
                descriptor_set_layout = VK_NULL_HANDLE;
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseHandler END


        struct Handler : BaseHandler {
        // Constructors:

            Handler() {}
            Handler(VkDescriptorSetLayout _descriptor_set_layout, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseHandler{_descriptor_set_layout, _device, _destroy_callback} {}
            Handler(const BaseHandler &_handler) : BaseHandler{_handler} {}
            Handler(BaseHandler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }
            Handler(const Handler &_handler) : BaseHandler{_handler} {}
            Handler(Handler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }

            ~Handler() {
                if (descriptor_set_layout != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
                    destroyVkObject();
            }


        // Assignment:

            Handler& operator=(const Handler &_handler) {
                descriptor_set_layout = _handler.descriptor_set_layout;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                descriptor_set_layout = _handler.descriptor_set_layout;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }
        }; // Handler END
    }; // VKFWDescriptorSetLayout END


    namespace VKFWDescriptorPool {
        struct BaseHandler {
        // Data:

            VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkDescriptorPool _descriptor_pool, VkDevice _device = VK_NULL_HANDLE, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : descriptor_pool{_descriptor_pool}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseHandler(const BaseHandler &_handler)
                            : descriptor_pool{_handler.descriptor_pool}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseHandler(BaseHandler &&_handler)
                            : descriptor_pool{_handler.descriptor_pool}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                descriptor_pool = _handler.descriptor_pool;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                descriptor_pool = _handler.descriptor_pool;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkDescriptorPool* ptr() const { return &descriptor_pool; }


        // Destroy Vk object:

            void destroyVkObject() {
                vkDestroyDescriptorPool(device, descriptor_pool, destroy_callback);
            }


        // Clear state:

            void clearHandler() {
                descriptor_pool = VK_NULL_HANDLE;
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseHandler END


        struct Handler : BaseHandler {
        // Constructors:

            Handler() {}
            Handler(VkDescriptorPool _descriptor_pool, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseHandler{_descriptor_pool, _device, _destroy_callback} {}
            Handler(const BaseHandler &_handler) : BaseHandler{_handler} {}
            Handler(BaseHandler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }
            Handler(const Handler &_handler) : BaseHandler{_handler} {}
            Handler(Handler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }

            ~Handler() {
                if (descriptor_pool != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
                    destroyVkObject();
            }


        // Assignment:

            Handler& operator=(const Handler &_handler) {
                descriptor_pool = _handler.descriptor_pool;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                descriptor_pool = _handler.descriptor_pool;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }
        }; // Handler END
    }; // VKFWDescriptorPool END


    namespace VKFWFramebuffer {
        struct BaseHandler {
        // Data:

            VkFramebuffer framebuffer = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkFramebuffer _framebuffer, VkDevice _device = VK_NULL_HANDLE, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : framebuffer{_framebuffer}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseHandler(const BaseHandler &_handler)
                            : framebuffer{_handler.framebuffer}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseHandler(BaseHandler &&_handler)
                            : framebuffer{_handler.framebuffer}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                framebuffer = _handler.framebuffer;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                framebuffer = _handler.framebuffer;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkFramebuffer* ptr() const { return &framebuffer; }


        // Destroy Vk object:

            void destroyVkObject() {
                vkDestroyFramebuffer(device, framebuffer, destroy_callback);
            }


        // Clear state:

            void clearHandler() {
                framebuffer = VK_NULL_HANDLE;
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseHandler END


        struct Handler : BaseHandler {
        // Constructors:

            Handler() {}
            Handler(VkFramebuffer _framebuffer, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseHandler{_framebuffer, _device, _destroy_callback} {}
            Handler(const BaseHandler &_handler) : BaseHandler{_handler} {}
            Handler(BaseHandler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }
            Handler(const Handler &_handler) : BaseHandler{_handler} {}
            Handler(Handler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }

            ~Handler() {
                if (framebuffer != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
                    destroyVkObject();
            }


        // Assignment:

            Handler& operator=(const Handler &_handler) {
                framebuffer = _handler.framebuffer;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                framebuffer = _handler.framebuffer;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }
        }; // Handler END


        struct BaseVecHandler {
        // Data:

            std::vector<VkFramebuffer> framebuffers;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseVecHandler() {}
            BaseVecHandler(const std::vector<VkFramebuffer> &_framebuffers, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : framebuffers{_framebuffers}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseVecHandler(const BaseHandler &_handler)
                            : framebuffers{_handler.framebuffer}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseVecHandler(BaseHandler &&_handler)
                            : framebuffers{_handler.framebuffer}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            BaseVecHandler(const BaseVecHandler &_handler)
                            : framebuffers{_handler.framebuffers}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseVecHandler(BaseVecHandler &&_handler)
                            : framebuffers{_handler.framebuffers}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearVecHandler();
            }
            virtual ~BaseVecHandler() {
                clearVecHandler();
            }


        // Assignment:

            BaseVecHandler& operator=(const BaseHandler &_handler) {
                framebuffers = { _handler.framebuffer };
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseVecHandler& operator=(BaseHandler &&_handler) {
                framebuffers = { _handler.framebuffer };
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }

            BaseVecHandler& operator=(const BaseVecHandler &_handler) {
                framebuffers = _handler.framebuffers;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseVecHandler& operator=(BaseVecHandler &&_handler) {
                framebuffers = _handler.framebuffers;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearVecHandler();
                return *this;
            }


        // Access by index:

            VkFramebuffer operator[](uint32_t _index) const { return framebuffers[_index]; }
            BaseHandler getBaseHandler(uint32_t _index) const { return { framebuffers[_index], device, destroy_callback }; }


        // Pointer and count:

            const VkFramebuffer* ptr() const { return framebuffers.data(); }
            uint32_t count() const { return framebuffers.size(); }


        // Destroy all Vk objects in vector:

            void destroyAllVkObjects() {
                for (auto framebuffer : framebuffers)
                    vkDestroyFramebuffer(device, framebuffer, destroy_callback);
            }


        // Clear state:

            void clearVecHandler() {
                framebuffers.clear();
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseVecHandler END


        struct VecHandler : BaseVecHandler {
        // Constructors:

            VecHandler() {}
            VecHandler(const std::vector<VkFramebuffer> &_framebuffers, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseVecHandler{framebuffers, device, destroy_callback} {}
            VecHandler(const BaseHandler &_handler) : BaseVecHandler{_handler} {}
            VecHandler(BaseHandler &&_handler) : BaseVecHandler{_handler} {
                _handler.clearHandler();
            }
            VecHandler(const BaseVecHandler &_handler) : BaseVecHandler{_handler} {}
            VecHandler(BaseVecHandler &&_handler) : BaseVecHandler{_handler} {
                _handler.clearVecHandler();
            }
            VecHandler(const VecHandler &_handler) : BaseVecHandler{_handler} {}
            VecHandler(VecHandler &&_handler) : BaseVecHandler{_handler} {
                _handler.clearVecHandler();
            }

            ~VecHandler() {
                if (!framebuffers.empty() && device != VK_NULL_HANDLE)
                    destroyAllVkObjects();
            }


        // Assignment:

            VecHandler& operator=(const VecHandler &_handler) {
                framebuffers = _handler.framebuffers;
                        device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            VecHandler& operator=(VecHandler &&_handler) {
                framebuffers = _handler.framebuffers;
                        device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearVecHandler();
                return *this;
            }
        }; // VecHandler END
    }; // VKFWFramebuffer END


    namespace VKFWRenderPass {
        struct BaseHandler {
        // Data:

            VkRenderPass render_pass = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkRenderPass _render_pass, VkDevice _device = VK_NULL_HANDLE, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : render_pass{_render_pass}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseHandler(const BaseHandler &_handler)
                            : render_pass{_handler.render_pass}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseHandler(BaseHandler &&_handler)
                            : render_pass{_handler.render_pass}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                render_pass = _handler.render_pass;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                render_pass = _handler.render_pass;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkRenderPass* ptr() const { return &render_pass; }


        // Destroy Vk object:

            void destroyVkObject() {
                vkDestroyRenderPass(device, render_pass, destroy_callback);
            }


        // Clear state:

            void clearHandler() {
                render_pass = VK_NULL_HANDLE;
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseHandler END


        struct Handler : BaseHandler {
        // Constructors:

            Handler() {}
            Handler(VkRenderPass _render_pass, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseHandler{_render_pass, _device, _destroy_callback} {}
            Handler(const BaseHandler &_handler) : BaseHandler{_handler} {}
            Handler(BaseHandler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }
            Handler(const Handler &_handler) : BaseHandler{_handler} {}
            Handler(Handler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }

            ~Handler() {
                if (render_pass != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
                    destroyVkObject();
            }


        // Assignment:

            Handler& operator=(const Handler &_handler) {
                render_pass = _handler.render_pass;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                render_pass = _handler.render_pass;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }
        }; // Handler END
    }; // VKFWRenderPass END


    namespace VKFWCommandPool {
        struct BaseHandler {
        // Data:

            VkCommandPool command_pool = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkCommandPool _command_pool, VkDevice _device = VK_NULL_HANDLE, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : command_pool{_command_pool}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseHandler(const BaseHandler &_handler)
                            : command_pool{_handler.command_pool}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseHandler(BaseHandler &&_handler)
                            : command_pool{_handler.command_pool}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                command_pool = _handler.command_pool;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                command_pool = _handler.command_pool;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkCommandPool* ptr() const { return &command_pool; }


        // Destroy Vk object:

            void destroyVkObject() {
                vkDestroyCommandPool(device, command_pool, destroy_callback);
            }


        // Clear state:

            void clearHandler() {
                command_pool = VK_NULL_HANDLE;
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseHandler END


        struct Handler : BaseHandler {
        // Constructors:

            Handler() {}
            Handler(VkCommandPool _command_pool, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseHandler{_command_pool, _device, _destroy_callback} {}
            Handler(const BaseHandler &_handler) : BaseHandler{_handler} {}
            Handler(BaseHandler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }
            Handler(const Handler &_handler) : BaseHandler{_handler} {}
            Handler(Handler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }

            ~Handler() {
                if (command_pool != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
                    destroyVkObject();
            }


        // Assignment:

            Handler& operator=(const Handler &_handler) {
                command_pool = _handler.command_pool;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                command_pool = _handler.command_pool;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }
        }; // Handler END
    }; // VKFWCommandPool END


    namespace VKFWSamplerYcbcrConversion {
        struct BaseHandler {
        // Data:

            VkSamplerYcbcrConversion ycbcr_conversion = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkSamplerYcbcrConversion _ycbcr_conversion, VkDevice _device = VK_NULL_HANDLE, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : ycbcr_conversion{_ycbcr_conversion}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseHandler(const BaseHandler &_handler)
                            : ycbcr_conversion{_handler.ycbcr_conversion}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseHandler(BaseHandler &&_handler)
                            : ycbcr_conversion{_handler.ycbcr_conversion}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                ycbcr_conversion = _handler.ycbcr_conversion;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                ycbcr_conversion = _handler.ycbcr_conversion;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkSamplerYcbcrConversion* ptr() const { return &ycbcr_conversion; }


        // Destroy Vk object:

            void destroyVkObject() {
                vkDestroySamplerYcbcrConversion(device, ycbcr_conversion, destroy_callback);
            }


        // Clear state:

            void clearHandler() {
                ycbcr_conversion = VK_NULL_HANDLE;
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseHandler END


        struct Handler : BaseHandler {
        // Constructors:

            Handler() {}
            Handler(VkSamplerYcbcrConversion _ycbcr_conversion, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseHandler{_ycbcr_conversion, _device, _destroy_callback} {}
            Handler(const BaseHandler &_handler) : BaseHandler{_handler} {}
            Handler(BaseHandler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }
            Handler(const Handler &_handler) : BaseHandler{_handler} {}
            Handler(Handler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }

            ~Handler() {
                if (ycbcr_conversion != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
                    destroyVkObject();
            }


        // Assignment:

            Handler& operator=(const Handler &_handler) {
                ycbcr_conversion = _handler.ycbcr_conversion;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                ycbcr_conversion = _handler.ycbcr_conversion;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }
        }; // Handler END
    }; // VKFWSamplerYcbcrConversion END


    namespace VKFWDescriptorUpdateTemplate {
        struct BaseHandler {
        // Data:

            VkDescriptorUpdateTemplate descriptor_update_template = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkDescriptorUpdateTemplate _descriptor_update_template, VkDevice _device = VK_NULL_HANDLE, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : descriptor_update_template{_descriptor_update_template}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseHandler(const BaseHandler &_handler)
                            : descriptor_update_template{_handler.descriptor_update_template}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseHandler(BaseHandler &&_handler)
                            : descriptor_update_template{_handler.descriptor_update_template}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                descriptor_update_template = _handler.descriptor_update_template;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                descriptor_update_template = _handler.descriptor_update_template;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkDescriptorUpdateTemplate* ptr() const { return &descriptor_update_template; }


        // Destroy Vk object:

            void destroyVkObject() {
                vkDestroyDescriptorUpdateTemplate(device, descriptor_update_template, destroy_callback);
            }


        // Clear state:

            void clearHandler() {
                descriptor_update_template = VK_NULL_HANDLE;
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseHandler END


        struct Handler : BaseHandler {
        // Constructors:

            Handler() {}
            Handler(VkDescriptorUpdateTemplate _descriptor_update_template, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseHandler{_descriptor_update_template, _device, _destroy_callback} {}
            Handler(const BaseHandler &_handler) : BaseHandler{_handler} {}
            Handler(BaseHandler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }
            Handler(const Handler &_handler) : BaseHandler{_handler} {}
            Handler(Handler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }

            ~Handler() {
                if (descriptor_update_template != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
                    destroyVkObject();
            }


        // Assignment:

            Handler& operator=(const Handler &_handler) {
                descriptor_update_template = _handler.descriptor_update_template;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                descriptor_update_template = _handler.descriptor_update_template;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }
        }; // Handler END
    }; // VKFWDescriptorUpdateTemplate END


    namespace VKFWPrivateDataSlot {
        struct BaseHandler {
        // Data:

            VkPrivateDataSlot private_data_slot = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkPrivateDataSlot _private_data_slot, VkDevice _device = VK_NULL_HANDLE, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : private_data_slot{_private_data_slot}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseHandler(const BaseHandler &_handler)
                            : private_data_slot{_handler.private_data_slot}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseHandler(BaseHandler &&_handler)
                            : private_data_slot{_handler.private_data_slot}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                private_data_slot = _handler.private_data_slot;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                private_data_slot = _handler.private_data_slot;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkPrivateDataSlot* ptr() const { return &private_data_slot; }


        // Destroy Vk object:

            void destroyVkObject() {
                vkDestroyPrivateDataSlot(device, private_data_slot, destroy_callback);
            }


        // Clear state:

            void clearHandler() {
                private_data_slot = VK_NULL_HANDLE;
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseHandler END


        struct Handler : BaseHandler {
        // Constructors:

            Handler() {}
            Handler(VkPrivateDataSlot _private_data_slot, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseHandler{_private_data_slot, _device, _destroy_callback} {}
            Handler(const BaseHandler &_handler) : BaseHandler{_handler} {}
            Handler(BaseHandler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }
            Handler(const Handler &_handler) : BaseHandler{_handler} {}
            Handler(Handler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }

            ~Handler() {
                if (private_data_slot != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
                    destroyVkObject();
            }


        // Assignment:

            Handler& operator=(const Handler &_handler) {
                private_data_slot = _handler.private_data_slot;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                private_data_slot = _handler.private_data_slot;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }
        }; // Handler END
    }; // VKFWPrivateDataSlot END


    namespace VKFWSurfaceKHR {
        struct BaseHandler {
        // Data:

            VkSurfaceKHR surface = VK_NULL_HANDLE;
            VkInstance instance = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkSurfaceKHR _surface, VkInstance _instance = VK_NULL_HANDLE, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : surface{_surface}, instance{_instance}, destroy_callback{_destroy_callback} {}
            BaseHandler(const BaseHandler &_handler)
                            : surface{_handler.surface}, instance{_handler.instance}, destroy_callback{_handler.destroy_callback} {}
            BaseHandler(BaseHandler &&_handler)
                            : surface{_handler.surface}, instance{_handler.instance}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                surface = _handler.surface;
                instance = _handler.instance;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                surface = _handler.surface;
                instance = _handler.instance;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkSurfaceKHR* ptr() const { return &surface; }


        // Destroy Vk object:

            void destroyVkObject() {
                vkDestroySurfaceKHR(instance, surface, destroy_callback);
            }


        // Clear state:

            void clearHandler() {
                surface = VK_NULL_HANDLE;
                instance = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseHandler END


        struct Handler : BaseHandler {
        // Constructors:

            Handler() {}
            Handler(VkSurfaceKHR _surface, VkInstance _instance, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseHandler{_surface, _instance, _destroy_callback} {}
            Handler(const BaseHandler &_handler) : BaseHandler{_handler} {}
            Handler(BaseHandler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }
            Handler(const Handler &_handler) : BaseHandler{_handler} {}
            Handler(Handler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }

            ~Handler() {
                if (surface != VK_NULL_HANDLE && instance != VK_NULL_HANDLE)
                    destroyVkObject();
            }


        // Assignment:

            Handler& operator=(const Handler &_handler) {
                surface = _handler.surface;
                instance = _handler.instance;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                surface = _handler.surface;
                instance = _handler.instance;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }
        }; // Handler END
    }; // VKFWSurfaceKHR END


    namespace VKFWSwapchainKHR {
        struct BaseHandler {
        // Data:

            VkSwapchainKHR swapchain = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkSwapchainKHR _swapchain, VkDevice _device = VK_NULL_HANDLE, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : swapchain{_swapchain}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseHandler(const BaseHandler &_handler)
                            : swapchain{_handler.swapchain}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseHandler(BaseHandler &&_handler)
                            : swapchain{_handler.swapchain}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                swapchain = _handler.swapchain;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                swapchain = _handler.swapchain;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkSwapchainKHR* ptr() const { return &swapchain; }


        // Destroy Vk object:

            void destroyVkObject() {
                vkDestroySwapchainKHR(device, swapchain, destroy_callback);
            }


        // Clear state:

            void clearHandler() {
                swapchain = VK_NULL_HANDLE;
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseHandler END


        struct Handler : BaseHandler {
        // Constructors:

            Handler() {}
            Handler(VkSwapchainKHR _swapchain, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseHandler{_swapchain, _device, _destroy_callback} {}
            Handler(const BaseHandler &_handler) : BaseHandler{_handler} {}
            Handler(BaseHandler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }
            Handler(const Handler &_handler) : BaseHandler{_handler} {}
            Handler(Handler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }

            ~Handler() {
                if (swapchain != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
                    destroyVkObject();
            }


        // Assignment:

            Handler& operator=(const Handler &_handler) {
                swapchain = _handler.swapchain;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                swapchain = _handler.swapchain;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }
        }; // Handler END


        struct BaseVecHandler {
        // Data:

            std::vector<VkSwapchainKHR> swapchains;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseVecHandler() {}
            BaseVecHandler(const std::vector<VkSwapchainKHR> &_swapchains, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : swapchains{_swapchains}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseVecHandler(const BaseHandler &_handler)
                            : swapchains{_handler.swapchain}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseVecHandler(BaseHandler &&_handler)
                            : swapchains{_handler.swapchain}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            BaseVecHandler(const BaseVecHandler &_handler)
                            : swapchains{_handler.swapchains}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseVecHandler(BaseVecHandler &&_handler)
                            : swapchains{_handler.swapchains}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearVecHandler();
            }
            virtual ~BaseVecHandler() {
                clearVecHandler();
            }


        // Assignment:

            BaseVecHandler& operator=(const BaseHandler &_handler) {
                swapchains = { _handler.swapchain };
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseVecHandler& operator=(BaseHandler &&_handler) {
                swapchains = { _handler.swapchain };
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }

            BaseVecHandler& operator=(const BaseVecHandler &_handler) {
                swapchains = _handler.swapchains;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseVecHandler& operator=(BaseVecHandler &&_handler) {
                swapchains = _handler.swapchains;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearVecHandler();
                return *this;
            }


        // Access by index:

            VkSwapchainKHR operator[](uint32_t _index) const { return swapchains[_index]; }
            BaseHandler getBaseHandler(uint32_t _index) const { return { swapchains[_index], device, destroy_callback }; }


        // Pointer and count:

            const VkSwapchainKHR* ptr() const { return swapchains.data(); }
            uint32_t count() const { return swapchains.size(); }


        // Destroy all Vk objects in vector:

            void destroyAllVkObjects() {
                for (auto swapchain : swapchains)
                    vkDestroySwapchainKHR(device, swapchain, destroy_callback);
            }


        // Clear state:

            void clearVecHandler() {
                swapchains.clear();
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseVecHandler END


        struct VecHandler : BaseVecHandler {
        // Constructors:

            VecHandler() {}
            VecHandler(const std::vector<VkSwapchainKHR> &_swapchains, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseVecHandler{swapchains, device, destroy_callback} {}
            VecHandler(const BaseHandler &_handler) : BaseVecHandler{_handler} {}
            VecHandler(BaseHandler &&_handler) : BaseVecHandler{_handler} {
                _handler.clearHandler();
            }
            VecHandler(const BaseVecHandler &_handler) : BaseVecHandler{_handler} {}
            VecHandler(BaseVecHandler &&_handler) : BaseVecHandler{_handler} {
                _handler.clearVecHandler();
            }
            VecHandler(const VecHandler &_handler) : BaseVecHandler{_handler} {}
            VecHandler(VecHandler &&_handler) : BaseVecHandler{_handler} {
                _handler.clearVecHandler();
            }

            ~VecHandler() {
                if (!swapchains.empty() && device != VK_NULL_HANDLE)
                    destroyAllVkObjects();
            }


        // Assignment:

            VecHandler& operator=(const VecHandler &_handler) {
                swapchains = _handler.swapchains;
                        device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            VecHandler& operator=(VecHandler &&_handler) {
                swapchains = _handler.swapchains;
                        device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearVecHandler();
                return *this;
            }
        }; // VecHandler END
    }; // VKFWSwapchainKHR END


    namespace VKFWVideoSessionKHR {
        struct BaseHandler {
        // Data:

            VkVideoSessionKHR video_session = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkVideoSessionKHR _video_session, VkDevice _device = VK_NULL_HANDLE, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : video_session{_video_session}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseHandler(const BaseHandler &_handler)
                            : video_session{_handler.video_session}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseHandler(BaseHandler &&_handler)
                            : video_session{_handler.video_session}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                video_session = _handler.video_session;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                video_session = _handler.video_session;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkVideoSessionKHR* ptr() const { return &video_session; }


        // Destroy Vk object:

            void destroyVkObject() {
                vkDestroyVideoSessionKHR(device, video_session, destroy_callback);
            }


        // Clear state:

            void clearHandler() {
                video_session = VK_NULL_HANDLE;
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseHandler END


        struct Handler : BaseHandler {
        // Constructors:

            Handler() {}
            Handler(VkVideoSessionKHR _video_session, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseHandler{_video_session, _device, _destroy_callback} {}
            Handler(const BaseHandler &_handler) : BaseHandler{_handler} {}
            Handler(BaseHandler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }
            Handler(const Handler &_handler) : BaseHandler{_handler} {}
            Handler(Handler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }

            ~Handler() {
                if (video_session != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
                    destroyVkObject();
            }


        // Assignment:

            Handler& operator=(const Handler &_handler) {
                video_session = _handler.video_session;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                video_session = _handler.video_session;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }
        }; // Handler END
    }; // VKFWVideoSessionKHR END


    namespace VKFWVideoSessionParametersKHR {
        struct BaseHandler {
        // Data:

            VkVideoSessionParametersKHR video_session_parameters = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkVideoSessionParametersKHR _video_session_parameters, VkDevice _device = VK_NULL_HANDLE, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : video_session_parameters{_video_session_parameters}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseHandler(const BaseHandler &_handler)
                            : video_session_parameters{_handler.video_session_parameters}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseHandler(BaseHandler &&_handler)
                            : video_session_parameters{_handler.video_session_parameters}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                video_session_parameters = _handler.video_session_parameters;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                video_session_parameters = _handler.video_session_parameters;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkVideoSessionParametersKHR* ptr() const { return &video_session_parameters; }


        // Destroy Vk object:

            void destroyVkObject() {
                vkDestroyVideoSessionParametersKHR(device, video_session_parameters, destroy_callback);
            }


        // Clear state:

            void clearHandler() {
                video_session_parameters = VK_NULL_HANDLE;
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseHandler END


        struct Handler : BaseHandler {
        // Constructors:

            Handler() {}
            Handler(VkVideoSessionParametersKHR _video_session_parameters, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseHandler{_video_session_parameters, _device, _destroy_callback} {}
            Handler(const BaseHandler &_handler) : BaseHandler{_handler} {}
            Handler(BaseHandler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }
            Handler(const Handler &_handler) : BaseHandler{_handler} {}
            Handler(Handler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }

            ~Handler() {
                if (video_session_parameters != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
                    destroyVkObject();
            }


        // Assignment:

            Handler& operator=(const Handler &_handler) {
                video_session_parameters = _handler.video_session_parameters;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                video_session_parameters = _handler.video_session_parameters;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }
        }; // Handler END
    }; // VKFWVideoSessionParametersKHR END


    namespace VKFWDescriptorUpdateTemplateKHR {
        struct BaseHandler {
        // Data:

            VkDescriptorUpdateTemplate descriptor_update_template = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkDescriptorUpdateTemplate _descriptor_update_template, VkDevice _device = VK_NULL_HANDLE, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : descriptor_update_template{_descriptor_update_template}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseHandler(const BaseHandler &_handler)
                            : descriptor_update_template{_handler.descriptor_update_template}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseHandler(BaseHandler &&_handler)
                            : descriptor_update_template{_handler.descriptor_update_template}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                descriptor_update_template = _handler.descriptor_update_template;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                descriptor_update_template = _handler.descriptor_update_template;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkDescriptorUpdateTemplate* ptr() const { return &descriptor_update_template; }


        // Destroy Vk object:

            void destroyVkObject() {
                vkDestroyDescriptorUpdateTemplate(device, descriptor_update_template, destroy_callback);
            }


        // Clear state:

            void clearHandler() {
                descriptor_update_template = VK_NULL_HANDLE;
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseHandler END


        struct Handler : BaseHandler {
        // Constructors:

            Handler() {}
            Handler(VkDescriptorUpdateTemplate _descriptor_update_template, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseHandler{_descriptor_update_template, _device, _destroy_callback} {}
            Handler(const BaseHandler &_handler) : BaseHandler{_handler} {}
            Handler(BaseHandler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }
            Handler(const Handler &_handler) : BaseHandler{_handler} {}
            Handler(Handler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }

            ~Handler() {
                if (descriptor_update_template != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
                    destroyVkObject();
            }


        // Assignment:

            Handler& operator=(const Handler &_handler) {
                descriptor_update_template = _handler.descriptor_update_template;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                descriptor_update_template = _handler.descriptor_update_template;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }
        }; // Handler END
    }; // VKFWDescriptorUpdateTemplateKHR END


    namespace VKFWSamplerYcbcrConversionKHR {
        struct BaseHandler {
        // Data:

            VkSamplerYcbcrConversion ycbcr_conversion = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkSamplerYcbcrConversion _ycbcr_conversion, VkDevice _device = VK_NULL_HANDLE, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : ycbcr_conversion{_ycbcr_conversion}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseHandler(const BaseHandler &_handler)
                            : ycbcr_conversion{_handler.ycbcr_conversion}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseHandler(BaseHandler &&_handler)
                            : ycbcr_conversion{_handler.ycbcr_conversion}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                ycbcr_conversion = _handler.ycbcr_conversion;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                ycbcr_conversion = _handler.ycbcr_conversion;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkSamplerYcbcrConversion* ptr() const { return &ycbcr_conversion; }


        // Destroy Vk object:

            void destroyVkObject() {
                vkDestroySamplerYcbcrConversion(device, ycbcr_conversion, destroy_callback);
            }


        // Clear state:

            void clearHandler() {
                ycbcr_conversion = VK_NULL_HANDLE;
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseHandler END


        struct Handler : BaseHandler {
        // Constructors:

            Handler() {}
            Handler(VkSamplerYcbcrConversion _ycbcr_conversion, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseHandler{_ycbcr_conversion, _device, _destroy_callback} {}
            Handler(const BaseHandler &_handler) : BaseHandler{_handler} {}
            Handler(BaseHandler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }
            Handler(const Handler &_handler) : BaseHandler{_handler} {}
            Handler(Handler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }

            ~Handler() {
                if (ycbcr_conversion != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
                    destroyVkObject();
            }


        // Assignment:

            Handler& operator=(const Handler &_handler) {
                ycbcr_conversion = _handler.ycbcr_conversion;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                ycbcr_conversion = _handler.ycbcr_conversion;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }
        }; // Handler END
    }; // VKFWSamplerYcbcrConversionKHR END


    namespace VKFWDeferredOperationKHR {
        struct BaseHandler {
        // Data:

            VkDeferredOperationKHR operation = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkDeferredOperationKHR _operation, VkDevice _device = VK_NULL_HANDLE, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : operation{_operation}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseHandler(const BaseHandler &_handler)
                            : operation{_handler.operation}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseHandler(BaseHandler &&_handler)
                            : operation{_handler.operation}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                operation = _handler.operation;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                operation = _handler.operation;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkDeferredOperationKHR* ptr() const { return &operation; }


        // Destroy Vk object:

            void destroyVkObject() {
                vkDestroyDeferredOperationKHR(device, operation, destroy_callback);
            }


        // Clear state:

            void clearHandler() {
                operation = VK_NULL_HANDLE;
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseHandler END


        struct Handler : BaseHandler {
        // Constructors:

            Handler() {}
            Handler(VkDeferredOperationKHR _operation, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseHandler{_operation, _device, _destroy_callback} {}
            Handler(const BaseHandler &_handler) : BaseHandler{_handler} {}
            Handler(BaseHandler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }
            Handler(const Handler &_handler) : BaseHandler{_handler} {}
            Handler(Handler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }

            ~Handler() {
                if (operation != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
                    destroyVkObject();
            }


        // Assignment:

            Handler& operator=(const Handler &_handler) {
                operation = _handler.operation;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                operation = _handler.operation;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }
        }; // Handler END
    }; // VKFWDeferredOperationKHR END


    namespace VKFWDebugReportCallbackEXT {
        struct BaseHandler {
        // Data:

            VkDebugReportCallbackEXT callback = VK_NULL_HANDLE;
            VkInstance instance = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkDebugReportCallbackEXT _callback, VkInstance _instance = VK_NULL_HANDLE, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : callback{_callback}, instance{_instance}, destroy_callback{_destroy_callback} {}
            BaseHandler(const BaseHandler &_handler)
                            : callback{_handler.callback}, instance{_handler.instance}, destroy_callback{_handler.destroy_callback} {}
            BaseHandler(BaseHandler &&_handler)
                            : callback{_handler.callback}, instance{_handler.instance}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                callback = _handler.callback;
                instance = _handler.instance;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                callback = _handler.callback;
                instance = _handler.instance;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkDebugReportCallbackEXT* ptr() const { return &callback; }


        // Destroy Vk object:

            void destroyVkObject() {
                vkDestroyDebugReportCallbackEXT(instance, callback, destroy_callback);
            }


        // Clear state:

            void clearHandler() {
                callback = VK_NULL_HANDLE;
                instance = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseHandler END


        struct Handler : BaseHandler {
        // Constructors:

            Handler() {}
            Handler(VkDebugReportCallbackEXT _callback, VkInstance _instance, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseHandler{_callback, _instance, _destroy_callback} {}
            Handler(const BaseHandler &_handler) : BaseHandler{_handler} {}
            Handler(BaseHandler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }
            Handler(const Handler &_handler) : BaseHandler{_handler} {}
            Handler(Handler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }

            ~Handler() {
                if (callback != VK_NULL_HANDLE && instance != VK_NULL_HANDLE)
                    destroyVkObject();
            }


        // Assignment:

            Handler& operator=(const Handler &_handler) {
                callback = _handler.callback;
                instance = _handler.instance;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                callback = _handler.callback;
                instance = _handler.instance;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }
        }; // Handler END
    }; // VKFWDebugReportCallbackEXT END


    namespace VKFWCuModuleNVX {
        struct BaseHandler {
        // Data:

            VkCuModuleNVX module = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkCuModuleNVX _module, VkDevice _device = VK_NULL_HANDLE, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : module{_module}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseHandler(const BaseHandler &_handler)
                            : module{_handler.module}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseHandler(BaseHandler &&_handler)
                            : module{_handler.module}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                module = _handler.module;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                module = _handler.module;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkCuModuleNVX* ptr() const { return &module; }


        // Destroy Vk object:

            void destroyVkObject() {
                vkDestroyCuModuleNVX(device, module, destroy_callback);
            }


        // Clear state:

            void clearHandler() {
                module = VK_NULL_HANDLE;
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseHandler END


        struct Handler : BaseHandler {
        // Constructors:

            Handler() {}
            Handler(VkCuModuleNVX _module, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseHandler{_module, _device, _destroy_callback} {}
            Handler(const BaseHandler &_handler) : BaseHandler{_handler} {}
            Handler(BaseHandler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }
            Handler(const Handler &_handler) : BaseHandler{_handler} {}
            Handler(Handler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }

            ~Handler() {
                if (module != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
                    destroyVkObject();
            }


        // Assignment:

            Handler& operator=(const Handler &_handler) {
                module = _handler.module;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                module = _handler.module;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }
        }; // Handler END
    }; // VKFWCuModuleNVX END


    namespace VKFWCuFunctionNVX {
        struct BaseHandler {
        // Data:

            VkCuFunctionNVX function = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkCuFunctionNVX _function, VkDevice _device = VK_NULL_HANDLE, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : function{_function}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseHandler(const BaseHandler &_handler)
                            : function{_handler.function}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseHandler(BaseHandler &&_handler)
                            : function{_handler.function}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                function = _handler.function;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                function = _handler.function;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkCuFunctionNVX* ptr() const { return &function; }


        // Destroy Vk object:

            void destroyVkObject() {
                vkDestroyCuFunctionNVX(device, function, destroy_callback);
            }


        // Clear state:

            void clearHandler() {
                function = VK_NULL_HANDLE;
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseHandler END


        struct Handler : BaseHandler {
        // Constructors:

            Handler() {}
            Handler(VkCuFunctionNVX _function, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseHandler{_function, _device, _destroy_callback} {}
            Handler(const BaseHandler &_handler) : BaseHandler{_handler} {}
            Handler(BaseHandler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }
            Handler(const Handler &_handler) : BaseHandler{_handler} {}
            Handler(Handler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }

            ~Handler() {
                if (function != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
                    destroyVkObject();
            }


        // Assignment:

            Handler& operator=(const Handler &_handler) {
                function = _handler.function;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                function = _handler.function;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }
        }; // Handler END
    }; // VKFWCuFunctionNVX END


    namespace VKFWDebugMessengerEXT {
        struct BaseHandler {
        // Data:

            VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
            VkInstance instance = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkDebugUtilsMessengerEXT _messenger, VkInstance _instance = VK_NULL_HANDLE, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : messenger{_messenger}, instance{_instance}, destroy_callback{_destroy_callback} {}
            BaseHandler(const BaseHandler &_handler)
                            : messenger{_handler.messenger}, instance{_handler.instance}, destroy_callback{_handler.destroy_callback} {}
            BaseHandler(BaseHandler &&_handler)
                            : messenger{_handler.messenger}, instance{_handler.instance}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                messenger = _handler.messenger;
                instance = _handler.instance;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                messenger = _handler.messenger;
                instance = _handler.instance;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkDebugUtilsMessengerEXT* ptr() const { return &messenger; }


        // Destroy Vk object:

            void destroyVkObject() {
                vkDestroyDebugUtilsMessengerEXT(instance, messenger, destroy_callback);
            }


        // Clear state:

            void clearHandler() {
                messenger = VK_NULL_HANDLE;
                instance = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseHandler END


        struct Handler : BaseHandler {
        // Constructors:

            Handler() {}
            Handler(VkDebugUtilsMessengerEXT _messenger, VkInstance _instance, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseHandler{_messenger, _instance, _destroy_callback} {}
            Handler(const BaseHandler &_handler) : BaseHandler{_handler} {}
            Handler(BaseHandler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }
            Handler(const Handler &_handler) : BaseHandler{_handler} {}
            Handler(Handler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }

            ~Handler() {
                if (messenger != VK_NULL_HANDLE && instance != VK_NULL_HANDLE)
                    destroyVkObject();
            }


        // Assignment:

            Handler& operator=(const Handler &_handler) {
                messenger = _handler.messenger;
                instance = _handler.instance;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                messenger = _handler.messenger;
                instance = _handler.instance;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }
        }; // Handler END
    }; // VKFWDebugMessengerEXT END


    namespace VKFWValidationCacheEXT {
        struct BaseHandler {
        // Data:

            VkValidationCacheEXT validation_cache = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkValidationCacheEXT _validation_cache, VkDevice _device = VK_NULL_HANDLE, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : validation_cache{_validation_cache}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseHandler(const BaseHandler &_handler)
                            : validation_cache{_handler.validation_cache}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseHandler(BaseHandler &&_handler)
                            : validation_cache{_handler.validation_cache}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                validation_cache = _handler.validation_cache;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                validation_cache = _handler.validation_cache;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkValidationCacheEXT* ptr() const { return &validation_cache; }


        // Destroy Vk object:

            void destroyVkObject() {
                vkDestroyValidationCacheEXT(device, validation_cache, destroy_callback);
            }


        // Clear state:

            void clearHandler() {
                validation_cache = VK_NULL_HANDLE;
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseHandler END


        struct Handler : BaseHandler {
        // Constructors:

            Handler() {}
            Handler(VkValidationCacheEXT _validation_cache, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseHandler{_validation_cache, _device, _destroy_callback} {}
            Handler(const BaseHandler &_handler) : BaseHandler{_handler} {}
            Handler(BaseHandler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }
            Handler(const Handler &_handler) : BaseHandler{_handler} {}
            Handler(Handler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }

            ~Handler() {
                if (validation_cache != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
                    destroyVkObject();
            }


        // Assignment:

            Handler& operator=(const Handler &_handler) {
                validation_cache = _handler.validation_cache;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                validation_cache = _handler.validation_cache;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }
        }; // Handler END
    }; // VKFWValidationCacheEXT END


    namespace VKFWAccelStructNV {
        struct BaseHandler {
        // Data:

            VkAccelerationStructureNV accel_struct = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkAccelerationStructureNV _accel_struct, VkDevice _device = VK_NULL_HANDLE, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : accel_struct{_accel_struct}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseHandler(const BaseHandler &_handler)
                            : accel_struct{_handler.accel_struct}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseHandler(BaseHandler &&_handler)
                            : accel_struct{_handler.accel_struct}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                accel_struct = _handler.accel_struct;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                accel_struct = _handler.accel_struct;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkAccelerationStructureNV* ptr() const { return &accel_struct; }


        // Destroy Vk object:

            void destroyVkObject() {
                vkDestroyAccelerationStructureNV(device, accel_struct, destroy_callback);
            }


        // Clear state:

            void clearHandler() {
                accel_struct = VK_NULL_HANDLE;
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseHandler END


        struct Handler : BaseHandler {
        // Constructors:

            Handler() {}
            Handler(VkAccelerationStructureNV _accel_struct, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseHandler{_accel_struct, _device, _destroy_callback} {}
            Handler(const BaseHandler &_handler) : BaseHandler{_handler} {}
            Handler(BaseHandler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }
            Handler(const Handler &_handler) : BaseHandler{_handler} {}
            Handler(Handler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }

            ~Handler() {
                if (accel_struct != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
                    destroyVkObject();
            }


        // Assignment:

            Handler& operator=(const Handler &_handler) {
                accel_struct = _handler.accel_struct;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                accel_struct = _handler.accel_struct;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }
        }; // Handler END
    }; // VKFWAccelStructNV END


    namespace VKFWIndirectCommandsLayoutNV {
        struct BaseHandler {
        // Data:

            VkIndirectCommandsLayoutNV indirect_commands_layout = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkIndirectCommandsLayoutNV _indirect_commands_layout, VkDevice _device = VK_NULL_HANDLE, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : indirect_commands_layout{_indirect_commands_layout}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseHandler(const BaseHandler &_handler)
                            : indirect_commands_layout{_handler.indirect_commands_layout}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseHandler(BaseHandler &&_handler)
                            : indirect_commands_layout{_handler.indirect_commands_layout}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                indirect_commands_layout = _handler.indirect_commands_layout;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                indirect_commands_layout = _handler.indirect_commands_layout;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkIndirectCommandsLayoutNV* ptr() const { return &indirect_commands_layout; }


        // Destroy Vk object:

            void destroyVkObject() {
                vkDestroyIndirectCommandsLayoutNV(device, indirect_commands_layout, destroy_callback);
            }


        // Clear state:

            void clearHandler() {
                indirect_commands_layout = VK_NULL_HANDLE;
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseHandler END


        struct Handler : BaseHandler {
        // Constructors:

            Handler() {}
            Handler(VkIndirectCommandsLayoutNV _indirect_commands_layout, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseHandler{_indirect_commands_layout, _device, _destroy_callback} {}
            Handler(const BaseHandler &_handler) : BaseHandler{_handler} {}
            Handler(BaseHandler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }
            Handler(const Handler &_handler) : BaseHandler{_handler} {}
            Handler(Handler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }

            ~Handler() {
                if (indirect_commands_layout != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
                    destroyVkObject();
            }


        // Assignment:

            Handler& operator=(const Handler &_handler) {
                indirect_commands_layout = _handler.indirect_commands_layout;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                indirect_commands_layout = _handler.indirect_commands_layout;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }
        }; // Handler END
    }; // VKFWIndirectCommandsLayoutNV END


    namespace VKFWPrivateDataSlotEXT {
        struct BaseHandler {
        // Data:

            VkPrivateDataSlot private_data_slot = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkPrivateDataSlot _private_data_slot, VkDevice _device = VK_NULL_HANDLE, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : private_data_slot{_private_data_slot}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseHandler(const BaseHandler &_handler)
                            : private_data_slot{_handler.private_data_slot}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseHandler(BaseHandler &&_handler)
                            : private_data_slot{_handler.private_data_slot}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                private_data_slot = _handler.private_data_slot;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                private_data_slot = _handler.private_data_slot;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkPrivateDataSlot* ptr() const { return &private_data_slot; }


        // Destroy Vk object:

            void destroyVkObject() {
                vkDestroyPrivateDataSlot(device, private_data_slot, destroy_callback);
            }


        // Clear state:

            void clearHandler() {
                private_data_slot = VK_NULL_HANDLE;
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseHandler END


        struct Handler : BaseHandler {
        // Constructors:

            Handler() {}
            Handler(VkPrivateDataSlot _private_data_slot, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseHandler{_private_data_slot, _device, _destroy_callback} {}
            Handler(const BaseHandler &_handler) : BaseHandler{_handler} {}
            Handler(BaseHandler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }
            Handler(const Handler &_handler) : BaseHandler{_handler} {}
            Handler(Handler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }

            ~Handler() {
                if (private_data_slot != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
                    destroyVkObject();
            }


        // Assignment:

            Handler& operator=(const Handler &_handler) {
                private_data_slot = _handler.private_data_slot;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                private_data_slot = _handler.private_data_slot;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }
        }; // Handler END
    }; // VKFWPrivateDataSlotEXT END


    namespace VKFWCudaModuleNV {
        struct BaseHandler {
        // Data:

            VkCudaModuleNV module = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkCudaModuleNV _module, VkDevice _device = VK_NULL_HANDLE, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : module{_module}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseHandler(const BaseHandler &_handler)
                            : module{_handler.module}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseHandler(BaseHandler &&_handler)
                            : module{_handler.module}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                module = _handler.module;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                module = _handler.module;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkCudaModuleNV* ptr() const { return &module; }


        // Destroy Vk object:

            void destroyVkObject() {
                vkDestroyCudaModuleNV(device, module, destroy_callback);
            }


        // Clear state:

            void clearHandler() {
                module = VK_NULL_HANDLE;
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseHandler END


        struct Handler : BaseHandler {
        // Constructors:

            Handler() {}
            Handler(VkCudaModuleNV _module, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseHandler{_module, _device, _destroy_callback} {}
            Handler(const BaseHandler &_handler) : BaseHandler{_handler} {}
            Handler(BaseHandler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }
            Handler(const Handler &_handler) : BaseHandler{_handler} {}
            Handler(Handler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }

            ~Handler() {
                if (module != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
                    destroyVkObject();
            }


        // Assignment:

            Handler& operator=(const Handler &_handler) {
                module = _handler.module;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                module = _handler.module;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }
        }; // Handler END
    }; // VKFWCudaModuleNV END


    namespace VKFWCudaFunctionNV {
        struct BaseHandler {
        // Data:

            VkCudaFunctionNV function = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkCudaFunctionNV _function, VkDevice _device = VK_NULL_HANDLE, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : function{_function}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseHandler(const BaseHandler &_handler)
                            : function{_handler.function}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseHandler(BaseHandler &&_handler)
                            : function{_handler.function}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                function = _handler.function;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                function = _handler.function;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkCudaFunctionNV* ptr() const { return &function; }


        // Destroy Vk object:

            void destroyVkObject() {
                vkDestroyCudaFunctionNV(device, function, destroy_callback);
            }


        // Clear state:

            void clearHandler() {
                function = VK_NULL_HANDLE;
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseHandler END


        struct Handler : BaseHandler {
        // Constructors:

            Handler() {}
            Handler(VkCudaFunctionNV _function, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseHandler{_function, _device, _destroy_callback} {}
            Handler(const BaseHandler &_handler) : BaseHandler{_handler} {}
            Handler(BaseHandler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }
            Handler(const Handler &_handler) : BaseHandler{_handler} {}
            Handler(Handler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }

            ~Handler() {
                if (function != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
                    destroyVkObject();
            }


        // Assignment:

            Handler& operator=(const Handler &_handler) {
                function = _handler.function;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                function = _handler.function;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }
        }; // Handler END
    }; // VKFWCudaFunctionNV END


    namespace VKFWMicromapEXT {
        struct BaseHandler {
        // Data:

            VkMicromapEXT micromap = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkMicromapEXT _micromap, VkDevice _device = VK_NULL_HANDLE, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : micromap{_micromap}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseHandler(const BaseHandler &_handler)
                            : micromap{_handler.micromap}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseHandler(BaseHandler &&_handler)
                            : micromap{_handler.micromap}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                micromap = _handler.micromap;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                micromap = _handler.micromap;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkMicromapEXT* ptr() const { return &micromap; }


        // Destroy Vk object:

            void destroyVkObject() {
                vkDestroyMicromapEXT(device, micromap, destroy_callback);
            }


        // Clear state:

            void clearHandler() {
                micromap = VK_NULL_HANDLE;
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseHandler END


        struct Handler : BaseHandler {
        // Constructors:

            Handler() {}
            Handler(VkMicromapEXT _micromap, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseHandler{_micromap, _device, _destroy_callback} {}
            Handler(const BaseHandler &_handler) : BaseHandler{_handler} {}
            Handler(BaseHandler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }
            Handler(const Handler &_handler) : BaseHandler{_handler} {}
            Handler(Handler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }

            ~Handler() {
                if (micromap != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
                    destroyVkObject();
            }


        // Assignment:

            Handler& operator=(const Handler &_handler) {
                micromap = _handler.micromap;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                micromap = _handler.micromap;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }
        }; // Handler END
    }; // VKFWMicromapEXT END


    namespace VKFWOpticalFlowSessionNV {
        struct BaseHandler {
        // Data:

            VkOpticalFlowSessionNV session = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkOpticalFlowSessionNV _session, VkDevice _device = VK_NULL_HANDLE, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : session{_session}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseHandler(const BaseHandler &_handler)
                            : session{_handler.session}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseHandler(BaseHandler &&_handler)
                            : session{_handler.session}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                session = _handler.session;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                session = _handler.session;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkOpticalFlowSessionNV* ptr() const { return &session; }


        // Destroy Vk object:

            void destroyVkObject() {
                vkDestroyOpticalFlowSessionNV(device, session, destroy_callback);
            }


        // Clear state:

            void clearHandler() {
                session = VK_NULL_HANDLE;
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseHandler END


        struct Handler : BaseHandler {
        // Constructors:

            Handler() {}
            Handler(VkOpticalFlowSessionNV _session, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseHandler{_session, _device, _destroy_callback} {}
            Handler(const BaseHandler &_handler) : BaseHandler{_handler} {}
            Handler(BaseHandler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }
            Handler(const Handler &_handler) : BaseHandler{_handler} {}
            Handler(Handler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }

            ~Handler() {
                if (session != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
                    destroyVkObject();
            }


        // Assignment:

            Handler& operator=(const Handler &_handler) {
                session = _handler.session;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                session = _handler.session;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }
        }; // Handler END
    }; // VKFWOpticalFlowSessionNV END


    namespace VKFWShaderEXT {
        struct BaseHandler {
        // Data:

            VkShaderEXT shader = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkShaderEXT _shader, VkDevice _device = VK_NULL_HANDLE, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : shader{_shader}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseHandler(const BaseHandler &_handler)
                            : shader{_handler.shader}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseHandler(BaseHandler &&_handler)
                            : shader{_handler.shader}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                shader = _handler.shader;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                shader = _handler.shader;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkShaderEXT* ptr() const { return &shader; }


        // Destroy Vk object:

            void destroyVkObject() {
                vkDestroyShaderEXT(device, shader, destroy_callback);
            }


        // Clear state:

            void clearHandler() {
                shader = VK_NULL_HANDLE;
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseHandler END


        struct Handler : BaseHandler {
        // Constructors:

            Handler() {}
            Handler(VkShaderEXT _shader, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseHandler{_shader, _device, _destroy_callback} {}
            Handler(const BaseHandler &_handler) : BaseHandler{_handler} {}
            Handler(BaseHandler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }
            Handler(const Handler &_handler) : BaseHandler{_handler} {}
            Handler(Handler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }

            ~Handler() {
                if (shader != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
                    destroyVkObject();
            }


        // Assignment:

            Handler& operator=(const Handler &_handler) {
                shader = _handler.shader;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                shader = _handler.shader;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }
        }; // Handler END
    }; // VKFWShaderEXT END


    namespace VKFWAccelStructKHR {
        struct BaseHandler {
        // Data:

            VkAccelerationStructureKHR accel_struct = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks* destroy_callback = nullptr;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkAccelerationStructureKHR _accel_struct, VkDevice _device = VK_NULL_HANDLE, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : accel_struct{_accel_struct}, device{_device}, destroy_callback{_destroy_callback} {}
            BaseHandler(const BaseHandler &_handler)
                            : accel_struct{_handler.accel_struct}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {}
            BaseHandler(BaseHandler &&_handler)
                            : accel_struct{_handler.accel_struct}, device{_handler.device}, destroy_callback{_handler.destroy_callback} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                accel_struct = _handler.accel_struct;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                accel_struct = _handler.accel_struct;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkAccelerationStructureKHR* ptr() const { return &accel_struct; }


        // Destroy Vk object:

            void destroyVkObject() {
                vkDestroyAccelerationStructureKHR(device, accel_struct, destroy_callback);
            }


        // Clear state:

            void clearHandler() {
                accel_struct = VK_NULL_HANDLE;
                device = VK_NULL_HANDLE;
                destroy_callback = nullptr;
            }
        }; // BaseHandler END


        struct Handler : BaseHandler {
        // Constructors:

            Handler() {}
            Handler(VkAccelerationStructureKHR _accel_struct, VkDevice _device, const VkAllocationCallbacks* _destroy_callback = nullptr)
                            : BaseHandler{_accel_struct, _device, _destroy_callback} {}
            Handler(const BaseHandler &_handler) : BaseHandler{_handler} {}
            Handler(BaseHandler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }
            Handler(const Handler &_handler) : BaseHandler{_handler} {}
            Handler(Handler &&_handler) : BaseHandler{_handler} {
                _handler.clearHandler();
            }

            ~Handler() {
                if (accel_struct != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
                    destroyVkObject();
            }


        // Assignment:

            Handler& operator=(const Handler &_handler) {
                accel_struct = _handler.accel_struct;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                accel_struct = _handler.accel_struct;
                device = _handler.device;
                destroy_callback = _handler.destroy_callback;
                _handler.clearHandler();
                return *this;
            }
        }; // Handler END
    }; // VKFWAccelStructKHR END


//  ===============================================  \\
                Non-destroyable objects
//  ===============================================  \\

    namespace VKFWPhysDevice {
        struct BaseHandler {
        // Data:

            VkPhysicalDevice phys_device = VK_NULL_HANDLE;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkPhysicalDevice _phys_device)
                            : phys_device{_phys_device} {}
            BaseHandler(const BaseHandler &_handler)
                            : phys_device{_handler.phys_device} {}
            BaseHandler(BaseHandler &&_handler)
                            : phys_device{_handler.phys_device} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                phys_device = _handler.phys_device;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                phys_device = _handler.phys_device;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkPhysicalDevice* ptr() const { return &phys_device; }


        // Clear state:

            void clearHandler() {
                phys_device = VK_NULL_HANDLE;
            }
        }; // BaseHandler END

        using Handler = BaseHandler;


        struct BaseVecHandler {
        // Data:

            std::vector<VkPhysicalDevice> phys_devices;


        // Constructors:

            BaseVecHandler() {}
            BaseVecHandler(const std::vector<VkPhysicalDevice> &_phys_devices)
                            : phys_devices{_phys_devices} {}
            BaseVecHandler(const BaseHandler &_handler)
                            : phys_devices{_handler.phys_device} {}
            BaseVecHandler(BaseHandler &&_handler)
                            : phys_devices{_handler.phys_device} {
                _handler.clearHandler();
            }
            BaseVecHandler(const BaseVecHandler &_handler)
                            : phys_devices{_handler.phys_devices} {}
            BaseVecHandler(BaseVecHandler &&_handler)
                            : phys_devices{_handler.phys_devices} {
                _handler.clearVecHandler();
            }
            virtual ~BaseVecHandler() {
                clearVecHandler();
            }


        // Assignment:

            BaseVecHandler& operator=(const BaseHandler &_handler) {
                phys_devices = { _handler.phys_device };

                return *this;
            }

            BaseVecHandler& operator=(BaseHandler &&_handler) {
                phys_devices = { _handler.phys_device };

                _handler.clearHandler();
                return *this;
            }

            BaseVecHandler& operator=(const BaseVecHandler &_handler) {
                phys_devices = _handler.phys_devices;

                return *this;
            }

            BaseVecHandler& operator=(BaseVecHandler &&_handler) {
                phys_devices = _handler.phys_devices;

                _handler.clearVecHandler();
                return *this;
            }


        // Access by index:

            VkPhysicalDevice operator[](uint32_t _index) const { return phys_devices[_index]; }
            BaseHandler getBaseHandler(uint32_t _index) const { return { phys_devices[_index] }; }


        // Pointer and count:

            const VkPhysicalDevice* ptr() const { return phys_devices.data(); }
            uint32_t count() const { return phys_devices.size(); }


        // Clear state:

            void clearVecHandler() {
                phys_devices.clear();
            }
        }; // BaseVecHandler END

        using VecHandler = BaseVecHandler;
    }; // VKFWPhysDevice END


    namespace VKFWQueueFamily {
        struct BaseHandler {
        // Data:

            uint32_t index = -1;


        // Constructors:

            BaseHandler() {}
            BaseHandler(uint32_t _index)
                            : index{_index} {}
            BaseHandler(const BaseHandler &_handler)
                            : index{_handler.index} {}
            BaseHandler(BaseHandler &&_handler)
                            : index{_handler.index} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                index = _handler.index;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                index = _handler.index;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const uint32_t* ptr() const { return &index; }


        // Clear state:

            void clearHandler() {
                index = -1;
            }
        }; // BaseHandler END

        using Handler = BaseHandler;


        struct BaseVecHandler {
        // Data:

            std::vector<uint32_t> indices;


        // Constructors:

            BaseVecHandler() {}
            BaseVecHandler(const std::vector<uint32_t> &_indices)
                            : indices{_indices} {}
            BaseVecHandler(const BaseHandler &_handler)
                            : indices{_handler.index} {}
            BaseVecHandler(BaseHandler &&_handler)
                            : indices{_handler.index} {
                _handler.clearHandler();
            }
            BaseVecHandler(const BaseVecHandler &_handler)
                            : indices{_handler.indices} {}
            BaseVecHandler(BaseVecHandler &&_handler)
                            : indices{_handler.indices} {
                _handler.clearVecHandler();
            }
            virtual ~BaseVecHandler() {
                clearVecHandler();
            }


        // Assignment:

            BaseVecHandler& operator=(const BaseHandler &_handler) {
                indices = { _handler.index };

                return *this;
            }

            BaseVecHandler& operator=(BaseHandler &&_handler) {
                indices = { _handler.index };

                _handler.clearHandler();
                return *this;
            }

            BaseVecHandler& operator=(const BaseVecHandler &_handler) {
                indices = _handler.indices;

                return *this;
            }

            BaseVecHandler& operator=(BaseVecHandler &&_handler) {
                indices = _handler.indices;

                _handler.clearVecHandler();
                return *this;
            }


        // Access by index:

            uint32_t operator[](uint32_t _index) const { return indices[_index]; }
            BaseHandler getBaseHandler(uint32_t _index) const { return { indices[_index] }; }


        // Pointer and count:

            const uint32_t* ptr() const { return indices.data(); }
            uint32_t count() const { return indices.size(); }


        // Clear state:

            void clearVecHandler() {
                indices.clear();
            }
        }; // BaseVecHandler END

        using VecHandler = BaseVecHandler;
    }; // VKFWQueueFamily END


    namespace VKFWQueue {
        struct BaseHandler {
        // Data:

            VkQueue queue = VK_NULL_HANDLE;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkQueue _queue)
                            : queue{_queue} {}
            BaseHandler(const BaseHandler &_handler)
                            : queue{_handler.queue} {}
            BaseHandler(BaseHandler &&_handler)
                            : queue{_handler.queue} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                queue = _handler.queue;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                queue = _handler.queue;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkQueue* ptr() const { return &queue; }


        // Clear state:

            void clearHandler() {
                queue = VK_NULL_HANDLE;
            }
        }; // BaseHandler END

        using Handler = BaseHandler;


        struct BaseVecHandler {
        // Data:

            std::vector<VkQueue> queues;


        // Constructors:

            BaseVecHandler() {}
            BaseVecHandler(const std::vector<VkQueue> &_queues)
                            : queues{_queues} {}
            BaseVecHandler(const BaseHandler &_handler)
                            : queues{_handler.queue} {}
            BaseVecHandler(BaseHandler &&_handler)
                            : queues{_handler.queue} {
                _handler.clearHandler();
            }
            BaseVecHandler(const BaseVecHandler &_handler)
                            : queues{_handler.queues} {}
            BaseVecHandler(BaseVecHandler &&_handler)
                            : queues{_handler.queues} {
                _handler.clearVecHandler();
            }
            virtual ~BaseVecHandler() {
                clearVecHandler();
            }


        // Assignment:

            BaseVecHandler& operator=(const BaseHandler &_handler) {
                queues = { _handler.queue };

                return *this;
            }

            BaseVecHandler& operator=(BaseHandler &&_handler) {
                queues = { _handler.queue };

                _handler.clearHandler();
                return *this;
            }

            BaseVecHandler& operator=(const BaseVecHandler &_handler) {
                queues = _handler.queues;

                return *this;
            }

            BaseVecHandler& operator=(BaseVecHandler &&_handler) {
                queues = _handler.queues;

                _handler.clearVecHandler();
                return *this;
            }


        // Access by index:

            VkQueue operator[](uint32_t _index) const { return queues[_index]; }
            BaseHandler getBaseHandler(uint32_t _index) const { return { queues[_index] }; }


        // Pointer and count:

            const VkQueue* ptr() const { return queues.data(); }
            uint32_t count() const { return queues.size(); }


        // Clear state:

            void clearVecHandler() {
                queues.clear();
            }
        }; // BaseVecHandler END

        using VecHandler = BaseVecHandler;
    }; // VKFWQueue END


    namespace VKFWCommandBuffer {
        struct BaseHandler {
        // Data:

            VkCommandBuffer command_buffer = VK_NULL_HANDLE;


        // Constructors:

            BaseHandler() {}
            BaseHandler(VkCommandBuffer _command_buffer)
                            : command_buffer{_command_buffer} {}
            BaseHandler(const BaseHandler &_handler)
                            : command_buffer{_handler.command_buffer} {}
            BaseHandler(BaseHandler &&_handler)
                            : command_buffer{_handler.command_buffer} {
                _handler.clearHandler();
            }
            virtual ~BaseHandler() {
                clearHandler();
            }


        // Assignment:

            BaseHandler& operator=(const BaseHandler &_handler) {
                command_buffer = _handler.command_buffer;
                return *this;
            }

            BaseHandler& operator=(BaseHandler &&_handler) {
                command_buffer = _handler.command_buffer;
                _handler.clearHandler();
                return *this;
            }


        // Pointer access:

            const VkCommandBuffer* ptr() const { return &command_buffer; }


        // Clear state:

            void clearHandler() {
                command_buffer = VK_NULL_HANDLE;
            }
        }; // BaseHandler END

        using Handler = BaseHandler;


        struct BaseVecHandler {
        // Data:

            std::vector<VkCommandBuffer> command_buffers;


        // Constructors:

            BaseVecHandler() {}
            BaseVecHandler(const std::vector<VkCommandBuffer> &_command_buffers)
                            : command_buffers{_command_buffers} {}
            BaseVecHandler(const BaseHandler &_handler)
                            : command_buffers{_handler.command_buffer} {}
            BaseVecHandler(BaseHandler &&_handler)
                            : command_buffers{_handler.command_buffer} {
                _handler.clearHandler();
            }
            BaseVecHandler(const BaseVecHandler &_handler)
                            : command_buffers{_handler.command_buffers} {}
            BaseVecHandler(BaseVecHandler &&_handler)
                            : command_buffers{_handler.command_buffers} {
                _handler.clearVecHandler();
            }
            virtual ~BaseVecHandler() {
                clearVecHandler();
            }


        // Assignment:

            BaseVecHandler& operator=(const BaseHandler &_handler) {
                command_buffers = { _handler.command_buffer };

                return *this;
            }

            BaseVecHandler& operator=(BaseHandler &&_handler) {
                command_buffers = { _handler.command_buffer };

                _handler.clearHandler();
                return *this;
            }

            BaseVecHandler& operator=(const BaseVecHandler &_handler) {
                command_buffers = _handler.command_buffers;

                return *this;
            }

            BaseVecHandler& operator=(BaseVecHandler &&_handler) {
                command_buffers = _handler.command_buffers;

                _handler.clearVecHandler();
                return *this;
            }


        // Access by index:

            VkCommandBuffer operator[](uint32_t _index) const { return command_buffers[_index]; }
            BaseHandler getBaseHandler(uint32_t _index) const { return { command_buffers[_index] }; }


        // Pointer and count:

            const VkCommandBuffer* ptr() const { return command_buffers.data(); }
            uint32_t count() const { return command_buffers.size(); }


        // Clear state:

            void clearVecHandler() {
                command_buffers.clear();
            }
        }; // BaseVecHandler END

        using VecHandler = BaseVecHandler;
    }; // VKFWCommandBuffer END
}; // Simple END

#endif