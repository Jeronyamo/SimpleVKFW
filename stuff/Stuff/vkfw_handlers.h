#ifndef SVKFW_VKFW_HANDLERS_H
#define SVKFW_VKFW_HANDLERS_H

#include "vkfw_base.h"


namespace Simple {
    namespace VKFW {

//  ===================================================  \\
                Vulkan framework - handlers
//  ===================================================  \\



        namespace Instance {

            struct Handler : ObjectHandlerBase {
                VkInstance handler = VK_NULL_HANDLE;

                Handler(VkInstance _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkInstance* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkInstance* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_INSTANCE; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_INSTANCE || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkInstance) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: Instance :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkInstance _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyInstance(handler, _destroy_callback);
                    return 0u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkInstance> handlers;

                VecHandler(std::vector<VkInstance> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkInstance* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkInstance* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkInstance  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_INSTANCE; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_INSTANCE || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkInstance) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: Instance :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkInstance> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    for (uint32_t i = 0u; i < handlers.size(); ++i)
                        vkDestroyInstance(handlers[i], _destroy_callback);
                    return 0u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_INSTANCE; }

                uint32_t destroy(ObjectInfoType _handler, VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyInstance((VkInstance)_handler.obj_handle, _destroy_callback);
                    return 0u;
                }
            }; // BuilderBase END

        }; // Instance END


        namespace PhysicalDevice {

            struct Handler : ObjectHandlerBase {
                VkPhysicalDevice handler = VK_NULL_HANDLE;

                Handler(VkPhysicalDevice _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkPhysicalDevice* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkPhysicalDevice* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_PHYSICAL_DEVICE; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_PHYSICAL_DEVICE || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkPhysicalDevice) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: PhysicalDevice :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkPhysicalDevice _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    return 1u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkPhysicalDevice> handlers;

                VecHandler(std::vector<VkPhysicalDevice> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkPhysicalDevice* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkPhysicalDevice* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkPhysicalDevice  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_PHYSICAL_DEVICE; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_PHYSICAL_DEVICE || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkPhysicalDevice) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: PhysicalDevice :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkPhysicalDevice> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    return 1u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_PHYSICAL_DEVICE; }
            }; // BuilderBase END

        }; // PhysicalDevice END


        namespace Device {

            struct Handler : ObjectHandlerBase {
                VkDevice handler = VK_NULL_HANDLE;

                Handler(VkDevice _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkDevice* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkDevice* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_DEVICE; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_DEVICE || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkDevice) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: Device :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkDevice _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyDevice(handler, _destroy_callback);
                    return 0u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkDevice> handlers;

                VecHandler(std::vector<VkDevice> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkDevice* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkDevice* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkDevice  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_DEVICE; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_DEVICE || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkDevice) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: Device :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkDevice> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    for (uint32_t i = 0u; i < handlers.size(); ++i)
                        vkDestroyDevice(handlers[i], _destroy_callback);
                    return 0u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_DEVICE; }

                uint32_t destroy(ObjectInfoType _handler, VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyDevice((VkDevice)_handler.obj_handle, _destroy_callback);
                    return 0u;
                }
            }; // BuilderBase END

        }; // Device END


        namespace Queue {

            struct Handler : ObjectHandlerBase {
                VkQueue handler = VK_NULL_HANDLE;

                Handler(VkQueue _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkQueue* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkQueue* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_QUEUE; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_QUEUE || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkQueue) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: Queue :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkQueue _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    return 1u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkQueue> handlers;

                VecHandler(std::vector<VkQueue> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkQueue* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkQueue* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkQueue  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_QUEUE; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_QUEUE || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkQueue) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: Queue :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkQueue> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    return 1u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_QUEUE; }
            }; // BuilderBase END

        }; // Queue END


        namespace Semaphore {

            struct Handler : ObjectHandlerBase {
                VkSemaphore handler = VK_NULL_HANDLE;

                Handler(VkSemaphore _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkSemaphore* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkSemaphore* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_SEMAPHORE; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_SEMAPHORE || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkSemaphore) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: Semaphore :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkSemaphore _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroySemaphore(_device, handler, _destroy_callback);
                    return 0u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkSemaphore> handlers;

                VecHandler(std::vector<VkSemaphore> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkSemaphore* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkSemaphore* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkSemaphore  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_SEMAPHORE; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_SEMAPHORE || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkSemaphore) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: Semaphore :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkSemaphore> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    for (uint32_t i = 0u; i < handlers.size(); ++i)
                        vkDestroySemaphore(_device, handlers[i], _destroy_callback);
                    return 0u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_SEMAPHORE; }

                uint32_t destroy(ObjectInfoType _handler, VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroySemaphore(_device, (VkSemaphore)_handler.obj_handle, _destroy_callback);
                    return 0u;
                }
            }; // BuilderBase END

        }; // Semaphore END


        namespace CommandBuffer {

            struct Handler : ObjectHandlerBase {
                VkCommandBuffer handler = VK_NULL_HANDLE;

                Handler(VkCommandBuffer _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkCommandBuffer* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkCommandBuffer* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_COMMAND_BUFFER; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_COMMAND_BUFFER || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkCommandBuffer) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: CommandBuffer :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkCommandBuffer _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    return 1u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkCommandBuffer> handlers;

                VecHandler(std::vector<VkCommandBuffer> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkCommandBuffer* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkCommandBuffer* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkCommandBuffer  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_COMMAND_BUFFER; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_COMMAND_BUFFER || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkCommandBuffer) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: CommandBuffer :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkCommandBuffer> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    return 1u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_COMMAND_BUFFER; }
            }; // BuilderBase END

        }; // CommandBuffer END


        namespace Fence {

            struct Handler : ObjectHandlerBase {
                VkFence handler = VK_NULL_HANDLE;

                Handler(VkFence _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkFence* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkFence* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_FENCE; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_FENCE || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkFence) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: Fence :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkFence _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyFence(_device, handler, _destroy_callback);
                    return 0u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkFence> handlers;

                VecHandler(std::vector<VkFence> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkFence* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkFence* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkFence  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_FENCE; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_FENCE || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkFence) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: Fence :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkFence> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    for (uint32_t i = 0u; i < handlers.size(); ++i)
                        vkDestroyFence(_device, handlers[i], _destroy_callback);
                    return 0u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_FENCE; }

                uint32_t destroy(ObjectInfoType _handler, VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyFence(_device, (VkFence)_handler.obj_handle, _destroy_callback);
                    return 0u;
                }
            }; // BuilderBase END

        }; // Fence END


        namespace DeviceMemory {

            struct Handler : ObjectHandlerBase {
                VkDeviceMemory handler = VK_NULL_HANDLE;

                Handler(VkDeviceMemory _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkDeviceMemory* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkDeviceMemory* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_DEVICE_MEMORY; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_DEVICE_MEMORY || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkDeviceMemory) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: DeviceMemory :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkDeviceMemory _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    return 1u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkDeviceMemory> handlers;

                VecHandler(std::vector<VkDeviceMemory> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkDeviceMemory* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkDeviceMemory* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkDeviceMemory  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_DEVICE_MEMORY; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_DEVICE_MEMORY || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkDeviceMemory) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: DeviceMemory :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkDeviceMemory> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    return 1u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_DEVICE_MEMORY; }
            }; // BuilderBase END

        }; // DeviceMemory END


        namespace Buffer {

            struct Handler : ObjectHandlerBase {
                VkBuffer handler = VK_NULL_HANDLE;

                Handler(VkBuffer _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkBuffer* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkBuffer* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_BUFFER; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_BUFFER || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkBuffer) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: Buffer :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkBuffer _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyBuffer(_device, handler, _destroy_callback);
                    return 0u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkBuffer> handlers;

                VecHandler(std::vector<VkBuffer> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkBuffer* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkBuffer* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkBuffer  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_BUFFER; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_BUFFER || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkBuffer) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: Buffer :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkBuffer> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    for (uint32_t i = 0u; i < handlers.size(); ++i)
                        vkDestroyBuffer(_device, handlers[i], _destroy_callback);
                    return 0u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_BUFFER; }

                uint32_t destroy(ObjectInfoType _handler, VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyBuffer(_device, (VkBuffer)_handler.obj_handle, _destroy_callback);
                    return 0u;
                }
            }; // BuilderBase END

        }; // Buffer END


        namespace Image {

            struct Handler : ObjectHandlerBase {
                VkImage handler = VK_NULL_HANDLE;

                Handler(VkImage _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkImage* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkImage* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_IMAGE; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_IMAGE || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkImage) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: Image :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkImage _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyImage(_device, handler, _destroy_callback);
                    return 0u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkImage> handlers;

                VecHandler(std::vector<VkImage> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkImage* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkImage* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkImage  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_IMAGE; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_IMAGE || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkImage) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: Image :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkImage> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    for (uint32_t i = 0u; i < handlers.size(); ++i)
                        vkDestroyImage(_device, handlers[i], _destroy_callback);
                    return 0u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_IMAGE; }

                uint32_t destroy(ObjectInfoType _handler, VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyImage(_device, (VkImage)_handler.obj_handle, _destroy_callback);
                    return 0u;
                }
            }; // BuilderBase END

        }; // Image END


        namespace Event {

            struct Handler : ObjectHandlerBase {
                VkEvent handler = VK_NULL_HANDLE;

                Handler(VkEvent _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkEvent* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkEvent* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_EVENT; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_EVENT || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkEvent) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: Event :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkEvent _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyEvent(_device, handler, _destroy_callback);
                    return 0u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkEvent> handlers;

                VecHandler(std::vector<VkEvent> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkEvent* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkEvent* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkEvent  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_EVENT; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_EVENT || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkEvent) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: Event :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkEvent> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    for (uint32_t i = 0u; i < handlers.size(); ++i)
                        vkDestroyEvent(_device, handlers[i], _destroy_callback);
                    return 0u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_EVENT; }

                uint32_t destroy(ObjectInfoType _handler, VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyEvent(_device, (VkEvent)_handler.obj_handle, _destroy_callback);
                    return 0u;
                }
            }; // BuilderBase END

        }; // Event END


        namespace QueryPool {

            struct Handler : ObjectHandlerBase {
                VkQueryPool handler = VK_NULL_HANDLE;

                Handler(VkQueryPool _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkQueryPool* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkQueryPool* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_QUERY_POOL; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_QUERY_POOL || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkQueryPool) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: QueryPool :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkQueryPool _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyQueryPool(_device, handler, _destroy_callback);
                    return 0u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkQueryPool> handlers;

                VecHandler(std::vector<VkQueryPool> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkQueryPool* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkQueryPool* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkQueryPool  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_QUERY_POOL; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_QUERY_POOL || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkQueryPool) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: QueryPool :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkQueryPool> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    for (uint32_t i = 0u; i < handlers.size(); ++i)
                        vkDestroyQueryPool(_device, handlers[i], _destroy_callback);
                    return 0u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_QUERY_POOL; }

                uint32_t destroy(ObjectInfoType _handler, VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyQueryPool(_device, (VkQueryPool)_handler.obj_handle, _destroy_callback);
                    return 0u;
                }
            }; // BuilderBase END

        }; // QueryPool END


        namespace BufferView {

            struct Handler : ObjectHandlerBase {
                VkBufferView handler = VK_NULL_HANDLE;

                Handler(VkBufferView _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkBufferView* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkBufferView* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_BUFFER_VIEW; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_BUFFER_VIEW || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkBufferView) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: BufferView :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkBufferView _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyBufferView(_device, handler, _destroy_callback);
                    return 0u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkBufferView> handlers;

                VecHandler(std::vector<VkBufferView> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkBufferView* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkBufferView* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkBufferView  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_BUFFER_VIEW; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_BUFFER_VIEW || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkBufferView) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: BufferView :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkBufferView> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    for (uint32_t i = 0u; i < handlers.size(); ++i)
                        vkDestroyBufferView(_device, handlers[i], _destroy_callback);
                    return 0u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_BUFFER_VIEW; }

                uint32_t destroy(ObjectInfoType _handler, VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyBufferView(_device, (VkBufferView)_handler.obj_handle, _destroy_callback);
                    return 0u;
                }
            }; // BuilderBase END

        }; // BufferView END


        namespace ImageView {

            struct Handler : ObjectHandlerBase {
                VkImageView handler = VK_NULL_HANDLE;

                Handler(VkImageView _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkImageView* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkImageView* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_IMAGE_VIEW; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_IMAGE_VIEW || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkImageView) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: ImageView :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkImageView _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyImageView(_device, handler, _destroy_callback);
                    return 0u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkImageView> handlers;

                VecHandler(std::vector<VkImageView> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkImageView* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkImageView* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkImageView  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_IMAGE_VIEW; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_IMAGE_VIEW || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkImageView) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: ImageView :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkImageView> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    for (uint32_t i = 0u; i < handlers.size(); ++i)
                        vkDestroyImageView(_device, handlers[i], _destroy_callback);
                    return 0u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_IMAGE_VIEW; }

                uint32_t destroy(ObjectInfoType _handler, VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyImageView(_device, (VkImageView)_handler.obj_handle, _destroy_callback);
                    return 0u;
                }
            }; // BuilderBase END

        }; // ImageView END


        namespace ShaderModule {

            struct Handler : ObjectHandlerBase {
                VkShaderModule handler = VK_NULL_HANDLE;

                Handler(VkShaderModule _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkShaderModule* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkShaderModule* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_SHADER_MODULE; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_SHADER_MODULE || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkShaderModule) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: ShaderModule :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkShaderModule _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyShaderModule(_device, handler, _destroy_callback);
                    return 0u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkShaderModule> handlers;

                VecHandler(std::vector<VkShaderModule> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkShaderModule* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkShaderModule* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkShaderModule  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_SHADER_MODULE; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_SHADER_MODULE || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkShaderModule) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: ShaderModule :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkShaderModule> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    for (uint32_t i = 0u; i < handlers.size(); ++i)
                        vkDestroyShaderModule(_device, handlers[i], _destroy_callback);
                    return 0u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_SHADER_MODULE; }

                uint32_t destroy(ObjectInfoType _handler, VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyShaderModule(_device, (VkShaderModule)_handler.obj_handle, _destroy_callback);
                    return 0u;
                }
            }; // BuilderBase END

        }; // ShaderModule END


        namespace PipelineCache {

            struct Handler : ObjectHandlerBase {
                VkPipelineCache handler = VK_NULL_HANDLE;

                Handler(VkPipelineCache _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkPipelineCache* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkPipelineCache* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_PIPELINE_CACHE; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_PIPELINE_CACHE || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkPipelineCache) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: PipelineCache :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkPipelineCache _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyPipelineCache(_device, handler, _destroy_callback);
                    return 0u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkPipelineCache> handlers;

                VecHandler(std::vector<VkPipelineCache> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkPipelineCache* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkPipelineCache* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkPipelineCache  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_PIPELINE_CACHE; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_PIPELINE_CACHE || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkPipelineCache) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: PipelineCache :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkPipelineCache> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    for (uint32_t i = 0u; i < handlers.size(); ++i)
                        vkDestroyPipelineCache(_device, handlers[i], _destroy_callback);
                    return 0u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_PIPELINE_CACHE; }

                uint32_t destroy(ObjectInfoType _handler, VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyPipelineCache(_device, (VkPipelineCache)_handler.obj_handle, _destroy_callback);
                    return 0u;
                }
            }; // BuilderBase END

        }; // PipelineCache END


        namespace PipelineLayout {

            struct Handler : ObjectHandlerBase {
                VkPipelineLayout handler = VK_NULL_HANDLE;

                Handler(VkPipelineLayout _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkPipelineLayout* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkPipelineLayout* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_PIPELINE_LAYOUT; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_PIPELINE_LAYOUT || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkPipelineLayout) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: PipelineLayout :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkPipelineLayout _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyPipelineLayout(_device, handler, _destroy_callback);
                    return 0u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkPipelineLayout> handlers;

                VecHandler(std::vector<VkPipelineLayout> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkPipelineLayout* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkPipelineLayout* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkPipelineLayout  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_PIPELINE_LAYOUT; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_PIPELINE_LAYOUT || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkPipelineLayout) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: PipelineLayout :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkPipelineLayout> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    for (uint32_t i = 0u; i < handlers.size(); ++i)
                        vkDestroyPipelineLayout(_device, handlers[i], _destroy_callback);
                    return 0u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_PIPELINE_LAYOUT; }

                uint32_t destroy(ObjectInfoType _handler, VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyPipelineLayout(_device, (VkPipelineLayout)_handler.obj_handle, _destroy_callback);
                    return 0u;
                }
            }; // BuilderBase END

        }; // PipelineLayout END


        namespace RenderPass {

            struct Handler : ObjectHandlerBase {
                VkRenderPass handler = VK_NULL_HANDLE;

                Handler(VkRenderPass _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkRenderPass* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkRenderPass* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_RENDER_PASS; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_RENDER_PASS || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkRenderPass) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: RenderPass :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkRenderPass _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyRenderPass(_device, handler, _destroy_callback);
                    return 0u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkRenderPass> handlers;

                VecHandler(std::vector<VkRenderPass> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkRenderPass* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkRenderPass* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkRenderPass  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_RENDER_PASS; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_RENDER_PASS || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkRenderPass) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: RenderPass :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkRenderPass> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    for (uint32_t i = 0u; i < handlers.size(); ++i)
                        vkDestroyRenderPass(_device, handlers[i], _destroy_callback);
                    return 0u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_RENDER_PASS; }

                uint32_t destroy(ObjectInfoType _handler, VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyRenderPass(_device, (VkRenderPass)_handler.obj_handle, _destroy_callback);
                    return 0u;
                }
            }; // BuilderBase END

        }; // RenderPass END


        namespace Pipeline {

            struct Handler : ObjectHandlerBase {
                VkPipeline handler = VK_NULL_HANDLE;

                Handler(VkPipeline _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkPipeline* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkPipeline* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_PIPELINE; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_PIPELINE || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkPipeline) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: Pipeline :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkPipeline _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyPipeline(_device, handler, _destroy_callback);
                    return 0u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkPipeline> handlers;

                VecHandler(std::vector<VkPipeline> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkPipeline* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkPipeline* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkPipeline  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_PIPELINE; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_PIPELINE || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkPipeline) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: Pipeline :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkPipeline> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    for (uint32_t i = 0u; i < handlers.size(); ++i)
                        vkDestroyPipeline(_device, handlers[i], _destroy_callback);
                    return 0u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_PIPELINE; }

                uint32_t destroy(ObjectInfoType _handler, VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyPipeline(_device, (VkPipeline)_handler.obj_handle, _destroy_callback);
                    return 0u;
                }
            }; // BuilderBase END

        }; // Pipeline END


        namespace DescriptorSetLayout {

            struct Handler : ObjectHandlerBase {
                VkDescriptorSetLayout handler = VK_NULL_HANDLE;

                Handler(VkDescriptorSetLayout _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkDescriptorSetLayout* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkDescriptorSetLayout* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkDescriptorSetLayout) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: DescriptorSetLayout :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkDescriptorSetLayout _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyDescriptorSetLayout(_device, handler, _destroy_callback);
                    return 0u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkDescriptorSetLayout> handlers;

                VecHandler(std::vector<VkDescriptorSetLayout> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkDescriptorSetLayout* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkDescriptorSetLayout* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkDescriptorSetLayout  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkDescriptorSetLayout) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: DescriptorSetLayout :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkDescriptorSetLayout> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    for (uint32_t i = 0u; i < handlers.size(); ++i)
                        vkDestroyDescriptorSetLayout(_device, handlers[i], _destroy_callback);
                    return 0u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT; }

                uint32_t destroy(ObjectInfoType _handler, VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyDescriptorSetLayout(_device, (VkDescriptorSetLayout)_handler.obj_handle, _destroy_callback);
                    return 0u;
                }
            }; // BuilderBase END

        }; // DescriptorSetLayout END


        namespace Sampler {

            struct Handler : ObjectHandlerBase {
                VkSampler handler = VK_NULL_HANDLE;

                Handler(VkSampler _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkSampler* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkSampler* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_SAMPLER; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_SAMPLER || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkSampler) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: Sampler :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkSampler _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroySampler(_device, handler, _destroy_callback);
                    return 0u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkSampler> handlers;

                VecHandler(std::vector<VkSampler> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkSampler* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkSampler* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkSampler  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_SAMPLER; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_SAMPLER || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkSampler) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: Sampler :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkSampler> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    for (uint32_t i = 0u; i < handlers.size(); ++i)
                        vkDestroySampler(_device, handlers[i], _destroy_callback);
                    return 0u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_SAMPLER; }

                uint32_t destroy(ObjectInfoType _handler, VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroySampler(_device, (VkSampler)_handler.obj_handle, _destroy_callback);
                    return 0u;
                }
            }; // BuilderBase END

        }; // Sampler END


        namespace DescriptorPool {

            struct Handler : ObjectHandlerBase {
                VkDescriptorPool handler = VK_NULL_HANDLE;

                Handler(VkDescriptorPool _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkDescriptorPool* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkDescriptorPool* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_DESCRIPTOR_POOL; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_DESCRIPTOR_POOL || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkDescriptorPool) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: DescriptorPool :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkDescriptorPool _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyDescriptorPool(_device, handler, _destroy_callback);
                    return 0u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkDescriptorPool> handlers;

                VecHandler(std::vector<VkDescriptorPool> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkDescriptorPool* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkDescriptorPool* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkDescriptorPool  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_DESCRIPTOR_POOL; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_DESCRIPTOR_POOL || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkDescriptorPool) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: DescriptorPool :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkDescriptorPool> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    for (uint32_t i = 0u; i < handlers.size(); ++i)
                        vkDestroyDescriptorPool(_device, handlers[i], _destroy_callback);
                    return 0u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_DESCRIPTOR_POOL; }

                uint32_t destroy(ObjectInfoType _handler, VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyDescriptorPool(_device, (VkDescriptorPool)_handler.obj_handle, _destroy_callback);
                    return 0u;
                }
            }; // BuilderBase END

        }; // DescriptorPool END


        namespace DescriptorSet {

            struct Handler : ObjectHandlerBase {
                VkDescriptorSet handler = VK_NULL_HANDLE;

                Handler(VkDescriptorSet _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkDescriptorSet* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkDescriptorSet* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_DESCRIPTOR_SET; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_DESCRIPTOR_SET || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkDescriptorSet) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: DescriptorSet :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkDescriptorSet _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    return 1u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkDescriptorSet> handlers;

                VecHandler(std::vector<VkDescriptorSet> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkDescriptorSet* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkDescriptorSet* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkDescriptorSet  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_DESCRIPTOR_SET; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_DESCRIPTOR_SET || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkDescriptorSet) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: DescriptorSet :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkDescriptorSet> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    return 1u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_DESCRIPTOR_SET; }
            }; // BuilderBase END

        }; // DescriptorSet END


        namespace Framebuffer {

            struct Handler : ObjectHandlerBase {
                VkFramebuffer handler = VK_NULL_HANDLE;

                Handler(VkFramebuffer _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkFramebuffer* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkFramebuffer* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_FRAMEBUFFER; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_FRAMEBUFFER || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkFramebuffer) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: Framebuffer :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkFramebuffer _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyFramebuffer(_device, handler, _destroy_callback);
                    return 0u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkFramebuffer> handlers;

                VecHandler(std::vector<VkFramebuffer> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkFramebuffer* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkFramebuffer* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkFramebuffer  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_FRAMEBUFFER; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_FRAMEBUFFER || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkFramebuffer) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: Framebuffer :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkFramebuffer> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    for (uint32_t i = 0u; i < handlers.size(); ++i)
                        vkDestroyFramebuffer(_device, handlers[i], _destroy_callback);
                    return 0u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_FRAMEBUFFER; }

                uint32_t destroy(ObjectInfoType _handler, VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyFramebuffer(_device, (VkFramebuffer)_handler.obj_handle, _destroy_callback);
                    return 0u;
                }
            }; // BuilderBase END

        }; // Framebuffer END


        namespace CommandPool {

            struct Handler : ObjectHandlerBase {
                VkCommandPool handler = VK_NULL_HANDLE;

                Handler(VkCommandPool _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkCommandPool* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkCommandPool* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_COMMAND_POOL; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_COMMAND_POOL || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkCommandPool) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: CommandPool :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkCommandPool _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyCommandPool(_device, handler, _destroy_callback);
                    return 0u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkCommandPool> handlers;

                VecHandler(std::vector<VkCommandPool> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkCommandPool* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkCommandPool* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkCommandPool  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_COMMAND_POOL; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_COMMAND_POOL || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkCommandPool) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: CommandPool :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkCommandPool> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    for (uint32_t i = 0u; i < handlers.size(); ++i)
                        vkDestroyCommandPool(_device, handlers[i], _destroy_callback);
                    return 0u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_COMMAND_POOL; }

                uint32_t destroy(ObjectInfoType _handler, VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyCommandPool(_device, (VkCommandPool)_handler.obj_handle, _destroy_callback);
                    return 0u;
                }
            }; // BuilderBase END

        }; // CommandPool END


        namespace SamplerYcbcrConversion {

            struct Handler : ObjectHandlerBase {
                VkSamplerYcbcrConversion handler = VK_NULL_HANDLE;

                Handler(VkSamplerYcbcrConversion _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkSamplerYcbcrConversion* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkSamplerYcbcrConversion* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkSamplerYcbcrConversion) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: SamplerYcbcrConversion :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkSamplerYcbcrConversion _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroySamplerYcbcrConversion(_device, handler, _destroy_callback);
                    return 0u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkSamplerYcbcrConversion> handlers;

                VecHandler(std::vector<VkSamplerYcbcrConversion> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkSamplerYcbcrConversion* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkSamplerYcbcrConversion* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkSamplerYcbcrConversion  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkSamplerYcbcrConversion) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: SamplerYcbcrConversion :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkSamplerYcbcrConversion> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    for (uint32_t i = 0u; i < handlers.size(); ++i)
                        vkDestroySamplerYcbcrConversion(_device, handlers[i], _destroy_callback);
                    return 0u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION; }

                uint32_t destroy(ObjectInfoType _handler, VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroySamplerYcbcrConversion(_device, (VkSamplerYcbcrConversion)_handler.obj_handle, _destroy_callback);
                    return 0u;
                }
            }; // BuilderBase END

        }; // SamplerYcbcrConversion END


        namespace DescriptorUpdateTemplate {

            struct Handler : ObjectHandlerBase {
                VkDescriptorUpdateTemplate handler = VK_NULL_HANDLE;

                Handler(VkDescriptorUpdateTemplate _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkDescriptorUpdateTemplate* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkDescriptorUpdateTemplate* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkDescriptorUpdateTemplate) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: DescriptorUpdateTemplate :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkDescriptorUpdateTemplate _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyDescriptorUpdateTemplate(_device, handler, _destroy_callback);
                    return 0u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkDescriptorUpdateTemplate> handlers;

                VecHandler(std::vector<VkDescriptorUpdateTemplate> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkDescriptorUpdateTemplate* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkDescriptorUpdateTemplate* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkDescriptorUpdateTemplate  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkDescriptorUpdateTemplate) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: DescriptorUpdateTemplate :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkDescriptorUpdateTemplate> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    for (uint32_t i = 0u; i < handlers.size(); ++i)
                        vkDestroyDescriptorUpdateTemplate(_device, handlers[i], _destroy_callback);
                    return 0u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE; }

                uint32_t destroy(ObjectInfoType _handler, VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyDescriptorUpdateTemplate(_device, (VkDescriptorUpdateTemplate)_handler.obj_handle, _destroy_callback);
                    return 0u;
                }
            }; // BuilderBase END

        }; // DescriptorUpdateTemplate END


        namespace PrivateDataSlot {

            struct Handler : ObjectHandlerBase {
                VkPrivateDataSlot handler = VK_NULL_HANDLE;

                Handler(VkPrivateDataSlot _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkPrivateDataSlot* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkPrivateDataSlot* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_PRIVATE_DATA_SLOT; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_PRIVATE_DATA_SLOT || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkPrivateDataSlot) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: PrivateDataSlot :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkPrivateDataSlot _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyPrivateDataSlot(_device, handler, _destroy_callback);
                    return 0u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkPrivateDataSlot> handlers;

                VecHandler(std::vector<VkPrivateDataSlot> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkPrivateDataSlot* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkPrivateDataSlot* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkPrivateDataSlot  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_PRIVATE_DATA_SLOT; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_PRIVATE_DATA_SLOT || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkPrivateDataSlot) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: PrivateDataSlot :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkPrivateDataSlot> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    for (uint32_t i = 0u; i < handlers.size(); ++i)
                        vkDestroyPrivateDataSlot(_device, handlers[i], _destroy_callback);
                    return 0u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_PRIVATE_DATA_SLOT; }

                uint32_t destroy(ObjectInfoType _handler, VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyPrivateDataSlot(_device, (VkPrivateDataSlot)_handler.obj_handle, _destroy_callback);
                    return 0u;
                }
            }; // BuilderBase END

        }; // PrivateDataSlot END


        namespace SurfaceKHR {

            struct Handler : ObjectHandlerBase {
                VkSurfaceKHR handler = VK_NULL_HANDLE;

                Handler(VkSurfaceKHR _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkSurfaceKHR* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkSurfaceKHR* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_SURFACE_KHR; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_SURFACE_KHR || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkSurfaceKHR) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: SurfaceKHR :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkSurfaceKHR _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroySurfaceKHR(_instance, handler, _destroy_callback);
                    return 0u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkSurfaceKHR> handlers;

                VecHandler(std::vector<VkSurfaceKHR> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkSurfaceKHR* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkSurfaceKHR* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkSurfaceKHR  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_SURFACE_KHR; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_SURFACE_KHR || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkSurfaceKHR) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: SurfaceKHR :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkSurfaceKHR> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    for (uint32_t i = 0u; i < handlers.size(); ++i)
                        vkDestroySurfaceKHR(_instance, handlers[i], _destroy_callback);
                    return 0u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_SURFACE_KHR; }

                uint32_t destroy(ObjectInfoType _handler, VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroySurfaceKHR(_instance, (VkSurfaceKHR)_handler.obj_handle, _destroy_callback);
                    return 0u;
                }
            }; // BuilderBase END

        }; // SurfaceKHR END


        namespace SwapchainKHR {

            struct Handler : ObjectHandlerBase {
                VkSwapchainKHR handler = VK_NULL_HANDLE;

                Handler(VkSwapchainKHR _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkSwapchainKHR* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkSwapchainKHR* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_SWAPCHAIN_KHR; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_SWAPCHAIN_KHR || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkSwapchainKHR) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: SwapchainKHR :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkSwapchainKHR _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroySwapchainKHR(_device, handler, _destroy_callback);
                    return 0u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkSwapchainKHR> handlers;

                VecHandler(std::vector<VkSwapchainKHR> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkSwapchainKHR* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkSwapchainKHR* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkSwapchainKHR  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_SWAPCHAIN_KHR; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_SWAPCHAIN_KHR || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkSwapchainKHR) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: SwapchainKHR :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkSwapchainKHR> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    for (uint32_t i = 0u; i < handlers.size(); ++i)
                        vkDestroySwapchainKHR(_device, handlers[i], _destroy_callback);
                    return 0u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_SWAPCHAIN_KHR; }

                uint32_t destroy(ObjectInfoType _handler, VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroySwapchainKHR(_device, (VkSwapchainKHR)_handler.obj_handle, _destroy_callback);
                    return 0u;
                }
            }; // BuilderBase END

        }; // SwapchainKHR END


        namespace DisplayKHR {

            struct Handler : ObjectHandlerBase {
                VkDisplayKHR handler = VK_NULL_HANDLE;

                Handler(VkDisplayKHR _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkDisplayKHR* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkDisplayKHR* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_DISPLAY_KHR; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_DISPLAY_KHR || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkDisplayKHR) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: DisplayKHR :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkDisplayKHR _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    return 1u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkDisplayKHR> handlers;

                VecHandler(std::vector<VkDisplayKHR> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkDisplayKHR* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkDisplayKHR* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkDisplayKHR  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_DISPLAY_KHR; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_DISPLAY_KHR || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkDisplayKHR) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: DisplayKHR :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkDisplayKHR> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    return 1u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_DISPLAY_KHR; }
            }; // BuilderBase END

        }; // DisplayKHR END


        namespace DisplayModeKHR {

            struct Handler : ObjectHandlerBase {
                VkDisplayModeKHR handler = VK_NULL_HANDLE;

                Handler(VkDisplayModeKHR _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkDisplayModeKHR* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkDisplayModeKHR* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_DISPLAY_MODE_KHR; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_DISPLAY_MODE_KHR || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkDisplayModeKHR) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: DisplayModeKHR :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkDisplayModeKHR _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    return 1u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkDisplayModeKHR> handlers;

                VecHandler(std::vector<VkDisplayModeKHR> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkDisplayModeKHR* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkDisplayModeKHR* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkDisplayModeKHR  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_DISPLAY_MODE_KHR; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_DISPLAY_MODE_KHR || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkDisplayModeKHR) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: DisplayModeKHR :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkDisplayModeKHR> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    return 1u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_DISPLAY_MODE_KHR; }
            }; // BuilderBase END

        }; // DisplayModeKHR END


        namespace DebugReportCallbackEXT {

            struct Handler : ObjectHandlerBase {
                VkDebugReportCallbackEXT handler = VK_NULL_HANDLE;

                Handler(VkDebugReportCallbackEXT _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkDebugReportCallbackEXT* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkDebugReportCallbackEXT* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkDebugReportCallbackEXT) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: DebugReportCallbackEXT :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkDebugReportCallbackEXT _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyDebugReportCallbackEXT(_instance, handler, _destroy_callback);
                    return 0u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkDebugReportCallbackEXT> handlers;

                VecHandler(std::vector<VkDebugReportCallbackEXT> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkDebugReportCallbackEXT* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkDebugReportCallbackEXT* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkDebugReportCallbackEXT  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkDebugReportCallbackEXT) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: DebugReportCallbackEXT :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkDebugReportCallbackEXT> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    for (uint32_t i = 0u; i < handlers.size(); ++i)
                        vkDestroyDebugReportCallbackEXT(_instance, handlers[i], _destroy_callback);
                    return 0u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT; }

                uint32_t destroy(ObjectInfoType _handler, VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyDebugReportCallbackEXT(_instance, (VkDebugReportCallbackEXT)_handler.obj_handle, _destroy_callback);
                    return 0u;
                }
            }; // BuilderBase END

        }; // DebugReportCallbackEXT END


        namespace VideoSessionKHR {

            struct Handler : ObjectHandlerBase {
                VkVideoSessionKHR handler = VK_NULL_HANDLE;

                Handler(VkVideoSessionKHR _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkVideoSessionKHR* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkVideoSessionKHR* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_VIDEO_SESSION_KHR; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_VIDEO_SESSION_KHR || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkVideoSessionKHR) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: VideoSessionKHR :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkVideoSessionKHR _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyVideoSessionKHR(_device, handler, _destroy_callback);
                    return 0u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkVideoSessionKHR> handlers;

                VecHandler(std::vector<VkVideoSessionKHR> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkVideoSessionKHR* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkVideoSessionKHR* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkVideoSessionKHR  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_VIDEO_SESSION_KHR; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_VIDEO_SESSION_KHR || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkVideoSessionKHR) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: VideoSessionKHR :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkVideoSessionKHR> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    for (uint32_t i = 0u; i < handlers.size(); ++i)
                        vkDestroyVideoSessionKHR(_device, handlers[i], _destroy_callback);
                    return 0u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_VIDEO_SESSION_KHR; }

                uint32_t destroy(ObjectInfoType _handler, VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyVideoSessionKHR(_device, (VkVideoSessionKHR)_handler.obj_handle, _destroy_callback);
                    return 0u;
                }
            }; // BuilderBase END

        }; // VideoSessionKHR END


        namespace VideoSessionParametersKHR {

            struct Handler : ObjectHandlerBase {
                VkVideoSessionParametersKHR handler = VK_NULL_HANDLE;

                Handler(VkVideoSessionParametersKHR _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkVideoSessionParametersKHR* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkVideoSessionParametersKHR* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_VIDEO_SESSION_PARAMETERS_KHR; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_VIDEO_SESSION_PARAMETERS_KHR || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkVideoSessionParametersKHR) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: VideoSessionParametersKHR :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkVideoSessionParametersKHR _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyVideoSessionParametersKHR(_device, handler, _destroy_callback);
                    return 0u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkVideoSessionParametersKHR> handlers;

                VecHandler(std::vector<VkVideoSessionParametersKHR> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkVideoSessionParametersKHR* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkVideoSessionParametersKHR* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkVideoSessionParametersKHR  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_VIDEO_SESSION_PARAMETERS_KHR; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_VIDEO_SESSION_PARAMETERS_KHR || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkVideoSessionParametersKHR) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: VideoSessionParametersKHR :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkVideoSessionParametersKHR> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    for (uint32_t i = 0u; i < handlers.size(); ++i)
                        vkDestroyVideoSessionParametersKHR(_device, handlers[i], _destroy_callback);
                    return 0u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_VIDEO_SESSION_PARAMETERS_KHR; }

                uint32_t destroy(ObjectInfoType _handler, VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyVideoSessionParametersKHR(_device, (VkVideoSessionParametersKHR)_handler.obj_handle, _destroy_callback);
                    return 0u;
                }
            }; // BuilderBase END

        }; // VideoSessionParametersKHR END


        namespace CuModuleNVX {

            struct Handler : ObjectHandlerBase {
                VkCuModuleNVX handler = VK_NULL_HANDLE;

                Handler(VkCuModuleNVX _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkCuModuleNVX* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkCuModuleNVX* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_CU_MODULE_NVX; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_CU_MODULE_NVX || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkCuModuleNVX) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: CuModuleNVX :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkCuModuleNVX _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyCuModuleNVX(_device, handler, _destroy_callback);
                    return 0u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkCuModuleNVX> handlers;

                VecHandler(std::vector<VkCuModuleNVX> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkCuModuleNVX* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkCuModuleNVX* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkCuModuleNVX  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_CU_MODULE_NVX; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_CU_MODULE_NVX || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkCuModuleNVX) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: CuModuleNVX :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkCuModuleNVX> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    for (uint32_t i = 0u; i < handlers.size(); ++i)
                        vkDestroyCuModuleNVX(_device, handlers[i], _destroy_callback);
                    return 0u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_CU_MODULE_NVX; }

                uint32_t destroy(ObjectInfoType _handler, VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyCuModuleNVX(_device, (VkCuModuleNVX)_handler.obj_handle, _destroy_callback);
                    return 0u;
                }
            }; // BuilderBase END

        }; // CuModuleNVX END


        namespace CuFunctionNVX {

            struct Handler : ObjectHandlerBase {
                VkCuFunctionNVX handler = VK_NULL_HANDLE;

                Handler(VkCuFunctionNVX _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkCuFunctionNVX* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkCuFunctionNVX* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_CU_FUNCTION_NVX; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_CU_FUNCTION_NVX || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkCuFunctionNVX) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: CuFunctionNVX :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkCuFunctionNVX _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyCuFunctionNVX(_device, handler, _destroy_callback);
                    return 0u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkCuFunctionNVX> handlers;

                VecHandler(std::vector<VkCuFunctionNVX> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkCuFunctionNVX* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkCuFunctionNVX* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkCuFunctionNVX  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_CU_FUNCTION_NVX; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_CU_FUNCTION_NVX || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkCuFunctionNVX) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: CuFunctionNVX :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkCuFunctionNVX> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    for (uint32_t i = 0u; i < handlers.size(); ++i)
                        vkDestroyCuFunctionNVX(_device, handlers[i], _destroy_callback);
                    return 0u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_CU_FUNCTION_NVX; }

                uint32_t destroy(ObjectInfoType _handler, VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyCuFunctionNVX(_device, (VkCuFunctionNVX)_handler.obj_handle, _destroy_callback);
                    return 0u;
                }
            }; // BuilderBase END

        }; // CuFunctionNVX END


        namespace DebugUtilsMessengerEXT {

            struct Handler : ObjectHandlerBase {
                VkDebugUtilsMessengerEXT handler = VK_NULL_HANDLE;

                Handler(VkDebugUtilsMessengerEXT _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkDebugUtilsMessengerEXT* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkDebugUtilsMessengerEXT* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkDebugUtilsMessengerEXT) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: DebugUtilsMessengerEXT :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkDebugUtilsMessengerEXT _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyDebugUtilsMessengerEXT(_instance, handler, _destroy_callback);
                    return 0u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkDebugUtilsMessengerEXT> handlers;

                VecHandler(std::vector<VkDebugUtilsMessengerEXT> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkDebugUtilsMessengerEXT* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkDebugUtilsMessengerEXT* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkDebugUtilsMessengerEXT  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkDebugUtilsMessengerEXT) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: DebugUtilsMessengerEXT :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkDebugUtilsMessengerEXT> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    for (uint32_t i = 0u; i < handlers.size(); ++i)
                        vkDestroyDebugUtilsMessengerEXT(_instance, handlers[i], _destroy_callback);
                    return 0u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT; }

                uint32_t destroy(ObjectInfoType _handler, VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyDebugUtilsMessengerEXT(_instance, (VkDebugUtilsMessengerEXT)_handler.obj_handle, _destroy_callback);
                    return 0u;
                }
            }; // BuilderBase END

        }; // DebugUtilsMessengerEXT END


        namespace AccelerationStructureKHR {

            struct Handler : ObjectHandlerBase {
                VkAccelerationStructureKHR handler = VK_NULL_HANDLE;

                Handler(VkAccelerationStructureKHR _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkAccelerationStructureKHR* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkAccelerationStructureKHR* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkAccelerationStructureKHR) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: AccelerationStructureKHR :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkAccelerationStructureKHR _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyAccelerationStructureKHR(_device, handler, _destroy_callback);
                    return 0u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkAccelerationStructureKHR> handlers;

                VecHandler(std::vector<VkAccelerationStructureKHR> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkAccelerationStructureKHR* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkAccelerationStructureKHR* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkAccelerationStructureKHR  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkAccelerationStructureKHR) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: AccelerationStructureKHR :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkAccelerationStructureKHR> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    for (uint32_t i = 0u; i < handlers.size(); ++i)
                        vkDestroyAccelerationStructureKHR(_device, handlers[i], _destroy_callback);
                    return 0u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR; }

                uint32_t destroy(ObjectInfoType _handler, VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyAccelerationStructureKHR(_device, (VkAccelerationStructureKHR)_handler.obj_handle, _destroy_callback);
                    return 0u;
                }
            }; // BuilderBase END

        }; // AccelerationStructureKHR END


        namespace ValidationCacheEXT {

            struct Handler : ObjectHandlerBase {
                VkValidationCacheEXT handler = VK_NULL_HANDLE;

                Handler(VkValidationCacheEXT _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkValidationCacheEXT* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkValidationCacheEXT* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_VALIDATION_CACHE_EXT; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_VALIDATION_CACHE_EXT || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkValidationCacheEXT) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: ValidationCacheEXT :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkValidationCacheEXT _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyValidationCacheEXT(_device, handler, _destroy_callback);
                    return 0u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkValidationCacheEXT> handlers;

                VecHandler(std::vector<VkValidationCacheEXT> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkValidationCacheEXT* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkValidationCacheEXT* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkValidationCacheEXT  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_VALIDATION_CACHE_EXT; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_VALIDATION_CACHE_EXT || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkValidationCacheEXT) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: ValidationCacheEXT :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkValidationCacheEXT> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    for (uint32_t i = 0u; i < handlers.size(); ++i)
                        vkDestroyValidationCacheEXT(_device, handlers[i], _destroy_callback);
                    return 0u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_VALIDATION_CACHE_EXT; }

                uint32_t destroy(ObjectInfoType _handler, VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyValidationCacheEXT(_device, (VkValidationCacheEXT)_handler.obj_handle, _destroy_callback);
                    return 0u;
                }
            }; // BuilderBase END

        }; // ValidationCacheEXT END


        namespace AccelerationStructureNV {

            struct Handler : ObjectHandlerBase {
                VkAccelerationStructureNV handler = VK_NULL_HANDLE;

                Handler(VkAccelerationStructureNV _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkAccelerationStructureNV* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkAccelerationStructureNV* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkAccelerationStructureNV) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: AccelerationStructureNV :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkAccelerationStructureNV _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyAccelerationStructureNV(_device, handler, _destroy_callback);
                    return 0u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkAccelerationStructureNV> handlers;

                VecHandler(std::vector<VkAccelerationStructureNV> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkAccelerationStructureNV* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkAccelerationStructureNV* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkAccelerationStructureNV  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkAccelerationStructureNV) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: AccelerationStructureNV :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkAccelerationStructureNV> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    for (uint32_t i = 0u; i < handlers.size(); ++i)
                        vkDestroyAccelerationStructureNV(_device, handlers[i], _destroy_callback);
                    return 0u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV; }

                uint32_t destroy(ObjectInfoType _handler, VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyAccelerationStructureNV(_device, (VkAccelerationStructureNV)_handler.obj_handle, _destroy_callback);
                    return 0u;
                }
            }; // BuilderBase END

        }; // AccelerationStructureNV END


        namespace PerformanceConfigurationINTEL {

            struct Handler : ObjectHandlerBase {
                VkPerformanceConfigurationINTEL handler = VK_NULL_HANDLE;

                Handler(VkPerformanceConfigurationINTEL _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkPerformanceConfigurationINTEL* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkPerformanceConfigurationINTEL* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_PERFORMANCE_CONFIGURATION_INTEL; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_PERFORMANCE_CONFIGURATION_INTEL || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkPerformanceConfigurationINTEL) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: PerformanceConfigurationINTEL :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkPerformanceConfigurationINTEL _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    return 1u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkPerformanceConfigurationINTEL> handlers;

                VecHandler(std::vector<VkPerformanceConfigurationINTEL> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkPerformanceConfigurationINTEL* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkPerformanceConfigurationINTEL* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkPerformanceConfigurationINTEL  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_PERFORMANCE_CONFIGURATION_INTEL; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_PERFORMANCE_CONFIGURATION_INTEL || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkPerformanceConfigurationINTEL) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: PerformanceConfigurationINTEL :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkPerformanceConfigurationINTEL> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    return 1u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_PERFORMANCE_CONFIGURATION_INTEL; }
            }; // BuilderBase END

        }; // PerformanceConfigurationINTEL END


        namespace DeferredOperationKHR {

            struct Handler : ObjectHandlerBase {
                VkDeferredOperationKHR handler = VK_NULL_HANDLE;

                Handler(VkDeferredOperationKHR _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkDeferredOperationKHR* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkDeferredOperationKHR* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_DEFERRED_OPERATION_KHR; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_DEFERRED_OPERATION_KHR || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkDeferredOperationKHR) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: DeferredOperationKHR :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkDeferredOperationKHR _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyDeferredOperationKHR(_device, handler, _destroy_callback);
                    return 0u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkDeferredOperationKHR> handlers;

                VecHandler(std::vector<VkDeferredOperationKHR> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkDeferredOperationKHR* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkDeferredOperationKHR* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkDeferredOperationKHR  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_DEFERRED_OPERATION_KHR; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_DEFERRED_OPERATION_KHR || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkDeferredOperationKHR) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: DeferredOperationKHR :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkDeferredOperationKHR> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    for (uint32_t i = 0u; i < handlers.size(); ++i)
                        vkDestroyDeferredOperationKHR(_device, handlers[i], _destroy_callback);
                    return 0u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_DEFERRED_OPERATION_KHR; }

                uint32_t destroy(ObjectInfoType _handler, VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyDeferredOperationKHR(_device, (VkDeferredOperationKHR)_handler.obj_handle, _destroy_callback);
                    return 0u;
                }
            }; // BuilderBase END

        }; // DeferredOperationKHR END


        namespace IndirectCommandsLayoutNV {

            struct Handler : ObjectHandlerBase {
                VkIndirectCommandsLayoutNV handler = VK_NULL_HANDLE;

                Handler(VkIndirectCommandsLayoutNV _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkIndirectCommandsLayoutNV* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkIndirectCommandsLayoutNV* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkIndirectCommandsLayoutNV) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: IndirectCommandsLayoutNV :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkIndirectCommandsLayoutNV _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyIndirectCommandsLayoutNV(_device, handler, _destroy_callback);
                    return 0u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkIndirectCommandsLayoutNV> handlers;

                VecHandler(std::vector<VkIndirectCommandsLayoutNV> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkIndirectCommandsLayoutNV* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkIndirectCommandsLayoutNV* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkIndirectCommandsLayoutNV  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkIndirectCommandsLayoutNV) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: IndirectCommandsLayoutNV :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkIndirectCommandsLayoutNV> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    for (uint32_t i = 0u; i < handlers.size(); ++i)
                        vkDestroyIndirectCommandsLayoutNV(_device, handlers[i], _destroy_callback);
                    return 0u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV; }

                uint32_t destroy(ObjectInfoType _handler, VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyIndirectCommandsLayoutNV(_device, (VkIndirectCommandsLayoutNV)_handler.obj_handle, _destroy_callback);
                    return 0u;
                }
            }; // BuilderBase END

        }; // IndirectCommandsLayoutNV END


        namespace CudaModuleNV {

            struct Handler : ObjectHandlerBase {
                VkCudaModuleNV handler = VK_NULL_HANDLE;

                Handler(VkCudaModuleNV _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkCudaModuleNV* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkCudaModuleNV* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_CUDA_MODULE_NV; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_CUDA_MODULE_NV || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkCudaModuleNV) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: CudaModuleNV :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkCudaModuleNV _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyCudaModuleNV(_device, handler, _destroy_callback);
                    return 0u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkCudaModuleNV> handlers;

                VecHandler(std::vector<VkCudaModuleNV> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkCudaModuleNV* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkCudaModuleNV* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkCudaModuleNV  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_CUDA_MODULE_NV; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_CUDA_MODULE_NV || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkCudaModuleNV) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: CudaModuleNV :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkCudaModuleNV> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    for (uint32_t i = 0u; i < handlers.size(); ++i)
                        vkDestroyCudaModuleNV(_device, handlers[i], _destroy_callback);
                    return 0u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_CUDA_MODULE_NV; }

                uint32_t destroy(ObjectInfoType _handler, VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyCudaModuleNV(_device, (VkCudaModuleNV)_handler.obj_handle, _destroy_callback);
                    return 0u;
                }
            }; // BuilderBase END

        }; // CudaModuleNV END


        namespace CudaFunctionNV {

            struct Handler : ObjectHandlerBase {
                VkCudaFunctionNV handler = VK_NULL_HANDLE;

                Handler(VkCudaFunctionNV _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkCudaFunctionNV* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkCudaFunctionNV* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_CUDA_FUNCTION_NV; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_CUDA_FUNCTION_NV || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkCudaFunctionNV) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: CudaFunctionNV :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkCudaFunctionNV _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyCudaFunctionNV(_device, handler, _destroy_callback);
                    return 0u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkCudaFunctionNV> handlers;

                VecHandler(std::vector<VkCudaFunctionNV> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkCudaFunctionNV* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkCudaFunctionNV* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkCudaFunctionNV  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_CUDA_FUNCTION_NV; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_CUDA_FUNCTION_NV || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkCudaFunctionNV) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: CudaFunctionNV :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkCudaFunctionNV> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    for (uint32_t i = 0u; i < handlers.size(); ++i)
                        vkDestroyCudaFunctionNV(_device, handlers[i], _destroy_callback);
                    return 0u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_CUDA_FUNCTION_NV; }

                uint32_t destroy(ObjectInfoType _handler, VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyCudaFunctionNV(_device, (VkCudaFunctionNV)_handler.obj_handle, _destroy_callback);
                    return 0u;
                }
            }; // BuilderBase END

        }; // CudaFunctionNV END


        namespace MicromapEXT {

            struct Handler : ObjectHandlerBase {
                VkMicromapEXT handler = VK_NULL_HANDLE;

                Handler(VkMicromapEXT _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkMicromapEXT* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkMicromapEXT* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_MICROMAP_EXT; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_MICROMAP_EXT || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkMicromapEXT) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: MicromapEXT :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkMicromapEXT _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyMicromapEXT(_device, handler, _destroy_callback);
                    return 0u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkMicromapEXT> handlers;

                VecHandler(std::vector<VkMicromapEXT> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkMicromapEXT* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkMicromapEXT* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkMicromapEXT  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_MICROMAP_EXT; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_MICROMAP_EXT || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkMicromapEXT) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: MicromapEXT :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkMicromapEXT> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    for (uint32_t i = 0u; i < handlers.size(); ++i)
                        vkDestroyMicromapEXT(_device, handlers[i], _destroy_callback);
                    return 0u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_MICROMAP_EXT; }

                uint32_t destroy(ObjectInfoType _handler, VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyMicromapEXT(_device, (VkMicromapEXT)_handler.obj_handle, _destroy_callback);
                    return 0u;
                }
            }; // BuilderBase END

        }; // MicromapEXT END


        namespace OpticalFlowSessionNV {

            struct Handler : ObjectHandlerBase {
                VkOpticalFlowSessionNV handler = VK_NULL_HANDLE;

                Handler(VkOpticalFlowSessionNV _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkOpticalFlowSessionNV* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkOpticalFlowSessionNV* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_OPTICAL_FLOW_SESSION_NV; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_OPTICAL_FLOW_SESSION_NV || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkOpticalFlowSessionNV) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: OpticalFlowSessionNV :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkOpticalFlowSessionNV _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyOpticalFlowSessionNV(_device, handler, _destroy_callback);
                    return 0u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkOpticalFlowSessionNV> handlers;

                VecHandler(std::vector<VkOpticalFlowSessionNV> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkOpticalFlowSessionNV* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkOpticalFlowSessionNV* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkOpticalFlowSessionNV  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_OPTICAL_FLOW_SESSION_NV; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_OPTICAL_FLOW_SESSION_NV || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkOpticalFlowSessionNV) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: OpticalFlowSessionNV :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkOpticalFlowSessionNV> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    for (uint32_t i = 0u; i < handlers.size(); ++i)
                        vkDestroyOpticalFlowSessionNV(_device, handlers[i], _destroy_callback);
                    return 0u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_OPTICAL_FLOW_SESSION_NV; }

                uint32_t destroy(ObjectInfoType _handler, VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyOpticalFlowSessionNV(_device, (VkOpticalFlowSessionNV)_handler.obj_handle, _destroy_callback);
                    return 0u;
                }
            }; // BuilderBase END

        }; // OpticalFlowSessionNV END


        namespace ShaderEXT {

            struct Handler : ObjectHandlerBase {
                VkShaderEXT handler = VK_NULL_HANDLE;

                Handler(VkShaderEXT _handler = VK_NULL_HANDLE) : handler{_handler} {}
                Handler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                Handler(const Handler &_handler)
                                : handler{_handler.handler} {}
                Handler(Handler &&_handler)
                                : handler{_handler.handler} {
                    _handler.clear();
                }
                ~Handler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handler; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handler; }
                      VkShaderEXT* handlerPtr(uint32_t _index = 0u)       { return &handler; }
                const VkShaderEXT* handlerPtr(uint32_t _index = 0u) const { return &handler; }
                      uint32_t count() const { return 1; }

                void clear() override { handler = VK_NULL_HANDLE; }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_SHADER_EXT; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_SHADER_EXT || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handler = (VkShaderEXT) _obj_info.obj_handle;
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: ShaderEXT :: Handler", "attempting to set a handler of different type\n"));
                }
                void set(VkShaderEXT _handler) {
                    handler = _handler;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyShaderEXT(_device, handler, _destroy_callback);
                    return 0u;
                }

                Handler& operator=(const Handler &_handler) {
                    handler = _handler.handler;
                    return *this;
                }

                Handler& operator=(Handler &&_handler) {
                    handler = _handler.handler;
                    _handler.clear();
                    return *this;
                }
            }; // Handler END


            struct VecHandler : ObjectHandlerBase {
                std::vector<VkShaderEXT> handlers;

                VecHandler(std::vector<VkShaderEXT> _handlers = {}) : handlers{_handlers} {}
                VecHandler(const ObjectInfoType &_obj_info) {
                    set(_obj_info);
                }
                VecHandler(const Handler &_handler)
                                : handlers{_handler.handler} {}
                VecHandler(Handler &&_handler)
                                : handlers{_handler.handler} {
                    _handler.clear();
                }
                VecHandler(const VecHandler &_handlers)
                                : handlers{_handlers.handlers} {}
                VecHandler(VecHandler &&_handlers)
                                : handlers{_handlers.handlers} {
                    _handlers.clear();
                }
                ~VecHandler() {
                    clear();
                }

                      ObjectCommonType* ptr(uint32_t _index = 0u)       override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType* ptr(uint32_t _index = 0u) const override { return (ObjectCommonType*) &handlers[_index]; }
                const ObjectCommonType  get(uint32_t _index = 0u) const override { return  handlers[_index]; }
                      VkShaderEXT* handlerPtr(uint32_t _index = 0u)       { return &handlers[_index]; }
                const VkShaderEXT* handlerPtr(uint32_t _index = 0u) const { return &handlers[_index]; }
                const VkShaderEXT  handlerGet(uint32_t _index = 0u) const { return  handlers[_index]; }
                      uint32_t count() const override { return handlers.size(); }

                void clear() override {
                    handlers.clear();
                }
                VkObjectType getType() const override { return VK_OBJECT_TYPE_SHADER_EXT; }
                void set(const ObjectInfoType &_obj_info) override {
                    if (_obj_info.obj_type == VK_OBJECT_TYPE_SHADER_EXT || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
                        handlers = {(VkShaderEXT) _obj_info.obj_handle};
                    else
                        fprintf(outswarn, SVKFW_WRAPWARN("VKFW :: ShaderEXT :: VecHandler", "attempting to set a handler of different type\n"));
                }
                void set(std::vector<VkShaderEXT> _handlers) {
                    handlers = _handlers;
                }
                uint32_t destroy(VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    for (uint32_t i = 0u; i < handlers.size(); ++i)
                        vkDestroyShaderEXT(_device, handlers[i], _destroy_callback);
                    return 0u;
                }

                VecHandler& operator=(const VecHandler &_handler) {
                    handlers = _handler.handlers;
                    return *this;
                }

                VecHandler& operator=(VecHandler &&_handler) {
                    handlers = _handler.handlers;
                    _handler.clear();
                    return *this;
                }
            }; // VecHandler END


            struct BuilderBase : ObjectHandlerExBase {
                BuilderBase() {}
                ~BuilderBase() {}

                VkObjectType getType() const override { return VK_OBJECT_TYPE_SHADER_EXT; }

                uint32_t destroy(ObjectInfoType _handler, VkInstance _instance, VkDevice _device,
                                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {
                    vkDestroyShaderEXT(_device, (VkShaderEXT)_handler.obj_handle, _destroy_callback);
                    return 0u;
                }
            }; // BuilderBase END

        }; // ShaderEXT END
    }; // VKFW END
}; // Simple END

#endif