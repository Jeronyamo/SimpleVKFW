#ifndef SVKFW_VKFW_WRAP_H
#define SVKFW_VKFW_WRAP_H

#include "vkfw_handlers.h"

#include <fstream>


namespace Simple {
    namespace VKFW {

//  ============  Vulkan Instance  ============  \\

    namespace Instance {
        void checkInstExtAvailability(const StringVec &_extensions, bool *_res, const char *_layer = nullptr);
        void checkInstLayerAvailability(const StringVec &_layers, bool *_res);
        bool allInstanceExtAvailable(const StringVec &_extensions, const char *_layer = nullptr);
        bool allInstanceLayersAvailable(const StringVec &_layers);
        uint32_t getAllInstanceExtensions(std::vector<VkExtensionProperties> &_ext_list, const char *_layer = nullptr);
        uint32_t getAllInstanceValLayers(std::vector<VkLayerProperties> &_layer_list);


        struct Builder : BuilderBase {
            VkApplicationInfo app_info{VK_STRUCTURE_TYPE_APPLICATION_INFO};
            StringVec layers, extensions;
            VkBool32 volk_load_inst_only = VK_FALSE;


            Builder() {
                app_info.apiVersion = VulkanAPIVersion::available();
            }
            Builder(const Builder &_builder)
                            : app_info{_builder.app_info}, layers{_builder.layers}, extensions{_builder.extensions} {}
            ~Builder() {
                clear();
            }


            void clear() {
                app_info = VkApplicationInfo{};
                app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
                app_info.apiVersion = VulkanAPIVersion::available();

                layers.clear();
                extensions.clear();

                volk_load_inst_only = VK_FALSE;
            }


            ObjectInfoType createInfoObject(void *_p_next = nullptr, VkInstanceCreateFlags _flags = 0u,
                                            const VkAllocationCallbacks * _create_cb = nullptr) {
                return { create(_p_next, _flags, _create_cb), VK_OBJECT_TYPE_INSTANCE };
            }

            ObjectCommonType create(void *_p_next = nullptr, VkInstanceCreateFlags _flags = 0u,
                                    const VkAllocationCallbacks * _create_cb = nullptr) override {
                VkInstanceCreateInfo __info = createInfoStruct(_p_next, _flags);
                VkInstance _instance = VK_NULL_HANDLE;

                if (vkCreateInstance(&__info, _create_cb, &_instance) != VK_SUCCESS)
                    throw std::runtime_error(SVKFW_WRAPERR(" Instance Builder :: create", "could not create instance"));

                if (volk_load_inst_only) {
                     volkLoadInstanceOnly(_instance);
                     printf(SVKFW_WRAPINFO(" Instance Builder :: create", "only instance-based function pointers are loaded. Call volkLoadDevice() after VkDevice creation.\n"));
                }
                else volkLoadInstance(_instance);

                return _instance;
            }

            inline VkInstance createObject(void *_p_next = nullptr, VkInstanceCreateFlags _flags = 0u,
                                             const VkAllocationCallbacks * _create_cb = nullptr) {
                return (VkInstance) create(_p_next, _flags, _create_cb);
            }

            VkInstanceCreateInfo createInfoStruct(void *_p_next = nullptr, VkInstanceCreateFlags _flags = 0u) {
                VkInstanceCreateInfo __info{};
                __info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
                __info.pNext = _p_next;
                __info.flags = _flags;
                __info.pApplicationInfo = &app_info;
                __info.enabledLayerCount = layers.count();
                __info.ppEnabledLayerNames = layers.entries();
                __info.enabledExtensionCount = extensions.count();
                __info.ppEnabledExtensionNames = extensions.entries();

                return __info;
            }

        // Setters

            void setApplicationInfo(const char *_app_name, UVec3 _app_version,
                                    const char * _ng_name, UVec3  _ng_version,
                                    uint32_t _api_version = 0u, void *_p_next = nullptr) {
                app_info.pNext = _p_next; // defaults: 0.0.0, "TESTAPP", "No engine"
                app_info.pApplicationName = _app_name;
                app_info.applicationVersion = VulkanAPIVersion::make(_app_version);
                app_info.pEngineName = _ng_name;
                app_info.engineVersion = VulkanAPIVersion::make(_ng_version);
                app_info.apiVersion = _api_version ? _api_version : VulkanAPIVersion::available();
            }

            void setVolkInitInstanceOnly(bool _inst_only = true) {
                volk_load_inst_only = _inst_only;
            }

            void setLayers(const StringVec &_layers) {
                layers = _layers;
            }

            void setExtensions(const StringVec &_extensions) {
                extensions = _extensions;
            }

        // Additional state modifiers

            void addLayers(const StringVec &_layers) {
                layers.addEntries(_layers.list);
            }

            void addExtensions(const StringVec &_extensions) {
                extensions.addEntries(_extensions.list);
            }

            void addValLayers(const StringVec &_val_layers = {{ "VK_LAYER_KHRONOS_validation" }}) {
                if (!allInstanceLayersAvailable(_val_layers))
                    throw std::runtime_error(SVKFW_WRAPERR(" Instance Builder :: addValLayers","has unavailable layers"));

                addLayers(_val_layers);
            }

            void addGLFWExtensions() { // As Vulkan + GLFW is the main stack for now
                uint32_t _count = 0u;
                const char **_ext = glfwGetRequiredInstanceExtensions(&_count);
                addExtensions({_ext, _count});
            }

            void addDUExtension() {
                if (!allInstanceExtAvailable({{ VK_EXT_DEBUG_UTILS_EXTENSION_NAME }}))
                    throw std::runtime_error(SVKFW_WRAPERR(" Instance Builder :: addDUExtension","debug message callback extension is unavailable"));

                addExtensions({{ VK_EXT_DEBUG_UTILS_EXTENSION_NAME }});
            }
        }; // Builder END


    // Functions definition

        uint32_t getAllInstanceValLayers(std::vector<VkLayerProperties> &_layerList) {
            uint32_t __layerCount;
            vkEnumerateInstanceLayerProperties(&__layerCount, nullptr);

            _layerList.resize(__layerCount);
            vkEnumerateInstanceLayerProperties(&__layerCount, _layerList.data());
            return __layerCount;
        }

        uint32_t getAllInstanceExtensions(std::vector<VkExtensionProperties> &_extList, const char *_layer) {
            uint32_t __extCount;
            vkEnumerateInstanceExtensionProperties(_layer, &__extCount, nullptr);

            _extList.resize(__extCount);
            vkEnumerateInstanceExtensionProperties(_layer, &__extCount, _extList.data());
            return __extCount;
        }

        bool allInstanceLayersAvailable(const StringVec &_layers) {
            uint32_t __layerCount;
            std::vector<VkLayerProperties> _layerList;
            getAllInstanceValLayers(_layerList);

            bool __res = true;
            for (const std::string &layerName : _layers.list) {
                bool layerFound = false;

                for (const VkLayerProperties &layerProperties : _layerList) {
                    if (layerName == layerProperties.layerName) {
                        layerFound = true;
                        break;
                    }
                }

                if (!layerFound) {
                    fprintf(outswarn, "%s %s\n", SVKFW_WRAPWARN("Instance :: allInstanceLayersAvailable","could not find layer"), layerName.c_str());
                    // fflush(outswarn);

                    __res = false;
                }
            }
            return __res;
        }

        bool allInstanceExtAvailable(const StringVec &_extensions, const char *_layer) {
            uint32_t __extCount;
            std::vector<VkExtensionProperties> _extList;
            getAllInstanceExtensions(_extList, _layer);

            bool __res = true;
            for (const std::string &extName : _extensions.list) {
                bool extFound = false;

                for (const VkExtensionProperties &extProperties : _extList) {
                    if (extName == extProperties.extensionName) {
                        extFound = true;
                        break;
                    }
                }

                if (!extFound) {
                    fprintf(outswarn, "%s %s\n", SVKFW_WRAPWARN("Instance :: allInstanceExtAvailable","could not find extension "), extName.c_str());
                    // fflush(outswarn);

                    __res = false;
                }
            }
            return __res;
        }

        // for each layer name in _layers returns if it is available
        // (_res must have at least the same length (in elements) as _layers)
        void checkInstLayerAvailability(const StringVec &_layers, bool *_res) {
            uint32_t __layerCount;
            std::vector<VkLayerProperties> _layerList;
            getAllInstanceValLayers(_layerList);

            for (uint32_t i = 0u; i < _layers.list.size(); ++i) {
                bool __layerFound = false;

                for (const VkLayerProperties &layerProperties : _layerList) {
                    if (_layers.list[i] == layerProperties.layerName) {
                        __layerFound = true;
                        break;
                    }
                }
                _res[i] = __layerFound;
            }
        }

        // for each extension name in _extensions returns if it is available
        // (_res must have at least the same length (in elements) as _extensions)
        void checkInstExtAvailability(const StringVec &_extensions, bool *_res, const char *_layer) {
            uint32_t __extCount;
            std::vector<VkExtensionProperties> __extList;
            getAllInstanceExtensions(__extList, _layer);

            for (uint32_t i = 0u; i < _extensions.list.size(); ++i) {
                bool __extFound = false;

                for (const VkExtensionProperties  &extProperties : __extList) {
                    if (_extensions.list[i] == extProperties.extensionName) {
                        __extFound = true;
                        break;
                    }
                }
                _res[i] = __extFound;
            }
        }
    }; // Instance END


//  ============  Vulkan Debug Messenger Callback  ============  \\

    namespace DebugUtilsMessengerEXT {
        VKAPI_ATTR VkBool32 VKAPI_CALL vkfwDebugMessageCallback(
                                            VkDebugUtilsMessageSeverityFlagBitsEXT _message_severity,
                                            VkDebugUtilsMessageTypeFlagsEXT _message_type,
                                            const VkDebugUtilsMessengerCallbackDataEXT* _callback_data,
                                            void* _user_data);


        struct Builder : BuilderBase {
            VkInstance instance = VK_NULL_HANDLE;
            VkDebugUtilsMessageSeverityFlagsEXT message_severity = 0;
            VkDebugUtilsMessageTypeFlagsEXT message_type = 0;
            PFN_vkDebugUtilsMessengerCallbackEXT user_callback = nullptr;
            void* user_data = nullptr;


            Builder(const Instance::Handler &_instance = {}) : instance{_instance.handler} {
                message_severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT    |
                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
                message_type = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT    |
                               VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
                user_callback = vkfwDebugMessageCallback;
                user_data = nullptr;
            }
            Builder(const Builder &_builder) : instance{_builder.instance}, message_severity{_builder.message_severity},
                                               message_type{_builder.message_type}, user_callback{_builder.user_callback},
                                               user_data{_builder.user_data} {}
            ~Builder() {
                clear();
            }


            void clear() {
                instance = VK_NULL_HANDLE;
                message_severity = 0;
                message_type = 0;
                user_callback = nullptr;
                user_data = nullptr;
            }


            ObjectInfoType createInfoObject(void *_p_next = nullptr, VkDebugUtilsMessengerCreateFlagsEXT _flags = 0u,
                                            const VkAllocationCallbacks * _create_cb = nullptr) {
                return { create(_p_next, _flags, _create_cb), VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT };
            }

            ObjectCommonType create(void *_p_next = nullptr, VkDebugUtilsMessengerCreateFlagsEXT _flags = 0u,
                                    const VkAllocationCallbacks * _create_cb = nullptr) override {
                VkDebugUtilsMessengerCreateInfoEXT __info = createInfoStruct(_p_next, _flags);
                VkDebugUtilsMessengerEXT _messenger = VK_NULL_HANDLE;

                if (vkCreateDebugUtilsMessengerEXT(instance, &__info, _create_cb, &_messenger) != VK_SUCCESS)
                    throw std::runtime_error(SVKFW_WRAPERR(" DebugMessenger Builder :: create","could not create debug messenger"));

                return _messenger;
            }

            inline VkDebugUtilsMessengerEXT createObject(void *_p_next = nullptr, VkDebugUtilsMessengerCreateFlagsEXT _flags = 0u,
                                                 const VkAllocationCallbacks * _create_cb = nullptr) {
                return (VkDebugUtilsMessengerEXT) create(_p_next, _flags, _create_cb);
            }

            VkDebugUtilsMessengerCreateInfoEXT createInfoStruct(void *_p_next = nullptr, VkDebugUtilsMessengerCreateFlagsEXT _flags = 0u) {
                VkDebugUtilsMessengerCreateInfoEXT __info{};
                __info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
                __info.pNext = _p_next;
                __info.flags = _flags;
                __info.messageSeverity = message_severity;
                __info.messageType = message_type;
                __info.pfnUserCallback = user_callback;
                __info.pUserData = user_data;

                return __info;
            }


            void setInstance(const Instance::Handler &_instance) {
                instance = _instance.handler;
            }

            void setInfo(VkDebugUtilsMessageSeverityFlagsEXT _msg_severity,
                         VkDebugUtilsMessageTypeFlagsEXT _msg_type,
                         PFN_vkDebugUtilsMessengerCallbackEXT _user_cb = nullptr,
                         void *_user_data = nullptr) {
                message_severity = _msg_severity;
                message_type = _msg_type;
                user_callback = _user_cb != nullptr ? _user_cb : user_callback;
            }
        }; // Builder END


        VKAPI_ATTR VkBool32 VKAPI_CALL vkfwDebugMessageCallback(
                    VkDebugUtilsMessageSeverityFlagBitsEXT _message_severity,
                    VkDebugUtilsMessageTypeFlagsEXT _message_type,
                    const VkDebugUtilsMessengerCallbackDataEXT* _callback_data,
                    void* _user_data) {
            // Severity
            FILE* __outs = stdout;
            const char *_severity = CSISEQ(SGR_COL_FG_MAGENTA) "Untracked severity message" CSISEQ(SGR_COL_FG_DEFAULT);

            switch (_message_severity) {
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
                    _severity = CSISEQ(SGR_COL_FG_CYAN)   "Verbose" CSISEQ(SGR_COL_FG_DEFAULT); break;
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                    _severity = CSISEQ(SGR_COL_FG_CYAN)   "Info"    CSISEQ(SGR_COL_FG_DEFAULT); break;
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                    __outs = outswarn;
                    _severity = CSISEQ(SGR_COL_FG_YELLOW) "Warning" CSISEQ(SGR_COL_FG_DEFAULT); break;
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                    __outs = stderr;
                    _severity = CSISEQ(SGR_COL_FG_RED)    "Error"   CSISEQ(SGR_COL_FG_DEFAULT); break;
            }

            // Message type
            const char *_msgtype[] = { "General", "Validation", "Performance",
                                       "Device address binding", "Untracked type" };

            bool _typeFlags[] = { bool(_message_type & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT),
                                  bool(_message_type & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT),
                                  bool(_message_type & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT),
                                  bool(_message_type & VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT),
                                       _message_type > VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT };
            bool _multFlags = false;
            std::string _typestr{};
            for (uint32_t i = 0u; i < 5u; ++i) {
                if (_typeFlags[i]) {
                    if (!_typestr.empty()) {
                        _multFlags = true;
                        _typestr += " & ";
                    }
                    _typestr += _msgtype[i];
                }
            }
            if (_multFlags) _typestr = '(' + _typestr + ')';

            // Output
            if (_message_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
                // there are multi-line "Info" messages, adding prefix results in poor readability
                fprintf(__outs, CSISEQ(SGR_EMPH_BOLD) "[ VLayer %s %s]" CSISEQ("") "\n",
                                _typestr.c_str(), _severity);
            fprintf(__outs, "%s\n", _callback_data->pMessage);

            return VK_FALSE;
        }
    }; // DebugMessengerEXT END


//  ============  Vulkan Physical Device  ============  \\

    namespace PhysicalDevice {
        bool allExtAvailable(VkPhysicalDevice _phys_device, const StringVec &_extensions, bool *_flags);
        bool allExtAvailable(VkPhysicalDevice _phys_device, const StringVec &_extensions);
        void getAllDeviceExtensions(VkPhysicalDevice _phys_device, std::vector<VkExtensionProperties> &_extensions, const char *_layerName = nullptr);


        // This type contains all properties (retreivable via vulkan functions)
        // about one physical device - element of Manager::phys_devices vector.
        // It contains references to phys_devices[i], features[i] and properties[i].
        struct InfoElement {
            const VkPhysicalDevice &phys_device;
            const VkPhysicalDeviceFeatures &features;
            const VkPhysicalDeviceProperties &properties;

            InfoElement(const VkPhysicalDevice &_phys_device,
                        const VkPhysicalDeviceFeatures &_features,
                        const VkPhysicalDeviceProperties &_properties)
                            : phys_device{_phys_device}, features{_features}, properties{_properties} {}
            InfoElement(const InfoElement &_elem_view)
                            : phys_device{_elem_view.phys_device}, features{_elem_view.features},
                                properties{_elem_view.properties} {}


            UVec4 apiVersion() {
                UVec4 __ver;

                __ver["xyz"] = VulkanAPIVersion::divide(properties.apiVersion, &__ver.w);
                return __ver;
            }

            std::string infoString() {
                std::string _out = "Physical Device Info:\n";
                UVec4 _ver = apiVersion();
                _out = _out + "Device name: " + properties.deviceName + '\n'
                            + "API version: " + std::to_string(_ver.x) + '.'
                                                + std::to_string(_ver.y) + '.'
                                                + std::to_string(_ver.z) +
                                    " (Variant " + std::to_string(_ver.w) + ")\n";
                return _out;
            }

            void getAllDeviceExtensions(std::vector<VkExtensionProperties> &_extensions, const char *_layerName = nullptr) const {
                return PhysicalDevice::getAllDeviceExtensions(phys_device, _extensions, _layerName);
            }

            bool allExtAvailable(const StringVec &_extensions) const {
                return PhysicalDevice::allExtAvailable(phys_device, _extensions);
            }

            bool allExtAvailable(const StringVec &_extensions, bool *_flags) const {
                return PhysicalDevice::allExtAvailable(phys_device, _extensions, _flags);
            }
        }; // InfoElement END


        namespace Suitability {
            typedef bool (*TestFunc) (const InfoElement &_phys_device_info);
            typedef  int (*RankFunc) (const InfoElement &_phys_device_info);
            // typedef  int (*CompFunc) (const void *_pd1, const void *_pd2);

            // TODO: more useful defaults

            bool defaultGraphicsTest(const InfoElement &_phys_device_info) {
                return _phys_device_info.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
                    _phys_device_info.features.geometryShader && _phys_device_info.allExtAvailable({ {VK_KHR_SWAPCHAIN_EXTENSION_NAME} });
            }
        }; // Suitability END


        struct Manager {
            std::vector<VkPhysicalDevice> phys_devices;
            std::vector<VkPhysicalDeviceFeatures> features;
            std::vector<VkPhysicalDeviceProperties> properties;
            VkInstance instance = VK_NULL_HANDLE;


            Manager(const Instance::Handler &_instance = {}) : instance{_instance.handler} {}
            ~Manager() {
                clearManager();
            }


            void clearManager() {
                features.clear();
                properties.clear();
                phys_devices.clear();
                instance = VK_NULL_HANDLE;
            }


            const InfoElement getElemInfo(uint32_t _index) { return { phys_devices[_index], features[_index], properties[_index] }; }


            void loadInfo() {
                if (instance == VK_NULL_HANDLE)
                    throw std::runtime_error(SVKFW_WRAPERR(" PhysicalDevice Manager :: loadInfo", "instance is not set"));

                uint32_t __devCount = 0;
                vkEnumeratePhysicalDevices(instance, &__devCount, nullptr);

                if (__devCount == 0)
                    throw std::runtime_error(SVKFW_WRAPERR(" PhysicalDevice Manager :: loadInfo", "no GPUs with Vulkan support available"));

                features.resize(__devCount);
                properties.resize(__devCount);
                phys_devices.resize(__devCount);
                vkEnumeratePhysicalDevices(instance, &__devCount, phys_devices.data());

                for (uint32_t i = 0u; i < __devCount; ++i) {
                    vkGetPhysicalDeviceProperties(phys_devices[i], &properties[i]);
                    vkGetPhysicalDeviceFeatures  (phys_devices[i], &  features[i]);
                }
            }


            void setInstance(const Instance::Handler &_instance) {
                instance = _instance.handler;
            }


            // Returns the first physical device for which _testFunc({device, features, properties}) == true
            VkPhysicalDevice testPickOne(const Suitability::TestFunc _testFunc) {
                for (uint32_t i = 0u; i < phys_devices.size(); ++i) {
                    if (_testFunc({phys_devices[i], features[i], properties[i]}))
                        return phys_devices[i];
                }

                throw std::runtime_error(SVKFW_WRAPERR(" PhysicalDevice Manager :: testPickOne", "failed to find a suitable GPU"));
                return VK_NULL_HANDLE;
            }

            // Returns all physical devices for which _testFunc({device, features, properties}) == true
            std::vector<VkPhysicalDevice> testPickAll(const Suitability::TestFunc _testFunc) {
                std::vector<VkPhysicalDevice> __res_devices;
                for (uint32_t i = 0u; i < phys_devices.size(); ++i) {
                    if (_testFunc({phys_devices[i], features[i], properties[i]}))
                        __res_devices.push_back(phys_devices[i]);
                }

                return __res_devices;
            }

            // Ranks all physical devices and returns one with max _rankFunc({device, features, properties})
            VkPhysicalDevice rankPickOne(const Suitability::RankFunc _rankFunc) {
                int __maxRank = 0;
                uint32_t __ind = phys_devices.size();
                for (uint32_t i = 0u; i < phys_devices.size(); ++i) {
                    int __rank = _rankFunc({phys_devices[i], features[i], properties[i]});
                    if (__rank > __maxRank) {
                        __maxRank = __rank;
                        __ind = i;
                    }
                }
                if (__ind >= phys_devices.size())
                    throw std::runtime_error(SVKFW_WRAPERR(" PhysicalDevice Manager :: rankPickOne", "failed to find a suitable GPU"));
                return phys_devices[__ind];
            }

            // Ranks all physical devices and returns one which passes the comparison to _threshold (default: >=)
            template <CompareOp COMP_OP = CompareOp::GE>
            VkPhysicalDevice rankPickOne(const Suitability::RankFunc _rankFunc, int _threshold) {
                for (uint32_t i = 0u; i < phys_devices.size(); ++i) {
                    switch (COMP_OP) {
                        case CompareOp::NE: {
                            if (_rankFunc({phys_devices[i], features[i], properties[i]}) != _threshold)
                                return phys_devices[i];
                            break;
                        }
                        case CompareOp::EQ: {
                            if (_rankFunc({phys_devices[i], features[i], properties[i]}) == _threshold)
                                return phys_devices[i];
                            break;
                        }
                        case CompareOp::LT: {
                            if (_rankFunc({phys_devices[i], features[i], properties[i]}) <  _threshold)
                                return phys_devices[i];
                            break;
                        }
                        case CompareOp::LE: {
                            if (_rankFunc({phys_devices[i], features[i], properties[i]}) <= _threshold)
                                return phys_devices[i];
                            break;
                        }
                        case CompareOp::GE: {
                            if (_rankFunc({phys_devices[i], features[i], properties[i]}) >= _threshold)
                                return phys_devices[i];
                            break;
                        }
                        case CompareOp::GT: {
                            if (_rankFunc({phys_devices[i], features[i], properties[i]}) >  _threshold)
                                return phys_devices[i];
                            break;
                        }
                    }
                }
                throw std::runtime_error(SVKFW_WRAPERR(" PhysicalDevice Manager :: rankPickOne (with threshold)", "failed to find a suitable GPU"));
                return VK_NULL_HANDLE; // TODO: Maybe return first GPU from the list
            }

            // Ranks all physical devices and returns all which pass the comparison to _threshold (default: >=)
            template <CompareOp COMP_OP = CompareOp::GE>
            std::vector<VkPhysicalDevice> rankPickAll(const Suitability::RankFunc _rankFunc, int _threshold) {
                std::vector<VkPhysicalDevice> __res_devices;

                for (uint32_t i = 0u; i < phys_devices.size(); ++i) {
                    switch (COMP_OP) {
                        case CompareOp::NE: {
                            if (_rankFunc({phys_devices[i], features[i], properties[i]}) != _threshold)
                                __res_devices.push_back(phys_devices[i]);
                            break;
                        }
                        case CompareOp::EQ: {
                            if (_rankFunc({phys_devices[i], features[i], properties[i]}) == _threshold)
                                __res_devices.push_back(phys_devices[i]);
                            break;
                        }
                        case CompareOp::LT: {
                            if (_rankFunc({phys_devices[i], features[i], properties[i]}) <  _threshold)
                                __res_devices.push_back(phys_devices[i]);
                            break;
                        }
                        case CompareOp::LE: {
                            if (_rankFunc({phys_devices[i], features[i], properties[i]}) <= _threshold)
                                __res_devices.push_back(phys_devices[i]);
                            break;
                        }
                        case CompareOp::GE: {
                            if (_rankFunc({phys_devices[i], features[i], properties[i]}) >= _threshold)
                                __res_devices.push_back(phys_devices[i]);
                            break;
                        }
                        case CompareOp::GT: {
                            if (_rankFunc({phys_devices[i], features[i], properties[i]}) >  _threshold)
                                __res_devices.push_back(phys_devices[i]);
                            break;
                        }
                    }
                }

                return __res_devices;
            }

            // Ranks all physical devices and returns all which pass the comparison to _threshold (default: >=)
            // Adds ranks of selected physical devices to _ranks.
            template <CompareOp COMP_OP = CompareOp::GE>
            std::vector<VkPhysicalDevice> rankPickAll(const Suitability::RankFunc _rankFunc, std::vector<int> &_ranks, int _threshold) {
                std::vector<VkPhysicalDevice> __res_devices;

                for (uint32_t i = 0u; i < phys_devices.size(); ++i) {
                    int __rank = _rankFunc({phys_devices[i], features[i], properties[i]});
                    switch (COMP_OP) {
                        case CompareOp::NE: {
                            if (__rank != _threshold)
                                _ranks.push_back(__rank);
                                __res_devices.push_back(phys_devices[i]);
                            break;
                        }
                        case CompareOp::EQ: {
                            if (__rank == _threshold)
                                _ranks.push_back(__rank);
                                __res_devices.push_back(phys_devices[i]);
                            break;
                        }
                        case CompareOp::LT: {
                            if (__rank <  _threshold)
                                _ranks.push_back(__rank);
                                __res_devices.push_back(phys_devices[i]);
                            break;
                        }
                        case CompareOp::LE: {
                            if (__rank <= _threshold)
                                _ranks.push_back(__rank);
                                __res_devices.push_back(phys_devices[i]);
                            break;
                        }
                        case CompareOp::GE: {
                            if (__rank >= _threshold)
                                _ranks.push_back(__rank);
                                __res_devices.push_back(phys_devices[i]);
                            break;
                        }
                        case CompareOp::GT: {
                            if (__rank >  _threshold)
                                _ranks.push_back(__rank);
                                __res_devices.push_back(phys_devices[i]);
                            break;
                        }
                    }
                }

                return __res_devices;
            }
        }; // Manager END


        void getAllDeviceExtensions(VkPhysicalDevice _phys_device, std::vector<VkExtensionProperties> &_extensions, const char *_layerName) {
            uint32_t __extCount = 0;
            vkEnumerateDeviceExtensionProperties(_phys_device, _layerName, &__extCount, nullptr);
            _extensions.resize(__extCount);
            vkEnumerateDeviceExtensionProperties(_phys_device, _layerName, &__extCount, _extensions.data());
        }

        bool allExtAvailable(VkPhysicalDevice _phys_device, const StringVec &_extensions) {
            std::vector<VkExtensionProperties> __exts;
            getAllDeviceExtensions(_phys_device, __exts);

            for (uint32_t i = 0u; i < _extensions.list.size(); ++i) {
                bool __is_available = false;
                for (const VkExtensionProperties &ext : __exts)
                    if (_extensions.list[i] == ext.extensionName) {
                        __is_available = true;
                        break;
                    }
                if (!__is_available)  return false;
            }
            return true;
        }

        bool allExtAvailable(VkPhysicalDevice _phys_device, const StringVec &_extensions, bool *_flags) {
            std::vector<VkExtensionProperties> __exts;
            getAllDeviceExtensions(_phys_device, __exts);

            bool __res = true;
            for (uint32_t i = 0u; i < _extensions.list.size(); ++i) {
                bool __is_available = false;
                for (const VkExtensionProperties &ext : __exts)
                    if (_extensions.list[i] == ext.extensionName) {
                        __is_available = true;
                        break;
                    }
                if (_flags != nullptr) _flags[i] = __is_available;
                if (!__is_available)  __res = false;
            }
            return __res;
        }
    }; // PhysicalDevice END


//  ============  Vulkan Queue Family  ============  \\

    namespace QueueFamily {
        // TODO: HandlerLike, actual object handlers have pointer type,
        //       but uint32_t is used for QF indices in Vulkan
        struct Handler {
            uint32_t index = -1;

            Handler() {}
            Handler(uint32_t _index)
                            : index{_index} {}
            Handler(const Handler &_handler)
                            : index{_handler.index} {}
            Handler(Handler &&_handler)
                            : index{_handler.index} {
                _handler.clear();
            }
            virtual ~Handler() {
                clear();
            }


            Handler& operator=(const Handler &_handler) {
                index = _handler.index;
                return *this;
            }

            Handler& operator=(Handler &&_handler) {
                index = _handler.index;
                _handler.clear();
                return *this;
            }

            const uint32_t* cptr() const { return &index; }

            void clear() {
                index = -1;
            }
        }; // Handler END


        struct VecHandler {
            std::vector<uint32_t> handlers;

            VecHandler() {}
            VecHandler(const std::vector<uint32_t> &_handlers)
                            : handlers{_handlers} {}
            VecHandler(const Handler &_handler)
                            : handlers{_handler.index} {}
            VecHandler(Handler &&_handler)
                            : handlers{_handler.index} {
                _handler.clear();
            }
            VecHandler(const VecHandler &_handler)
                            : handlers{_handler.handlers} {}
            VecHandler(VecHandler &&_handler)
                            : handlers{_handler.handlers} {
                _handler.clear();
            }
            virtual ~VecHandler() {
                clear();
            }


            VecHandler& operator=(const Handler &_handler) {
                handlers = { _handler.index };

                return *this;
            }

            VecHandler& operator=(Handler &&_handler) {
                handlers = { _handler.index };

                _handler.clear();
                return *this;
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


            uint32_t operator[](uint32_t _index) const { return handlers[_index]; }
            Handler getHandler(uint32_t _index) const { return { handlers[_index] }; }


        // Pointer and count:

            const uint32_t* ptr(uint32_t _index = 0u) const { return handlers.data() + _index; }
            uint32_t count() const { return handlers.size(); }


        // Clear state:

            void clear() {
                handlers.clear();
            }
        }; // VecHandler END

        struct InfoElement {
            const VkQueueFamilyProperties &properties;

            InfoElement(const VkQueueFamilyProperties &_props) : properties{_props} {}

            bool supportsGraphics()        const { return properties.queueFlags &  VK_QUEUE_GRAPHICS_BIT;         }
            bool supportsCompute()         const { return properties.queueFlags &  VK_QUEUE_COMPUTE_BIT;          }
            bool supportsTransfer()        const { return properties.queueFlags & (VK_QUEUE_GRAPHICS_BIT |
                                                                                   VK_QUEUE_COMPUTE_BIT  |
                                                                                   VK_QUEUE_TRANSFER_BIT);        }
            bool supportsSparseBinding()   const { return properties.queueFlags &  VK_QUEUE_SPARSE_BINDING_BIT;   }
            bool supportsProtectedMemory() const { return properties.queueFlags &  VK_QUEUE_PROTECTED_BIT;        }
            bool supportsVideoDecode()     const { return properties.queueFlags &  VK_QUEUE_VIDEO_DECODE_BIT_KHR; }
            bool supportsVideoEncode()     const { return properties.queueFlags &  VK_QUEUE_VIDEO_ENCODE_BIT_KHR; }
            bool supportsOpticalFlow()     const { return properties.queueFlags &  VK_QUEUE_OPTICAL_FLOW_BIT_NV;  }

            uint32_t queuesNum()                const { return   properties.queueCount; }
            uint32_t timestampValidRangeBits()  const { return   properties.timestampValidBits; }
            UVec3 minImageTransferGranularity() const { return { properties.minImageTransferGranularity.width,
                                                                 properties.minImageTransferGranularity.height,
                                                                 properties.minImageTransferGranularity.depth };  }

            std::string infoString() const {
                std::string __res{"Queue family info:\n"};
                UVec3 __gran{minImageTransferGranularity()};
                return __res +
                        "supports graphics:          " + (supportsGraphics()        ? "yes" : "no") + '\n' +
                        "supports compute:           " + (supportsCompute()         ? "yes" : "no") + '\n' +
                        "supports transfer:          " + (supportsTransfer()        ? "yes" : "no") + '\n' +
                        "supports sparse binding:    " + (supportsSparseBinding()   ? "yes" : "no") + '\n' +
                        "supports protected memory:  " + (supportsProtectedMemory() ? "yes" : "no") + '\n' +
                        "supports video decode:      " + (supportsVideoDecode()     ? "yes" : "no") + '\n' +
                        "supports video encode:      " + (supportsVideoEncode()     ? "yes" : "no") + '\n' +
                        "supports optical flow:      " + (supportsOpticalFlow()     ? "yes" : "no") + '\n' +
                        "number of queues:           " + std::to_string(queuesNum())                + '\n' +
                        "timestamp valid range bits: " + std::to_string(timestampValidRangeBits())  + '\n' +
                        "minimal granularity:       (" + std::to_string(__gran.x) + ", "
                                                       + std::to_string(__gran.y) + ", "
                                                       + std::to_string(__gran.z) + ")\n";
            }
        }; // InfoElement END


        namespace Suitability {
            typedef bool (*TestFunc) (const InfoElement &_info, bool _surf_support);
            typedef  int (*RankFunc) (const InfoElement &_info, bool _surf_support);

            bool testGraphics(const InfoElement &_info, bool _surf_support)
                { return _info.supportsGraphics(); }
            bool testCompute (const InfoElement &_info, bool _surf_support)
                { return _info.supportsCompute (); }
            bool testTransfer(const InfoElement &_info, bool _surf_support)
                { return _info.supportsTransfer(); }
            bool testSparseBinding(const InfoElement &_info, bool _surf_support)
                { return _info.supportsSparseBinding(); }
            bool testProtectedMemory(const InfoElement &_info, bool _surf_support)
                { return _info.supportsProtectedMemory(); }
            bool testVideoDecode(const InfoElement &_info, bool _surf_support)
                { return _info.supportsVideoDecode(); }
            bool testVideoEncode(const InfoElement &_info, bool _surf_support)
                { return _info.supportsVideoEncode(); }
            bool testOpticalFlow(const InfoElement &_info, bool _surf_support)
                { return _info.supportsOpticalFlow(); }

            bool testSurfAndGraphics(const InfoElement &_info, bool _surf_support)
                { return _surf_support && _info.supportsGraphics(); }
            bool testSurfAndCompute (const InfoElement &_info, bool _surf_support)
                { return _surf_support && _info.supportsCompute (); }
            bool testSurfAndTransfer(const InfoElement &_info, bool _surf_support)
                { return _surf_support && _info.supportsTransfer(); }
            bool testSurfAndSparseBinding(const InfoElement &_info, bool _surf_support)
                { return _surf_support && _info.supportsSparseBinding(); }
            bool testSurfAndProtectedMemory(const InfoElement &_info, bool _surf_support)
                { return _surf_support && _info.supportsProtectedMemory(); }
            bool testSurfAndVideoDecode(const InfoElement &_info, bool _surf_support)
                { return _surf_support && _info.supportsVideoDecode(); }
            bool testSurfAndVideoEncode(const InfoElement &_info, bool _surf_support)
                { return _surf_support && _info.supportsVideoEncode(); }
            bool testSurfAndOpticalFlow(const InfoElement &_info, bool _surf_support)
                { return _surf_support && _info.supportsOpticalFlow(); }


        // TODO: practice, then redo these rank functions
        // For now, result >0 means score; if <=0, then queue family doesn't have required feature.

            int rankGraphics(const InfoElement &_info, bool _surf_support) {
                return 5 * _info.supportsGraphics()
                         - _info.supportsCompute()
                         - _info.supportsVideoDecode()
                         - _info.supportsVideoEncode()
                         - _info.supportsOpticalFlow();
            }
            int rankCompute (const InfoElement &_info, bool _surf_support) {
                return 5 * _info.supportsCompute()
                         - _info.supportsGraphics()
                         - _info.supportsVideoDecode()
                         - _info.supportsVideoEncode()
                         - _info.supportsOpticalFlow();
            }
            int rankTransfer(const InfoElement &_info, bool _surf_support) {
                return 6 * _info.supportsTransfer()
                         - _info.supportsCompute()
                         - _info.supportsGraphics()
                         - _info.supportsVideoDecode()
                         - _info.supportsVideoEncode()
                         - _info.supportsOpticalFlow();
            }
            int rankSparseBinding  (const InfoElement &_info, bool _surf_support) {
                return 6 * _info.supportsSparseBinding()
                         - _info.supportsCompute()
                         - _info.supportsGraphics()
                         - _info.supportsVideoDecode()
                         - _info.supportsVideoEncode()
                         - _info.supportsOpticalFlow();
            }
            int rankProtectedMemory(const InfoElement &_info, bool _surf_support) {
                return _info.supportsProtectedMemory() ? 1 : -1;
            }
            int rankVideoDecode(const InfoElement &_info, bool _surf_support) {
                return 5 * _info.supportsVideoDecode()
                         - _info.supportsGraphics()
                         - _info.supportsCompute()
                         - _info.supportsVideoEncode()
                         - _info.supportsOpticalFlow();
            }
            int rankVideoEncode(const InfoElement &_info, bool _surf_support) {
                return 5 * _info.supportsVideoEncode()
                         - _info.supportsGraphics()
                         - _info.supportsCompute()
                         - _info.supportsVideoDecode()
                         - _info.supportsOpticalFlow();
            }
            int rankOpticalFlow(const InfoElement &_info, bool _surf_support) {
                return 5 * _info.supportsOpticalFlow()
                         - _info.supportsGraphics()
                         - _info.supportsCompute()
                         - _info.supportsVideoDecode()
                         - _info.supportsVideoEncode();
            }
        }; // Suitability END


        struct Manager {
        // Data:

            VkInstance instance = VK_NULL_HANDLE;
            VkPhysicalDevice phys_device = VK_NULL_HANDLE;
            std::vector<VkQueueFamilyProperties> properties;
            std::vector<bool> surface_support;


        // Constructors:

            Manager(const Instance::Handler  &_instance = {},
                    const PhysicalDevice::Handler &_phys_device = {})
                            : instance{_instance.handler}, phys_device{_phys_device.handler} {}
            ~Manager() {
                clearManager();
            }


        // Access element info by index:

            const InfoElement getElemInfo(uint32_t _index) const { return { properties[_index] }; }


        // Clear state:

            void clearManager() {
                instance = VK_NULL_HANDLE;
                phys_device = VK_NULL_HANDLE;
                properties.clear();
                surface_support.clear();
            }


        // Load properties:

            void loadInfo() {
                if (instance == VK_NULL_HANDLE)
                    throw std::runtime_error(SVKFW_WRAPERR(" Queue Family Manager :: loadInfo", "instance is not set"));
                if (phys_device == VK_NULL_HANDLE)
                    throw std::runtime_error(SVKFW_WRAPERR(" Queue Family Manager :: loadInfo", "physical device is not set"));

                uint32_t __qfamily_count = 0;
                vkGetPhysicalDeviceQueueFamilyProperties(phys_device, &__qfamily_count, nullptr);

                properties.resize(__qfamily_count);
                vkGetPhysicalDeviceQueueFamilyProperties(phys_device, &__qfamily_count, properties.data());

                surface_support.resize(properties.size());
                for (uint32_t i = 0u; i < properties.size(); ++i)
                    surface_support[i] = glfwGetPhysicalDevicePresentationSupport(instance, phys_device, i);
            }


        // Methods to select elements:

            Handler testPickOne(Suitability::TestFunc _test_func) {
                for (uint32_t i = 0u; i < properties.size(); ++i)
                    if (_test_func(properties[i], surface_support[i])) return { i };

                fprintf(outswarn, SVKFW_WRAPWARN(" Queue Family Manager :: testPickOne", "could not find queue family"));
                return {uint32_t(-1)};
            }

            VecHandler testPickAll(Suitability::TestFunc _test_func) {
                std::vector<uint32_t> __indices;

                for (uint32_t i = 0u; i < properties.size(); ++i)
                    if (_test_func(properties[i], surface_support[i]))
                        __indices.push_back(i);

                if (__indices.empty())
                    fprintf(outswarn, SVKFW_WRAPWARN(" Queue Family Manager :: testPickAll", "could not find queue family"));
                return { __indices };
            }

            Handler rankPickOne(Suitability::RankFunc _rank_func) {
                uint32_t __maxI = properties.size();
                int  __maxR = INT_MIN;

                for (uint32_t i = 0u; i < properties.size(); ++i) {
                    int __rank = _rank_func(properties[i], surface_support[i]);
                    if (__rank > __maxR) {
                        __maxI = i;
                        __maxR = __rank;
                    }
                }

                if (__maxI >= properties.size()) {
                    fprintf(outswarn, SVKFW_WRAPWARN(" Queue Family Manager :: rankPickOne", "could not find queue family"));
                    return {uint32_t(-1)};
                }
                return { __maxI };
            }

            template <CompareOp COMP_OP = CompareOp::GE>
            VecHandler rankPickAll(Suitability::RankFunc _rank_func, int _threshold) {
                std::vector<uint32_t> __indices;

                for (uint32_t i = 0u; i < properties.size(); ++i) {
                    int __rank = _rank_func(properties[i], surface_support[i]);
                    switch (COMP_OP) {
                        case CompareOp::NE: {
                            if (__rank != _threshold)
                                __indices.push_back(i);
                            break;
                        }
                        case CompareOp::EQ: {
                            if (__rank == _threshold)
                                __indices.push_back(i);
                            break;
                        }
                        case CompareOp::LT: {
                            if (__rank < _threshold)
                                __indices.push_back(i);
                            break;
                        }
                        case CompareOp::LE: {
                            if (__rank <= _threshold)
                                __indices.push_back(i);
                            break;
                        }
                        case CompareOp::GE: {
                            if (__rank >= _threshold)
                                __indices.push_back(i);
                            break;
                        }
                        case CompareOp::GT: {
                            if (__rank > _threshold)
                                __indices.push_back(i);
                            break;
                        }
                    }
                }

                if (__indices.empty())
                    fprintf(outswarn, SVKFW_WRAPWARN(" Queue Family Manager :: rankPickAll", "could not find queue family"));
                return { __indices };
            }
        }; // Manager END

        // TODO
        struct Builder {
            // VkDeviceQueueCreateInfo info;
        }; // Builder END
    }; // QueueFamily END

    namespace Queue {
        struct Builder : BuilderBase {
        // Data:

            VkDevice device = VK_NULL_HANDLE;
            uint32_t queue_family = -1u;


        // Constructors:

            Builder(const Device::Handler &_device = {},
                    const QueueFamily::Handler &_queue_family = {})
                            : device{_device.handler}, queue_family{_queue_family.index} {}
            ~Builder() {
                clear();
            }


        // Clear state:

            void clear() {
                device = VK_NULL_HANDLE;
                queue_family = 0u;
            }


        // Create handlers/CreateInfo struct:

            ObjectCommonType create(void *_p_next = nullptr, VkDebugUtilsMessengerCreateFlagsEXT _flags = 0u,
                                    const VkAllocationCallbacks * _create_cb = nullptr) override {
                VkQueue __queue = VK_NULL_HANDLE;
                uint32_t __queue_index = _flags;
                vkGetDeviceQueue(device, queue_family, __queue_index, &__queue);

                return __queue;
            }

            VkQueue createObject(uint32_t _queue_index) {
                VkQueue __queue = VK_NULL_HANDLE;
                vkGetDeviceQueue(device, queue_family, _queue_index, &__queue);

                return __queue;
            }


        // Setters:

            void setDevice(const Device::Handler &_device) {
                device = _device.handler;
            }

            void setQueueFamily(const QueueFamily::Handler &_queue_family) {
                queue_family = _queue_family.index;
            }
        };
    }; // Queue END


//  ============  Vulkan Logical Device  ============  \\

    namespace Device {
    // Functions declaration

        void checkDeviceExtAvailability(const PhysicalDevice::Handler &_phys_device, const StringVec &_extensions, bool *_res, const char *_layer = nullptr);
        void checkDeviceLayerAvailability(const PhysicalDevice::Handler &_phys_device, const StringVec &_layers, bool *_res);
        bool allDeviceExtAvailable(const PhysicalDevice::Handler &_phys_device, const StringVec &_extensions, const char *_layer = nullptr);
        bool allDeviceLayersAvailable(const PhysicalDevice::Handler &_phys_device, const StringVec &_layers);
        uint32_t getAllDeviceExtensions(const PhysicalDevice::Handler &_phys_device, std::vector<VkExtensionProperties> &_ext_list, const char *_layer = nullptr);
        uint32_t getAllDeviceValLayers(const PhysicalDevice::Handler &_phys_device, std::vector<VkLayerProperties> &_layer_list);
        VkQueue getDeviceQueueHandle(const Device::Handler &_device, const QueueFamily::Handler &_queue_family, uint32_t _queue_index);
        std::vector<VkQueue> getAllDeviceQueueHandles(const Device::Handler &_device, const QueueFamily::Handler &_queue_family, uint32_t _queue_index_offset, uint32_t _queue_index_count);


    // Classes definition

        struct Builder : BuilderBase {
            VkPhysicalDevice phys_device = VK_NULL_HANDLE;
            VkPhysicalDeviceFeatures *req_features = nullptr;
            std::vector<VkDeviceQueueCreateInfo> queue_info;
            StringVec layers, extensions;


            Builder(PhysicalDevice::Handler _phys_device = {}) : phys_device{_phys_device.handler} {}
            Builder(const Builder &_builder)
                            : phys_device{_builder.phys_device}, req_features{_builder.req_features},
                              queue_info{_builder.queue_info}, layers{_builder.layers}, extensions{_builder.extensions} {
                for (uint32_t i = 0u; i < queue_info.size(); ++i) {
                    float *__ptr = new float[queue_info[i].queueCount];
                    memcpy(__ptr, queue_info[i].pQueuePriorities, queue_info[i].queueCount * sizeof(float));
                    queue_info[i].pQueuePriorities = __ptr;
                }
                if (_builder.req_features != nullptr) {
                    req_features = new VkPhysicalDeviceFeatures{*_builder.req_features};
                }
            }
            ~Builder() {
                clear();
            }


        // Clear state:

            void clear() {
                phys_device = VK_NULL_HANDLE;

                if (req_features != nullptr)
                    delete req_features;
                req_features = nullptr;

                for (uint32_t i = 0u; i < queue_info.size(); ++i)
                    delete[] queue_info[i].pQueuePriorities;
                queue_info.clear();

                layers.clear();
                extensions.clear();
            }


            ObjectInfoType createInfoObject(void *_p_next = nullptr, VkDeviceCreateFlags _flags = 0u,
                                            const VkAllocationCallbacks * _create_cb = nullptr) {
                return { create(_p_next, _flags, _create_cb), VK_OBJECT_TYPE_DEVICE };
            }

            ObjectCommonType create(void *_p_next = nullptr, VkDeviceCreateFlags _flags = 0u,
                          const VkAllocationCallbacks * _create_cb = nullptr) override {
                if (phys_device == VK_NULL_HANDLE)
                    throw std::logic_error(SVKFW_WRAPERR(" Device Builder :: create", "physical device handle was not set"));

                VkDeviceCreateInfo __info = createInfoStruct(_p_next, _flags);
                VkDevice __device = VK_NULL_HANDLE;

                if (vkCreateDevice(phys_device, &__info, _create_cb, &__device) != VK_SUCCESS)
                    throw std::runtime_error(SVKFW_WRAPERR(" Device Builder :: create","could not create device"));
                return __device;
            }

            inline VkDevice createObject(void *_p_next = nullptr, VkDeviceCreateFlags _flags = 0u,
                          const VkAllocationCallbacks * _create_cb = nullptr) {
                return (VkDevice) create(_p_next, _flags, _create_cb);
            }

            VkDeviceCreateInfo createInfoStruct(void *_p_next = nullptr, VkDeviceCreateFlags _flags = 0u) {
                VkDeviceCreateInfo __info;
                __info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
                __info.flags = _flags;
                __info.pNext = _p_next;
                __info.enabledLayerCount = layers.count();
                __info.ppEnabledLayerNames = layers.entries();
                __info.enabledExtensionCount = extensions.count();
                __info.ppEnabledExtensionNames = extensions.entries();
                __info.pQueueCreateInfos = queue_info.data();
                __info.queueCreateInfoCount = queue_info.size();
                __info.pEnabledFeatures = req_features;

                return __info;
            }


            void setPhysicalDevice(PhysicalDevice::Handler _phys_device) {
                phys_device = _phys_device.handler;
            }

            void setRequiredFeatures(const VkPhysicalDeviceFeatures &_features) {
                if (req_features == nullptr)
                    req_features = new VkPhysicalDeviceFeatures;
                *req_features = _features;
            }

            void setLayers(const StringVec &_layers) {
                layers = _layers;
            }

            void setExtensions(const StringVec &_extensions) {
                extensions = _extensions;
            }

            // TODO: setVecQueueInfo() ?


        // Additional methods:

            // _queueCount can be either 0 (all available queues are used) or [1, family queueCount].
            // if _priorities.size() < _queueCount:
            //     priorities [_priorities.size(), _queueCount) are assumed to be 1.f
            // if _priorities.size() > _queueCount:
            //     priorities [_queueCount, _priorities.size()) are ignored
            void addQueueInfo(const VkQueueFamilyProperties &_properties, QueueFamily::Handler _family,
                              uint32_t _queue_count, const std::vector<float> &_priorities = {1.f},
                              VkDeviceQueueCreateFlags _queue_create_flags = 0u) {
                if (_queue_count == 0u)
                    _queue_count = _properties.queueCount;
                if (_queue_count > _properties.queueCount)
                    throw std::invalid_argument(SVKFW_WRAPERR(" Device Builder :: addQueueInfo","_queue_count exceeds the supported number of queues for queue family"));
                if ( (_queue_create_flags & VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT) &&
                    !(_properties.queueFlags & VK_QUEUE_PROTECTED_BIT))
                    throw std::invalid_argument(SVKFW_WRAPERR(" Device Builder :: addQueueInfo","setting protected memory bit when the feature is disabled"));


                queue_info.emplace_back();
                VkDeviceQueueCreateInfo &__newinfo = queue_info.back();

                __newinfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                __newinfo.pNext = nullptr;
                __newinfo.flags = 0;
                __newinfo.flags = _queue_create_flags;
                __newinfo.queueFamilyIndex = _family.index;
                __newinfo.queueCount = _queue_count;

                float *__ptr = new float[_queue_count];
                for (uint32_t i = 0u; i < _queue_count; ++i) {
                    if (i >= _priorities.size())
                        __ptr[i] = 1.f;
                    else {
                        __ptr[i] = Simple::Func::clamp_GE_LE(_priorities[i], 0.f, 1.f);
                        if (__ptr[i] != _priorities[i])
                            fprintf(outswarn, "%s: %f\n", SVKFW_WRAPWARN(" Device Builder :: addQueueInfo","unnormalized priority value"), _priorities[i]);
                    }
                }
                __newinfo.pQueuePriorities = __ptr;
            }

            void addVecQueueInfo(const std::vector<VkQueueFamilyProperties> &_properties, QueueFamily::VecHandler _families,
                                 const std::vector<uint32_t> &_queue_count, const std::vector<std::vector<float>> &_priorities = {{1.f}},
                                 const std::vector<VkDeviceQueueCreateFlags> &_queue_create_flags = { 0u }) {
                if (_properties.size() < _families.count())
                    throw std::invalid_argument(SVKFW_WRAPERR(" Device Builder :: addVecQueueInfo (1)","length of VkQueueFamilyProperties vector is less than queue families count"));

                for (uint32_t i = 0u; i < _families.count(); ++i) {
                    VkDeviceQueueCreateFlags __q_create_flags = 0u;
                    if (i < _queue_create_flags.size())
                        __q_create_flags = _queue_create_flags[i];

                    addQueueInfo(_properties[i], _families.getHandler(i), _queue_count[i],
                                 (i <_priorities.size() ? _priorities[i] : std::vector<float>{1.f}), __q_create_flags);
                }
            }

            void addVecQueueInfo(const QueueFamily::Manager &_qf_manager, QueueFamily::VecHandler _families,
                                 const std::vector<uint32_t> &_queue_count, const std::vector<std::vector<float>> &_priorities = {{1.f}},
                                 const std::vector<VkDeviceQueueCreateFlags> &_queue_create_flags = { 0u }) {
                for (uint32_t i = 0u; i < _families.count(); ++i) {
                    VkDeviceQueueCreateFlags __q_create_flags = 0u;
                    if (i < _queue_create_flags.size())
                        __q_create_flags = _queue_create_flags[i];

                    if (_qf_manager.properties.size() < _families[i])
                        throw std::invalid_argument(SVKFW_WRAPERR(" Device Builder :: addVecQueueInfo (2)",
                                                        "number of VkQueueFamilyProperties in _qf_manager is less than queue family index"));

                    addQueueInfo(_qf_manager.properties[_families[i]], _families.getHandler(i), _queue_count[i],
                                 (i <_priorities.size() ? _priorities[i] : std::vector<float>{1.f}), __q_create_flags);
                }
            }

            void deleteQueueInfo(uint32_t _index) {
                delete[] queue_info[_index].pQueuePriorities;
                queue_info.erase(queue_info.begin() + _index);
            }

            void addValLayers(const StringVec &_val_layers = {{ "VK_LAYER_KHRONOS_validation" }}) {
                if (phys_device == VK_NULL_HANDLE)
                    throw std::logic_error(SVKFW_WRAPERR(" Device Builder :: addValLayers", "physical device handle was not set"));
                if (!allDeviceLayersAvailable(phys_device, _val_layers))
                    throw std::runtime_error(SVKFW_WRAPERR(" Device Builder :: addValLayers","has unavailable device layers"));

                addLayers(_val_layers);
            }

            void addLayers(const StringVec &_layers) {
                layers.addEntries(_layers.list);
            }

            void addExtensions(const StringVec &_extensions) {
                extensions.addEntries(_extensions.list);
            }
        }; // Builder END


    // Functions definition

        uint32_t getAllDeviceValLayers(const PhysicalDevice::Handler &_phys_device, std::vector<VkLayerProperties> &_layer_list) {
            uint32_t __layer_count;
            vkEnumerateDeviceLayerProperties(_phys_device.handler, &__layer_count, nullptr);

            _layer_list.resize(__layer_count);
            vkEnumerateDeviceLayerProperties(_phys_device.handler, &__layer_count, _layer_list.data());
            return __layer_count;
        }

        uint32_t getAllDeviceExtensions(const PhysicalDevice::Handler &_phys_device, std::vector<VkExtensionProperties> &_ext_list, const char *_layer) {
            uint32_t __ext_count;
            vkEnumerateDeviceExtensionProperties(_phys_device.handler, _layer, &__ext_count, nullptr);

            _ext_list.resize(__ext_count);
            vkEnumerateDeviceExtensionProperties(_phys_device.handler, _layer, &__ext_count, _ext_list.data());
            return __ext_count;
        }


        bool allDeviceLayersAvailable(const PhysicalDevice::Handler &_phys_device, const StringVec &_layers) {
            std::vector<VkLayerProperties> __layer_list;
            getAllDeviceValLayers(_phys_device, __layer_list);

            bool __res = true;
            for (const std::string &layer_name : _layers.list) {
                bool __layer_found = false;

                for (const VkLayerProperties& layerProperties : __layer_list) {
                    if (layer_name == layerProperties.layerName) {
                        __layer_found = true;
                        break;
                    }
                }

                if (!__layer_found) {
                    fprintf(outswarn, "%s %s\n", SVKFW_WRAPWARN("Device :: allDeviceLayersAvailable","could not find layer"), layer_name.c_str());
                    // fflush(outswarn);

                    __res = false;
                }
            }
            return __res;
        }

        bool allDeviceExtAvailable(const PhysicalDevice::Handler &_phys_device, const StringVec &_extensions, const char *_layer) {
            std::vector<VkExtensionProperties> _ext_list;
            getAllDeviceExtensions(_phys_device, _ext_list, _layer);

            bool __res = true;
            for (const std::string &ext_name : _extensions.list) {
                bool __ext_found = false;

                for (const VkExtensionProperties& ext_properties : _ext_list) {
                    if (ext_name == ext_properties.extensionName) {
                        __ext_found = true;
                        break;
                    }
                }

                if (!__ext_found) {
                    fprintf(outswarn, "%s %s\n", SVKFW_WRAPWARN("Device :: allDeviceExtAvailable","could not find extension"), ext_name.c_str());
                    // fflush(outswarn);

                    __res = false;
                }
            }
            return __res;
        }

        // for each layer name in _layers returns if it is available
        // (_res must have at least the same length (in elements) as _layers)
        void checkDeviceLayerAvailability(const PhysicalDevice::Handler &_phys_device, const StringVec &_layers, bool *_res) {
            std::vector<VkLayerProperties> __layer_list;
            getAllDeviceValLayers(_phys_device, __layer_list);

            for (uint32_t i = 0u; i < _layers.list.size(); ++i) {
                bool __layer_found = false;

                for (const VkLayerProperties& layer_properties : __layer_list) {
                    if (_layers.list[i] == layer_properties.layerName) {
                        __layer_found = true;
                        break;
                    }
                }
                _res[i] = __layer_found;
            }
        }

        // for each extension name in _extensions returns if it is available
        // (_res must have at least the same length (in elements) as _extensions)
        void checkDeviceExtAvailability(const PhysicalDevice::Handler &_phys_device, const StringVec &_extensions,
                                        bool *_res, const char *_layer) {
            std::vector<VkExtensionProperties> __ext_list;
            getAllDeviceExtensions(_phys_device, __ext_list, _layer);

            for (uint32_t i = 0u; i < _extensions.list.size(); ++i) {
                bool __ext_found = false;

                for (const VkExtensionProperties& ext_properties : __ext_list) {
                    if (_extensions.list[i] == ext_properties.extensionName) {
                        __ext_found = true;
                        break;
                    }
                }
                _res[i] = __ext_found;
            }
        }

        VkQueue getDeviceQueueHandle(const Device::Handler &_device,
                                     const QueueFamily::Handler &_queue_family, uint32_t _queue_index) {
            VkQueue __res = VK_NULL_HANDLE;
            vkGetDeviceQueue(_device.handler, _queue_family.index, _queue_index, &__res);

            return __res;
        }

        std::vector<VkQueue> getAllDeviceQueueHandles(const Device::Handler &_device,
                                                      const QueueFamily::Handler &_queue_family,
                                                      uint32_t _queue_index_offset, uint32_t _queue_index_count) {
            std::vector<VkQueue> __res;
            __res.resize(_queue_index_count);
            for (uint32_t i = 0u; i < __res.size(); ++i)
                vkGetDeviceQueue(_device.handler, _queue_family.index, i + _queue_index_offset, &__res[i]);

            return __res;
        }
    }; // Device END


//  ============  Vulkan Surface  ============  \\

    namespace SurfaceKHR {
        struct Builder : BuilderBase { // Although GLFW covers all portability, TODO: add "canonical" Builder for VkSurfaceKHR
            VkInstance instance = VK_NULL_HANDLE;
            GLFWwindow *glfw_window = nullptr;

            Builder(const Instance::Handler &_instance = {})
                            : instance{_instance.handler} {}
            Builder(const Instance::Handler &_instance, const Window &_glfw_window)
                            : instance{_instance.handler}, glfw_window{_glfw_window.window} {}
            Builder(const Builder &_builder) : instance{_builder.instance}, glfw_window{_builder.glfw_window} {}
            ~Builder() {
                clear();
            }

            void clear() {
                instance = VK_NULL_HANDLE;
                glfw_window = nullptr;
            }


            ObjectInfoType createInfoObject(void *_p_next = nullptr, VkFlags _flags = 0u,
                                          const VkAllocationCallbacks * _create_cb = nullptr) {
                return { create(_p_next, _flags, _create_cb), VK_OBJECT_TYPE_SURFACE_KHR };
            }

            ObjectCommonType create(void *_p_next = nullptr, VkFlags _flags = 0u, // these two parameters are not used
                              const VkAllocationCallbacks * _create_cb = nullptr) override {
                VkSurfaceKHR __surface = VK_NULL_HANDLE;
                if (glfwCreateWindowSurface(instance, glfw_window, _create_cb, &__surface) != VK_SUCCESS)
                    throw std::runtime_error(SVKFW_WRAPERR(" Surface Builder :: create","could not create surface"));
                return __surface;
            }

            inline VkSurfaceKHR createObject(void *_p_next = nullptr, VkFlags _flags = 0u, // these two parameters are not used
                                     const VkAllocationCallbacks * _create_cb = nullptr) {
                return (VkSurfaceKHR) create(_p_next, _flags, _create_cb);
            }


        // Setters:

            void setInstance(const Instance::Handler &_instance) {
                instance = _instance.handler;
            }

            void setWindow(const Window _glfw_window) {
                glfw_window = _glfw_window.window;
            }
        }; // Builder END
    }; // SurfaceKHR END


//  ============ Vulkan Swapchain  ============  \\

    namespace SwapchainKHR {
        std::vector<VkSurfaceFormatKHR> getAllFormats(VkPhysicalDevice _phys_device, const VkSurfaceKHR &_surface);
        std::vector<VkPresentModeKHR> getAllPresentationModes(VkPhysicalDevice _phys_device, const VkSurfaceKHR &_surface);
        std::vector <VkImage> getAllSwapchainImages(VkDevice _device, VkSwapchainKHR _swapchain);


        struct Suitability {
            typedef bool (*FormatTestFunc)(const VkSurfaceFormatKHR &_format);
            typedef int  (*FormatRankFunc)(const VkSurfaceFormatKHR &_format);
            typedef bool (*PresentModeTestFunc)(VkPresentModeKHR _present_mode);
            typedef int  (*PresentModeRankFunc)(VkPresentModeKHR _present_mode);

            static bool defaultFormatTest(const VkSurfaceFormatKHR &_format) {
                return _format.format == VK_FORMAT_B8G8R8A8_SRGB && _format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR;
            }

            static bool defaultPresentModeTest(VkPresentModeKHR _present_mode) {
                return _present_mode == VK_PRESENT_MODE_FIFO_KHR;
            }

            static bool mailboxPresentModeTest(VkPresentModeKHR _present_mode) {
                return _present_mode == VK_PRESENT_MODE_MAILBOX_KHR;
            }
        }; // Suitability END


        struct Builder : BuilderBase {
            VkPhysicalDevice phys_device = VK_NULL_HANDLE;
            VkSurfaceKHR surface = VK_NULL_HANDLE;
            VkSurfaceCapabilitiesKHR capabilities;
            VkDevice device = VK_NULL_HANDLE;
            VkSwapchainKHR old_swapchain = VK_NULL_HANDLE;

            uint32_t image_count = 0u;
            struct ImageInfo {
                VkFormat format = VK_FORMAT_UNDEFINED;
                VkColorSpaceKHR color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
                VkExtent2D extent = { 0u, 0u };
                uint32_t array_layers = 1u;
                VkImageUsageFlags usage = 0u;
                // VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE; // set automatically by queue_families (its non-emptiness)
            } image_info;

            std::vector<uint32_t> queue_families;
            VkSurfaceTransformFlagBitsKHR transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
            VkCompositeAlphaFlagBitsKHR composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // Alpha among windows - almost always opaque
            VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
            bool is_clipped = false;


            Builder(PhysicalDevice::Handler _phys_device = {},
                    const SurfaceKHR::Handler &_surface = {},
                    const Device::Handler &_device = {})
                            : phys_device{_phys_device.handler},
                              surface{_surface.handler}, device{_device.handler} {
                if (phys_device != VK_NULL_HANDLE && surface != VK_NULL_HANDLE)
                    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phys_device, surface, &capabilities);
            }
            Builder(const Builder &_builder) = default;
            ~Builder() {
                clear();
            }


            void clear() {
                surface = VK_NULL_HANDLE;
                capabilities = VkSurfaceCapabilitiesKHR{};
                phys_device = VK_NULL_HANDLE;
            }


            // if Handler's swapchain is not VK_NULL_HANDLE, sets old swapchain and logical device
            // use makeHandler() or clear()
            void initHandler(Handler &_handle, void *_p_next = nullptr, VkSwapchainCreateFlagsKHR _flags = 0u,
                             const VkAllocationCallbacks * _create_cb = nullptr,
                             const VkAllocationCallbacks *_destroy_cb = nullptr) {
                if (_handle.handler != VK_NULL_HANDLE)
                    setOldSwapchain(_handle.handler);

                _handle.handler = createObject(_p_next, _flags, _create_cb);
            }

            ObjectInfoType createInfoObject(void *_p_next = nullptr, VkSwapchainCreateFlagsKHR _flags = 0u,
                                            const VkAllocationCallbacks * _create_cb = nullptr) {
                return { create(_p_next, _flags, _create_cb), VK_OBJECT_TYPE_SWAPCHAIN_KHR };
            }

            ObjectCommonType create(void *_p_next = nullptr, VkSwapchainCreateFlagsKHR _flags = 0u,
                                    const VkAllocationCallbacks * _create_cb = nullptr) override {
                VkSwapchainCreateInfoKHR __info = createInfoStruct(_p_next, _flags);
                VkSwapchainKHR __swapchain = VK_NULL_HANDLE;

                // Can return an error depending on current state of the surface (?) TODO: proper error handling
                if (vkCreateSwapchainKHR(device, &__info, _create_cb, &__swapchain) != VK_SUCCESS)
                    throw std::runtime_error(SVKFW_WRAPERR(" Swapchain Builder :: create","could not create swapchain"));
                return __swapchain;
            }

            inline VkSwapchainKHR createObject(void *_p_next = nullptr, VkSwapchainCreateFlagsKHR _flags = 0u,
                                const VkAllocationCallbacks * _create_cb = nullptr) {
                return (VkSwapchainKHR) create(_p_next, _flags, _create_cb);
            }

            VkSwapchainCreateInfoKHR createInfoStruct(void *_p_next = nullptr, VkSwapchainCreateFlagsKHR _flags = 0u) {
                VkSwapchainCreateInfoKHR __info;
                __info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
                __info.pNext = _p_next;
                __info.flags = _flags;
                __info.surface = surface;
                __info.minImageCount = image_count;
                __info.imageFormat = image_info.format;
                __info.imageColorSpace = image_info.color_space;
                __info.imageExtent = image_info.extent;
                __info.imageArrayLayers = image_info.array_layers;
                __info.imageUsage = image_info.usage;
                __info.imageSharingMode = queue_families.size() ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
                __info.queueFamilyIndexCount = queue_families.size();
                __info.pQueueFamilyIndices = queue_families.data();
                __info.preTransform = transform;
                __info.compositeAlpha = composite_alpha;
                __info.presentMode = present_mode;
                __info.clipped = is_clipped ? VK_TRUE : VK_FALSE;
                __info.oldSwapchain = old_swapchain;

                return __info;
            }

            // TODO: some "initVecHandler" / "makeVecHandler" with vkCreateSharedSwapchainsKHR


            void setPhysicalDevice(PhysicalDevice::Handler _phys_device) {
                phys_device = _phys_device.handler;
            }

            void setDevice(const Device::Handler &_device) {
                device = _device.handler;
            }

            void setOldSwapchain(VkSwapchainKHR _swapchain) {
                old_swapchain = _swapchain;
            }

            void setOldSwapchainWithDevice(Handler &_swapchain, Device::Handler _device) {
                old_swapchain = _swapchain.handler;
                device = _device.handler;
                _swapchain.clear();
            }

            void setSurface(const SurfaceKHR::Handler &_surface) {
                surface = _surface.handler;
            }

            void setSurfaceCapabilities() {
                vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phys_device, surface, &capabilities);
            }

            void setSurfaceCapabilities(const VkSurfaceCapabilitiesKHR &_capabilities) {
                capabilities = _capabilities;
            }

            void setImageCount(uint32_t _count) {
                image_count = _count;
            }

            void setImageExtent(VkExtent2D _extent) {
                image_info.extent = _extent;
            }

            void setCurrentExtent() {
                image_info.extent = capabilities.currentExtent;
            }

            void setImageUsage(VkImageUsageFlags _usage) {
                image_info.usage = _usage;
            }

            void setQueueFaimilies(const QueueFamily::VecHandler &_q_families) {
                queue_families = _q_families.handlers;
            }

            void setTransform(VkSurfaceTransformFlagBitsKHR _transform) {
                transform = _transform;
            }

            void setCurrentTransform() {
                transform = capabilities.currentTransform;
            }

            void setCompositeAlpha(VkCompositeAlphaFlagBitsKHR _composite_alpha) {
                composite_alpha = _composite_alpha;
            }

            void setClipped(bool _is_clipped) {
                is_clipped = _is_clipped;
            }


            // Must be called before most other methods of this class. Called in constructor if non-empty handlers are passed
            void loadSurfaceCapabilities(PhysicalDevice::Handler _phys_device = {},
                                         const SurfaceKHR::Handler &_surface = {}) {
                if (_phys_device.handler != VK_NULL_HANDLE)
                    phys_device = _phys_device.handler;
                if (_surface.handler != VK_NULL_HANDLE)
                    surface = _surface.handler;

                vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phys_device, surface, &capabilities);
            }


            void clearQueueFamilies() {
                queue_families.clear();
            }

            VkSurfaceTransformFlagsKHR getSupportedTransforms() {
                return capabilities.supportedTransforms;
            }

            // Returns input bits after applying capabilities.supportedUsageFlags as a mask.
            // If not equal to input, then some usage flags are not supported
            // and the parameter is not set.
            VkImageUsageFlags checkAndSetImageUsage(VkImageUsageFlags _usage) {
                VkImageUsageFlags __res = _usage & capabilities.supportedUsageFlags;

                if (__res == _usage)
                    image_info.usage = _usage;
                else
                    fprintf(outswarn, SVKFW_WRAPWARN("SwapchainKHR Builder :: checkAndSetImageUsage",
                                                     "some usage flags are not supported\n"));
                return __res;
            }

            void chooseImageCount(uint32_t _count) {
                image_count = Simple::Func::clamp_GE_LE(_count,
                                                capabilities.minImageCount,
                                                capabilities.maxImageCount ? // (== 0) means no max
                                                capabilities.maxImageCount : uint32_t(-1));

                if (image_count != _count)
                    fprintf(outswarn, SVKFW_WRAPWARN(" Swapchain Builder :: chooseImageCount",
                                    "image count (%d) was clamped to %d to fit the interval [%d, %d]\n"),
                                        _count, image_count, capabilities.minImageCount, capabilities.maxImageCount);
                else printf(SVKFW_WRAPINFO(" Swapchain Builder :: chooseImageCount", "imageCount: %d, the interval: [%d, %d]\n"),
                                        image_count, capabilities.minImageCount, capabilities.maxImageCount);
            }

            bool chooseExtent(UVec2 _extent) { // TODO: either more info from functions (like this bool) or revert
                image_info.extent = { _extent.x, _extent.y };

                if (capabilities.currentExtent.width  != uint32_t(-1) &&
                    capabilities.currentExtent.height != uint32_t(-1)) {
                    image_info.extent = { Simple::Func::clamp_GE_LE(_extent.x,
                                            capabilities.minImageExtent.width,
                                            capabilities.maxImageExtent.width),
                                          Simple::Func::clamp_GE_LE(_extent.y,
                                            capabilities.minImageExtent.height,
                                            capabilities.maxImageExtent.height)  };
                    printf(SVKFW_WRAPINFO(" Swapchain Builder :: chooseExtent (1)", "resulting swap extent: [%d, %d]\n"),
                                            image_info.extent.width, image_info.extent.height);
                }
                return image_info.extent.width  == _extent.x &&
                       image_info.extent.height == _extent.y;
            }

            void chooseCurrentExtent(const Window &_window) {
                if (capabilities.currentExtent.width != uint32_t(-1)) {
                    image_info.extent = capabilities.currentExtent;
                    printf(SVKFW_WRAPINFO(" Swapchain Builder :: chooseExtent (2)", "from window's current swap extent: [%d, %d]\n"),
                                            image_info.extent.width, image_info.extent.height);
                    return;
                }
                else {
                    UVec2 __fbsize{_window.getFramebufferSize()};

                    image_info.extent = { Simple::Func::clamp_GE_LE(__fbsize.x,
                                            capabilities.minImageExtent.width,
                                            capabilities.maxImageExtent.width),
                                          Simple::Func::clamp_GE_LE(__fbsize.y,
                                            capabilities.minImageExtent.height,
                                            capabilities.maxImageExtent.height)  };
                    printf(SVKFW_WRAPINFO(" Swapchain Builder :: chooseExtent (2)", "from framebuffer size: [%d, %d]\n"),
                                            image_info.extent.width, image_info.extent.height);
                }
            }


            void chooseFormat(const Suitability::FormatTestFunc _test_func = Suitability::defaultFormatTest) {
                std::vector<VkSurfaceFormatKHR> __formats = getAllFormats(phys_device, surface);

                for (VkSurfaceFormatKHR __format : __formats) {
                    if (_test_func(__format)) {
                        image_info.format      = __format.format;
                        image_info.color_space = __format.colorSpace;
                        printf(SVKFW_WRAPINFO(" Swapchain Builder :: chooseFormat (1)", "format: %d, color space: %d\n"), uint32_t(__format.format), uint32_t(__format.colorSpace));
                        return;
                    }
                }
                throw std::runtime_error(SVKFW_WRAPERR(" Swapchain Builder :: chooseFormat (1)", "could not choose format"));
            }

            void chooseFormat(const Suitability::FormatRankFunc _rank_func) {
                std::vector<VkSurfaceFormatKHR> __formats = getAllFormats(phys_device, surface);

                int __max_rank = INT_MIN;
                uint32_t __curr_ind = -1;
                for (uint32_t i = 0u; i < __formats.size(); ++i) {
                    int __rank = _rank_func(__formats[i]);
                    if (__rank > __max_rank) {
                        __max_rank = __rank;
                        __curr_ind = i;
                    }
                }

                if (__curr_ind > __formats.size())
                    throw std::runtime_error(SVKFW_WRAPERR(" Swapchain Builder :: chooseFormat (2)", "could not choose format"));
                printf(SVKFW_WRAPINFO(" Swapchain Builder :: chooseFormat (2)", "format: %d, color space: %d\n"),
                        uint32_t(__formats[__curr_ind].format), uint32_t(__formats[__curr_ind].colorSpace));
                image_info.format      = __formats[__curr_ind].format;
                image_info.color_space = __formats[__curr_ind].colorSpace;
            }

            // returns true if successfully chose presentation mode using test function
            bool choosePresentationMode(const Suitability::PresentModeTestFunc _test_func = Suitability::defaultPresentModeTest) {
                std::vector<VkPresentModeKHR> __pres_modes = getAllPresentationModes(phys_device, surface);

                for (VkPresentModeKHR __mode : __pres_modes)
                    if (_test_func(__mode)) {
                        present_mode = __mode;
                        printf(SVKFW_WRAPINFO(" Swapchain Builder :: choosePresentationMode (1)", "mode %d\n"), uint32_t(__mode));
                        return true;
                    }

                fprintf(outswarn, SVKFW_WRAPWARN(" Swapchain Builder :: choosePresentationMode (1)",
                                                    "could not choose presentation mode, defaults to VK_PRESENT_MODE_FIFO_KHR\n"));
                present_mode = VK_PRESENT_MODE_FIFO_KHR;
                return false;
            }

            void choosePresentationMode(const Suitability::PresentModeRankFunc _rank_func) {
                std::vector<VkPresentModeKHR> __pres_modes = getAllPresentationModes(phys_device, surface);

                int __max_rank = INT_MIN;
                VkPresentModeKHR __curr_mode = VK_PRESENT_MODE_FIFO_KHR;
                for (VkPresentModeKHR __mode : __pres_modes) {
                    int __rank = _rank_func(__mode);
                    if (__rank >= __max_rank) {
                        __max_rank  = __rank;
                        __curr_mode = __mode;
                    }
                }
                printf(SVKFW_WRAPINFO(" Swapchain Builder :: choosePresentationMode (2)", "mode %d\n"), uint32_t(__curr_mode));
                present_mode = __curr_mode;
            }
        }; // Builder END


    // Functions definition

        std::vector<VkSurfaceFormatKHR> getAllFormats(VkPhysicalDevice _phys_device, const VkSurfaceKHR &_surface) {
            std::vector<VkSurfaceFormatKHR> __formats;
            uint32_t __format_count = 0u;

            vkGetPhysicalDeviceSurfaceFormatsKHR(_phys_device,
                                                _surface, &__format_count, nullptr);
            if (__format_count != 0u) {
                __formats.resize(__format_count);
                vkGetPhysicalDeviceSurfaceFormatsKHR(_phys_device,
                                                _surface, &__format_count, __formats.data());
            }
            else {
                printf(SVKFW_WRAPINFO(" Swapchain :: getAllFormats", "did not find surface formats.\n"));
            }
            return __formats;
        }

        std::vector<VkPresentModeKHR> getAllPresentationModes(VkPhysicalDevice _phys_device, const VkSurfaceKHR &_surface) {
            std::vector<VkPresentModeKHR> __pres_modes;
            uint32_t __pres_mode_count = 0u;
            vkGetPhysicalDeviceSurfacePresentModesKHR(_phys_device,
                                                    _surface, &__pres_mode_count, nullptr);

            if (__pres_mode_count != 0) {
                __pres_modes.resize(__pres_mode_count);
                vkGetPhysicalDeviceSurfacePresentModesKHR(_phys_device,
                                                    _surface, &__pres_mode_count, __pres_modes.data());
            }
            else {
                printf(SVKFW_WRAPINFO(" Swapchain :: getAllPresentationModes", "did not find presentation modes.\n"));
            }
            return __pres_modes;
        }

        std::vector<VkImage> getAllSwapchainImages(VkDevice _device, VkSwapchainKHR _swapchain) {
            std::vector<VkImage> __images;
            uint32_t __img_count = 0u;
            vkGetSwapchainImagesKHR(_device, _swapchain, &__img_count, nullptr);
            __images.resize(__img_count);
            vkGetSwapchainImagesKHR(_device, _swapchain, &__img_count, __images.data());
            return __images;
        }
    }; // SwapchainKHR END


//  ============  Vulkan Image  ============  \\

    namespace Image {
        // TODO:
        struct Builder {
            VkImageCreateInfo info;
        }; // Builder END
    };  // Image END


//  ============  Vulkan Image View  ============  \\

    namespace ImageView {
        struct Builder : BuilderBase {
            VkDevice device = VK_NULL_HANDLE;
            VkImage image = VK_NULL_HANDLE;
            VkImageViewType view_type = VK_IMAGE_VIEW_TYPE_MAX_ENUM;
            VkFormat format = VK_FORMAT_UNDEFINED;
            VkComponentMapping components{};
            VkImageSubresourceRange subresource_range{};


            Builder(const Device::Handler &_device = {}, const Image::Handler &_image = {})
                            : device{_device.handler}, image{_image.handler} {}
            ~Builder() {
                clear();
            }


            void clear() {
                view_type = VK_IMAGE_VIEW_TYPE_MAX_ENUM;
                image = VK_NULL_HANDLE;
                format = VK_FORMAT_UNDEFINED;
                components = VkComponentMapping{};
                subresource_range = VkImageSubresourceRange{};
            }


            ObjectInfoType createInfoObject(void *_p_next = nullptr, VkImageViewCreateFlags _flags = 0u,
                                          const VkAllocationCallbacks * _create_cb = nullptr) {
                return { create(_p_next, _flags, _create_cb), VK_OBJECT_TYPE_IMAGE_VIEW };
            }

            ObjectCommonType create(void *_p_next = nullptr, VkImageViewCreateFlags _flags = 0u,
                                const VkAllocationCallbacks * _create_cb = nullptr) override {
                VkImageViewCreateInfo __info = createInfoStruct(_p_next, _flags);
                VkImageView __image_view = VK_NULL_HANDLE;
                if (vkCreateImageView(device, &__info, _create_cb, &__image_view) != VK_SUCCESS)
                    throw std::runtime_error(SVKFW_WRAPERR(" Image View Builder :: create",
                                                            "failed to create image view"));

                return __image_view;
            }

            VkImageView createObject(void *_p_next = nullptr, VkImageViewCreateFlags _flags = 0u,
                                const VkAllocationCallbacks * _create_cb = nullptr) {
                return (VkImageView) create(_p_next, _flags, _create_cb);
            }

            VkImageViewCreateInfo createInfoStruct(void *_p_next = nullptr, VkImageViewCreateFlags _flags = 0u) {
                VkImageViewCreateInfo __info;
                __info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                __info.pNext = _p_next;
                __info.flags = _flags;
                __info.image = image;
                __info.viewType = view_type;
                __info.format = format;
                __info.components = components;
                __info.subresourceRange = subresource_range;

                return __info;
            }


            void setImage(const Image::Handler &_image) {
                image = _image.handler;
            }

            void setRange(VkImageAspectFlags _aspect_mask, uint32_t _base_mip_level,
                          uint32_t _level_count, uint32_t _base_array_layer, uint32_t _array_level_count) {
                subresource_range.aspectMask = _aspect_mask;
                subresource_range.baseMipLevel = _base_mip_level;
                subresource_range.levelCount = _level_count;
                subresource_range.baseArrayLayer = _base_array_layer;
                subresource_range.layerCount = _array_level_count;
            }

            void setComponents(VkComponentSwizzle _r = VK_COMPONENT_SWIZZLE_IDENTITY, VkComponentSwizzle _g = VK_COMPONENT_SWIZZLE_IDENTITY,
                               VkComponentSwizzle _b = VK_COMPONENT_SWIZZLE_IDENTITY, VkComponentSwizzle _a = VK_COMPONENT_SWIZZLE_IDENTITY) {
                components.r = _r;
                components.g = _g;
                components.b = _b;
                components.a = _a;
            }

            void setViewType(VkImageViewType _view_type) {
                view_type = _view_type;
            }

            void setFormat(VkFormat _format) {
                format = _format;
            }
        }; // Builder END
    }; // ImageView END


//  ============  Vulkan Shader Module  ============  \\

    namespace ShaderModule {
        struct Builder : BuilderBase {
        // Data:

            std::vector<char> buffer;
            VkDevice device = VK_NULL_HANDLE;


        // Constructors:

            Builder(const Device::Handler &_device = {}) : device{_device.handler} {}
            Builder(const char *_shader_fpath, const Device::Handler &_device) : device{_device.handler} {
                fromFile(_shader_fpath);
            }
            Builder(const char *_buffer_str, size_t _buffer_size, const Device::Handler &_device) : device{_device.handler} {
                fromString(_buffer_str, _buffer_size);
            }
            ~Builder() {
                clear();
            }


        // Clear state:

            void clear() {
                buffer.clear();
                device = VK_NULL_HANDLE;
            }


            ObjectInfoType createInfoObject(void *_p_next = nullptr, VkShaderModuleCreateFlags _flags = 0u,
                                          const VkAllocationCallbacks * _create_cb = nullptr) {
                return { create(_p_next, _flags, _create_cb), VK_OBJECT_TYPE_SHADER_MODULE };
            }

            ObjectCommonType create(void *_p_next = nullptr, VkShaderModuleCreateFlags _flags = 0u,
                                const VkAllocationCallbacks * _create_cb = nullptr) {
                VkShaderModuleCreateInfo __info = createInfoStruct(_p_next, _flags);
                VkShaderModule __shader = VK_NULL_HANDLE;
                if (vkCreateShaderModule(device, &__info, _create_cb, &__shader) != VK_SUCCESS)
                    throw std::runtime_error(SVKFW_WRAPERR(" Shader Module Builder :: create",
                                                            "failed to create shader module"));

                return __shader;
            }

            VkShaderModule createObject(void *_p_next = nullptr, VkShaderModuleCreateFlags _flags = 0u,
                                const VkAllocationCallbacks * _create_cb = nullptr) {
                return (VkShaderModule) create(_p_next, _flags, _create_cb);
            }

            VkShaderModuleCreateInfo createInfoStruct(void *_p_next = nullptr, VkShaderModuleCreateFlags _flags = 0u) {
                VkShaderModuleCreateInfo __info;
                __info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                __info.pNext = _p_next;
                __info.flags = _flags;
                __info.codeSize = buffer.size();
                __info.pCode = reinterpret_cast<const uint32_t *>(buffer.data());

                return __info;
            }


            void setDevice(const Device::Handler &_device) {
                device = _device.handler;
            }

            void setBuffer(const std::vector<char> &_buffer) {
                buffer = _buffer;
            }


            void fromFile(const char *_fpath) {
                std::ifstream __shader_file{_fpath, std::ios::ate | std::ios::binary};

                if (!__shader_file.is_open()) {
                    throw std::runtime_error(SVKFW_WRAPERR(" Shader Module Builder :: fromFile",
                                                            "failed to open file for reading"));
                }

                size_t __shader_file_size = (size_t) __shader_file.tellg();
                buffer.resize(__shader_file_size);

                __shader_file.seekg(0);
                __shader_file.read(buffer.data(), __shader_file_size);
                __shader_file.close();
            }

            void fromString(const char *_sh_str, size_t _length = 0u) {
                if (_sh_str == nullptr)
                    throw std::invalid_argument(SVKFW_WRAPERR(" Shader Module Builder :: fromString (1)", "no string provided"));
                if (_length == 0u) {
                    while (_sh_str[_length] != '\0') ++_length;
                    if (_length == 0u)
                        throw std::invalid_argument(SVKFW_WRAPERR(" Shader Module Builder :: fromString (1)", "string is empty"));
                }

                buffer.clear();
                buffer.insert(buffer.end(), _sh_str, _sh_str + _length);
            }

            void fromString(const std::string &_str_buffer) {
                if (_str_buffer.length() == 0u)
                    throw std::invalid_argument(SVKFW_WRAPERR(" Shader Module Builder :: fromString (2)", "string is empty"));

                buffer.clear();
                buffer.insert(buffer.end(), _str_buffer.begin(), _str_buffer.end());
            }
        }; // Builder END
    }; // ShaderModule END


//  ============  Vulkan Specialization Info  ============  \\

    namespace SpecializationInfo {

    // TODO: Make InfoHandler (like Handler/Handler, but for all Vulkan Info structures), with their functions inverted:
    //       InfoHandler destroys all internal array/pointer members (NOT RECURSIVELY), and InfoHandler : InfoHandler has a shallow copy.
    // Then replace all this with SctHandler (Struct handler): a handler for any Vulkan struct that has sType member


        struct InfoHandler {
            VkSpecializationInfo spec_info{0u, nullptr, 0u, nullptr};

            InfoHandler() {}
            InfoHandler(const VkSpecializationInfo &_spec_info) : spec_info{_spec_info} {}
            InfoHandler(const InfoHandler &_handler) : spec_info{_handler.spec_info} {}
            InfoHandler(InfoHandler &&_handler) : spec_info{_handler.spec_info} {
                _handler.clear();
            }
            virtual ~InfoHandler() {
                deletePointerMembers();
                clear();
            }


            void clear() {
                spec_info = VkSpecializationInfo{0u, nullptr, 0u, nullptr};
            }

            void deletePointerMembers() {
                if (spec_info.mapEntryCount && spec_info.pMapEntries != nullptr)
                    delete[] spec_info.pMapEntries;
                if (spec_info.dataSize && spec_info.pData != nullptr)
                    delete[] (const char*) spec_info.pData; // TODO: using new char[] is required for InfoHandler
            }

            const VkSpecializationInfo* ptr() const { return &spec_info; }

            InfoHandler& operator=(const InfoHandler &_handler) {
                spec_info = _handler.spec_info;
                return *this;
            }

            InfoHandler& operator=(InfoHandler &&_handler) {
                spec_info = _handler.spec_info;
                _handler.clear();
                return *this;
            }
        }; // InfoHandler END


        struct InfoHandlerView : InfoHandler {
            InfoHandlerView() {}
            InfoHandlerView(const VkSpecializationInfo &_spec_info) : InfoHandler{_spec_info} {}
            InfoHandlerView(const InfoHandlerView &_handler) : InfoHandler{_handler.spec_info} {}
            InfoHandlerView(InfoHandlerView &&_handler) : InfoHandler{_handler.spec_info} {
                _handler.clear();
            }
            ~InfoHandlerView() {
                clear(); // All the data is shallow-copied => nullifying all pointers, so no "delete[]" is called in base destructor
            }


            InfoHandlerView& operator=(const InfoHandlerView &_handler) {
                spec_info = _handler.spec_info;
                return *this;
            }

            InfoHandlerView& operator=(InfoHandlerView &&_handler) {
                spec_info = _handler.spec_info;
                _handler.clear();
                return *this;
            }
        }; // InfoHandlerView END


        struct Builder {
            std::vector<VkSpecializationMapEntry> map_entries;
            std::vector<unsigned char> data;


            Builder() {}
            Builder(const Builder &_builder) = default;
            // TODO: Constructor - from values (VkSpecializationInfo, VkSpecializationMapEntry vector) (?)
            ~Builder() {
                clear();
            }


            void clear() {
                map_entries.clear();
                data.clear();
            }


            // Has deep copied arrays, which are deleted in InfoHandler upon destruction
            InfoHandler makeInfoHandler() {
                VkSpecializationInfo __info{0u, nullptr, 0u, nullptr};
                __info.mapEntryCount = map_entries.size();

                if (__info.mapEntryCount) {
                    VkSpecializationMapEntry *__tmp_entries = new VkSpecializationMapEntry[__info.mapEntryCount];
                    memcpy(__tmp_entries, map_entries.data(), sizeof(VkSpecializationMapEntry) * __info.mapEntryCount);
                    __info.pMapEntries = __tmp_entries;

                    __info.dataSize = map_entries.back().offset + map_entries.back().size;
                    void *__tmp_data_ptr = new unsigned char[__info.dataSize];
                    memcpy(__tmp_data_ptr, data.data(), __info.dataSize);
                    __info.pData = data.data();
                }

                return __info;
            }

            // Has shallow copies of arrays, valid until Builder object is destroyed
            inline InfoHandlerView makeInfoHandlerView() {
                return makeInfoStruct();
            }

            // Has shallow copies of arrays, valid until Builder object is destroyed
            VkSpecializationInfo makeInfoStruct() {
                VkSpecializationInfo __info{0u, nullptr, 0u, nullptr};
                __info.mapEntryCount = map_entries.size();

                if (__info.mapEntryCount) {
                    __info.pMapEntries = map_entries.data();
                    __info.dataSize = map_entries.back().offset + map_entries.back().size;
                    __info.pData = data.data();
                }

                return __info;
            }


            bool empty() const {
                return map_entries.empty();
            }

            void addEntry(uint32_t _const_id, void *_data, size_t _size_bytes) {
                if (data.size() > uint32_t(-1)) // hypothetical error
                    throw std::runtime_error(SVKFW_WRAPERR(" Specialization Info :: addEntry", "data size makes offset invalid"));

                VkSpecializationMapEntry __new_entry{ .constantID= _const_id, .offset= (uint32_t) data.size(), .size= _size_bytes };
                const char *__data_reint = reinterpret_cast<const char*>(_data);

                data.insert(data.end(), __data_reint, __data_reint + _size_bytes);
                map_entries.push_back(__new_entry);
            }

        // not implemented
            void delEntry(uint32_t _index) {
                // TODO:
            }
        }; // Builder END
    };



//  ================================================  \\
                Vulkan graphics pipeline
//  ================================================  \\

    struct Viewport {
        struct SctBasic {
            VkViewport viewport;

            SctBasic(const VkExtent2D &_extent, float _x = 0.f, float _y = 0.f, float _depthMin = 0.f, float _depthMax = 1.f) :
                        viewport{.x= (float)_x, .y= (float)_y, .width= (float)_extent.width, .height= (float)_extent.height,
                                                .minDepth= _depthMin, .maxDepth= _depthMax} {}
            SctBasic(const Vec2 &_extent, const Vec2 &_offset = 0.f, const Vec2 &_depth = { 0.f, 1.f }) :
                        viewport{.x= _offset.x, .y= _offset.y, .width= _extent.x, .height= _extent.y,
                                                .minDepth= _depth.x, .maxDepth= _depth.y} {}
            SctBasic(const SctBasic &_viewport) : viewport{_viewport.viewport} {}
            ~SctBasic() {
                clearStruct();
            }


            void clearStruct() {
                viewport = VkViewport{};
            }

            const VkViewport* ptr() const { return &viewport; }

            void setOffset(const VkOffset2D &_offset) { viewport.x = _offset.x; viewport.y = _offset.y; }
            void setExtent(const VkExtent2D &_extent) { viewport.width = _extent.width; viewport.height = _extent.height; }

            void setOffset(const Vec2 &_offset) { viewport.x = _offset.x; viewport.y = _offset.y; }
            void setExtent(const Vec2 &_extent) { viewport.width = _extent.x; viewport.height = _extent.y; }
            void setDepth (const Vec2 &_depth)  { viewport.minDepth = _depth.x; viewport.maxDepth = _depth.y; }

            Vec2 offset() const { return { viewport.x, viewport.y }; }
            Vec2 extent() const { return { viewport.width, viewport.height }; }
            Vec2  depth() const { return { viewport.minDepth, viewport.maxDepth }; }
        }; // SctBasic END
    }; // Viewport END


    namespace Rect2D {
        struct SctBasic {
            VkRect2D rectangle{};

            SctBasic() {}
            SctBasic(const SctBasic &_stct) : rectangle{_stct.rectangle} {}
            SctBasic(const VkExtent2D &_extent, const VkOffset2D &_offset = {0, 0})
                            : rectangle{ .offset= {_offset.x, _offset.y}, .extent= {_extent.width, _extent.height} } {}
            SctBasic(const UVec2 &_extent, const IVec2 &_offset = 0)
                            : rectangle{ .offset= {_offset.x, _offset.y}, .extent= {_extent.x, _extent.y} } {}
            ~SctBasic() {
                clearStruct();
            }


            void clearStruct() {
                rectangle = VkRect2D{};
            }

            const VkRect2D* ptr() const { return &rectangle; }

            void setOffset(const IVec2 &_offset) { rectangle.offset.x = _offset.x; rectangle.offset.y = _offset.y; }
            void setExtent(const UVec2 &_extent) { rectangle.extent.width = _extent.x; rectangle.extent.height = _extent.y; }

            IVec2 offset() const { return { rectangle.offset.x, rectangle.offset.y }; }
            UVec2 extent() const { return { rectangle.extent.width, rectangle.extent.height }; }
        }; // SctBasic END
    }; // Rect2D END


    namespace ClearValue {
        VkClearValue build(float _v0, float _v1, float _v2, float _v3) {
            VkClearValue __res{};
            __res.color.float32[0] = _v0;
            __res.color.float32[1] = _v1;
            __res.color.float32[2] = _v2;
            __res.color.float32[3] = _v3;
            return __res;
        }
        VkClearValue build(int32_t _v0, int32_t _v1, int32_t _v2, int32_t _v3) {
            VkClearValue __res{};
            __res.color.int32[0] = _v0;
            __res.color.int32[1] = _v1;
            __res.color.int32[2] = _v2;
            __res.color.int32[3] = _v3;
            return __res;
        }
        VkClearValue build(uint32_t _v0, uint32_t _v1, uint32_t _v2, uint32_t _v3) {
            VkClearValue __res{};
            __res.color.uint32[0] = _v0;
            __res.color.uint32[1] = _v1;
            __res.color.uint32[2] = _v2;
            __res.color.uint32[3] = _v3;
            return __res;
        }
        VkClearValue build(float _depth, uint32_t _stencil) {
            VkClearValue __res{ .depthStencil = {_depth, _stencil} };
            return __res;
        }
    }; // ClearValue END


    namespace AttachmentDescription {
        struct SctBasic {
            VkAttachmentDescription description{};

            SctBasic() {}
            SctBasic(const SctBasic &_stct) : description{_stct.description} {}
            SctBasic(const VkAttachmentDescription &_description) :
                        description{_description} {}
            ~SctBasic() {
                clearStruct();
            }


            const VkAttachmentDescription *ptr() const { return &description; }

            void clearStruct() {
                description = VkAttachmentDescription{};
            }

            void setColorOps(VkAttachmentLoadOp _loadOp, VkAttachmentStoreOp _storeOp) {
                description.loadOp = _loadOp;
                description.storeOp = _storeOp;
            }
            void setStencilOps(VkAttachmentLoadOp _loadOp, VkAttachmentStoreOp _storeOp) {
                description.stencilLoadOp = _loadOp;
                description.stencilStoreOp = _storeOp;
            }
            void setLayouts(VkImageLayout _initLayout, VkImageLayout _finalLayout) {
                description.initialLayout = _initLayout;
                description.finalLayout = _finalLayout;
            }
            void setFormatAndSize(VkFormat _format, VkSampleCountFlagBits _samples) {
                description.format = _format;
                description.samples = _samples;
            }
        }; // SctBasic END
    }; // AttachmentDescription END


    namespace SubpassDescription {
        struct SctHandler {
            VkSubpassDescription subpass_description{};

            SctHandler() {}
            SctHandler(const VkSubpassDescription &_subpass_description) : subpass_description{_subpass_description} {}
            SctHandler(const SctHandler &_handler) : subpass_description{_handler.subpass_description} {}
            SctHandler(SctHandler &&_handler) : subpass_description{_handler.subpass_description} {
                _handler.clear();
            }
            virtual ~SctHandler() {
                deletePointerMembers();
                clear();
            }


            void deletePointerMembers() {
                if (subpass_description.inputAttachmentCount > 0u && subpass_description.pInputAttachments != nullptr)
                    delete[] subpass_description.pInputAttachments;
                if (subpass_description.colorAttachmentCount > 0u) {
                    if (subpass_description.pColorAttachments != nullptr)
                        delete[] subpass_description.pColorAttachments;
                    if (subpass_description.pResolveAttachments != nullptr)
                        delete[] subpass_description.pResolveAttachments;
                }
                if (subpass_description.pDepthStencilAttachment != nullptr)
                    delete subpass_description.pDepthStencilAttachment;
                if (subpass_description.preserveAttachmentCount > 0u && subpass_description.pPreserveAttachments != nullptr)
                    delete[] subpass_description.pPreserveAttachments;
            }

            void clear() {
                subpass_description = VkSubpassDescription{};
            }

            const VkSubpassDescription* ptr() const { return &subpass_description; }

            SctHandler& operator=(const SctHandler &_handler) {
                subpass_description = _handler.subpass_description;
                return *this;
            }

            SctHandler& operator=(SctHandler &&_handler) {
                subpass_description = _handler.subpass_description;
                _handler.clear();
                return *this;
            }
        }; // SctHandler END


        struct SctHandlerView : SctHandler {
            SctHandlerView() {}
            SctHandlerView(const VkSubpassDescription &_subpass_description) : SctHandler{_subpass_description} {}
            SctHandlerView(const SctHandler &_handler) : SctHandler{_handler.subpass_description} {}
            SctHandlerView(SctHandler &&_handler) : SctHandler{_handler.subpass_description} {
                _handler.clear();
            }
            ~SctHandlerView() {
                clear();
            }


            SctHandlerView& operator=(const SctHandlerView &_handler) {
                subpass_description = _handler.subpass_description;
                return *this;
            }

            SctHandlerView& operator=(SctHandlerView &&_handler) {
                subpass_description = _handler.subpass_description;
                _handler.clear();
                return *this;
            }
        }; // SctHandlerView END


        struct Builder {
            VkPipelineBindPoint binding_point{VK_PIPELINE_BIND_POINT_MAX_ENUM};
            VkAttachmentReference depth_stencil_attachment;
            std::vector<VkAttachmentReference> color_attachments,
                                               input_attachments,
                                               resolve_attachments;
            std::vector<uint32_t> preserve_attachments;

            void clearBuilder() {
                binding_point = VK_PIPELINE_BIND_POINT_MAX_ENUM;
                depth_stencil_attachment = VkAttachmentReference{};
                color_attachments.clear();
                input_attachments.clear();
                resolve_attachments.clear();
                preserve_attachments.clear();
            }


            VkSubpassDescription makeInfoStruct(VkSubpassDescriptionFlags _flags = 0u) {
                VkSubpassDescription __subpass_description{};
                __subpass_description.flags = _flags;
                __subpass_description.colorAttachmentCount = color_attachments.size();
                __subpass_description.pColorAttachments    = color_attachments.data();
                __subpass_description.inputAttachmentCount = input_attachments.size();
                __subpass_description.pInputAttachments    = input_attachments.data();
                __subpass_description.preserveAttachmentCount = preserve_attachments.size();
                __subpass_description.pPreserveAttachments    = preserve_attachments.data();
                __subpass_description.pResolveAttachments  = resolve_attachments.size() ? resolve_attachments.data() : nullptr;
                return __subpass_description;
            }

            VkSubpassDescription makeInfoStructDCopy(VkSubpassDescriptionFlags _flags = 0u) {
                VkSubpassDescription __subpass_description{};
                __subpass_description.flags = _flags;
                __subpass_description.colorAttachmentCount = color_attachments.size();
                __subpass_description.inputAttachmentCount = input_attachments.size();
                __subpass_description.preserveAttachmentCount = preserve_attachments.size();

                VkAttachmentReference *__attach_memory = new VkAttachmentReference[color_attachments.size() + input_attachments.size() + resolve_attachments.size()];
                memcpy(__attach_memory,                            color_attachments.data(), sizeof(VkAttachmentReference) * color_attachments.size());
                memcpy(__attach_memory + color_attachments.size(), input_attachments.data(), sizeof(VkAttachmentReference) * input_attachments.size());
                if (resolve_attachments.size())
                    memcpy(__attach_memory + color_attachments.size() + input_attachments.size(), input_attachments.data(), sizeof(VkAttachmentReference) * input_attachments.size());

                __subpass_description.pColorAttachments    = __attach_memory;
                __subpass_description.pInputAttachments    = __attach_memory + color_attachments.size();
                __subpass_description.pResolveAttachments  = resolve_attachments.size() ? (__attach_memory + color_attachments.size() + input_attachments.size()) : nullptr;

                uint32_t *__arr_memory = new uint32_t[preserve_attachments.size()];
                memcpy(__arr_memory, preserve_attachments.data(), sizeof(uint32_t) * preserve_attachments.size());
                __subpass_description.pPreserveAttachments = __arr_memory;

                return __subpass_description;
            }


            void setColorAttachments(const std::vector<VkAttachmentReference> &_color_attachments) {
                color_attachments = _color_attachments;
            }

            void setInputAttachments(const std::vector<VkAttachmentReference> &_input_attachments) {
                input_attachments = _input_attachments;
            }

            void setResolveAttachments(const std::vector<VkAttachmentReference> &_resolveAttachments) {
                resolve_attachments = _resolveAttachments;
            }

            void setDepthStencilAttachment(VkAttachmentReference _ds_attachment) {
                depth_stencil_attachment = _ds_attachment;
            }

            void setPreservedAttachments(const std::vector<uint32_t> &_preserve_attachments) {
                preserve_attachments = _preserve_attachments;
            }

            void setBindingPoint(VkPipelineBindPoint _binding_point) {
                binding_point = _binding_point;
            }
        }; // Builder END
    }; // SubpassDescription END


    namespace SubpassDependency {
        struct SctBasic {
            VkSubpassDependency subpass_dependency{};

            SctBasic() {}
            SctBasic(const SctBasic &_stct) : subpass_dependency{_stct.subpass_dependency} {}
            SctBasic(const VkSubpassDependency &_subpass_dependency) : subpass_dependency{_subpass_dependency} {}
            ~SctBasic() {
                clearStruct();
            }


            const VkSubpassDependency *ptr() const { return &subpass_dependency; }

            void clearStruct() {
                subpass_dependency = VkSubpassDependency{};
            }

            void setSrcSubpass(uint32_t _src_subpass) {
                subpass_dependency.srcSubpass = _src_subpass;
            }

            void setDstSubpass(uint32_t _dst_subpass) {
                subpass_dependency.dstSubpass = _dst_subpass;
            }

            void setSrcAccessMask(VkAccessFlags _src_acc_mask) {
                subpass_dependency.srcAccessMask = _src_acc_mask;
            }

            void setDstAccessMask(VkAccessFlags _dst_acc_mask) {
                subpass_dependency.dstAccessMask = _dst_acc_mask;
            }

            void setSrcStageMask(VkPipelineStageFlags _src_stage_mask) {
                subpass_dependency.srcStageMask = _src_stage_mask;
            }

            void setDstStageMask(VkPipelineStageFlags _dst_stage_mask) {
                subpass_dependency.dstStageMask = _dst_stage_mask;
            }

            void setFlags(VkDependencyFlags _flags) {
                subpass_dependency.dependencyFlags = _flags;
            }

            void setSubpasses(uint32_t _src_subpass, uint32_t _dst_subpass) {
                subpass_dependency.srcSubpass = _src_subpass;
                subpass_dependency.dstSubpass = _dst_subpass;
            }

            void setSrcMasks(VkPipelineStageFlags _src_stage_mask, VkAccessFlags _src_acc_mask) {
                subpass_dependency.srcStageMask = _src_stage_mask;
                subpass_dependency.srcAccessMask = _src_acc_mask;
            }

            void setDstMasks(VkPipelineStageFlags _dst_stage_mask, VkAccessFlags _dst_acc_mask) {
                subpass_dependency.dstStageMask = _dst_stage_mask;
                subpass_dependency.dstAccessMask = _dst_acc_mask;
            }
        }; // SctBasic END
    }; // SubpassDependency END


//  ============  Vulkan Render Pass  ============  \\

    namespace RenderPass {
        struct Builder : BuilderBase {
            std::vector<VkAttachmentDescription> attachments;
            std::vector<VkSubpassDescription> subpasses;
            std::vector<VkSubpassDependency> dependencies;
            VkDevice device = VK_NULL_HANDLE;

            Builder(const Device::Handler &_device = {}) : device{_device.handler} {}
            Builder(const Builder &_builder) : attachments{_builder.attachments}, subpasses{_builder.subpasses},
                                               dependencies{_builder.dependencies} {}
            ~Builder() {
                clear();
            }


            void clear() {
                attachments.clear();
                subpasses.clear();
                dependencies.clear();
                device = VK_NULL_HANDLE;
            }


            ObjectInfoType createInfoObject(void *_p_next = nullptr, VkRenderPassCreateFlags _flags = 0u,
                                          const VkAllocationCallbacks * _create_cb = nullptr) {
                return { create(_p_next, _flags, _create_cb), VK_OBJECT_TYPE_RENDER_PASS };
            }

            ObjectCommonType create(void *_p_next = nullptr, VkRenderPassCreateFlags _flags = 0u,
                                const VkAllocationCallbacks * _create_cb = nullptr) override {
                VkRenderPassCreateInfo __info = createInfoStruct(_p_next, _flags);
                VkRenderPass __render_pass = VK_NULL_HANDLE;
                if (vkCreateRenderPass(device, &__info, _create_cb, &__render_pass) != VK_SUCCESS)
                    throw std::runtime_error(SVKFW_WRAPERR(" Render Pass Builder :: create",
                                                            "failed to create render pass"));

                return __render_pass;
            }

            VkRenderPass createObject(void *_p_next = nullptr, VkRenderPassCreateFlags _flags = 0u,
                              const VkAllocationCallbacks * _create_cb = nullptr) {
                return (VkRenderPass) create(_p_next, _flags, _create_cb);
            }

            VkRenderPassCreateInfo createInfoStruct(void *_p_next = nullptr, VkRenderPassCreateFlags _flags = 0u) {
                VkRenderPassCreateInfo __info;
                __info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
                __info.pNext = _p_next;
                __info.flags = _flags;
                __info.attachmentCount = attachments.size();
                __info.pAttachments = attachments.data();
                __info.subpassCount = subpasses.size();
                __info.pSubpasses = subpasses.data();
                __info.dependencyCount = dependencies.size();
                __info.pDependencies = dependencies.data();

                return __info;
            }


        // Setters:

            void setAttachments(const std::vector<VkAttachmentDescription> &_attachments) {
                attachments = _attachments;
            }

            void setAttachments(const std::vector<AttachmentDescription::SctBasic> &_attachments) {
                attachments.resize(_attachments.size());
                for (uint32_t i = 0u; i < _attachments.size(); ++i)
                    attachments[i] = _attachments[i].description;
            }

            void setSubpasses(const std::vector<VkSubpassDescription> &_subpasses) {
                subpasses = _subpasses;
            }

            // TODO: SubpassDescription::SctVecHandler

            // void setSubpasses(const std::vector<SubpassDescription::SctHandler> &_subpasses) {
            //     subpasses.resize(_subpasses.size());
            //     for (uint32_t i = 0u; i < _subpasses.size(); ++i)
            //         subpasses[i] = _subpasses[i].subpass_description;
            // }

            // void setSubpasses(const std::vector<SubpassDescription::SctHandler> &_subpasses) {
            //     subpasses.resize(_subpasses.size());
            //     for (uint32_t i = 0u; i < _subpasses.size(); ++i)
            //         subpasses[i] = _subpasses[i].subpass_description;
            // }

            void setDependencies(const std::vector<VkSubpassDependency> &_dependencies) {
                dependencies = _dependencies;
            }
        }; // Builder END
    }; // RenderPass END


//  ============  Vulkan Pipeline Layout  ============  \\

    namespace PipelineLayout {
        struct Builder : BuilderBase {
            std::vector <VkDescriptorSetLayout> layouts;
            std::vector <VkPushConstantRange> constants;
            VkDevice device = VK_NULL_HANDLE;

            Builder(const Device::Handler &_device = {}) : device{_device.handler} {}
            Builder(const Builder &_builder) : layouts{_builder.layouts}, constants{_builder.constants}, device{_builder.device} {}
            ~Builder() {
                clear();
            }

            void clear() {
                layouts.clear();
                constants.clear();
            }


            ObjectInfoType createInfoObject(void *_p_next = nullptr, VkPipelineLayoutCreateFlags _flags = 0u,
                                          const VkAllocationCallbacks * _create_cb = nullptr) {
                return { create(_p_next, _flags, _create_cb), VK_OBJECT_TYPE_PIPELINE_LAYOUT };
            }

            ObjectCommonType create(void *_p_next = nullptr, VkPipelineLayoutCreateFlags _flags = 0u,
                                  const VkAllocationCallbacks * _create_cb = nullptr) override {
                VkPipelineLayoutCreateInfo __info = createInfoStruct(_p_next, _flags);
                VkPipelineLayout __layout = VK_NULL_HANDLE;
                if (vkCreatePipelineLayout(device, &__info, _create_cb, &__layout) != VK_SUCCESS)
                    throw std::runtime_error(SVKFW_WRAPERR(" Pipeline Layout Builder :: create", "could not create pipeline layout"));

                return __layout;
            }

            VkPipelineLayout createObject(void *_p_next = nullptr, VkPipelineLayoutCreateFlags _flags = 0u,
                                  const VkAllocationCallbacks * _create_cb = nullptr) {
                return (VkPipelineLayout) create(_p_next, _flags, _create_cb);
            }

            VkPipelineLayoutCreateInfo createInfoStruct(void *_p_next = nullptr, VkPipelineLayoutCreateFlags _flags = 0u) {
                VkPipelineLayoutCreateInfo __info;
                __info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
                __info.pNext = _p_next;
                __info.flags = _flags;
                __info.setLayoutCount = layouts.size();
                __info.pSetLayouts = layouts.size() ? layouts.data() : nullptr;
                __info.pushConstantRangeCount = constants.size();
                __info.pPushConstantRanges = constants.size() ? constants.data() : nullptr;

                return __info;
            }


            void setLayouts(const std::vector<VkDescriptorSetLayout> &_layouts) {
                layouts = _layouts;
            }

            void setConstRanges(const std::vector<VkPushConstantRange> &_constants) {
                constants = _constants;
            }
        }; // Builder END
    }; // PipelineLayout END


//  ============  Vulkan Pipeline  ============  \\

    namespace Pipeline {

//  ============  Pipeline Shader Stage  ============  \\

        namespace ShaderStage {
            // For now it requires all SpecializationInfo::InfoHandler and ShaderStage::Builder
            // to exist until the creation of pipeline
            // (Builder - for VkSpecializationInfo*, SpecializationInfo handler - for its pointers/arrays)
            // TODO: now there is an idea to provide deep copy methods
            //       and handlers that expect deep-copied pointer members (and delete them upon destruction, if they are not nullptr)

            struct InfoHandler {
                VkPipelineShaderStageCreateInfo shader_stage_create_info{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};



                InfoHandler() {}
                InfoHandler(const VkPipelineShaderStageCreateInfo &_shader_stage_create_info) : shader_stage_create_info{_shader_stage_create_info} {}
                InfoHandler(const InfoHandler &_handler) : shader_stage_create_info{_handler.shader_stage_create_info} {}
                InfoHandler(InfoHandler &&_handler) : shader_stage_create_info{_handler.shader_stage_create_info} {
                    _handler.clear();
                }
                virtual ~InfoHandler() {
                    deletePointerMembers();
                    clear();
                }


                void deletePointerMembers() {
                    if (shader_stage_create_info.pSpecializationInfo != nullptr)
                        delete shader_stage_create_info.pSpecializationInfo;
                }


                void clear() {
                    shader_stage_create_info = VkPipelineShaderStageCreateInfo{};
                    shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                }

                const VkPipelineShaderStageCreateInfo* ptr() const { return &shader_stage_create_info; }

                InfoHandler& operator=(const InfoHandler &_handler) {
                    shader_stage_create_info = _handler.shader_stage_create_info;
                    return *this;
                }

                InfoHandler& operator=(InfoHandler &&_handler) {
                    shader_stage_create_info = _handler.shader_stage_create_info;
                    _handler.clear();
                    return *this;
                }
            }; // InfoHandler END


            struct InfoHandlerView : InfoHandler {
                InfoHandlerView() {}
                InfoHandlerView(const VkPipelineShaderStageCreateInfo &_shader_stage_create_info) : InfoHandler{_shader_stage_create_info} {}
                InfoHandlerView(const InfoHandlerView &_handler) : InfoHandler{_handler.shader_stage_create_info} {}
                InfoHandlerView(InfoHandlerView &&_handler) : InfoHandler{_handler.shader_stage_create_info} {
                    _handler.clear();
                }
                ~InfoHandlerView() {
                    clear();
                }


                InfoHandlerView& operator=(const InfoHandlerView &_handler) {
                    shader_stage_create_info = _handler.shader_stage_create_info;
                    return *this;
                }

                InfoHandlerView& operator=(InfoHandlerView &&_handler) {
                    shader_stage_create_info = _handler.shader_stage_create_info;
                    _handler.clear();
                    return *this;
                }
            }; // InfoHandlerView END


            // TODO: InfoVecHandler - deletes pointer members in destructor for each element of vector,
            // v     and InfoVecHandler destructor clears before that. This is the first InfoVecHandler - complete it and use as a template.

            struct InfoVecHandler {
                std::vector<VkPipelineShaderStageCreateInfo> shader_stages;

                InfoVecHandler() {}
                InfoVecHandler(const std::vector<VkPipelineShaderStageCreateInfo> &_shader_stages)
                                : shader_stages{_shader_stages} {}
                InfoVecHandler(const std::vector<InfoHandler> &_shader_stages) {
                    shader_stages.resize(_shader_stages.size());
                    for (uint32_t i = 0u; i < _shader_stages.size(); ++i)
                        shader_stages[i] = _shader_stages[i].shader_stage_create_info;
                }
                InfoVecHandler(const InfoVecHandler & _handler)
                                : shader_stages{_handler.shader_stages} {}
                InfoVecHandler(InfoVecHandler && _handler)
                                : shader_stages{_handler.shader_stages} {
                    _handler.clear();
                }
                ~InfoVecHandler() {
                    clear();
                }


                void clear() {
                    shader_stages.clear();
                }

                const VkPipelineShaderStageCreateInfo* ptr() const { return shader_stages.data(); }
                size_t count() const { return shader_stages.size(); }
            };

            using InfoVecHandler = InfoVecHandler;

            // ^ TODO


            struct Builder {
                VkShaderModule shader_module = VK_NULL_HANDLE;
                const char *entry_point_name = nullptr;
                VkShaderStageFlagBits stage;
                VkSpecializationInfo spec_info{ 0u, nullptr, 0u, nullptr };

                Builder() {}
                Builder(const Builder &_builder)
                                : shader_module{_builder.shader_module}, entry_point_name{_builder.entry_point_name},
                                  stage{_builder.stage}, spec_info{_builder.spec_info} {}
                ~Builder() {
                    clear();
                }


                void clear() {
                    shader_module = VK_NULL_HANDLE;
                    entry_point_name = nullptr;
                    stage = VkShaderStageFlagBits(0);
                    spec_info = { 0u, nullptr, 0u, nullptr };
                }


                VkPipelineShaderStageCreateInfo makeInfoStruct(void *_p_next = nullptr, VkPipelineShaderStageCreateFlags _flags = 0u) {
                    VkPipelineShaderStageCreateInfo __info;
                    __info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                    __info.pNext = _p_next;
                    __info.flags = _flags;
                    __info.stage = stage;
                    __info.module = shader_module;
                    __info.pName = entry_point_name;
                    __info.pSpecializationInfo = spec_info.mapEntryCount ? &spec_info : nullptr;

                    return __info;
                }

                VkPipelineShaderStageCreateInfo makeInfoStructDCopy(void *_p_next = nullptr, VkPipelineShaderStageCreateFlags _flags = 0u) {
                    VkPipelineShaderStageCreateInfo __info;
                    __info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                    __info.pNext = _p_next;
                    __info.flags = _flags;
                    __info.stage = stage;
                    __info.module = shader_module;
                    __info.pName = entry_point_name;
                    __info.pSpecializationInfo = spec_info.mapEntryCount ? new VkSpecializationInfo(spec_info) : nullptr;

                    return __info;
                }


                void setShaderInfo(const ShaderModule::Handler &_shader, VkShaderStageFlagBits _stage, const char *_name) {
                    shader_module = _shader.handler;
                    stage = _stage;
                    entry_point_name = _name;
                }

                void setSpecializationInfo(const VkSpecializationInfo &_spec_info) {
                    spec_info = _spec_info;
                }
            }; // Builder END
        }; // ShaderStage END


//  ============  Pipeline Dynamic State  ============  \\

        namespace DynamicState {
            struct InfoHandler {
                VkPipelineDynamicStateCreateInfo dynamic_state_create_info{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};

                InfoHandler() {}
                InfoHandler(const VkPipelineDynamicStateCreateInfo &_dynamic_state_create_info)
                                : dynamic_state_create_info{_dynamic_state_create_info} {}
                InfoHandler(const InfoHandler &_handler) : dynamic_state_create_info{_handler.dynamic_state_create_info} {}
                InfoHandler(InfoHandler &&_handler) : dynamic_state_create_info{_handler.dynamic_state_create_info} {
                    _handler.clear();
                }
                virtual ~InfoHandler() {
                    deletePointerMembers();
                    clear();
                }


                void deletePointerMembers() {
                    if (dynamic_state_create_info.dynamicStateCount > 0u && dynamic_state_create_info.pDynamicStates != nullptr)
                        delete[] dynamic_state_create_info.pDynamicStates;
                }


                void clear() {
                    dynamic_state_create_info = VkPipelineDynamicStateCreateInfo{};
                    dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
                }

                const VkPipelineDynamicStateCreateInfo* ptr() const { return &dynamic_state_create_info; }

                InfoHandler& operator=(const InfoHandler &_handler) {
                    dynamic_state_create_info = _handler.dynamic_state_create_info;
                    return *this;
                }

                InfoHandler& operator=(InfoHandler &&_handler) {
                    dynamic_state_create_info = _handler.dynamic_state_create_info;
                    _handler.clear();
                    return *this;
                }
            }; // InfoHandler END


            struct InfoHandlerView : InfoHandler {
                InfoHandlerView() {}
                InfoHandlerView(const VkPipelineDynamicStateCreateInfo &_dynamic_state_create_info) : InfoHandler{_dynamic_state_create_info} {}
                InfoHandlerView(const InfoHandlerView &_handler) : InfoHandler{_handler.dynamic_state_create_info} {}
                InfoHandlerView(InfoHandlerView &&_handler) : InfoHandler{_handler.dynamic_state_create_info} {
                    _handler.clear();
                }
                ~InfoHandlerView() {
                    clear();
                }


                InfoHandlerView& operator=(const InfoHandlerView &_handler) {
                    dynamic_state_create_info = _handler.dynamic_state_create_info;
                    return *this;
                }

                InfoHandlerView& operator=(InfoHandlerView &&_handler) {
                    dynamic_state_create_info = _handler.dynamic_state_create_info;
                    _handler.clear();
                    return *this;
                }
            }; // InfoHandlerView END


            struct Builder {
                std::vector<VkDynamicState> dynamic_states;

                Builder() {}
                Builder(const Builder &_builder) : dynamic_states{_builder.dynamic_states} {}
                ~Builder() {
                    clear();
                }


                void clear() {
                    dynamic_states.clear();
                }


                VkPipelineDynamicStateCreateInfo makeInfoStruct(void *_p_next = nullptr, VkPipelineDynamicStateCreateFlags _flags = 0u) {
                    VkPipelineDynamicStateCreateInfo __info;
                    __info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
                    __info.pNext = _p_next;
                    __info.flags = _flags;
                    __info.dynamicStateCount = dynamic_states.size();
                    __info.pDynamicStates = dynamic_states.data();

                    return __info;
                }

                VkPipelineDynamicStateCreateInfo makeInfoStructDCopy(void *_p_next = nullptr, VkPipelineDynamicStateCreateFlags _flags = 0u) {
                    VkPipelineDynamicStateCreateInfo __info;
                    __info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
                    __info.pNext = _p_next;
                    __info.flags = _flags;
                    __info.dynamicStateCount = dynamic_states.size();

                    VkDynamicState *__dynamic_states = new VkDynamicState[dynamic_states.size()];
                    for (uint32_t i = 0u; i < dynamic_states.size(); ++i)
                        __dynamic_states[i] = dynamic_states[i];
                    __info.pDynamicStates = __dynamic_states;

                    return __info;
                }


                void setDynamicStates(const std::vector<VkDynamicState> &_dynamic_states) {
                    dynamic_states = _dynamic_states;
                }

                // returns false if state is already included in "dynamic_states", true if successfully added
                bool addDynamicState(VkDynamicState _state) {
                    for (const VkDynamicState &st : dynamic_states)
                        if (st == _state) return false;

                    dynamic_states.push_back(_state);
                    return true;
                }

                // returns false if state is not in "dynamic_states", true if successfully removed first entry
                bool delDynamicState(VkDynamicState _state) {
                    for (uint32_t i = 0u; i < dynamic_states.size(); ++i) {
                        if (dynamic_states[i] == _state) {
                            dynamic_states.erase(dynamic_states.begin() + i);
                            return true;
                        }
                    }
                    return false;
                }
            }; // Builder END
        }; // DynamicState END


//  ============  Pipeline Vertex Input State  ============  \\

        namespace VertexInputState {
            struct InfoHandler {
                VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

                InfoHandler() {}
                InfoHandler(const VkPipelineVertexInputStateCreateInfo &_vertex_input_state_create_info)
                                : vertex_input_state_create_info{_vertex_input_state_create_info} {}
                InfoHandler(const InfoHandler &_handler) : vertex_input_state_create_info{_handler.vertex_input_state_create_info} {}
                InfoHandler(InfoHandler &&_handler) : vertex_input_state_create_info{_handler.vertex_input_state_create_info} {
                    _handler.clear();
                }
                virtual ~InfoHandler() {
                    deletePointerMembers();
                    clear();
                }


                void deletePointerMembers() {
                    if (vertex_input_state_create_info.vertexBindingDescriptionCount > 0u && vertex_input_state_create_info.pVertexBindingDescriptions != nullptr)
                        delete[] vertex_input_state_create_info.pVertexBindingDescriptions;
                    if (vertex_input_state_create_info.vertexAttributeDescriptionCount > 0u && vertex_input_state_create_info.pVertexAttributeDescriptions != nullptr)
                        delete[] vertex_input_state_create_info.pVertexAttributeDescriptions;
                }

                void clear() {
                    vertex_input_state_create_info = VkPipelineVertexInputStateCreateInfo{};
                    vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
                }

                const VkPipelineVertexInputStateCreateInfo* ptr() const { return &vertex_input_state_create_info; }

                InfoHandler& operator=(const InfoHandler &_handler) {
                    vertex_input_state_create_info = _handler.vertex_input_state_create_info;
                    return *this;
                }

                InfoHandler& operator=(InfoHandler &&_handler) {
                    vertex_input_state_create_info = _handler.vertex_input_state_create_info;
                    _handler.clear();
                    return *this;
                }
            }; // InfoHandler END


            struct InfoHandlerView : InfoHandler {
                InfoHandlerView() {}
                InfoHandlerView(const VkPipelineVertexInputStateCreateInfo &_vertex_input_state_create_info) : InfoHandler{_vertex_input_state_create_info} {}
                InfoHandlerView(const InfoHandlerView &_handler) : InfoHandler{_handler.vertex_input_state_create_info} {}
                InfoHandlerView(InfoHandlerView &&_handler) : InfoHandler{_handler.vertex_input_state_create_info} {
                    _handler.clear();
                }
                ~InfoHandlerView() {
                    clear();
                }


                InfoHandlerView& operator=(const InfoHandlerView &_handler) {
                    vertex_input_state_create_info = _handler.vertex_input_state_create_info;
                    return *this;
                }

                InfoHandlerView& operator=(InfoHandlerView &&_handler) {
                    vertex_input_state_create_info = _handler.vertex_input_state_create_info;
                    _handler.clear();
                    return *this;
                }
            }; // InfoHandlerView END


            struct Builder {
                std::vector <VkVertexInputBindingDescription> bindings;
                std::vector <VkVertexInputAttributeDescription> attributes;

                Builder() {}
                ~Builder() {
                    clear();
                }


                void clear() {
                    bindings.clear();
                    attributes.clear();
                }

                VkPipelineVertexInputStateCreateInfo makeInfoStruct(void *_p_next = nullptr, VkPipelineVertexInputStateCreateFlags _flags = 0u) {
                    VkPipelineVertexInputStateCreateInfo __info{};
                    __info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
                    __info.flags = _flags;
                    __info.pNext = _p_next;
                    __info.vertexBindingDescriptionCount = bindings.size();
                    __info.pVertexBindingDescriptions    = bindings.size() ? bindings.data() : nullptr;
                    __info.vertexAttributeDescriptionCount = attributes.size();
                    __info.pVertexAttributeDescriptions    = attributes.size() ? attributes.data() : nullptr;

                    return __info;
                }

                VkPipelineVertexInputStateCreateInfo makeInfoStructDCopy(void *_p_next = nullptr, VkPipelineVertexInputStateCreateFlags _flags = 0u) {
                    VkPipelineVertexInputStateCreateInfo __info{};
                    __info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
                    __info.flags = _flags;
                    __info.pNext = _p_next;
                    __info.vertexBindingDescriptionCount = bindings.size();
                    __info.vertexAttributeDescriptionCount = attributes.size();

                    if (bindings.size()) {
                        VkVertexInputBindingDescription *__descr_ptr = new VkVertexInputBindingDescription[bindings.size()];
                        for (uint32_t i = 0u; i < bindings.size(); ++i)
                            __descr_ptr[i] = bindings[i];
                        __info.pVertexBindingDescriptions = __descr_ptr;
                    }
                    if (attributes.size()) {
                        VkVertexInputAttributeDescription *__descr_ptr = new VkVertexInputAttributeDescription[attributes.size()];
                        for (uint32_t i = 0u; i < attributes.size(); ++i)
                            __descr_ptr[i] = attributes[i];
                        __info.pVertexAttributeDescriptions = __descr_ptr;
                    }

                    return __info;
                }

                void setBindings(const std::vector<VkVertexInputBindingDescription> &_bindings) {
                    bindings = _bindings;
                }

                void setAttributes(const std::vector<VkVertexInputAttributeDescription> &_attributes) {
                    attributes = _attributes;
                }
            }; // Builder END
        }; // VertexInputState END


//  ============  Pipeline Input Assembly State  ============  \\

        namespace InputAssemblyState {
            struct SctBasic {
                VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};

                SctBasic() {}
                SctBasic(const SctBasic &_stct) : input_assembly_state_create_info{_stct.input_assembly_state_create_info} {}
                SctBasic(const VkPipelineInputAssemblyStateCreateInfo & _input_assembly_state_create_info)
                                : input_assembly_state_create_info{_input_assembly_state_create_info} {}
                ~SctBasic() {
                    clearStruct();
                }


                void clearStruct() {
                    input_assembly_state_create_info = VkPipelineInputAssemblyStateCreateInfo{};
                    input_assembly_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
                }

                const VkPipelineInputAssemblyStateCreateInfo *ptr() const { return &input_assembly_state_create_info; }

                void setTopology(VkPrimitiveTopology _topology_type) {
                    input_assembly_state_create_info.topology = _topology_type;
                }

                void setRestart(bool _primitive_restart) {
                    input_assembly_state_create_info.primitiveRestartEnable = VkBool32(_primitive_restart);
                }
            }; // SctBasic END
        }; // InputAssemblyState END


//  ============  Pipeline Viewport State  ============  \\

        namespace ViewportState {
            struct InfoHandler {
                VkPipelineViewportStateCreateInfo viewport_state_create_info{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};

                InfoHandler() {}
                InfoHandler(const VkPipelineViewportStateCreateInfo &_viewport_state_create_info)
                                : viewport_state_create_info{_viewport_state_create_info} {}
                InfoHandler(const InfoHandler &_handler) : viewport_state_create_info{_handler.viewport_state_create_info} {}
                InfoHandler(InfoHandler &&_handler) : viewport_state_create_info{_handler.viewport_state_create_info} {
                    _handler.clear();
                }
                virtual ~InfoHandler() {
                    deletePointerMembers();
                    clear();
                }


                void deletePointerMembers() {
                    if (viewport_state_create_info.viewportCount > 0u && viewport_state_create_info.pViewports != nullptr)
                        delete[] viewport_state_create_info.pViewports;
                    if (viewport_state_create_info.scissorCount > 0u && viewport_state_create_info.pScissors != nullptr)
                        delete[] viewport_state_create_info.pScissors;
                }

                void clear() {
                    viewport_state_create_info = VkPipelineViewportStateCreateInfo{};
                    viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
                }

                const VkPipelineViewportStateCreateInfo* ptr() const { return &viewport_state_create_info; }


                InfoHandler& operator=(const InfoHandler &_handler) {
                    viewport_state_create_info = _handler.viewport_state_create_info;
                    return *this;
                }

                InfoHandler& operator=(InfoHandler &&_handler) {
                    viewport_state_create_info = _handler.viewport_state_create_info;
                    _handler.clear();
                    return *this;
                }
            }; // InfoHandler END


            struct InfoHandlerView : InfoHandler {
                InfoHandlerView() {}
                InfoHandlerView(const VkPipelineViewportStateCreateInfo &_viewport_state_create_info) : InfoHandler{_viewport_state_create_info} {}
                InfoHandlerView(const InfoHandlerView &_handler) : InfoHandler{_handler.viewport_state_create_info} {}
                InfoHandlerView(InfoHandlerView &&_handler) : InfoHandler{_handler.viewport_state_create_info} {
                    _handler.clear();
                }
                ~InfoHandlerView() {
                    clear();
                }


                InfoHandlerView& operator=(const InfoHandlerView &_handler) {
                    viewport_state_create_info = _handler.viewport_state_create_info;
                    return *this;
                }

                InfoHandlerView& operator=(InfoHandlerView &&_handler) {
                    viewport_state_create_info = _handler.viewport_state_create_info;
                    _handler.clear();
                    return *this;
                }
            }; // InfoHandlerView END


            struct Builder {
                std::vector<VkViewport> viewports;
                std::vector<VkRect2D> scissors;
                uint32_t viewports_dynamic_count = 0u, scissors_dynamic_count = 0u;
                // counts can be set with nullptr pointers - dynamic mode

                Builder() {}
                Builder(const Builder &_builder) : viewports{_builder.viewports}, scissors{_builder.scissors} {}
                ~Builder() {
                    clear();
                }

                void clear() {
                    viewports.clear();
                    scissors.clear();
                }

                VkPipelineViewportStateCreateInfo makeInfoStruct(void *_p_next = nullptr, VkPipelineViewportStateCreateFlags _flags = 0u) {
                    VkPipelineViewportStateCreateInfo __info{};
                    __info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
                    __info.flags = _flags;
                    __info.pNext = _p_next;
                    __info.viewportCount = viewports_dynamic_count;
                    if (!viewports_dynamic_count && viewports.size()) {
                        __info.viewportCount = viewports.size();
                        __info.pViewports    = viewports.data();
                    }
                    __info.scissorCount  = scissors_dynamic_count;
                    if (!scissors_dynamic_count && scissors.size()) {
                        __info.scissorCount  =  scissors.size();
                        __info.pScissors     =  scissors.data();
                    }

                    return __info;
                }

                VkPipelineViewportStateCreateInfo makeInfoStructDCopy(void *_p_next = nullptr, VkPipelineViewportStateCreateFlags _flags = 0u) {
                    VkPipelineViewportStateCreateInfo __info{};
                    __info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
                    __info.flags = _flags;
                    __info.pNext = _p_next;
                    __info.viewportCount = viewports_dynamic_count;
                    if (!viewports_dynamic_count && viewports.size()) {
                        __info.viewportCount = viewports.size();

                        VkViewport *__viewport_ptr = new VkViewport[__info.viewportCount];
                        for (uint32_t i = 0u; i < __info.viewportCount; ++i)
                            __viewport_ptr[i] = viewports[i];
                        __info.pViewports = __viewport_ptr;
                    }
                    __info.scissorCount  = scissors_dynamic_count;
                    if (!scissors_dynamic_count && scissors.size()) {
                        __info.scissorCount  =  scissors.size();

                        VkRect2D  *__scissor_ptr = new VkRect2D[__info.scissorCount];
                        for (uint32_t i = 0u; i < __info.scissorCount; ++i)
                            __scissor_ptr[i] = scissors[i];
                        __info.pScissors = __scissor_ptr;
                    }

                    return __info;
                }


                void setViewportsDynamic(uint32_t _viewports_count) {
                    viewports.clear();
                    viewports_dynamic_count = _viewports_count;
                }

                void setScissorsDynamic(uint32_t _scissors_count) {
                    scissors.clear();
                    scissors_dynamic_count = _scissors_count;
                }

                void setViewports(const std::vector<Viewport::SctBasic> &_viewports) {
                    viewports.resize(_viewports.size());
                    for (uint32_t i = 0u; i < _viewports.size(); ++i)
                        viewports[i] = _viewports[i].viewport;
                }

                void setScissors(const std::vector<Rect2D::SctBasic> &_scissors) {
                    scissors.resize(_scissors.size());
                    for (uint32_t i = 0u; i < _scissors.size(); ++i)
                        scissors[i] = _scissors[i].rectangle;
                }

                void setFromSwapchainExtent(VkExtent2D _extent) {
                    setViewports({ {_extent, 0.f, 0.f, 0.f, 1.f} });
                    setScissors ({ {_extent, {0, 0}} });
                }


                void addViewports(const std::vector<Viewport::SctBasic> &_viewports) {
                    viewports.reserve(viewports.size() + _viewports.size());
                    for (const auto &__viewport : _viewports)
                        viewports.push_back(__viewport.viewport);
                }

                void addScissors(const std::vector<Rect2D::SctBasic> &_scissors) {
                    scissors.reserve(scissors.size() + _scissors.size());
                    for (const auto &__scissor : _scissors)
                        scissors.push_back(__scissor.rectangle);
                }
            }; // Builder END
        }; // ViewportState END


//  ============  Pipeline Rasterization State  ============  \\

        namespace RasterizationState {
            struct SctBasic {
                VkPipelineRasterizationStateCreateInfo raster_state_create_info{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};

                SctBasic() {
                    defStruct();
                }
                SctBasic(VkPolygonMode _polygon, VkCullModeFlags _cull, VkFrontFace _front) {
                    defStruct();
                    raster_state_create_info.polygonMode = _polygon;
                    raster_state_create_info.cullMode = _cull;
                    raster_state_create_info.frontFace = _front;
                }


                void clearStruct() {
                    raster_state_create_info = VkPipelineRasterizationStateCreateInfo{};
                    raster_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
                }

                // TODO:
                void defStruct() {
                    raster_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
                    raster_state_create_info.pNext = nullptr; // options exist
                    raster_state_create_info.flags = 0u; // 0 in Vk1.3
                    raster_state_create_info.depthClampEnable = VK_FALSE;
                    raster_state_create_info.rasterizerDiscardEnable = VK_FALSE;
                    raster_state_create_info.polygonMode = VK_POLYGON_MODE_MAX_ENUM;
                    raster_state_create_info.cullMode = VK_CULL_MODE_NONE;
                    raster_state_create_info.frontFace = VK_FRONT_FACE_MAX_ENUM;
                    raster_state_create_info.depthBiasEnable = VK_FALSE;
                    raster_state_create_info.depthBiasConstantFactor = 0.f;
                    raster_state_create_info.depthBiasClamp = 0.f;
                    raster_state_create_info.depthBiasSlopeFactor = 0.f;
                    raster_state_create_info.lineWidth = 1.f;
                }

                const VkPipelineRasterizationStateCreateInfo *ptr() { return &raster_state_create_info; }

                void setDepthClamp(bool _enable) {
                    raster_state_create_info.depthClampEnable = VkBool32(_enable);
                }

                void setRasterizerDiscard(bool _enable) {
                    raster_state_create_info.rasterizerDiscardEnable = VkBool32(_enable);
                }

                void setPolygonMode(VkPolygonMode _mode) {
                    raster_state_create_info.polygonMode = _mode;
                }

                void setCullMode(VkCullModeFlags _mode) {
                    raster_state_create_info.cullMode = _mode;
                }

                void setFrontFace(VkFrontFace _front) {
                    raster_state_create_info.frontFace = _front;
                }

                void setDepthBias(bool _enable, float _const_factor, float _clamp, float _slope_factor) {
                    if (!_enable) {
                        _const_factor = 0.f;
                        _clamp = 0.f;
                        _slope_factor = 0.f;
                    }
                    raster_state_create_info.depthBiasEnable = VkBool32(_enable);
                    raster_state_create_info.depthBiasConstantFactor = _const_factor;
                    raster_state_create_info.depthBiasClamp = _clamp;
                    raster_state_create_info.depthBiasSlopeFactor = _slope_factor;
                }

                void setLineWidth(float _width) {
                    raster_state_create_info.lineWidth = _width;
                }
            }; // SctBasic END
        }; // RasterizationState END


//  ============  Pipeline Multisample State  ============  \\

        namespace MultisampleState {
            // TODO
            struct SctBasic {
                VkPipelineMultisampleStateCreateInfo multisample_state_create_info{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};

                SctBasic() {
                    defStruct();
                }


                void clearStruct() {
                    multisample_state_create_info = VkPipelineMultisampleStateCreateInfo{};
                    multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
                }

                void defStruct() {
                    multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
                    multisample_state_create_info.pNext = nullptr;
                    multisample_state_create_info.flags = 0u;
                    multisample_state_create_info.sampleShadingEnable = VK_FALSE;
                    multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
                    multisample_state_create_info.minSampleShading = 1.0f; // Optional
                    multisample_state_create_info.pSampleMask = nullptr; // Optional
                    multisample_state_create_info.alphaToCoverageEnable = VK_FALSE; // Optional
                    multisample_state_create_info.alphaToOneEnable = VK_FALSE; // Optional
                }

                const VkPipelineMultisampleStateCreateInfo *ptr() { return &multisample_state_create_info; }
            }; // SctBasic END
        }; // MultisampleState END


//  ============  Pipeline Depth Stencil State  ============  \\

        namespace DepthStencilState {
            // TODO
            struct SctBasic {
                VkPipelineDepthStencilStateCreateInfo depth_stencil_state_create_info{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};

                SctBasic() {
                    defStruct();
                }


                void clearStruct() {
                    depth_stencil_state_create_info = VkPipelineDepthStencilStateCreateInfo{};
                    depth_stencil_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
                }

                // TODO
                void defStruct() {
                    depth_stencil_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
                    depth_stencil_state_create_info.pNext = nullptr;
                    depth_stencil_state_create_info.flags = 0u;
                    depth_stencil_state_create_info.depthTestEnable = VK_FALSE;
                    depth_stencil_state_create_info.depthWriteEnable = VK_FALSE;
                    depth_stencil_state_create_info.depthCompareOp;
                    depth_stencil_state_create_info.depthBoundsTestEnable = VK_FALSE;
                    depth_stencil_state_create_info.stencilTestEnable = VK_FALSE;
                    depth_stencil_state_create_info.front;
                    depth_stencil_state_create_info.back;
                    depth_stencil_state_create_info.minDepthBounds;
                    depth_stencil_state_create_info.maxDepthBounds;
                }
            }; // SctBasic END
        }; // DepthStencilState END


//  ============  Pipeline Color Blend Attachment State  ============  \\

        namespace ColorBlendAttachmentState {
            struct SctBasic {
                VkPipelineColorBlendAttachmentState attachment;

                SctBasic() {
                    defStruct();
                }


                void clearStruct() {
                    attachment = VkPipelineColorBlendAttachmentState{};
                }

                void defStruct() {
                    // Default: Porter-Duff blend over
                    attachment.blendEnable = VK_TRUE;
                    attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
                    attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                    attachment.colorBlendOp = VK_BLEND_OP_ADD;
                    attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                    attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                    attachment.alphaBlendOp = VK_BLEND_OP_ADD;
                    attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
                }

                const VkPipelineColorBlendAttachmentState* ptr() const { return &attachment; }

                void setBlendEnable(bool _enable) {
                    attachment.blendEnable = VkBool32(_enable);
                }

                void setColorWriteMask(VkColorComponentFlags _colors) {
                    attachment.colorWriteMask = _colors;
                }

                void setColorBlend(VkBlendOp _op, VkBlendFactor _src_factor, VkBlendFactor _dst_factor) {
                    attachment.colorBlendOp = _op;
                    attachment.srcColorBlendFactor = _src_factor;
                    attachment.dstColorBlendFactor = _dst_factor;
                }

                void setAlphaBlend(VkBlendOp _op, VkBlendFactor _src_factor, VkBlendFactor _dst_factor) {
                    attachment.alphaBlendOp = _op;
                    attachment.srcAlphaBlendFactor = _src_factor;
                    attachment.dstAlphaBlendFactor = _dst_factor;
                }
            }; // SctBasic END
        }; // ColorBlendAttachmentState END


//  ============  Pipeline Color Blend State  ============  \\

        namespace ColorBlendState {
            struct InfoHandler {
                VkPipelineColorBlendStateCreateInfo color_blend_state_create_info{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};

                InfoHandler() {}
                InfoHandler(const VkPipelineColorBlendStateCreateInfo &_color_blend_state_create_info)
                                : color_blend_state_create_info{_color_blend_state_create_info} {}
                InfoHandler(const InfoHandler &_handler) : color_blend_state_create_info{_handler.color_blend_state_create_info} {}
                InfoHandler(InfoHandler &&_handler) : color_blend_state_create_info{_handler.color_blend_state_create_info} {
                    _handler.clear();
                }
                ~InfoHandler() {
                    deletePointerMembers();
                    clear();
                }


                void clear() {
                    color_blend_state_create_info = VkPipelineColorBlendStateCreateInfo{};
                    color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
                }

                void deletePointerMembers() {
                    if (color_blend_state_create_info.attachmentCount > 0u && color_blend_state_create_info.pAttachments != nullptr) {
                        delete[] color_blend_state_create_info.pAttachments;
                    }
                }

                const VkPipelineColorBlendStateCreateInfo* ptr() const { return &color_blend_state_create_info; }

                InfoHandler& operator=(const InfoHandler &_handler) {
                    color_blend_state_create_info = _handler.color_blend_state_create_info;
                    return *this;
                }

                InfoHandler& operator=(InfoHandler &&_handler) {
                    color_blend_state_create_info = _handler.color_blend_state_create_info;
                    _handler.clear();
                    return *this;
                }
            }; // InfoHandler END


            struct InfoHandlerView : InfoHandler {
                InfoHandlerView() {}
                InfoHandlerView(const VkPipelineColorBlendStateCreateInfo &_color_blend_state_create_info) : InfoHandler{_color_blend_state_create_info} {}
                InfoHandlerView(const InfoHandlerView &_handler) : InfoHandler{_handler.color_blend_state_create_info} {}
                InfoHandlerView(InfoHandlerView &&_handler) : InfoHandler{_handler.color_blend_state_create_info} {
                    _handler.clear();
                }
                ~InfoHandlerView() {
                    clear();
                }


                InfoHandlerView& operator=(const InfoHandlerView &_handler) {
                    color_blend_state_create_info = _handler.color_blend_state_create_info;
                    return *this;
                }

                InfoHandlerView& operator=(InfoHandlerView &&_handler) {
                    color_blend_state_create_info = _handler.color_blend_state_create_info;
                    _handler.clear();
                    return *this;
                }
            }; // InfoHandlerView END


            struct Builder {
                std::vector <VkPipelineColorBlendAttachmentState> attachments;
                Vec4 blend_constants;
                VkLogicOp logic_op{VK_LOGIC_OP_MAX_ENUM};

                Builder() {}
                Builder(const Builder &_builder) : attachments{_builder.attachments}, logic_op{_builder.logic_op} {}
                ~Builder() {
                    clear();
                }

                void clear() {
                    attachments.clear();
                    blend_constants = 0.f;
                    logic_op = VK_LOGIC_OP_MAX_ENUM;
                }

                VkPipelineColorBlendStateCreateInfo makeInfoStruct(void *_p_next = nullptr, VkPipelineColorBlendStateCreateFlags _flags = 0u) {
                    VkPipelineColorBlendStateCreateInfo __info{};
                    __info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
                    __info.flags = _flags;
                    __info.pNext = _p_next;
                    __info.logicOpEnable = VkBool32(logic_op != VK_LOGIC_OP_MAX_ENUM);
                    __info.logicOp = logic_op;
                    __info.attachmentCount = attachments.size();
                    __info.pAttachments = attachments.size() ? attachments.data() : nullptr;
                    __info.blendConstants[0] = blend_constants[0];
                    __info.blendConstants[1] = blend_constants[1];
                    __info.blendConstants[2] = blend_constants[2];
                    __info.blendConstants[3] = blend_constants[3];

                    return __info;
                }

                VkPipelineColorBlendStateCreateInfo makeInfoStructDCopy(void *_p_next = nullptr, VkPipelineColorBlendStateCreateFlags _flags = 0u) {
                    VkPipelineColorBlendStateCreateInfo __info{};
                    __info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
                    __info.flags = _flags;
                    __info.pNext = _p_next;
                    __info.logicOpEnable = VkBool32(logic_op != VK_LOGIC_OP_MAX_ENUM);
                    __info.logicOp = logic_op;

                    __info.attachmentCount = attachments.size();
                    if (attachments.size()) {
                        VkPipelineColorBlendAttachmentState *__attach_ptr = new VkPipelineColorBlendAttachmentState[attachments.size()];
                        for (uint32_t i = 0u; i < attachments.size(); ++i)
                            __attach_ptr[i] = attachments[i];
                        __info.pAttachments = __attach_ptr;
                    }

                    __info.blendConstants[0] = blend_constants[0];
                    __info.blendConstants[1] = blend_constants[1];
                    __info.blendConstants[2] = blend_constants[2];
                    __info.blendConstants[3] = blend_constants[3];

                    return __info;
                }

                void setLogicOp(VkLogicOp _op) {
                    logic_op = _op;
                }

                void setBlendConstants(const Vec4 &_blend_constants) {
                    blend_constants = _blend_constants;
                }

                void setAttachments(const std::vector<ColorBlendAttachmentState::SctBasic> &_states) {
                    attachments.resize(_states.size());
                    for (uint32_t i = 0u; i < _states.size(); ++i)
                        attachments[i] = _states[i].attachment;
                }

                void addAttachments(const std::vector<ColorBlendAttachmentState::SctBasic> &_states) {
                    attachments.reserve(attachments.size() + _states.size());
                    for (uint32_t i = 0u; i < _states.size(); ++i)
                        attachments.push_back(_states[i].attachment);
                }

                void clearLogicOp() {
                    logic_op = VK_LOGIC_OP_MAX_ENUM;
                }
            }; // Builder END
        }; // ColorBlendState END


//  ============  Pipeline Tessellation State  ============  \\

        struct TessellationState {
            struct SctBasic {
                VkPipelineTessellationStateCreateInfo tesselation_state_create_info{VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO};

                SctBasic() {}
                SctBasic(const SctBasic &_stct) : tesselation_state_create_info{_stct.tesselation_state_create_info} {}
                ~SctBasic() {
                    clearStruct();
                }


                void clearStruct() {
                    tesselation_state_create_info = VkPipelineTessellationStateCreateInfo{};
                    tesselation_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
                }

                const VkPipelineTessellationStateCreateInfo* ptr() const { return &tesselation_state_create_info; }

                void setPatchControlPoints(uint32_t _points_num) {
                    tesselation_state_create_info.patchControlPoints = _points_num;
                }
            }; // SctBasic END
        }; // TessellationState END


//  ============  Vulkan Graphics Pipeline  ============  \\

        namespace Graphics {
            struct Builder : BuilderBase {
                VkDevice device = VK_NULL_HANDLE;
                VkPipelineLayout layout = VK_NULL_HANDLE;
                VkRenderPass render_pass = VK_NULL_HANDLE;
                VkPipelineCache pipeline_cache = VK_NULL_HANDLE;
                VkPipeline base_pipeline_handle = VK_NULL_HANDLE;
                const VkPipelineVertexInputStateCreateInfo   *   vertex_input_state = VK_NULL_HANDLE;
                const VkPipelineInputAssemblyStateCreateInfo * input_assembly_state = VK_NULL_HANDLE;
                const VkPipelineTessellationStateCreateInfo  *   tessellation_state = VK_NULL_HANDLE;
                const VkPipelineViewportStateCreateInfo      *       viewport_state = VK_NULL_HANDLE;
                const VkPipelineRasterizationStateCreateInfo *  rasterization_state = VK_NULL_HANDLE;
                const VkPipelineMultisampleStateCreateInfo   *    multisample_state = VK_NULL_HANDLE;
                const VkPipelineDepthStencilStateCreateInfo  *  depth_btencil_state = VK_NULL_HANDLE;
                const VkPipelineColorBlendStateCreateInfo    *    color_blend_state = VK_NULL_HANDLE;
                const VkPipelineDynamicStateCreateInfo       *        dynamic_state = VK_NULL_HANDLE;
                std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
                int32_t base_pipeline_index = 0;
                uint32_t subpass = 0u;

                Builder(const Device::Handler &_device = {},
                        const PipelineLayout::Handler &_layout = {},
                        const PipelineCache::Handler &_cache = {})
                                : device{_device.handler}, layout{_layout.handler}, pipeline_cache{_cache.handler} {}
                ~Builder() {
                    clear();
                }


                void clear() {
                    device = VK_NULL_HANDLE;
                    layout = VK_NULL_HANDLE;
                    render_pass = VK_NULL_HANDLE;
                    pipeline_cache = VK_NULL_HANDLE;
                    base_pipeline_handle = VK_NULL_HANDLE;
                    vertex_input_state   = VK_NULL_HANDLE;
                    input_assembly_state = VK_NULL_HANDLE;
                    tessellation_state   = VK_NULL_HANDLE;
                    viewport_state       = VK_NULL_HANDLE;
                    rasterization_state  = VK_NULL_HANDLE;
                    multisample_state    = VK_NULL_HANDLE;
                    depth_btencil_state  = VK_NULL_HANDLE;
                    color_blend_state    = VK_NULL_HANDLE;
                    dynamic_state        = VK_NULL_HANDLE;
                    base_pipeline_index = 0;
                    shader_stages.clear();
                    subpass = 0u;
                }

                ObjectInfoType createInfoObject(void *_p_next = nullptr, VkPipelineCreateFlags _flags = 0u,
                                              const VkAllocationCallbacks * _create_cb = nullptr) {
                    return { create(_p_next, _flags, _create_cb), VK_OBJECT_TYPE_PIPELINE };
                }

                ObjectCommonType create(void *_p_next = nullptr, VkPipelineCreateFlags _flags = 0u,
                                        const VkAllocationCallbacks * _create_cb = nullptr) override {
                    VkGraphicsPipelineCreateInfo __info = createInfoStruct(_p_next, _flags);
                    VkPipeline __pipeline = VK_NULL_HANDLE;
                    if (vkCreateGraphicsPipelines(device, pipeline_cache, 1, &__info, _create_cb, &__pipeline) != VK_SUCCESS)
                        throw std::runtime_error(SVKFW_WRAPERR(" Graphics Pipeline :: create", "failed to create graphics pipeline"));

                    return __pipeline;
                }

                VkPipeline createObject(void *_p_next = nullptr, VkPipelineCreateFlags _flags = 0u,
                                const VkAllocationCallbacks * _create_cb = nullptr) {
                    return (VkPipeline) create(_p_next, _flags, _create_cb);
                }

                VkGraphicsPipelineCreateInfo createInfoStruct(void *_p_next = nullptr, VkPipelineCreateFlags _flags = 0u) {
                    VkGraphicsPipelineCreateInfo __info;
                    __info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
                    __info.pNext = _p_next;
                    __info.flags = _flags;
                    __info.stageCount = shader_stages.size();
                    __info.pStages = shader_stages.size() ? shader_stages.data() : nullptr;
                    __info.pVertexInputState = vertex_input_state;
                    __info.pInputAssemblyState = input_assembly_state;
                    __info.pTessellationState = tessellation_state;
                    __info.pViewportState = viewport_state;
                    __info.pRasterizationState = rasterization_state;
                    __info.pMultisampleState = multisample_state;
                    __info.pDepthStencilState = depth_btencil_state;
                    __info.pColorBlendState = color_blend_state;
                    __info.pDynamicState = dynamic_state;
                    __info.layout = layout;
                    __info.renderPass = render_pass;
                    __info.subpass = subpass;

                    return __info;
                }


                void setShaderStages(const std::vector<VkPipelineShaderStageCreateInfo> &_shader_stages) {
                    shader_stages = _shader_stages;
                }

                void setShaderStages(const ShaderStage::InfoVecHandler &_shader_stages) {
                    shader_stages = _shader_stages.shader_stages;
                }

                void setVertexInputState(const VkPipelineVertexInputStateCreateInfo *_vertex_input_state) {
                    vertex_input_state = _vertex_input_state;
                }

                void setInputAssemblyState(const VkPipelineInputAssemblyStateCreateInfo *_input_assembly_state) {
                    input_assembly_state = _input_assembly_state;
                }

                void setTessellationState(const VkPipelineTessellationStateCreateInfo *_tessellation_state) {
                    tessellation_state = _tessellation_state;
                }

                void setViewportState(const VkPipelineViewportStateCreateInfo *_viewport_state) {
                    viewport_state = _viewport_state;
                }

                void setRasterizationState(const VkPipelineRasterizationStateCreateInfo *_rasterization_state) {
                    rasterization_state = _rasterization_state;
                }

                void setMultisampleState(const VkPipelineMultisampleStateCreateInfo *_multisample_state) {
                    multisample_state = _multisample_state;
                }

                void setDepthStencilState(const VkPipelineDepthStencilStateCreateInfo *_depth_btencil_state) {
                    depth_btencil_state = _depth_btencil_state;
                }

                void setColorBlendState(const VkPipelineColorBlendStateCreateInfo *_color_blend_state) {
                    color_blend_state = _color_blend_state;
                }

                void setDynamicState(const VkPipelineDynamicStateCreateInfo *_dynamic_state) {
                    dynamic_state = _dynamic_state;
                }

                void setRenderPassWithSubpass(VkRenderPass _render_pass, uint32_t _subpass) {
                    render_pass = _render_pass;
                    subpass = _subpass;
                }

                void setRenderPassWithSubpass(const RenderPass::Handler &_render_pass, uint32_t _subpass) {
                    render_pass = _render_pass.handler;
                    subpass = _subpass;
                }

                void setBasePipeline(int32_t _base_pipeline_index = 0, const Pipeline::Handler &_base_pipeline_handle = {}) {
                    base_pipeline_handle = _base_pipeline_handle.handler;
                    base_pipeline_index = _base_pipeline_index;
                }

                void setPipelineLayout(const PipelineLayout::Handler &_layout) {
                    layout = _layout.handler;
                }

                void setPipelineCache(const PipelineCache::Handler &_cache) {
                    pipeline_cache = _cache.handler;
                }

                void setDevice(const Device::Handler &_device) {
                    device = _device.handler;
                }
            }; // Builder END
        }; // Graphics END


//  ============  Vulkan Compute Pipeline  ============  \\

        namespace Compute {
            struct Builder : BuilderBase {
                VkDevice device = VK_NULL_HANDLE;
                VkPipelineLayout layout = VK_NULL_HANDLE;
                VkPipelineCache pipeline_cache = VK_NULL_HANDLE;
                VkPipeline base_pipeline_handle = VK_NULL_HANDLE;
                VkPipelineShaderStageCreateInfo compute_shader_stage;
                int32_t base_pipeline_index = 0;

                Builder(const Device::Handler &_device = {},
                        const PipelineLayout::Handler &_layout = {},
                        const PipelineCache::Handler &_cache = {})
                                : device{_device.handler}, layout{_layout.handler},
                                  pipeline_cache{_cache.handler} {}
                ~Builder() {
                    clear();
                }


                void clear() {
                    device = VK_NULL_HANDLE;
                    layout = VK_NULL_HANDLE;
                    pipeline_cache = VK_NULL_HANDLE;
                    base_pipeline_index = 0;
                    compute_shader_stage = VkPipelineShaderStageCreateInfo{};
                    compute_shader_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                }


                ObjectInfoType createInfoObject(void *_p_next = nullptr, VkPipelineCreateFlags _flags = 0u,
                                              const VkAllocationCallbacks * _create_cb = nullptr) {
                    return { create(_p_next, _flags, _create_cb), VK_OBJECT_TYPE_PIPELINE };
                }

                ObjectCommonType create(void *_p_next = nullptr, VkPipelineCreateFlags _flags = 0u,
                          const VkAllocationCallbacks * _create_cb = nullptr) {
                    VkComputePipelineCreateInfo __info = createInfoStruct(_p_next, _flags);
                    VkPipeline __pipeline = VK_NULL_HANDLE;
                    if (vkCreateComputePipelines(device, pipeline_cache, 1, &__info, _create_cb, &__pipeline) != VK_SUCCESS)
                        throw std::runtime_error(SVKFW_WRAPERR(" Compute Pipeline :: create", "failed to create compute pipeline"));

                    return __pipeline;
                }

                VkPipeline createObject(void *_p_next = nullptr, VkPipelineCreateFlags _flags = 0u,
                                const VkAllocationCallbacks * _create_cb = nullptr) {
                    return (VkPipeline) create(_p_next, _flags, _create_cb);
                }

                VkComputePipelineCreateInfo createInfoStruct(void *_p_next = nullptr, VkPipelineCreateFlags _flags = 0u) {
                    VkComputePipelineCreateInfo __info;
                    __info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
                    __info.pNext = _p_next;
                    __info.flags = _flags;
                    __info.stage = compute_shader_stage;
                    __info.layout = layout;
                    __info.basePipelineHandle = base_pipeline_handle;
                    __info.basePipelineIndex = base_pipeline_index;

                    return __info;
                }


            // Setters:

                void setDevice(const Device::Handler &_device) {
                    device = _device.handler;
                }

                void setPipelineLayout(const PipelineLayout::Handler &_layout) {
                    layout = _layout.handler;
                }

                void setPipelineCache(const PipelineCache::Handler &_cache) {
                    pipeline_cache = _cache.handler;
                }

                void setBasePipeline(int32_t _base_pipeline_index = 0, const Pipeline::Handler &_base_pipeline_handle = {}) {
                    base_pipeline_handle = _base_pipeline_handle.handler;
                    base_pipeline_index = _base_pipeline_index;
                }
            }; // Builder END
        }; // Compute END
    }; // Pipeline END


//  ============  Vulkan Framebuffer  ============  \\

    namespace Framebuffer {
        struct Builder : BuilderBase {
        // Data:

            std::vector<VkImageView> attachments;
            VkDevice device = VK_NULL_HANDLE;
            VkRenderPass render_pass = VK_NULL_HANDLE;
            VkExtent2D buffer_size{0u, 0u};
            uint32_t layers = 0u;


        // Constructors:

            Builder(const Device::Handler &_device = {}, const RenderPass::Handler &_render_pass = {})
                            : device{_device.handler}, render_pass{_render_pass.handler} {}
            ~Builder() {
                clear();
            }


        // Clear state:

            void clear() {
                attachments.clear();
                device = VK_NULL_HANDLE;
                render_pass = VK_NULL_HANDLE;
                buffer_size = {0u, 0u};
                layers = 0u;
            }


            ObjectInfoType createInfoObject(void *_p_next = nullptr, VkFramebufferCreateFlags _flags = 0u,
                                            const VkAllocationCallbacks * _create_cb = nullptr) {
                return { create(_p_next, _flags, _create_cb), VK_OBJECT_TYPE_FRAMEBUFFER };
            }

            ObjectCommonType create(void *_p_next = nullptr, VkFramebufferCreateFlags _flags = 0u,
                        const VkAllocationCallbacks * _create_cb = nullptr) override {
                VkFramebufferCreateInfo __info = createInfoStruct(_p_next, _flags);
                VkFramebuffer __framebuffer = VK_NULL_HANDLE;
                if (vkCreateFramebuffer(device, &__info, _create_cb, &__framebuffer) != VK_SUCCESS)
                    throw std::runtime_error(SVKFW_WRAPERR(" Framebuffer :: create", "failed to create framebuffer"));

                return __framebuffer;
            }

            VkFramebuffer createObject(void *_p_next = nullptr, VkFramebufferCreateFlags _flags = 0u,
                        const VkAllocationCallbacks * _create_cb = nullptr) {
                return (VkFramebuffer) create(_p_next, _flags, _create_cb);
            }

            VkFramebufferCreateInfo createInfoStruct(void *_p_next = nullptr, VkFramebufferCreateFlags _flags = 0u) {
                VkFramebufferCreateInfo __info;
                __info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                __info.pNext = _p_next;
                __info.flags = _flags;
                __info.renderPass = render_pass;
                __info.attachmentCount = attachments.size();
                __info.pAttachments = attachments.size() ? attachments.data() : nullptr;
                __info.width  = buffer_size.width;
                __info.height = buffer_size.height;
                __info.layers = layers;

                return __info;
            }


        // Setters:

            void setDevice(const Device::Handler &_device) {
                device = _device.handler;
            }

            void setRenderPass(const RenderPass::Handler &_render_pass) {
                render_pass = _render_pass.handler;
            }

            void setAttachments(const std::vector<VkImageView> &_attachments) {
                attachments = _attachments;
            }

            void setAttachments(const ImageView::VecHandler &_attachments) {
                attachments = _attachments.handlers;
            }

            void setAttachment(const ImageView::Handler &_attachment) {
                attachments = { _attachment.handler };
            }

            void setBufferSize(VkExtent2D _buffer_size) {
                buffer_size = _buffer_size;
            }

            void setLayers(uint32_t _layers) {
                layers = _layers;
            }

            void setBufferSizeAndLayers(VkExtent2D _buffer_size, uint32_t _layers) {
                buffer_size = _buffer_size;
                layers = _layers;
            }
        }; // Builder END
    }; // Framebuffer END


// ============  Vulkan Command Pool  ============  \\

    namespace CommandPool {
        struct Builder : BuilderBase {
        // Data:

            VkDevice device = VK_NULL_HANDLE;
            uint32_t queue_family = -1u;


        // Constructors:

            Builder(const Device::Handler &_device = {}, const QueueFamily::Handler &_queue_family = {})
                            : device{_device.handler}, queue_family{_queue_family.index} {}
            ~Builder() {
                clear();
            }


        // Clear state:

            void clear() {
                device = VK_NULL_HANDLE;
                queue_family = -1u;
            }


            ObjectInfoType createInfoObject(void *_p_next = nullptr, VkCommandPoolCreateFlags _flags = 0u,
                                            const VkAllocationCallbacks * _create_cb = nullptr) {
                return { create(_p_next, _flags, _create_cb), VK_OBJECT_TYPE_COMMAND_POOL };
            }

            ObjectCommonType create(void *_p_next = nullptr, VkCommandPoolCreateFlags _flags = 0u,
                        const VkAllocationCallbacks * _create_cb = nullptr) override {
                VkCommandPoolCreateInfo __info = createInfoStruct(_p_next, _flags);
                VkCommandPool __framebuffer = VK_NULL_HANDLE;
                if (vkCreateCommandPool(device, &__info, _create_cb, &__framebuffer) != VK_SUCCESS)
                    throw std::runtime_error(SVKFW_WRAPERR(" Command Pool :: create", "failed to create command pool"));

                return __framebuffer;
            }

            VkCommandPool createObject(void *_p_next = nullptr, VkCommandPoolCreateFlags _flags = 0u,
                        const VkAllocationCallbacks * _create_cb = nullptr) {
                return (VkCommandPool) create(_p_next, _flags, _create_cb);
            }

            VkCommandPoolCreateInfo createInfoStruct(void *_p_next = nullptr, VkCommandPoolCreateFlags _flags = 0u) {
                VkCommandPoolCreateInfo __info;
                __info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
                __info.pNext = _p_next;
                __info.flags = _flags;
                __info.queueFamilyIndex = queue_family;

                return __info;
            }


        // Setters:

            void setDevice(const Device::Handler &_device) {
                device = _device.handler;
            }

            void setQueueFamily(const QueueFamily::Handler &_queue_family) {
                queue_family = _queue_family.index;
            }
        }; // Builder
    }; // CommandPool END


    namespace CommandBufferInheritance {
        struct InfoHandler {
        // Data:

            VkCommandBufferInheritanceInfo inheritance_info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO};


        // Constructors:

            InfoHandler() {}
            InfoHandler(const VkCommandBufferInheritanceInfo &_inheritance_info) : inheritance_info{_inheritance_info} {}
            InfoHandler(const InfoHandler &_handler) : inheritance_info{_handler.inheritance_info} {}
            InfoHandler(InfoHandler &&_handler) : inheritance_info{_handler.inheritance_info} {
                _handler.clear();
            }
            virtual ~InfoHandler() {
                clear();
            }


        // Clear state:

            void clear() {
                inheritance_info = VkCommandBufferInheritanceInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO};
            }


        // Pointer access:

            const VkCommandBufferInheritanceInfo* ptr() const { return &inheritance_info; }


        // Assignment:

            InfoHandler& operator=(const InfoHandler &_handler) {
                inheritance_info = _handler.inheritance_info;
                return *this;
            }

            InfoHandler& operator=(InfoHandler &&_handler) {
                inheritance_info = _handler.inheritance_info;
                _handler.clear();
                return *this;
            }
        }; // InfoHandler END

        using InfoHandler = InfoHandler;


        struct Builder {
        // Data:

            VkRenderPass render_pass = VK_NULL_HANDLE;
            VkFramebuffer framebuffer = VK_NULL_HANDLE;
            uint32_t subpass = 0u;
            VkBool32 occlusion_query = VK_FALSE;
            VkQueryControlFlags query = 0u;
            VkQueryPipelineStatisticFlags pipeline_statistics = 0u;


        // Constructors:

            Builder(const RenderPass::Handler &_render_pass = {},
                    const Framebuffer::Handler &_framebuffer = {})
                            : render_pass{_render_pass.handler}, framebuffer{_framebuffer.handler} {}
            ~Builder() {
                clear();
            }


        // Clear state:

            void clear() {
                render_pass = VK_NULL_HANDLE;
                framebuffer = VK_NULL_HANDLE;
                subpass = 0u;
                occlusion_query = VK_FALSE;
                query = 0u;
                pipeline_statistics = 0u;
            }


        // Create handlers/CreateInfo struct:

            VkCommandBufferInheritanceInfo makeInfoStruct(void *_p_next = nullptr) {
                VkCommandBufferInheritanceInfo __info;
                __info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
                __info.pNext = _p_next;
                __info.renderPass = render_pass;
                __info.subpass = subpass;
                __info.framebuffer = framebuffer;
                __info.occlusionQueryEnable = occlusion_query;
                __info.queryFlags = query;
                __info.pipelineStatistics = pipeline_statistics;

                return __info;
            }
        }; // Builder END
    }; // CommandBufferInheritance


    namespace CommandBuffer {
        struct Builder : BuilderBase {
            VkDevice device = VK_NULL_HANDLE;
            VkCommandPool command_pool = VK_NULL_HANDLE;
            VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_MAX_ENUM;
            uint32_t buffer_count = 0u;

            Builder(const Device::Handler &_device = {}, const CommandPool::Handler &_command_pool = {})
                            : device{_device.handler}, command_pool{_command_pool.handler} {}
            ~Builder() {
                clear();
            }


            void clear() {
                command_pool = VK_NULL_HANDLE;
                level = VK_COMMAND_BUFFER_LEVEL_MAX_ENUM;
                buffer_count = 0u;
            }


            ObjectInfoType createInfoObject(void *_p_next = nullptr, VkFlags _flags = 0u,
                                          const VkAllocationCallbacks * _create_cb = nullptr) {
                return { create(_p_next, _flags, _create_cb), VK_OBJECT_TYPE_COMMAND_BUFFER };
            }

            ObjectCommonType create(void *_p_next = nullptr, VkFlags _flags = 0u,
                    const VkAllocationCallbacks * _create_cb = nullptr) override {
                VkCommandBufferAllocateInfo __info = createInfoStruct(_p_next);
                VkCommandBuffer __command_buffer = VK_NULL_HANDLE;
                if (vkAllocateCommandBuffers(device, &__info, &__command_buffer) != VK_SUCCESS)
                    throw std::runtime_error(SVKFW_WRAPERR(" Command Buffer :: create", "failed to allocate command buffer"));

                return __command_buffer;
            }

            VkCommandBuffer createObject(void *_p_next = nullptr, VkFlags _flags = 0u,
                                 const VkAllocationCallbacks * _create_cb = nullptr) {
                return (VkCommandBuffer) create(_p_next, _flags, _create_cb);
            }

            VkCommandBufferAllocateInfo createInfoStruct(void *_p_next = nullptr) {
                VkCommandBufferAllocateInfo __info;
                __info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                __info.pNext = _p_next;
                __info.commandPool = command_pool;
                __info.level = level;
                __info.commandBufferCount = buffer_count;

                return __info;
            }

            void setDevice(const Device::Handler &_device) {
                device = _device.handler;
            }

            void setCommandPool(const CommandPool::Handler &_command_pool) {
                command_pool = _command_pool.handler;
            }

            void setBufferCount(uint32_t _buffer_count) {
                buffer_count = _buffer_count;
            }

            void setLevel(VkCommandBufferLevel _level) {
                level = _level;
            }

            void setLevelPrimary() {
                level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            }

            void setLevelSecondary() {
                level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
            }
        }; // Builder END
    }; // CommandBuffer END


    namespace SubmitInfo {
        struct InfoHandler {
        // Data:

            VkSubmitInfo submit_info{VK_STRUCTURE_TYPE_SUBMIT_INFO};


        // Constructors:

            InfoHandler() {}
            InfoHandler(const VkSubmitInfo &_submit_info) : submit_info{_submit_info} {}
            InfoHandler(const InfoHandler &_handler) : submit_info{_handler.submit_info} {}
            InfoHandler(InfoHandler &&_handler) : submit_info{_handler.submit_info} {
                _handler.clear();
            }
            virtual ~InfoHandler() {
                clear();
            }


        // Clear state:

            void clear() {
                submit_info = VkSubmitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
            }


        // Pointer access:

            const VkSubmitInfo* ptr() const { return &submit_info; }


        // Assignment:

            InfoHandler& operator=(const InfoHandler &_handler) {
                submit_info = _handler.submit_info;
                return *this;
            }

            InfoHandler& operator=(InfoHandler &&_handler) {
                submit_info = _handler.submit_info;
                _handler.clear();
                return *this;
            }
        }; // InfoHandler END

        using InfoHandler = InfoHandler;


        struct Builder {
        // Data:

            std::vector<VkSemaphore> wait_semaphores;
            std::vector<VkPipelineStageFlags> wait_stages;
            std::vector<VkCommandBuffer> command_buffers;
            std::vector<VkSemaphore> signal_semaphores;


        // Constructors:

            Builder(const CommandBuffer::VecHandler &_command_buffers = {},
                    const Semaphore::VecHandler &_wait_semaphores = {},
                    const Semaphore::VecHandler &_signal_semaphores = {})
                            : command_buffers{_command_buffers.handlers},
                              wait_semaphores{_wait_semaphores.handlers},
                              signal_semaphores{_signal_semaphores.handlers} {}
            ~Builder() {
                clear();
            }


        // Clear state:

            void clear() {
                wait_stages.clear();
                wait_semaphores.clear();
                command_buffers.clear();
                signal_semaphores.clear();
            }


        // Create handlers/CreateInfo struct:

            VkSubmitInfo makeInfoStruct(void *_p_next = nullptr) {
                VkSubmitInfo __info;
                __info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                __info.pNext = _p_next;
                __info.waitSemaphoreCount = wait_semaphores.size();
                __info.pWaitSemaphores = wait_semaphores.data();
                __info.pWaitDstStageMask = wait_stages.data(); // same size as waitSemaphoreCount, apparently
                __info.commandBufferCount = command_buffers.size();
                __info.pCommandBuffers = command_buffers.data();
                __info.signalSemaphoreCount = signal_semaphores.size();
                __info.pSignalSemaphores = signal_semaphores.data();

                return __info;
            }


        // Setters:

            void setCommandBuffers(const CommandBuffer::VecHandler &_command_buffers) {
                command_buffers = _command_buffers.handlers;
            }

            void setWaitSemaphores(const Semaphore::VecHandler &_wait_semaphores) {
                wait_semaphores = _wait_semaphores.handlers;
            }

            void setWaitStages(const std::vector<VkPipelineStageFlags> &_wait_stages) {
                wait_stages = _wait_stages;
            }

            void setSignalSemaphores(const Semaphore::VecHandler &_signal_semaphores) {
                signal_semaphores = _signal_semaphores.handlers;
            }
        }; // Builder END
    }; // SubmitInfo END


    namespace PresentInfoKHR {
        struct InfoHandler {
        // Data:

            VkPresentInfoKHR present_info{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};


        // Constructors:

            InfoHandler() {}
            InfoHandler(const VkPresentInfoKHR &_present_info) : present_info{_present_info} {}
            InfoHandler(const InfoHandler &_handler) : present_info{_handler.present_info} {}
            InfoHandler(InfoHandler &&_handler) : present_info{_handler.present_info} {
                _handler.clear();
            }
            virtual ~InfoHandler() {
                clear();
            }


        // Clear state:

            void clear() {
                present_info = VkPresentInfoKHR{VK_STRUCTURE_TYPE_SUBMIT_INFO};
            }


        // Pointer access:

            const VkPresentInfoKHR* ptr() const { return &present_info; }


        // Assignment:

            InfoHandler& operator=(const InfoHandler &_handler) {
                present_info = _handler.present_info;
                return *this;
            }

            InfoHandler& operator=(InfoHandler &&_handler) {
                present_info = _handler.present_info;
                _handler.clear();
                return *this;
            }
        }; // InfoHandler END

        using InfoHandler = InfoHandler;


        struct Builder {
        // Data:

            std::vector<VkSemaphore> wait_semaphores;
            std::vector<VkSwapchainKHR> swapchains;
            std::vector<uint32_t> image_indices;


        // Constructors:

            Builder(const Semaphore::VecHandler &_wait_semaphores = {},
                    const SwapchainKHR::VecHandler &_swapchains = {})
                            : wait_semaphores{_wait_semaphores.handlers},
                              swapchains{_swapchains.handlers} {}
            ~Builder() {
                clear();
            }


        // Clear state:

            void clear() {
                wait_semaphores.clear();
                swapchains.clear();
                image_indices.clear();
            }


        // Create handlers/CreateInfo struct:

            VkPresentInfoKHR makeInfoStruct(void *_p_next = nullptr) {
                VkPresentInfoKHR __info;
                __info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
                __info.pNext = _p_next;
                __info.waitSemaphoreCount = wait_semaphores.size();
                __info.pWaitSemaphores = wait_semaphores.data();
                __info.swapchainCount = swapchains.size();
                __info.pSwapchains = swapchains.data();
                __info.pImageIndices = image_indices.data();
                __info.pResults = nullptr; // TODO: Manage results here too (it is managed in Func)

                return __info;
            }


        // Setters:

            void setWaitSemaphores(const Semaphore::VecHandler &_wait_semaphores) {
                wait_semaphores = _wait_semaphores.handlers;
            }

            void setSwapchains(const SwapchainKHR::VecHandler &_swapchains) {
                swapchains = _swapchains.handlers;
            }

            void setImageIndices(const std::vector<uint32_t> &_image_indices) {
                image_indices = _image_indices;
            }
        }; // Builder END
    }; // PresentInfoKHR END


//  ============  Vulkan syncronization  ============  \\

    namespace Semaphore {
        struct Builder : BuilderBase {
            VkDevice device = VK_NULL_HANDLE;

            Builder(const Device::Handler &_device = {}) : device{_device.handler} {}
            ~Builder() {
                clear();
            }

            void clear() {
                device = VK_NULL_HANDLE;
            }

            ObjectInfoType createInfoObject(void *_p_next = nullptr, VkSemaphoreCreateFlags _flags = 0u,
                                            const VkAllocationCallbacks * _create_cb = nullptr) {
                return { create(_p_next, _flags, _create_cb), VK_OBJECT_TYPE_SEMAPHORE };
            }

            ObjectCommonType create(void *_p_next = nullptr, VkSemaphoreCreateFlags _flags = 0u,
                                    const VkAllocationCallbacks * _create_cb = nullptr) override {
                VkSemaphoreCreateInfo __info = createInfoStruct(_p_next, _flags);
                VkSemaphore __semaphore = VK_NULL_HANDLE;
                if (vkCreateSemaphore(device, &__info, _create_cb, &__semaphore) != VK_SUCCESS)
                    throw std::runtime_error(SVKFW_WRAPERR(" Semaphore :: create", "failed to create semaphore"));

                return __semaphore;
            }

            VkSemaphore createObject(void *_p_next = nullptr, VkSemaphoreCreateFlags _flags = 0u,
                             const VkAllocationCallbacks * _create_cb = nullptr) {
                return (VkSemaphore) create(_p_next, _flags, _create_cb);
            }

            VkSemaphoreCreateInfo createInfoStruct(void *_p_next = nullptr, VkSemaphoreCreateFlags _flags = 0u) {
                VkSemaphoreCreateInfo __info;
                __info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
                __info.pNext = _p_next;
                __info.flags = _flags;

                return __info;
            }

            void setDevice(const Device::Handler &_device) {
                device = _device.handler;
            }
        }; // Builder END
    }; // Semaphore END


    namespace Fence {
        struct Builder : BuilderBase {
            VkDevice device = VK_NULL_HANDLE;

            Builder(const Device::Handler &_device = {}) : device{_device.handler} {}
            ~Builder() {
                clear();
            }

            void clear() {
                device = VK_NULL_HANDLE;
            }

            ObjectInfoType createInfoObject(void *_p_next = nullptr, VkFenceCreateFlags _flags = 0u,
                                            const VkAllocationCallbacks * _create_cb = nullptr) {
                return { create(_p_next, _flags, _create_cb), VK_OBJECT_TYPE_FENCE };
            }

            ObjectCommonType create(void *_p_next = nullptr, VkFenceCreateFlags _flags = 0u,
                                    const VkAllocationCallbacks * _create_cb = nullptr) {
                VkFenceCreateInfo __info = createInfoStruct(_p_next, _flags);
                VkFence __fence = VK_NULL_HANDLE;
                if (vkCreateFence(device, &__info, _create_cb, &__fence) != VK_SUCCESS)
                    throw std::runtime_error(SVKFW_WRAPERR(" Fence :: create", "failed to create fence"));

                return __fence;
            }

            VkFence createObject(void *_p_next = nullptr, VkFenceCreateFlags _flags = 0u,
                         const VkAllocationCallbacks * _create_cb = nullptr) {
                return (VkFence) create(_p_next, _flags, _create_cb);
            }

            VkFenceCreateInfo createInfoStruct(void *_p_next = nullptr, VkFenceCreateFlags _flags = 0u) {
                VkFenceCreateInfo __info;
                __info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
                __info.pNext = _p_next;
                __info.flags = _flags;

                return __info;
            }

            void setDevice(const Device::Handler &_device) {
                device = _device.handler;
            }
        }; // Builder END
    }; // Fence END

    // Function call wrappers
        namespace Func {
        void BeginCommandBuffer(const CommandBuffer::Handler &_command_buffer,
                                void *_p_next = nullptr, VkCommandBufferUsageFlags _flags = 0u,
                                VkCommandBufferInheritanceInfo *_inheritance_info = nullptr) {
            VkCommandBufferBeginInfo __begin_info;
            __begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            __begin_info.pNext = _p_next;
            __begin_info.flags = _flags;
            __begin_info.pInheritanceInfo = _inheritance_info;

            if (vkBeginCommandBuffer(_command_buffer.handler, &__begin_info) != VK_SUCCESS)
                throw std::runtime_error(SVKFW_WRAPERR("VKFW :: Func :: Begin Command Buffer", "failed to begin recording command buffer"));
        }

        void EndCommandBuffer(const CommandBuffer::Handler &_command_buffer) {
            vkEndCommandBuffer(_command_buffer.handler);
        }

        void ResetCommandBuffer(const CommandBuffer::Handler &_command_buffer, VkCommandBufferResetFlags _flags = 0u) {
            vkResetCommandBuffer(_command_buffer.handler, _flags);
        }

        void QueueSubmit(const Queue::Handler &_queue, const std::vector<VkSubmitInfo> &_submits,
                         const Fence::Handler &_fence = {}) {
            if (vkQueueSubmit(_queue.handler, _submits.size(), _submits.data(), _fence.handler) != VK_SUCCESS) {
                throw std::runtime_error(SVKFW_WRAPERR("VKFW :: Func :: Queue Submit", "failed to submit queue"));
            }
        }

        void QueuePresentKHR(const Queue::Handler &_queue,
                             const PresentInfoKHR::InfoHandler &_pres_info,
                             bool _check_results = false) {
            VkPresentInfoKHR __info = _pres_info.present_info;
            if (_check_results) {
                __info.pResults = new VkResult[__info.swapchainCount];
            }

            VkResult __queue_pres_res = vkQueuePresentKHR(_queue.handler, _pres_info.ptr());
            if (__queue_pres_res != VK_SUCCESS) {
                printf("Error code: %d\n", __queue_pres_res);
                throw std::runtime_error(SVKFW_WRAPERR("VKFW :: Func :: QueuePresentKHR", "failed to present queue"));
            }
            else if (_check_results) {
                for (uint32_t i = 0u; i < __info.swapchainCount; ++i) {
                    if (__info.pResults[i] != VK_SUCCESS)
                        fprintf(outswarn, "%s %d\n", SVKFW_WRAPWARN("VKFW :: Func :: QueuePresentKHR", "error with swapchain"), i);
                }
            }
        }

        void WaitForFences(const Device::Handler &_device, const Fence::VecHandler &_fences,
                           VkBool32 _wait_all = VK_TRUE, uint64_t _wait_time = UINT64_MAX) {
            vkWaitForFences(_device.handler, _fences.count(), _fences.handlerPtr(), _wait_all, _wait_time);
        }

        void ResetFences(const Device::Handler &_device, const Fence::VecHandler &_fences) {
            vkResetFences(_device.handler, _fences.count(), _fences.handlerPtr());
        }

        uint32_t AcquireNextImageKHR(const Device::Handler &_device, const SwapchainKHR::Handler &_swapchain,
                                     const Semaphore::Handler &_semaphore, const Fence::Handler &_fence = {},
                                     uint64_t _timeout = UINT64_MAX) {
            uint32_t __image_index = 0u;
            vkAcquireNextImageKHR(_device.handler, _swapchain.handler, _timeout, _semaphore.handler, _fence.handler, &__image_index);

            return __image_index;
        }

        Image::VecHandler GetSwapchainImages(const Device::Handler &_device, const SwapchainKHR::Handler &_swapchain) {
            std::vector<VkImage> __images;
            uint32_t __img_count = 0u;
            vkGetSwapchainImagesKHR(_device.handler, _swapchain.handler, &__img_count, nullptr);
            __images.resize(__img_count);
            vkGetSwapchainImagesKHR(_device.handler, _swapchain.handler, &__img_count, __images.data());
            return { __images };
        }
        }; // Func END


    // "vkCmd*" call wrappers
        namespace Cmd {
            struct BeginRenderPass {
                VkCommandBuffer command_buffer = VK_NULL_HANDLE;
                VkRenderPass  render_pass = VK_NULL_HANDLE;
                VkFramebuffer framebuffer = VK_NULL_HANDLE;
                std::vector<VkClearValue> clear_colors;
                VkSubpassContents contents{};
                VkRect2D render_area{};

                BeginRenderPass(const CommandBuffer::Handler &_command_buffer = {},
                                const RenderPass::Handler &_render_pass = {},
                                const Framebuffer::Handler &_framebuffer = {})
                                : command_buffer{_command_buffer.handler}, render_pass{_render_pass.handler},
                                  framebuffer{_framebuffer.handler} {}
                ~BeginRenderPass() {
                    clearCmdWrap();
                }


                void clearCmdWrap() {
                    command_buffer = VK_NULL_HANDLE;
                    render_pass = VK_NULL_HANDLE;
                    framebuffer = VK_NULL_HANDLE;
                    clear_colors.clear();
                }


                void cmdCall(void *_p_next = nullptr) {
                    VkRenderPassBeginInfo __render_pass_info{};
                    __render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                    __render_pass_info.pNext = _p_next;
                    __render_pass_info.renderPass = render_pass;
                    __render_pass_info.framebuffer = framebuffer;
                    __render_pass_info.renderArea = render_area;
                    __render_pass_info.clearValueCount = clear_colors.size();
                    __render_pass_info.pClearValues = clear_colors.data();
                    vkCmdBeginRenderPass(command_buffer, &__render_pass_info, contents);
                }

                void setCommandBuffer(const CommandBuffer::Handler &_command_buffer) {
                    command_buffer = _command_buffer.handler;
                }

                void setRenderPass(const RenderPass::Handler &_render_pass) {
                    render_pass = _render_pass.handler;
                }

                void setFramebuffer(const Framebuffer::Handler &_framebuffer) {
                    framebuffer = _framebuffer.handler;
                }

                void setClearColors(const std::vector<VkClearValue> &_clear_colors) {
                    clear_colors = _clear_colors;
                }

                void setSubpassContents(VkSubpassContents _contents) {
                    contents = _contents;
                }

                void setRenderArea(const Rect2D::SctBasic &_area) {
                    render_area = _area.rectangle;
                }


                void addBlackColor() {
                    clear_colors.push_back(ClearValue::build(0.f, 0.f, 0.f, 1.f));
                }
            }; // BeginRenderPass END


            struct BindPipeline {
                VkCommandBuffer command_buffer = VK_NULL_HANDLE;
                VkPipeline pipeline = VK_NULL_HANDLE;
                VkPipelineBindPoint bind_point = VK_PIPELINE_BIND_POINT_MAX_ENUM;

                BindPipeline(const CommandBuffer::Handler &_command_buffer = {},
                            const Pipeline::Handler &_pipeline = {})
                                : command_buffer{_command_buffer.handler}, pipeline{_pipeline.handler} {}
                ~BindPipeline() {
                    clearCmdWrap();
                }


                void clearCmdWrap() {
                    command_buffer = VK_NULL_HANDLE;
                    pipeline = VK_NULL_HANDLE;
                    bind_point = VK_PIPELINE_BIND_POINT_MAX_ENUM;
                }


                void cmdCall() {
                    vkCmdBindPipeline(command_buffer, bind_point, pipeline);
                }

                static void CmdCall(const CommandBuffer::Handler &_command_buffer,
                                    VkPipelineBindPoint _bind_point,
                                    const Pipeline::Handler &_pipeline) {
                    vkCmdBindPipeline(_command_buffer.handler, _bind_point, _pipeline.handler);
                }


                void setCommandBuffer(const CommandBuffer::Handler &_command_buffer) {
                    command_buffer = _command_buffer.handler;
                }

                void setPipeline(const Pipeline::Handler &_pipeline) {
                    pipeline = _pipeline.handler;
                }

                void setBindPoint(VkPipelineBindPoint _bind_point) {
                    bind_point = _bind_point;
                }
            }; // BindPipeline END


            struct SetViewport {
                // _count == 0 means [offset, _viewports.size() - 1]
                static void CmdCall(const CommandBuffer::Handler &_command_buffer,
                                    const std::vector<VkViewport> &_viewports, uint32_t _offset = 0u, uint32_t _count = 0u) {
                    if (_count == 0u)
                        _count = _viewports.size() - _offset;
                    vkCmdSetViewport(_command_buffer.handler, _offset, _count, _viewports.data());
                }
            }; // SetViewport END


            struct SetScissor {
                // _count == 0 means [offset, _viewports.size() - 1]
                static void CmdCall(const CommandBuffer::Handler &_command_buffer,
                                    const std::vector<VkRect2D> &_scissors, uint32_t _offset = 0u, uint32_t _count = 0u) {
                    if (_count == 0u)
                        _count = _scissors.size() - _offset;
                    vkCmdSetScissor(_command_buffer.handler, _offset, _count, _scissors.data());
                }
            }; // SetScissors END


            struct Draw {
                static void CmdCall(const CommandBuffer::Handler &_command_buffer,
                                    uint32_t _vert_count, uint32_t _inst_count, uint32_t _vert_offset = 0u, uint32_t _inst_offset = 0u) {
                    vkCmdDraw(_command_buffer.handler, _vert_count, _inst_count, _vert_offset, _inst_offset);
                }
            }; // Draw END


            struct EndRenderPass {
                static void CmdCall(const CommandBuffer::Handler &_command_buffer) {
                    vkCmdEndRenderPass(_command_buffer.handler);
                }
            }; // EndRenderPass END
        }; // Func END
    }; // VKFW END
}; // Simple END

#endif