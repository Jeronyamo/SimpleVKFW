#ifndef SVKFW_VKWRAP_H
#define SVKFW_VKWRAP_H

#include "common/terminal.h"
#include "glfwrap.h"

#include <vector>
#include <string>
#include <fstream>



// OLD VERSION
// Moved to vkwrap.h and generated vkwrap_handlers.h


namespace Simple {

//  ============  Utilities  ============  \\

// used to simplify work with all the "const char **" parameters in Vulkan
    struct StringVec {
        std::vector<std::string> list;

        StringVec() {}
        StringVec(const StringVec &_sv) : list{_sv.list} {}
        StringVec(const std::vector<std::string> &_l) : list{_l} {}
        StringVec(const char **_arr, uint _count) {
            list.insert(list.end(), _arr, _arr + _count);
        }

        const StringVec& operator=(const StringVec &_sv) {
            list = _sv.list;
            updateEntriesPtr();
            return *this;
        }


        uint count() { return list.size(); }

        const char **entries() {
            if (_ptr == nullptr)
                updateEntriesPtr();

            return _ptr;
        }

        void addEntries(const std::vector<std::string> &_l) {
            list.insert(list.end(), _l.begin(), _l.end());
            updateEntriesPtr();
        }

    private:
        const char **_ptr = nullptr;

        void updateEntriesPtr() {
            if (_ptr != nullptr) {
                delete[] _ptr;
                _ptr = nullptr;
            }

            const uint _size = list.size();
            if (_size) {
                _ptr = new const char *[_size];
                for (uint i = 0u; i < _size; ++i)
                    _ptr[i] = list[i].c_str();
            }
        }
    };


//  ============  Vulkan (general)  ============  \\

    namespace VKAPIVersion {
        uint32_t available(uint32_t *_variant = nullptr) {
            uint32_t _version = VK_API_VERSION_1_0;

            if (vkEnumerateInstanceVersion != nullptr)
                vkEnumerateInstanceVersion( &_version);

            if (_variant != nullptr)
                *_variant = VK_API_VERSION_VARIANT(_version);
            return _version - VK_API_VERSION_VARIANT(_version);
        }

        inline uint32_t make(const UVec3 &_version, uint32_t _variant = 0u) {
            return VK_MAKE_API_VERSION(_variant, _version.x, _version.y, _version.z);
        }

        inline UVec3 separate(uint32_t _version, uint32_t *_variant = nullptr) {
            if (_variant != nullptr)
                *_variant = VK_API_VERSION_VARIANT(_version);
            return { VK_API_VERSION_MAJOR(_version),
                     VK_API_VERSION_MINOR(_version),
                     VK_API_VERSION_PATCH(_version) };
        }
    };
// // TODO if needed
//     struct VKExtensionChecker {
//         std::vector<VkExtensionProperties> extensions;
//         // make an unordered_set? VkExtensionProperties requires hash/equal_to

//         VKExtensionChecker(const char *_layerName = nullptr) {
//             uint _count = 0u;
//             vkEnumerateInstanceExtensionProperties(_layerName, &_count, nullptr);
//             extensions.resize(_count);
//             vkEnumerateInstanceExtensionProperties(_layerName, &_count, extensions.data());
//         }

//         // TODO: any required functionality. Probably might implement search for extensions
//     };



//  ========================================  \\
                Vulkan framework
//  ========================================  \\

//  ============  Vulkan instance  ============  \\

    // Layer configuration: https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Validation_layers#page_Configuration
    // It is done via "Vulkan Configurator", but maybe there's more to that on the code side - TODO: check

    struct VKFWDebugMessenger;
    VKAPI_ATTR VkBool32 VKAPI_CALL vkfwDebugMessageCallback(
                VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                VkDebugUtilsMessageTypeFlagsEXT messageType,
                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                void* pUserData);

    // vkfwDebugMessageCallback is used in VKFWInstanceInfo
    // VKFWDebugMessenger       is used in VKFWInstance

    struct VKFWInstanceInfo {
        VkInstanceCreateInfo info;
        StringVec layers, extensions;

        VKFWInstanceInfo() : info{ .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                                   .pNext = nullptr,
                                   .flags = 0,
                                   .pApplicationInfo = nullptr,
                                   .enabledLayerCount = 0,
                                   .ppEnabledLayerNames = nullptr,
                                   .enabledExtensionCount = 0,
                                   .ppEnabledExtensionNames = nullptr } {}
        VKFWInstanceInfo(const StringVec &_layers, const StringVec &_extensions,
                         VkInstanceCreateFlags _flags, VkApplicationInfo *_appinfo = nullptr,
                         void * _pNext = nullptr) : layers{_layers}, extensions{_extensions},
                                                    info{ .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                                                          .pNext = _pNext,
                                                          .flags = _flags,
                                                          .pApplicationInfo = _appinfo,
                                                          .enabledLayerCount = 0u,
                                                          .ppEnabledLayerNames = nullptr,
                                                          .enabledExtensionCount = 0u,
                                                          .ppEnabledExtensionNames = nullptr } {
            info.enabledLayerCount = layers.count();
            info.ppEnabledLayerNames = layers.entries();
            info.enabledExtensionCount = extensions.count();
            info.ppEnabledExtensionNames = extensions.entries();
        }
        ~VKFWInstanceInfo() {
            if (info.pApplicationInfo != nullptr)
                delete info.pApplicationInfo;
        }

        void setAppInfo(const char *_appname, UVec3 _appversion,
                        const char * _ngname, UVec3  _ngversion,
                        void *_pNext = nullptr, uint32_t _apiversion = 0u) {
            if (info.pApplicationInfo != nullptr)
                delete info.pApplicationInfo;
            info.pApplicationInfo = new VkApplicationInfo{
                                        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                                        .pNext = _pNext,
                                        .pApplicationName = _appname,
                                        .applicationVersion = VKAPIVersion::make(_appversion),
                                        .pEngineName = _ngname,
                                        .engineVersion = VKAPIVersion::make(_ngversion),
                                        .apiVersion = _apiversion ? _apiversion : VKAPIVersion::available() }; // defaults: 0.1.0, "TESTAPP", "No engine"
        }


        void addValLayers(const StringVec &_valLayers = {{ "VK_LAYER_KHRONOS_validation" }}) {
            if (info.pApplicationInfo == nullptr)
                throw std::runtime_error(SVKFW_WRAPERR("add validation layers","appInfo is not set"));
            if (!allLayersAvailable(_valLayers))
                throw std::runtime_error(SVKFW_WRAPERR("add validation layers","some layers are unavailable"));

            addLayers(_valLayers);
        }

        void addGLFWExtensions() { // vulkan + GLFW is the main stack for now
            uint _count = 0u;
            const char **_ext = glfwGetRequiredInstanceExtensions(&_count);
            addExtensions({_ext, _count});
        }

        void addDMCExtension() {
            if (!allExtAvailable({{ VK_EXT_DEBUG_UTILS_EXTENSION_NAME }}))
                throw std::runtime_error(SVKFW_WRAPERR("add DMC ext","debug message callback extension is unavailable"));

            addExtensions({{ VK_EXT_DEBUG_UTILS_EXTENSION_NAME }});
        }


        void addLayers(const StringVec &_layers) {
            layers.addEntries(_layers.list);
            info.enabledLayerCount = layers.count();
            info.ppEnabledLayerNames = layers.entries();
        }

        void setLayers(const StringVec &_layers) {
            layers = _layers;
            info.enabledLayerCount = layers.count();
            info.ppEnabledLayerNames = layers.entries();
        }

        void addExtensions(const StringVec &_extensions) {
            extensions.addEntries(_extensions.list);
            info.enabledExtensionCount = extensions.count();
            info.ppEnabledExtensionNames = extensions.entries();
        }

        void setExtensions(const StringVec &_extensions) {
            extensions = _extensions;
            info.enabledExtensionCount = extensions.count();
            info.ppEnabledExtensionNames = extensions.entries();
        }


    // Static

        static uint32_t getAllInstanceValLayers(std::vector<VkLayerProperties> &_layerList) {
            uint32_t __layerCount;
            vkEnumerateInstanceLayerProperties(&__layerCount, nullptr);

            _layerList.resize(__layerCount);
            vkEnumerateInstanceLayerProperties(&__layerCount, _layerList.data());
            return __layerCount;
        }

        static uint32_t getAllInstanceExtensions(std::vector<VkExtensionProperties> &_extList, const char *_layer = nullptr) {
            uint32_t __extCount;
            vkEnumerateInstanceExtensionProperties(_layer, &__extCount, nullptr);

            _extList.resize(__extCount);
            vkEnumerateInstanceExtensionProperties(_layer, &__extCount, _extList.data());
            return __extCount;
        }


        static bool allLayersAvailable(const StringVec &_layers) {
            uint32_t __layerCount;
            std::vector<VkLayerProperties> _layerList;
            getAllInstanceValLayers(_layerList);

            bool __res = true;
            for (const std::string &layerName : _layers.list) {
                bool layerFound = false;

                for (const auto& layerProperties : _layerList) {
                    if (layerName == layerProperties.layerName) {
                        layerFound = true;
                        break;
                    }
                }

                if (!layerFound) {
                    fprintf(outswarn, "%s %s\n", SVKFW_WRAPWARN("vk layer check","could not find layer"), layerName.c_str());
                    fflush(outswarn);

                    __res = false;
                }
            }
            return __res;
        }

        static bool allExtAvailable(const StringVec &_extensions, const char *_layer = nullptr) {
            uint32_t __extCount;
            std::vector<VkExtensionProperties> _extList;
            getAllInstanceExtensions(_extList, _layer);

            bool __res = true;
            for (const std::string &extName : _extensions.list) {
                bool extFound = false;

                for (const auto& extProperties : _extList) {
                    if (extName == extProperties.extensionName) {
                        extFound = true;
                        break;
                    }
                }

                if (!extFound) {
                    fprintf(outswarn, "%s %s\n", SVKFW_WRAPWARN("vk ext check","could not find extension"), extName.c_str());
                    fflush(outswarn);

                    __res = false;
                }
            }
            return __res;
        }

        // for each layer name in _layers returns if it is available (_res must have at least the same length (in elements) as _layers)
        static void getLayerAvailabilityFlags(const StringVec &_layers, bool *_res) {
            uint32_t __layerCount;
            std::vector<VkLayerProperties> _layerList;
            getAllInstanceValLayers(_layerList);

            for (uint i = 0u; i < _layers.list.size(); ++i) {
                bool __layerFound = false;

                for (const auto& layerProperties : _layerList) {
                    if (_layers.list[i] == layerProperties.layerName) {
                        __layerFound = true;
                        break;
                    }
                }
                _res[i] = __layerFound;
            }
        }

        // for each extension name in _ext returns if it is available (_res must have at least the same length (in elements) as _ext)
        static void getExtAvailabilityFlags(const StringVec &_extensions, bool *_res, const char *_layer = nullptr) {
            uint32_t __extCount;
            std::vector<VkExtensionProperties> __extList;
            getAllInstanceExtensions(__extList, _layer);

            for (uint i = 0u; i < _extensions.list.size(); ++i) {
                bool __extFound = false;

                for (const auto& extProperties : __extList) {
                    if (_extensions.list[i] == extProperties.extensionName) {
                        __extFound = true;
                        break;
                    }
                }
                _res[i] = __extFound;
            }
        }
    }; // VKFWInstanceInfo END


    struct VKFWInstance {
        static VkInstance instance;
        std::vector <VKFWDebugMessenger> msgcallbacks;
        VkAllocationCallbacks *_destroyCb = nullptr;

        VKFWInstance() {}
        VKFWInstance(const VKFWInstanceInfo &_instInfo,
                     VkAllocationCallbacks *_cAlloc = nullptr,
                     VkAllocationCallbacks *_dAlloc = nullptr,
                     bool _instance_only = false) : _destroyCb{_dAlloc} {
            if (instance != VK_NULL_HANDLE)
                throw std::runtime_error(SVKFW_WRAPERR("VKFW instance","already initialized (!= NULL)"));
            if (vkCreateInstance(&_instInfo.info, _cAlloc, &instance) != VK_SUCCESS)
                throw std::runtime_error(SVKFW_WRAPERR("VKFW instance","could not create instance"));

            if (_instance_only) volkLoadInstanceOnly(instance);
            else volkLoadInstance(instance);
        }
        ~VKFWInstance() {
            msgcallbacks.clear();
            if (instance != VK_NULL_HANDLE) {
                vkDestroyInstance(instance, _destroyCb);
                instance = VK_NULL_HANDLE;
            }
        }


        void addDefaultDebugMessenger() {
            msgcallbacks.emplace_back();
        }

        void addDebugMessenger(VkDebugUtilsMessageSeverityFlagsEXT _severity,
                               VkDebugUtilsMessageTypeFlagsEXT _mtype,
                               PFN_vkDebugUtilsMessengerCallbackEXT _callback
                                    = vkfwDebugMessageCallback,
                               void *_userData = nullptr) {
            msgcallbacks.emplace_back(_severity, _mtype, _callback, _userData);
        }


        static void getAllPhysDevices(std::vector<VkPhysicalDevice> &_devices) {
            uint32_t __devCount = 0;
            vkEnumeratePhysicalDevices(instance, &__devCount, nullptr);

            if (__devCount == 0)
                throw std::runtime_error(SVKFW_WRAPERR("get all physical devices","no GPUs with Vulkan support available"));

            _devices.resize(__devCount);
            vkEnumeratePhysicalDevices(instance, &__devCount, _devices.data());
        }
    }; // VKFWInstance END
    VkInstance VKFWInstance::instance = VK_NULL_HANDLE;


//  ============  Vulkan debug messenger callback  ============  \\

    VKAPI_ATTR VkBool32 VKAPI_CALL vkfwDebugMessageCallback(
                VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                VkDebugUtilsMessageTypeFlagsEXT messageType,
                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                void* pUserData) {
        // Severity
        FILE* __outs = stdout;
        const char *_severity = CSISEQ(SGR_COL_FG_MAGENTA) "Untracked severity message" CSISEQ(SGR_COL_FG_DEFAULT);

        switch (messageSeverity) {
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

        bool _typeFlags[] = { bool(messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT),
                              bool(messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT),
                              bool(messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT),
                              bool(messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT),
                                   messageType > VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT };
        bool _multFlags = false;
        std::string _typestr{};
        for (uint i = 0u; i < 5u; ++i) {
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
        if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
            // there are multi-line "Info" messages, adding prefix results in poor readability
            fprintf(__outs, CSISEQ(SGR_EMPH_BOLD) "[VKFW VLayer %s %s]" CSISEQ("") "\n",
                            _typestr.c_str(), _severity);
        fprintf(__outs, "%s\n", pCallbackData->pMessage);

        return VK_FALSE;
    }

    struct VKFWDebugMessenger {
        VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;

        VKFWDebugMessenger() {
            VkDebugUtilsMessengerCreateInfoEXT __info{};
            __info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            __info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT    |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            __info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT    |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            __info.pfnUserCallback = vkfwDebugMessageCallback;
            __info.pUserData = nullptr;

            if (vkCreateDebugUtilsMessengerEXT(VKFWInstance::instance, &__info, nullptr, &messenger) != VK_SUCCESS)
                throw std::runtime_error(SVKFW_WRAPERR("VKFWDebugMessenger","could not create debug messenger"));
        }
        VKFWDebugMessenger(VkDebugUtilsMessageSeverityFlagsEXT _severity,
                           VkDebugUtilsMessageTypeFlagsEXT _mtype,
                           PFN_vkDebugUtilsMessengerCallbackEXT _callback = vkfwDebugMessageCallback,
                           void *_userData = nullptr) {
            VkDebugUtilsMessengerCreateInfoEXT __info{};
            __info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            __info.messageSeverity = _severity;
            __info.messageType = _mtype;
            __info.pfnUserCallback = _callback;
            __info.pUserData = _userData;

            if (vkCreateDebugUtilsMessengerEXT(VKFWInstance::instance, &__info, nullptr, &messenger) != VK_SUCCESS)
                throw std::runtime_error(SVKFW_WRAPERR("VKFWDebugMessenger","could not create debug messenger"));
        }

        ~VKFWDebugMessenger() {
            vkDestroyDebugUtilsMessengerEXT(VKFWInstance::instance, messenger, nullptr);
        }
    }; // VKFWDebugMessenger END


//  ============  Vulkan physical device  ============  \\

    struct VKFWPhysDevice {
        VkPhysicalDevice physDevice = VK_NULL_HANDLE;

        VKFWPhysDevice() {}
        VKFWPhysDevice(VkPhysicalDevice _pDevice) : physDevice{_pDevice} {}
        VKFWPhysDevice(const VKFWPhysDevice &_pDevice) : physDevice{_pDevice.physDevice} {}


    //  ============  Device info  ============  \\

    // Physical device info struct, probably should only wrap parts
    // which work differently in this project (like API version as UVec)
        struct DeviceInfo {
            VkPhysicalDeviceProperties properties;
            VkPhysicalDeviceFeatures features;

            DeviceInfo(VKFWPhysDevice _pDevice) : DeviceInfo{_pDevice.physDevice} {}
            DeviceInfo(VkPhysicalDevice _pDevice) {
                vkGetPhysicalDeviceProperties(_pDevice, &properties);
                vkGetPhysicalDeviceFeatures  (_pDevice, &features);
            }


            UVec4 apiVersion() {
                UVec4 __ver;

                __ver["xyz"] = VKAPIVersion::separate(properties.apiVersion, &__ver.w);
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
        };
        DeviceInfo info() { return { physDevice }; }


    //  ============  Finding devices  ============  \\

        void getAllDeviceExtensions(std::vector<VkExtensionProperties> &_extensions, const char *_layerName = nullptr) {
            uint32_t __extCount = 0;
            vkEnumerateDeviceExtensionProperties(physDevice, _layerName, &__extCount, nullptr);
            _extensions.resize(__extCount);
            vkEnumerateDeviceExtensionProperties(physDevice, _layerName, &__extCount, _extensions.data());
        }

        bool allExtAvailable(const StringVec &_extensions) {
            std::vector<VkExtensionProperties> __exts;
            getAllDeviceExtensions(__exts);

            for (uint i = 0u; i < _extensions.list.size(); ++i) {
                bool __is_available = false;
                for (const auto &ext : __exts)
                    if (_extensions.list[i] == ext.extensionName) {
                        __is_available = true;
                        break;
                    }
                if (!__is_available)  return false;
            }
            return true;
        }

        bool allExtAvailable(const StringVec &_extensions, bool *_flags) {
            std::vector<VkExtensionProperties> __exts;
            getAllDeviceExtensions(__exts);

            bool __res = true;
            for (uint i = 0u; i < _extensions.list.size(); ++i) {
                bool __is_available = false;
                for (const auto &ext : __exts)
                    if (_extensions.list[i] == ext.extensionName) {
                        __is_available = true;
                        break;
                    }
                if (_flags != nullptr) _flags[i] = __is_available;
                if (!__is_available)  __res = false;
            }
            return __res;
        }

    // TODO: namespace for physical device suitability function types and defaults.
        struct Suitability {
            typedef bool (*TestFunc) (VKFWPhysDevice _physDevice);
            typedef  int (*RankFunc) (VKFWPhysDevice _physDevice);
            typedef  int (*CompFunc) (const void *_pd1, const void *_pd2);


            static bool defaultTest(VKFWPhysDevice _physDevice) {
                DeviceInfo __info{_physDevice};
                return __info.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
                       __info.features.geometryShader && _physDevice.allExtAvailable({ {VK_KHR_SWAPCHAIN_EXTENSION_NAME} });
            }

            static int defaultRank(VKFWPhysDevice _physDevice) {
                DeviceInfo __info{_physDevice};
                return (__info.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) +
                       __info.features.geometryShader;
            }

            static int defaultComp(const void *_pd1, const void *_pd2) {
                int __v1 = defaultRank(*(VkPhysicalDevice*)_pd1),
                    __v2 = defaultRank(*(VkPhysicalDevice*)_pd2);
                return int(__v1 > __v2) - int(__v2 > __v1);
            }
        };


        // returns the first physical device for which _testFunc(device) == true
        static VKFWPhysDevice testPickPhysDevice(const Suitability::TestFunc _testFunc = Suitability::defaultTest) {
            std::vector<VkPhysicalDevice> __devices;
            VKFWInstance::getAllPhysDevices(__devices);

            for (auto _pDevice : __devices)
                if (_testFunc(_pDevice))
                    return {_pDevice};

            throw std::runtime_error(SVKFW_WRAPERR("testPickPhysDevice", "failed to find a suitable GPU"));
            return {};
        }

        // ranks all physical devices and returns one with max _rankFunc(device)
        static VKFWPhysDevice rankPickPhysDevice(const Suitability::RankFunc _rankFunc = Suitability::defaultRank) {
            std::vector<VkPhysicalDevice> __devices;
            VKFWInstance::getAllPhysDevices(__devices);

            int _maxRank = 0;
            uint __ind = __devices.size();
            for (uint i = 0u; i < __devices.size(); ++i) {
                int __rank = _rankFunc(__devices[i]);
                if (__rank > _maxRank) {
                    _maxRank = __rank;
                    __ind = i;
                }
            }
            if (__ind >= __devices.size())
                throw std::runtime_error(SVKFW_WRAPERR("rankPickPhysDevice", "failed to find a suitable GPU"));
            return {__devices[__ind]};
        }

        // adds to _devices all physical devices for which _testFunc(device) == true
        // returns the number of devices added
        static uint testSortPhysDevices(std::vector<VKFWPhysDevice> &_devices,
                                        Suitability::TestFunc _testFunc = Suitability::defaultTest) {
            std::vector<VkPhysicalDevice> __devices;
            VKFWInstance::getAllPhysDevices(__devices);

            // _devices.clear();
            uint _count = 0u;
            for (auto _pDevice : __devices)
                if (_testFunc(_pDevice)) {
                    _devices.push_back(_pDevice);
                    _count += 1u;
                }
            return _count;
        }

        // Adds to _devices all physical devices sorted by _compFunc in descending order
        static void rankSortPhysDevices(std::vector<VKFWPhysDevice> &_devices,
                                        Suitability::CompFunc _compFunc = Suitability::defaultComp) {
            std::vector<VkPhysicalDevice> __dev;
            VKFWInstance::getAllPhysDevices(__dev);

            std::qsort(__dev.data(), __dev.size(), sizeof(VkPhysicalDevice), _compFunc);
            _devices.insert(_devices.end(), __dev.rbegin(), __dev.rend());
        }
    }; // VKFWPhysDevice END


//  ============  Vulkan queue family  ============  \\

    struct VKFWQueueFamily {
        uint32_t family;
    }; // VKFWQueueFamily END


    struct VKFWQueueFamilyProperties {
        VkQueueFamilyProperties properties;

        VKFWQueueFamilyProperties(VkQueueFamilyProperties _props) : properties{_props} {}

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
                                                             properties.minImageTransferGranularity.depth }; }

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
                    "number of queues:           " + std::to_string(queuesNum()) + '\n' +
                    "timestamp valid range bits: " + std::to_string(timestampValidRangeBits()) + '\n' +
                    "minimal granularity: (" + std::to_string(__gran.x) + ", "
                                             + std::to_string(__gran.y) + ", "
                                             + std::to_string(__gran.z) + ")\n";
        }


        struct Suitability { // namespace
            typedef bool (*TestFunc) (const VkQueueFamilyProperties &_qfp);
            typedef  int (*RankFunc) (const VkQueueFamilyProperties &_qfp);
            typedef VKFWQueueFamilyProperties QFInfo;

            static bool testGraphics(const VkQueueFamilyProperties &_qfp) { return QFInfo{_qfp}.supportsGraphics(); }
            static bool testCompute (const VkQueueFamilyProperties &_qfp) { return QFInfo{_qfp}.supportsCompute (); }
            static bool testTransfer(const VkQueueFamilyProperties &_qfp) { return QFInfo{_qfp}.supportsTransfer(); }
            static bool testSparseBinding  (const VkQueueFamilyProperties &_qfp) { return QFInfo{_qfp}.supportsSparseBinding  (); }
            static bool testProtectedMemory(const VkQueueFamilyProperties &_qfp) { return QFInfo{_qfp}.supportsProtectedMemory(); }
            static bool testVideoDecode(const VkQueueFamilyProperties &_qfp) { return QFInfo{_qfp}.supportsVideoDecode(); }
            static bool testVideoEncode(const VkQueueFamilyProperties &_qfp) { return QFInfo{_qfp}.supportsVideoEncode(); }
            static bool testOpticalFlow(const VkQueueFamilyProperties &_qfp) { return QFInfo{_qfp}.supportsOpticalFlow(); }


        // TODO: practice, then redo these functions
        // For now, result >0 means score; if <0, then queue family
        // doesn't have required feature. These functions never return 0.

            static int rankGraphics(const VkQueueFamilyProperties &_qfp) {
                QFInfo __info{_qfp};
                return 5 * __info.supportsGraphics()
                         - __info.supportsCompute()
                         - __info.supportsVideoDecode()
                         - __info.supportsVideoEncode()
                         - __info.supportsOpticalFlow();
            }
            static int rankCompute (const VkQueueFamilyProperties &_qfp) {
                QFInfo __info{_qfp};
                return 5 * __info.supportsCompute()
                         - __info.supportsGraphics()
                         - __info.supportsVideoDecode()
                         - __info.supportsVideoEncode()
                         - __info.supportsOpticalFlow();
            }
            static int rankTransfer(const VkQueueFamilyProperties &_qfp) {
                QFInfo __info{_qfp};
                return 6 * __info.supportsTransfer()
                         - __info.supportsCompute()
                         - __info.supportsGraphics()
                         - __info.supportsVideoDecode()
                         - __info.supportsVideoEncode()
                         - __info.supportsOpticalFlow();
            }
            static int rankSparseBinding  (const VkQueueFamilyProperties &_qfp) {
                QFInfo __info{_qfp};
                return 6 * __info.supportsSparseBinding()
                         - __info.supportsCompute()
                         - __info.supportsGraphics()
                         - __info.supportsVideoDecode()
                         - __info.supportsVideoEncode()
                         - __info.supportsOpticalFlow();
            }
            static int rankProtectedMemory(const VkQueueFamilyProperties &_qfp) {
                QFInfo __info{_qfp};
                return __info.supportsProtectedMemory() ? 1 : -1;
            }
            static int rankVideoDecode(const VkQueueFamilyProperties &_qfp) {
                QFInfo __info{_qfp};
                return 5 * __info.supportsVideoDecode()
                         - __info.supportsGraphics()
                         - __info.supportsCompute()
                         - __info.supportsVideoEncode()
                         - __info.supportsOpticalFlow();
            }
            static int rankVideoEncode(const VkQueueFamilyProperties &_qfp) {
                QFInfo __info{_qfp};
                return 5 * __info.supportsVideoEncode()
                         - __info.supportsGraphics()
                         - __info.supportsCompute()
                         - __info.supportsVideoDecode()
                         - __info.supportsOpticalFlow();
            }
            static int rankOpticalFlow(const VkQueueFamilyProperties &_qfp) {
                QFInfo __info{_qfp};
                return 5 * __info.supportsOpticalFlow()
                         - __info.supportsGraphics()
                         - __info.supportsCompute()
                         - __info.supportsVideoDecode()
                         - __info.supportsVideoEncode();
            }
        }; // Suitability END
    }; // VKFWQueueFamilyProperties END


    struct VKFWSurface;

    // 1 per physical device, so (usually) 1 per program
    struct VKFWQFamiliesHandler {
        VKFWPhysDevice physDevice;
        std::vector<VKFWQueueFamilyProperties> properties;

        VKFWQFamiliesHandler(const VKFWPhysDevice &_physDevice) : physDevice{_physDevice} {
            uint32_t __QFCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(physDevice.physDevice, &__QFCount, nullptr);

            std::vector<VkQueueFamilyProperties> _qf((size_t)__QFCount);
            vkGetPhysicalDeviceQueueFamilyProperties(physDevice.physDevice, &__QFCount, _qf.data());
            properties.insert(properties.end(), _qf.begin(), _qf.end());
        }


        const VKFWQueueFamilyProperties& getQFProperties(VKFWQueueFamily _family) const { return properties[_family.family]; }

        VKFWQueueFamily findAnyQF(VKFWQueueFamilyProperties::Suitability::TestFunc _testFunc) {
            for (uint i = 0u; i < properties.size(); ++i)
                if (_testFunc(properties[i].properties)) return { i };

            fprintf(outswarn, SVKFW_WRAPWARN("find queue family (test)", "could not find queue family"));
            return {uint(-1)};
        }
        VKFWQueueFamily findAnyQF(VKFWQueueFamilyProperties::Suitability::RankFunc _rankFunc) {
            uint __maxI = properties.size();
            int  __maxR = INT_MIN;
            for (uint i = 0u; i < properties.size(); ++i) {
                // printf("%s", properties[i].infoString().c_str());
                int __rank = _rankFunc(properties[i].properties);
                if (__rank > __maxR) {
                    __maxI = i;
                    __maxR = __rank;
                }
            }
            if (__maxI >= properties.size()) {
                fprintf(outswarn, SVKFW_WRAPWARN("find queue family (rank)", "could not find queue family"));
                return {uint(-1)};
            }
            return { __maxI };
        }
        // TODO: findAllQFs(TestFunc|RankFunc) -
        // returns either a std::vector<VKFWQueueFamily> or new VKFWQueueFamilies class
        // which contains std::vector<VKFWQueueFamily> -
        // practice, see how multiple queue families
        // can be used, make VKFWQueueFamilies if necessary

    // Presetation support: Vk & GLFW versions

        bool checkQFPresentationSupport(VKFWQueueFamily _family, const VKFWSurface &_surface);
        VKFWQueueFamily findAnyQFWithSurfaceSupport(const VKFWSurface &_surface);


        bool checkQFPresentationSupportGLFW(VKFWQueueFamily _family) {
            return glfwGetPhysicalDevicePresentationSupport(VKFWInstance::instance, physDevice.physDevice, _family.family);
        }

        VKFWQueueFamily findAnyQFWithSurfaceSupportGLFW() {
            for (uint i = 0u; i < properties.size(); ++i)
                if (checkQFPresentationSupportGLFW({i})) return { i };

            fprintf(outswarn, SVKFW_WRAPWARN("find queue family (presentation, GLFW)", "could not find queue family"));
            return {uint(-1)};
        }
    }; // VKFWQFamiliesHandler END


//  ============  Logical device  ============  \\

    struct VKFWDeviceQueuesInfo {
        std::vector <VkDeviceQueueCreateInfo> info;

        VKFWDeviceQueuesInfo() {}
        VKFWDeviceQueuesInfo(const VKFWQFamiliesHandler &_handler,
                             VKFWQueueFamily _family, uint32_t _queueCount,
                             const std::vector<float> &_priorities = {1.f},
                             bool _set_protected_memory_bit = false) {
            addQueueInfo(_handler, _family, _queueCount, _priorities, _set_protected_memory_bit);
        }
        VKFWDeviceQueuesInfo(const VKFWDeviceQueuesInfo &_info) : info{_info.info} {
            for (uint i = 0u; i < info.size(); ++i) {
                float *__ptr = new float[info[i].queueCount];
                memcpy(__ptr, info[i].pQueuePriorities, info[i].queueCount * sizeof(float));
                info[i].pQueuePriorities = __ptr;
            }
        }

        ~VKFWDeviceQueuesInfo() {
            for (uint i = 0u; i < info.size(); ++i)
                delete[] info[i].pQueuePriorities;
            info.clear();
        }


        // _queueCount is either 0 (all available queues are used) or [1, family queueCount]
        // if _priorities.size() < _queueCount:
        //     priorities [_priorities.size(), _queueCount) are assumed to be 1.f
        // if _priorities.size() > _queueCount:
        //     priorities [_queueCount, _priorities.size()) are ignored
        void addQueueInfo(const VKFWQFamiliesHandler &_handler, VKFWQueueFamily _family,
                          uint32_t _queueCount, const std::vector<float> &_priorities = {1.f},
                          bool _set_protected_memory_bit = false) {
            uint32_t _familyQueueCount = 0u;
            for (const auto &_info : info)
                if (_info.queueFamilyIndex == _family.family)
                    _familyQueueCount += _info.queueCount;

            if (_queueCount == 0u)
                _queueCount = _handler.getQFProperties(_family).properties.queueCount;
            if (_queueCount > _handler.getQFProperties(_family).properties.queueCount)
                throw std::invalid_argument(SVKFW_WRAPERR("device queue info","_queueCount exceeds the number of queues for queue family"));
            if (_set_protected_memory_bit && !_handler.getQFProperties(_family).supportsProtectedMemory())
                throw std::invalid_argument(SVKFW_WRAPERR("device queue info","setting protected memory bit when the feature is disabled"));


            info.push_back({});
            VkDeviceQueueCreateInfo &_newinfo = info.rbegin()[0];

            _newinfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            _newinfo.pNext = nullptr;
            _newinfo.flags = 0;
            if (_set_protected_memory_bit)
                _newinfo.flags = VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT;
            _newinfo.queueFamilyIndex = _family.family;
            _newinfo.queueCount = _queueCount;

            float *__ptr = new float[_queueCount];
            for (uint i = 0u; i < _queueCount; ++i) {
                if (i >= _priorities.size())
                    __ptr[i] = 1.f;
                else {
                    __ptr[i] = Func::clamp_GE_LE(_priorities[i], 0.f, 1.f);
                    if (__ptr[i] != _priorities[i])
                        fprintf(outswarn, "%s: %f", SVKFW_WRAPWARN("device queue info","unnormalized priority value"), _priorities[i]);
                }
            }
            _newinfo.pQueuePriorities = __ptr;
        }

        void deleteQueueInfo(uint _index) {
            delete[] info[_index].pQueuePriorities;
            info.erase(info.begin() + _index);
        }
    };

    struct VKFWDeviceInfo {
        VkDeviceCreateInfo info;
        VkPhysicalDeviceFeatures reqFeatures;
        StringVec layers, extensions;

        VKFWDeviceInfo() {
            info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            info.flags = 0u;
        }
        VKFWDeviceInfo(const VKFWDeviceQueuesInfo &_qinfo, const StringVec &_layers,
                       const StringVec &_extensions, VkPhysicalDeviceFeatures *_features) :
                       reqFeatures{}, layers{_layers}, extensions{_extensions} {
            info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            info.flags = 0u;
            info.pNext = nullptr;
            info.enabledLayerCount = layers.count();
            info.ppEnabledLayerNames = layers.entries();
            info.enabledExtensionCount = extensions.count();
            info.ppEnabledExtensionNames = extensions.entries();
            info.pQueueCreateInfos = _qinfo.info.data();
            info.queueCreateInfoCount = _qinfo.info.size();

            
            if (_features != nullptr)
                reqFeatures = *_features;
            info.pEnabledFeatures = &reqFeatures;
        }

        // TODO: when I'll know what I'm doing
        void setRequiredFeatures() {

        }

        void setQueueInfo(const VKFWDeviceQueuesInfo &_qinfo) {
            info.pQueueCreateInfos    = _qinfo.info.data();
            info.queueCreateInfoCount = _qinfo.info.size();
        }

        void addLayers(const StringVec &_layers) {
            layers.addEntries(_layers.list);
            info.enabledLayerCount = layers.count();
            info.ppEnabledLayerNames = layers.entries();
        }

        void setLayers(const StringVec &_layers) {
            layers = _layers;
            info.enabledLayerCount = layers.count();
            info.ppEnabledLayerNames = layers.entries();
        }

        void addExtensions(const StringVec &_extensions) {
            extensions.addEntries(_extensions.list);
            info.enabledExtensionCount = extensions.count();
            info.ppEnabledExtensionNames = extensions.entries();
        }

        void setExtensions(const StringVec &_extensions) {
            extensions = _extensions;
            info.enabledExtensionCount = extensions.count();
            info.ppEnabledExtensionNames = extensions.entries();
        }


    // Static

        static uint32_t getAllDeviceValLayers(const VKFWPhysDevice &_physDevice, std::vector<VkLayerProperties> &_layerList) {
            uint32_t __layerCount;
            vkEnumerateDeviceLayerProperties(_physDevice.physDevice, &__layerCount, nullptr);

            _layerList.resize(__layerCount);
            vkEnumerateDeviceLayerProperties(_physDevice.physDevice, &__layerCount, _layerList.data());
            return __layerCount;
        }

        static uint32_t getAllDeviceExtensions(const VKFWPhysDevice &_physDevice, std::vector<VkExtensionProperties> &_extList, const char *_layer = nullptr) {
            uint32_t __extCount;
            vkEnumerateDeviceExtensionProperties(_physDevice.physDevice, _layer, &__extCount, nullptr);

            _extList.resize(__extCount);
            vkEnumerateDeviceExtensionProperties(_physDevice.physDevice, _layer, &__extCount, _extList.data());
            return __extCount;
        }


        static bool allLayersAvailable(const VKFWPhysDevice &_physDevice, const StringVec &_layers) {
            uint32_t __layerCount;
            std::vector<VkLayerProperties> _layerList;
            getAllDeviceValLayers(_physDevice, _layerList);

            bool __res = true;
            for (const std::string &layerName : _layers.list) {
                bool layerFound = false;

                for (const auto& layerProperties : _layerList) {
                    if (layerName == layerProperties.layerName) {
                        layerFound = true;
                        break;
                    }
                }

                if (!layerFound) {
                    fprintf(outswarn, "%s %s\n", SVKFW_WRAPWARN("vk layer check","could not find layer"), layerName.c_str());
                    fflush(outswarn);

                    __res = false;
                }
            }
            return __res;
        }

        static bool allExtAvailable(const VKFWPhysDevice &_physDevice, const StringVec &_extensions, const char *_layer = nullptr) {
            uint32_t __extCount;
            std::vector<VkExtensionProperties> _extList;
            getAllDeviceExtensions(_physDevice, _extList, _layer);

            bool __res = true;
            for (const std::string &extName : _extensions.list) {
                bool extFound = false;

                for (const auto& extProperties : _extList) {
                    if (extName == extProperties.extensionName) {
                        extFound = true;
                        break;
                    }
                }

                if (!extFound) {
                    fprintf(outswarn, "%s %s\n", SVKFW_WRAPWARN("vk ext check","could not find extension"), extName.c_str());
                    fflush(outswarn);

                    __res = false;
                }
            }
            return __res;
        }

        // for each layer name in _layers returns if it is available (_res must have at least the same length (in elements) as _layers)
        static void getLayerAvailabilityFlags(const VKFWPhysDevice &_physDevice, const StringVec &_layers, bool *_res) {
            uint32_t __layerCount;
            std::vector<VkLayerProperties> _layerList;
            getAllDeviceValLayers(_physDevice, _layerList);

            for (uint i = 0u; i < _layers.list.size(); ++i) {
                bool __layerFound = false;

                for (const auto& layerProperties : _layerList) {
                    if (_layers.list[i] == layerProperties.layerName) {
                        __layerFound = true;
                        break;
                    }
                }
                _res[i] = __layerFound;
            }
        }

        // for each extension name in _ext returns if it is available (_res must have at least the same length (in elements) as _ext)
        static void getExtAvailabilityFlags(const VKFWPhysDevice &_physDevice, const StringVec &_extensions, bool *_res, const char *_layer = nullptr) {
            uint32_t __extCount;
            std::vector<VkExtensionProperties> __extList;
            getAllDeviceExtensions(_physDevice, __extList, _layer);

            for (uint i = 0u; i < _extensions.list.size(); ++i) {
                bool __extFound = false;

                for (const auto& extProperties : __extList) {
                    if (_extensions.list[i] == extProperties.extensionName) {
                        __extFound = true;
                        break;
                    }
                }
                _res[i] = __extFound;
            }
        }
    };

    struct VKFWDevice {
        VkDevice device;
        const VkAllocationCallbacks *_destroyCb = nullptr;

        VKFWDevice(const VKFWDevice &_device) : device{_device.device},
                                                _destroyCb{_device._destroyCb} {}
        VKFWDevice(const VKFWPhysDevice &_physDevice, const VKFWDeviceInfo &_info,
                   const VkAllocationCallbacks *_cAlloc = nullptr,
                   const VkAllocationCallbacks *_dAlloc = nullptr) : _destroyCb{_dAlloc} {
            if (vkCreateDevice(_physDevice.physDevice, &_info.info, _cAlloc, &device) != VK_SUCCESS)
                throw std::runtime_error(SVKFW_WRAPERR("vk logical device","failed to create device"));
            volkLoadDevice(device);
        }

        ~VKFWDevice() {
            vkDestroyDevice(device, _destroyCb);
        }
    };


//  ============  Logical device - volk table interface version - do it after default version is done  ============  \\

    struct VKFWVolkTIDevice {
        VkDevice device;
        VolkDeviceTable table;
        const VkAllocationCallbacks *_destroyCb = nullptr;

        VKFWVolkTIDevice(const VKFWPhysDevice &_physDevice, const VKFWDeviceInfo &_info,
                         const VkAllocationCallbacks *_cAlloc = nullptr,
                         const VkAllocationCallbacks *_dAlloc = nullptr) : _destroyCb{_dAlloc} {
            if (vkCreateDevice(_physDevice.physDevice, &_info.info, _cAlloc, &device) != VK_SUCCESS)
                throw std::runtime_error(SVKFW_WRAPERR("vk logical device (TI)","failed to create device"));
            volkLoadDeviceTable(&table, device);
        }

        ~VKFWVolkTIDevice() {
            vkDestroyDevice(device, _destroyCb);
        }
    };


//  ============  Queue handle  ============  \\

    struct VKFWQueue {
        VkQueue queue;

        VKFWQueue(const VKFWDevice &_device, const VKFWQueueFamily &_family, uint32_t _qIndex) {
            vkGetDeviceQueue(_device.device, _family.family, _qIndex, &queue);
        }

        VKFWQueue(const VKFWVolkTIDevice &_device, const VKFWQueueFamily &_family, uint32_t _qIndex) {
            _device.table.vkGetDeviceQueue(_device.device, _family.family, _qIndex, &queue);
        }
    };


//  ============  Vulkan surface  ============  \\

    struct VKFWSurface {
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        const VkAllocationCallbacks *_destroyCb = nullptr;

        VKFWSurface(GLFWwindow *_window,
                    const VkAllocationCallbacks *_cAlloc = nullptr,
                    const VkAllocationCallbacks *_dAlloc = nullptr) : _destroyCb{_dAlloc} {
            // the easy way
            if (glfwCreateWindowSurface(VKFWInstance::instance, _window, _cAlloc, &surface) != VK_SUCCESS)
                throw std::runtime_error(SVKFW_WRAPERR("VKFWSurface","could not create surface"));
        }
        ~VKFWSurface() {
            vkDestroySurfaceKHR(VKFWInstance::instance, surface, _destroyCb);
        }
    };


// VKFWPhysDevice -> presentation support

    bool VKFWQFamiliesHandler::checkQFPresentationSupport(VKFWQueueFamily _family, const VKFWSurface &_surface) {
        VkBool32 __res = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(physDevice.physDevice, _family.family, _surface.surface, &__res);
        return __res;
    }

    VKFWQueueFamily VKFWQFamiliesHandler::findAnyQFWithSurfaceSupport(const VKFWSurface &_surface) {
        for (uint i = 0u; i < properties.size(); ++i)
            if (checkQFPresentationSupport({i}, _surface)) return { i };

        fprintf(outswarn, SVKFW_WRAPWARN("find queue family (presentation)","could not find queue family"));
        return {uint(-1)};
    }


//  ============  Vulkan swap chain info  ============  \\

    struct VKFWSwapChainInfo {
        VkSwapchainCreateInfoKHR info;
        VkSurfaceCapabilitiesKHR capabilities;
        VKFWPhysDevice physDevice;

        VKFWSwapChainInfo(VKFWPhysDevice _physDevice, const VKFWSurface &_surface) {
            physDevice = _physDevice;
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physDevice.physDevice, _surface.surface, &capabilities);

            info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            info.pNext = nullptr;
            info.flags = 0u;
            info.surface = _surface.surface;
            info.imageFormat = VK_FORMAT_UNDEFINED;
            info.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
            info.imageArrayLayers = 1; // gonna be hard-coded for a long time
            info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // a lot can be done here
            info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            info.queueFamilyIndexCount = 0;
            info.pQueueFamilyIndices = nullptr;
            info.preTransform = capabilities.currentTransform;
            info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            info.presentMode = VK_PRESENT_MODE_MAX_ENUM_KHR;
            info.clipped = VK_TRUE;
            info.oldSwapchain = VK_NULL_HANDLE;
        }

        ~VKFWSwapChainInfo() {
            if (info.pQueueFamilyIndices != nullptr) {
                delete[] info.pQueueFamilyIndices;
                info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
                info.queueFamilyIndexCount = 0;
                info.pQueueFamilyIndices = nullptr;
            }
        }


        struct Suitability {
            typedef bool (*FormatTestFunc)(const VkSurfaceFormatKHR &_format);
            typedef int  (*FormatRankFunc)(const VkSurfaceFormatKHR &_format);
            typedef bool (*PresentModeTestFunc)(VkPresentModeKHR _presentMode);
            typedef int  (*PresentModeRankFunc)(VkPresentModeKHR _presentMode);

            static bool defaultFormatTest(const VkSurfaceFormatKHR &_format) {
                return _format.format == VK_FORMAT_B8G8R8A8_SRGB && _format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR;
            }

            static bool defaultPresentModeTest(VkPresentModeKHR _presentMode) {
                return _presentMode == VK_PRESENT_MODE_FIFO_KHR;
            }
        };


        void setMultipleQueueFaimilies(const std::vector<VKFWQueueFamily> &_qfamilies) {
            info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            info.queueFamilyIndexCount = _qfamilies.size();

            uint32_t *__qfam_ptr = new uint32_t[_qfamilies.size()];
            for (uint i = 0u; i < _qfamilies.size(); ++i)
                __qfam_ptr[i] = _qfamilies[i].family;

            info.pQueueFamilyIndices = __qfam_ptr;
        }

        void setFlags(VkSwapchainCreateFlagsKHR _flags) {
            info.flags = _flags;
        }

        void setCompositeAlpha(VkCompositeAlphaFlagBitsKHR _alpha_mode) {
            info.compositeAlpha = _alpha_mode;
        }

        void setClipping(bool _clipped) {
            info.clipped = VkBool32(_clipped);
        }

        void chooseImageCount(uint32_t _count) {
            info.minImageCount = Func::clamp_GE_LE(_count,
                                                    capabilities.minImageCount,
                                                    capabilities.maxImageCount ? // (== 0) means no max
                                                    capabilities.maxImageCount : uint32_t(-1));

            if (info.minImageCount != _count)
                fprintf(outswarn, SVKFW_WRAPWARN("swapchain info, choose img count",
                                  "parameter \'_count\' (%d) was clamped to %d to fit the interval [%d, %d]\n"),
                                     _count, info.minImageCount, capabilities.minImageCount, capabilities.maxImageCount);
            else printf(SVKFW_WRAPINFO("swapchain info, choose img count", "imageCount: %d, the interval: [%d, %d]\n"),
                                     info.minImageCount, capabilities.minImageCount, capabilities.maxImageCount);
        }

        bool transformSupported(VkSurfaceTransformFlagBitsKHR _tform) {
            return capabilities.supportedTransforms & _tform;
        }

        void setTransform(VkSurfaceTransformFlagBitsKHR _tform) {
            info.preTransform = _tform;
        }

        void chooseSwapExtent(const Window &_window) {
            if (capabilities.currentExtent.width != uint32_t(-1)) {
                info.imageExtent = capabilities.currentExtent;
                printf(SVKFW_WRAPINFO("swapchain info, choose swap extent", "resulting extent: [%d, %d]\n"), 
                                        info.imageExtent.width, info.imageExtent.height);
                return;
            }
            else {
                UVec2 __fbsize{_window.getFramebufferSize()};

                info.imageExtent = { Func::clamp_GE_LE(__fbsize.x,
                                        capabilities.minImageExtent.width,
                                        capabilities.maxImageExtent.width),
                                     Func::clamp_GE_LE(__fbsize.y,
                                        capabilities.minImageExtent.height,
                                        capabilities.maxImageExtent.height)  };
                printf(SVKFW_WRAPINFO("swapchain info, choose swap extent", "resulting extent (no max): [%d, %d]\n"),
                                        info.imageExtent.width, info.imageExtent.height);
            }
        }

        void chooseFormat(const Suitability::FormatTestFunc _testFunc = Suitability::defaultFormatTest) {
            std::vector <VkSurfaceFormatKHR> __formats;
            getAllFormats(physDevice.physDevice, info.surface, __formats);

            for (auto __format : __formats) {
                if (_testFunc(__format)) {
                    info.imageFormat     = __format.format;
                    info.imageColorSpace = __format.colorSpace;
                    printf(SVKFW_WRAPINFO("swapchain info, choose surface format", "format: %d, color space: %d\n"), uint(__format.format), uint(__format.colorSpace));
                    return;
                }
            }
            throw std::runtime_error(SVKFW_WRAPERR("swapchain info, choose surface format (test)", "could not choose format"));
        }

        void chooseFormat(const Suitability::FormatRankFunc _rankFunc) {
            std::vector <VkSurfaceFormatKHR> __formats;
            getAllFormats(physDevice.physDevice, info.surface, __formats);

            int __maxRank = INT_MIN;
            uint __currInd = -1;
            for (uint i = 0u; i < __formats.size(); ++i) {
                int __rank = _rankFunc(__formats[i]);
                if (__rank > __maxRank) {
                    __maxRank = __rank;
                    __currInd = i;
                }
            }

            if (__currInd > __formats.size())
                throw std::runtime_error(SVKFW_WRAPERR("swapchain info, choose surface format (rank)", "could not choose format"));
            printf(SVKFW_WRAPINFO("swapchain info, choose surface format", "format: %d, color space: %d\n"),
                    uint(__formats[__currInd].format), uint(__formats[__currInd].colorSpace));
            info.imageFormat     = __formats[__currInd].format;
            info.imageColorSpace = __formats[__currInd].colorSpace;
        }

        // returns true if successfully chose presentation mode using test function
        bool choosePresentationMode(const Suitability::PresentModeTestFunc _testFunc = Suitability::defaultPresentModeTest) {
            std::vector <VkPresentModeKHR> __presModes;
            getAllPresentationModes(physDevice.physDevice, info.surface, __presModes);

            for (auto __mode : __presModes)
                if (_testFunc(__mode)) {
                    info.presentMode = __mode;
                    printf(SVKFW_WRAPINFO("swapchain info, choose presentation mode", "mode %d\n"), uint(__mode));
                    return true;
                }

            fprintf(outswarn, SVKFW_WRAPWARN("swapchain info, choose presentation mode",
                                                "could not choose presentation mode, defaults to VK_PRESENT_MODE_FIFO_KHR\n"));
            info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
            return false;
        }

        void choosePresentationMode(const Suitability::PresentModeRankFunc _rankFunc) {
            std::vector <VkPresentModeKHR> __presModes;
            getAllPresentationModes(physDevice.physDevice, info.surface, __presModes);

            int __maxRank = INT_MIN;
            VkPresentModeKHR __currMode = VK_PRESENT_MODE_FIFO_KHR;
            for (auto __mode : __presModes) {
                int __rank = _rankFunc(__mode);
                if (__rank >= __maxRank) {
                    __maxRank  = __rank;
                    __currMode = __mode;
                }
            }
            printf(SVKFW_WRAPINFO("swapchain info, choose presentation mode", "mode %d\n"), uint(__currMode));
            info.presentMode = __currMode;
        }


    // Static

        static void getAllFormats(VkPhysicalDevice _physDevice, const VkSurfaceKHR &_surface, std::vector <VkSurfaceFormatKHR> &_formats) {
            uint32_t __formatCount = 0u;
            vkGetPhysicalDeviceSurfaceFormatsKHR(_physDevice,
                                                _surface, &__formatCount, nullptr);
            if (__formatCount != 0u) {
                _formats.resize(__formatCount);
                vkGetPhysicalDeviceSurfaceFormatsKHR(_physDevice,
                                                _surface, &__formatCount, _formats.data());
            }
        }

        static void getAllPresentationModes(VkPhysicalDevice _physDevice, const VkSurfaceKHR &_surface, std::vector <VkPresentModeKHR> &_presModes) {
            uint32_t __presModeCount = 0u;
            vkGetPhysicalDeviceSurfacePresentModesKHR(_physDevice,
                                                    _surface, &__presModeCount, nullptr);

            if (__presModeCount != 0) {
                _presModes.resize(__presModeCount);
                vkGetPhysicalDeviceSurfacePresentModesKHR(_physDevice,
                                                    _surface, &__presModeCount, _presModes.data());
            }
        }
    };


//  ============  Vulkan image  ============  \\

    // TODO: apparently needs VKFWSwapChainInfo
    struct VKFWImageInfo {
        VkImageCreateInfo info;

        VKFWImageInfo() {
            info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            info.pNext = VK_NULL_HANDLE;
            info.flags = 0u;
        }
    };

    struct VKFWImage {
        VkImage image;
        VkDevice device;
        const VkAllocationCallbacks *_destroyCb = nullptr;

        VKFWImage(VkDevice _logicDevice, const VKFWImageInfo &_info,
                  const VkAllocationCallbacks *_cAlloc = nullptr,
                  const VkAllocationCallbacks *_dAlloc = nullptr) :
                    _destroyCb{_dAlloc}, device{device} {
            vkCreateImage(device, &_info.info, _cAlloc, &image);
        }
        ~VKFWImage() {
            vkDestroyImage(device, image, _destroyCb);
            device = 0;
        }
    };


//  ============  Vulkan image view  ============  \\

    struct VKFWImageViewInfo {
        VkImageViewCreateInfo info;

        VKFWImageViewInfo(const VkImage &_image, const VKFWSwapChainInfo &_sc_info) {
            info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            info.pNext = nullptr;
            info.flags = 0u;
            info.image = _image;
            info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            info.format = _sc_info.info.imageFormat;
        }

        void chooseComponents(VkComponentSwizzle _r, VkComponentSwizzle _g, VkComponentSwizzle _b, VkComponentSwizzle _a) {
            info.components.r = _r;
            info.components.g = _g;
            info.components.b = _b;
            info.components.a = _a;
        }
        void setRange() {
            info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            info.subresourceRange.baseMipLevel = 0;
            info.subresourceRange.levelCount = 1;
            info.subresourceRange.baseArrayLayer = 0;
            info.subresourceRange.layerCount = 1;
        }
    };

    struct VKFWImageView {
        VkImageView view;
        VkDevice device;
        const VkAllocationCallbacks *_createCb = nullptr, *_destroyCb = nullptr;

        VKFWImageView(VkDevice _logicDevice, const VKFWImageViewInfo &_info,
                      const VkAllocationCallbacks *_cAlloc = nullptr,
                      const VkAllocationCallbacks *_dAlloc = nullptr) :
                            device{_logicDevice}, _createCb{_cAlloc}, _destroyCb{_dAlloc} {
            if (vkCreateImageView(device, &_info.info, _createCb, &view) != VK_SUCCESS)
                throw std::runtime_error(SVKFW_WRAPERR("create image view", "could not create image view"));
        }
        ~VKFWImageView() {
            vkDestroyImageView(device, view, _destroyCb);
            device = 0u;
        }
    };


//  ============  Vulkan viewport / scissors(Rect2D)  ============  \\

    struct VKFWViewport {
        VkViewport viewport;

        VKFWViewport(const VkExtent2D &_extent, float _x = 0.f, float _y = 0.f, float _depthMin = 0.f, float _depthMax = 1.f) :
                    viewport{.x= (float)_x, .y= (float)_y, .width= (float)_extent.width, .height= (float)_extent.height,
                                            .minDepth= _depthMin, .maxDepth= _depthMax} {}
        VKFWViewport(const Vec2 &_extent, const Vec2 &_offset = 0.f, const Vec2 &_depth = { 0.f, 1.f }) :
                    viewport{.x= _offset.x, .y= _offset.y, .width= _extent.x, .height= _extent.y,
                                            .minDepth= _depth.x, .maxDepth= _depth.y} {}
        VKFWViewport(const VKFWViewport &_viewport) : viewport{_viewport.viewport} {}

        Vec2 offset() const { return { viewport.x, viewport.y }; }
        void setOffset(const Vec2 &_offset) { viewport.x = _offset.x; viewport.y = _offset.y; }
        Vec2 extent() const { return { viewport.width, viewport.height }; }
        void setExtent(const Vec2 &_extent) { viewport.width = _extent.x; viewport.height = _extent.y; }
        Vec2 depth() const { return { viewport.minDepth, viewport.maxDepth }; }
        void setDepth(const Vec2 &_depth) { viewport.minDepth = _depth.x; viewport.maxDepth = _depth.y; }
    };

    struct VKFWRect2D {
        VkRect2D rectangle;

        VKFWRect2D(const VkExtent2D &_extent, const VkOffset2D &_offset = {0, 0}) :
                rectangle{ .offset= {_offset.x, _offset.y}, .extent= {_extent.width, _extent.height} } {}
        VKFWRect2D(const UVec2 &_extent, const IVec2 &_offset = 0) :
                rectangle{ .offset= {_offset.x, _offset.y}, .extent= {_extent.x, _extent.y} } {}

        IVec2 offset() const { return { rectangle.offset.x, rectangle.offset.y }; }
        void setOffset(const IVec2 &_offset) { rectangle.offset.x = _offset.x; rectangle.offset.y = _offset.y; }
        UVec2 extent() const { return { rectangle.extent.width, rectangle.extent.height }; }
        void setExtent(const UVec2 &_extent) { rectangle.extent.width = _extent.x; rectangle.extent.height = _extent.y; }
    };


//  ============  Vulkan swap chain  ============  \\

    struct VKFWSwapChain {
        VkSwapchainKHR swapchain;
        std::vector <VkImage> images;
        std::vector <VKFWImageView> imageViews;
        VkDevice device;
        VkFormat format;
        VkExtent2D extent;
        const VkAllocationCallbacks *_createCb = nullptr, *_destroyCb = nullptr;

        VKFWSwapChain(VkDevice _logicDevice, const VKFWSwapChainInfo &_info,
                        const VkAllocationCallbacks *_cAlloc = nullptr,
                        const VkAllocationCallbacks *_dAlloc = nullptr) :
                               _createCb{_cAlloc}, _destroyCb{_dAlloc}, device{_logicDevice},
                               format{_info.info.imageFormat}, extent{_info.info.imageExtent} {
            if (vkCreateSwapchainKHR(device, &_info.info, _createCb, &swapchain) != VK_SUCCESS)
                throw std::runtime_error(SVKFW_WRAPERR("swap chain", "could not create swap chain"));

            getAllSwapChainImages(images);

            imageViews.reserve(images.size());
            for (const auto &__img : images) {
                VKFWImageViewInfo __viewInfo{__img, _info};
                __viewInfo.chooseComponents(VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                            VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY);
                __viewInfo.setRange();
                imageViews.emplace_back(device, __viewInfo);
            }
        }

        // VKFWSwapChain(const VKFWVolkTIDevice &_logicDevice, const VKFWSwapChainInfo &_info,
        //                 const VkAllocationCallbacks *_cAlloc = nullptr,
        //                 const VkAllocationCallbacks *_dAlloc = nullptr)
        //                     : _createCb{_cAlloc}, _destroyCb{_dAlloc}, device{_logicDevice} {
        //     if (_logicDevice.table.vkCreateSwapchainKHR(_logicDevice, &_info.info, _createCb, &swapchain) != VK_SUCCESS)
        //         throw std::runtime_error(SVKFW_WRAPERR("swap chain", "could not create swap chain"));
        // }

        ~VKFWSwapChain() {
            if (device != VK_NULL_HANDLE) {
                vkDestroySwapchainKHR(device, swapchain, _destroyCb);
                device = VK_NULL_HANDLE;
            }
        }

        void getAllSwapChainImages(std::vector <VkImage> &_images) {
            uint32_t __imCount = 0u;
            vkGetSwapchainImagesKHR(device, swapchain, &__imCount, nullptr);
            _images.resize(__imCount);
            vkGetSwapchainImagesKHR(device, swapchain, &__imCount, _images.data());
        }
    };


//  ============  Vulkan shader module  ============  \\

    struct VKFWShaderModule {
        VkShaderModule shader;
        VkDevice device;
        const VkAllocationCallbacks *_createCb = nullptr, *_destroyCb = nullptr;

        VKFWShaderModule(VkDevice _logicDevice,
                        const VkAllocationCallbacks *_cAlloc = nullptr,
                        const VkAllocationCallbacks *_dAlloc = nullptr) :
                            shader{VK_NULL_HANDLE}, device{_logicDevice},
                            _createCb{_cAlloc}, _destroyCb{_dAlloc} {}
        ~VKFWShaderModule() {
            vkDestroyShaderModule(device, shader, nullptr);
            device = VK_NULL_HANDLE;
        }

// static shader loaders

        void fromFile(const char *_fpath) {
            std::ifstream __sh_file{_fpath, std::ios::ate | std::ios::binary};

            if (!__sh_file.is_open()) {
                throw std::runtime_error(SVKFW_WRAPERR("VKFWShaderModule FromFile", "failed to open file"));
            }

            size_t __f_size = (size_t) __sh_file.tellg();
            std::vector<char> __buffer(__f_size);

            __sh_file.seekg(0);
            __sh_file.read(__buffer.data(), __f_size);
            __sh_file.close();

            createShaderModule(__buffer);
        }

        void fromString(const char *_sh_str, size_t _length = 0u) {
            if (_sh_str == nullptr)
                throw std::invalid_argument(SVKFW_WRAPERR("VKFWShaderModule fromString", "no string provided"));
            if (_length == 0u) {
                while (_sh_str[_length] != '\0') ++_length;
                if (_length == 0u)
                    throw std::invalid_argument(SVKFW_WRAPERR("VKFWShaderModule fromString", "string is empty"));
            }

            std::vector<char> __buffer(_length);
            // check if works with memcpy
            for (uint i = 0u; i < _length; ++i)
                __buffer[i] = _sh_str[i];

            createShaderModule(__buffer);
        }


// create shader module

        void createShaderModule(const std::vector<char> &_buffer) {
            VkShaderModuleCreateInfo __info{};
            __info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            __info.codeSize = _buffer.size();
            __info.pCode = reinterpret_cast<const uint32_t *>(_buffer.data());

            if (vkCreateShaderModule(device, &__info, nullptr, &shader) != VK_SUCCESS)
                throw std::runtime_error(SVKFW_WRAPERR("createShaderModule", "failed to create shader module"));
        }
    };


    struct VKFWSpecializationInfo {
        VkSpecializationInfo info;

        VKFWSpecializationInfo() : info{ 0u, nullptr, 0u, nullptr } {}
        VKFWSpecializationInfo(const std::vector<VkSpecializationMapEntry> &_entries,
                               const void *_data, uint32_t _data_size) {
            info.mapEntryCount = _entries.size();

            VkSpecializationMapEntry *__tmpptr = new VkSpecializationMapEntry[_entries.size()];
            memcpy(__tmpptr, _entries.data(), _entries.size() * sizeof(VkSpecializationMapEntry));
            info.pMapEntries = __tmpptr;

            info.dataSize = _data_size;
            info.pData = _data;
        }

        const VkSpecializationInfo *getInfo() { return &info; }
    };


//  ================================================  \\
                Vulkan graphics pipeline
//  ================================================  \\

//  ============  Vulkan graphics pipeline config namespace  ============  \\

    // contains wrappers for states/shader stages of graphics pipeline
    namespace VKFWGraphicsPipelineConfig {

        struct VKFWShaderStages {
            std::vector <VkShaderModule> stages;
            std::vector <VkPipelineShaderStageCreateInfo> stage_infos;
            VkDevice device;
            const VkAllocationCallbacks *_createCb = nullptr, *_destroyCb = nullptr;

            VKFWShaderStages(VkDevice _logicDevice,
                             const VkAllocationCallbacks *_cAlloc = nullptr,
                             const VkAllocationCallbacks *_dAlloc = nullptr) :
                                device{_logicDevice}, _createCb{_cAlloc}, _destroyCb{_dAlloc} {}
            ~VKFWShaderStages() {
                for (uint i = 0u; i < stages.size(); ++i)
                    vkDestroyShaderModule(device, stages[i], _destroyCb);
            }

// VKFWShaderStages: shader loaders

            uint32_t addFromFile(const char *_fpath) {
                std::ifstream __sh_file{_fpath, std::ios::ate | std::ios::binary};

                if (!__sh_file.is_open()) {
                    throw std::runtime_error(SVKFW_WRAPERR("VKFWShaderStages FromFile", "failed to open file"));
                }

                size_t __f_size = (size_t) __sh_file.tellg();
                std::vector<char> __buffer(__f_size);

                __sh_file.seekg(0);
                __sh_file.read(__buffer.data(), __f_size);
                __sh_file.close();

                return createVkShaderModule(__buffer);
            }

            uint32_t addFromString(const char *_sh_str, size_t _length = 0u) {
                if (_sh_str == nullptr)
                    throw std::invalid_argument(SVKFW_WRAPERR("VKFWShaderStages fromString", "no string provided"));
                if (_length == 0u) {
                    while (_sh_str[_length] != '\0') ++_length;
                    if (_length == 0u)
                        throw std::invalid_argument(SVKFW_WRAPERR("VKFWShaderStages fromString", "string is empty"));
                }

                std::vector<char> __buffer(_length);
                // check if works with memcpy
                for (uint i = 0u; i < _length; ++i)
                    __buffer[i] = _sh_str[i];

                return createVkShaderModule(__buffer);
            }

// VKFWShaderStages: create shader module

            uint32_t createVkShaderModule(const std::vector<char> &_buffer) {
                VkShaderModuleCreateInfo __info{};
                __info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                __info.pNext = nullptr;
                __info.flags = 0u;
                __info.codeSize = _buffer.size();
                __info.pCode = reinterpret_cast<const uint32_t *>(_buffer.data());

                VkShaderModule _shader = VK_NULL_HANDLE;
                if (vkCreateShaderModule(device, &__info, _createCb, &_shader) != VK_SUCCESS)
                    throw std::runtime_error(SVKFW_WRAPERR("VKFWShaderStages::createShaderModule",
                                                            "failed to create shader module"));

                stages.push_back(_shader);
                return stages.size() - 1;
            }

// VKFWShaderStages: create shader stage info (with existing shader module by its index)

            void createShaderStageInfo(uint32_t _shaderIndex, VkShaderStageFlagBits _stage,
                                       const char *_name, const VKFWSpecializationInfo &_spec_info = {}) {
                if (_shaderIndex >= stages.size())
                    throw std::invalid_argument(SVKFW_WRAPERR("VKFWShaderStages stage info",
                                                "shader stage index out of range")); // out_of_range exception

                if (_shaderIndex >= stage_infos.size())
                    stage_infos.resize(_shaderIndex + 1);

                VkPipelineShaderStageCreateInfo __info;
                __info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                __info.pNext  = nullptr;
                __info.flags  = 0u;
                __info.stage  = _stage;
                __info.module = stages[_shaderIndex];
                __info.pName  = _name;
                __info.pSpecializationInfo = nullptr;
                if (_spec_info.info.dataSize != 0u || _spec_info.info.mapEntryCount != 0u) {
                    VkSpecializationInfo *__spec_info = new VkSpecializationInfo{_spec_info.info};
                    __info.pSpecializationInfo = __spec_info;
                }
                stage_infos[_shaderIndex] = __info;
            }

        }; // VKFWShaderStages END


        // optional
        struct VKFWDynamicState {
            VkPipelineDynamicStateCreateInfo info;
            std::vector<VkDynamicState> states;

            VKFWDynamicState() {
                info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
                info.pNext = nullptr;
                info.flags = 0u;
            }


            // returns false if state is already included in "states", true if successfully added
            bool addDynamicState(VkDynamicState _state) {
                for (const VkDynamicState &st : states)
                    if (st == _state) return false;

                states.push_back(_state);
                return true;
            }

            // returns false if state is not in "states", true if successfully removed first entry
            bool delDynamicState(VkDynamicState _state) {
                for (uint i = 0u; i < states.size(); ++i) {
                    if (states[i] == _state) {
                        states.erase(states.begin() + i);
                        return true;
                    }
                }
                return false;
            }

            // adds all input states if not present
            void addDynamicStates(const std::vector<VkDynamicState> &_states) {
                for (const VkDynamicState &in_st : _states) {
                    bool __skip = false;
                    for (const VkDynamicState &st : states) {
                        if (in_st == st) {
                            __skip = true;
                            break;
                        }
                    }
                    if (!__skip)
                        states.push_back(in_st);
                }
            }

            // removes all input states if present. is "_states" is empty, clears all dynamic states
            void delDynamicStates(const std::vector<VkDynamicState> &_states) {
                if (_states.size() == 0u) states.clear();
                for (const VkDynamicState &in_st : _states) {
                    for (uint i = 0u; i < states.size(); ++i) {
                        if (in_st == states[i]) {
                            states.erase(states.begin() + i);
                            break;
                        }
                    }
                }
            }

            const VkPipelineDynamicStateCreateInfo *getInfo() {
                info.dynamicStateCount = states.size();
                info.pDynamicStates = states.size() ? states.data() : nullptr;
                return &info;
            }
        }; // VKFWDynamicState END


        struct VKFWVertexInputState {
            VkPipelineVertexInputStateCreateInfo info;
            std::vector <VkVertexInputBindingDescription> bindings;
            std::vector <VkVertexInputAttributeDescription> attributes;

            VKFWVertexInputState() {
                info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
                info.pNext = nullptr;
                info.flags = 0u;
                info.vertexBindingDescriptionCount = 0u;
                info.pVertexBindingDescriptions = nullptr;
                info.vertexAttributeDescriptionCount = 0u;
                info.pVertexAttributeDescriptions = nullptr;
            }


            void setBindings(const std::vector<VkVertexInputBindingDescription> &_bindings) {
                bindings = _bindings;

                info.vertexBindingDescriptionCount = bindings.size();
                info.pVertexBindingDescriptions = bindings.data();
                if (info.vertexBindingDescriptionCount == 0u)
                    info.pVertexBindingDescriptions = nullptr;
            }

            void setAttributes(const std::vector<VkVertexInputAttributeDescription> &_attributes) {
                attributes = _attributes;

                info.vertexAttributeDescriptionCount = attributes.size();
                info.pVertexAttributeDescriptions = attributes.data();
                if (info.vertexAttributeDescriptionCount == 0u)
                    info.pVertexAttributeDescriptions = nullptr;
            }

            const VkPipelineVertexInputStateCreateInfo *getInfo() { return &info; }
        }; // VKFWVertexInputState END


        struct VKFWInputAssemblyState {
            VkPipelineInputAssemblyStateCreateInfo info;

            VKFWInputAssemblyState() {
                info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
                info.pNext = nullptr;
                info.flags = 0u;
            }
            VKFWInputAssemblyState(VkPrimitiveTopology _topology_type, bool _primitive_restart = false) {
                info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
                info.pNext = nullptr;
                info.flags = 0u;
                info.topology = _topology_type;
                info.primitiveRestartEnable = VkBool32(_primitive_restart);
            }


            void setTopology(VkPrimitiveTopology _topology_type) {
                info.topology = _topology_type;
            }
            void setRestart(bool _primitive_restart) {
                info.primitiveRestartEnable = VkBool32(_primitive_restart);
            }

            const VkPipelineInputAssemblyStateCreateInfo *getInfo() {
                if (info.topology == VK_PRIMITIVE_TOPOLOGY_MAX_ENUM)
                    throw std::runtime_error(SVKFW_WRAPERR("Pipeline VKFWInputAssemblyState", "topology type not set"));
                return &info;
            }
        }; // VKFWInputAssemblyState END


        struct VKFWTessellationState {
            VkPipelineTessellationStateCreateInfo info;

            VKFWTessellationState() {
                info.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
                info.pNext = nullptr;
                info.flags = 0u; // 0 in Vk 1.3
                info.patchControlPoints = 0u;
            }

            void setPatchControlPoints(uint32_t _cPoints) {
                info.patchControlPoints = _cPoints;
            }

            const VkPipelineTessellationStateCreateInfo *getInfo() { return &info; }
        }; // VKFWTesselationState END


        struct VKFWViewportState {
            VkPipelineViewportStateCreateInfo info;
            std::vector<VkViewport> viewports;
            std::vector<VkRect2D> scissors;

            VKFWViewportState() {
                info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
                info.pNext = nullptr;
                info.flags = 0u;
            }


            void setViewportsDynamic(uint32_t _viewportCount) {
                viewports.clear();
                info.viewportCount = _viewportCount;
                info.pViewports = nullptr;
            }
            void setViewports(const std::vector<VKFWViewport> &_viewports) {
                viewports.resize(_viewports.size());
                for (uint i = 0u; i < _viewports.size(); ++i)
                    viewports[i] = _viewports[i].viewport;
                info.viewportCount = viewports.size();
                info.pViewports = viewports.size() ? viewports.data() : nullptr;
            }
            void addViewports(const std::vector<VKFWViewport> &_viewports) {
                for (const auto &__viewport : _viewports)
                    viewports.push_back(__viewport.viewport);
                info.viewportCount = viewports.size();
                info.pViewports = viewports.size() ? viewports.data() : nullptr;
            }

            void setScissorsDynamic(uint32_t _scissorCount) {
                scissors.clear();
                info.scissorCount = _scissorCount;
                info.pScissors = nullptr;
            }
            void setScissors(const std::vector<VKFWRect2D> &_scissors) {
                scissors.resize(info.scissorCount);
                for (uint i = 0u; i < info.scissorCount; ++i)
                    scissors[i] = _scissors[i].rectangle;
                info.scissorCount = scissors.size();
                info.pScissors = scissors.size() ? scissors.data() : nullptr;
            }
            void addScissors(const std::vector<VKFWRect2D> &_scissors) {
                for (const auto &__scissor : _scissors)
                    scissors.push_back(__scissor.rectangle);
                info.scissorCount = scissors.size();
                info.pScissors = scissors.size() ? scissors.data() : nullptr;
            }

            const VkPipelineViewportStateCreateInfo *getInfo() { return &info; }
        }; // VKFWViewportState END


        struct VKFWRasterizationState {
            VkPipelineRasterizationStateCreateInfo info;

            VKFWRasterizationState() {
                info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
                info.pNext = nullptr; // options exist
                info.flags = 0u; // 0 in Vk1.3
                info.depthClampEnable = VK_FALSE;
                info.rasterizerDiscardEnable = VK_FALSE;
                info.polygonMode = VK_POLYGON_MODE_MAX_ENUM;
                info.cullMode = VK_CULL_MODE_NONE;
                info.frontFace = VK_FRONT_FACE_MAX_ENUM;
                info.depthBiasEnable = VK_FALSE;
                info.depthBiasConstantFactor = 0.f;
                info.depthBiasClamp = 0.f;
                info.depthBiasSlopeFactor = 0.f;
                info.lineWidth = 1.f;
            }
            VKFWRasterizationState(VkPolygonMode _polygon, VkCullModeFlags _cull, VkFrontFace _front) {
                info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
                info.pNext = nullptr; // options exist
                info.flags = 0u; // 0 in Vk1.3
                info.depthClampEnable = VK_FALSE;
                info.rasterizerDiscardEnable = VK_FALSE;
                info.polygonMode = _polygon;
                info.cullMode = _cull;
                info.frontFace = _front;
                info.depthBiasEnable = VK_FALSE;
                info.depthBiasConstantFactor = 0.f;
                info.depthBiasClamp = 0.f;
                info.depthBiasSlopeFactor = 0.f;
                info.lineWidth = 1.f;
            }

            void setDepthClamp(bool _enable) { info.depthClampEnable = VkBool32(_enable); }
            void setRasterizerDiscard(bool _enable) { info.rasterizerDiscardEnable = VkBool32(_enable); }
            void setPolygonMode(VkPolygonMode _mode) { info.polygonMode = _mode; }
            void setCullMode(VkCullModeFlags _mode) { info.cullMode = _mode; }
            void setFrontFace(VkFrontFace _front) { info.frontFace = _front; }
            void setDepthBias(bool _enable, float _constFactor, float _clamp, float _slopeFactor) {
                if (!_enable) {
                    _constFactor = 0.f;
                    _clamp = 0.f;
                    _slopeFactor = 0.f;
                }
                info.depthBiasEnable = VkBool32(_enable);
                info.depthBiasConstantFactor = _constFactor;
                info.depthBiasClamp = _clamp;
                info.depthBiasSlopeFactor = _slopeFactor;
            }
            void setLineWidth(float _width) { info.lineWidth = _width; }

            const VkPipelineRasterizationStateCreateInfo *getInfo() { return &info; }
        }; // VKFWRasterizationState END


        struct VKFWMultisampleState {
            VkPipelineMultisampleStateCreateInfo info;

            VKFWMultisampleState() {
                info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
                info.pNext = nullptr;
                info.flags = 0u;
                info.sampleShadingEnable = VK_FALSE;
                info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
                info.minSampleShading = 1.0f; // Optional
                info.pSampleMask = nullptr; // Optional
                info.alphaToCoverageEnable = VK_FALSE; // Optional
                info.alphaToOneEnable = VK_FALSE; // Optional
            }

            // TODO

            const VkPipelineMultisampleStateCreateInfo *getInfo() { return &info; }
        }; // VKFWMultisampleState END


        struct VKFWDepthStencilState {
            VkPipelineDepthStencilStateCreateInfo info;

            VKFWDepthStencilState() {
                info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
                info.pNext = nullptr;
                info.flags = 0u;
                info.depthTestEnable = VK_FALSE;
                info.depthWriteEnable = VK_FALSE;
                info.depthCompareOp;
                info.depthBoundsTestEnable = VK_FALSE;
                info.stencilTestEnable = VK_FALSE;
                info.front;
                info.back;
                info.minDepthBounds;
                info.maxDepthBounds;
            }

            // TODO !!!

            const VkPipelineDepthStencilStateCreateInfo *getInfo() { return &info; }
        }; // VKFWDepthStencilState END


        struct VKFWColorBlendAttachmentState {
            VkPipelineColorBlendAttachmentState attachment;

            VKFWColorBlendAttachmentState() {
                // Porter-Duff blend over
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

            void setBlendEnable(bool _enable) { attachment.blendEnable = VkBool32(_enable); }
            void setColorWriteMask(VkColorComponentFlags _colors) { attachment.colorWriteMask = _colors; }
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
        }; // VKFWColorBlendAttachmentState END


        struct VKFWColorBlendState {
            VkPipelineColorBlendStateCreateInfo info;
            std::vector <VkPipelineColorBlendAttachmentState> attachments;

            VKFWColorBlendState() {
                info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
                info.pNext = nullptr;
                info.flags = 0u;
            }

            void setLogicOp(bool _enable, VkLogicOp _op) {
                info.logicOpEnable = VkBool32(_enable);
                info.logicOp = _op;
            }

            void setBlendConstants(const Vec4 &_constants) {
                info.blendConstants[0] = _constants.x;
                info.blendConstants[1] = _constants.y;
                info.blendConstants[2] = _constants.z;
                info.blendConstants[3] = _constants.w;
            }

            void addAttachments(const std::vector<VKFWColorBlendAttachmentState> &_states) {
                attachments.reserve(attachments.size() + _states.size());
                for (uint i = 0u; i < _states.size(); ++i)
                    attachments.push_back(_states[i].attachment);
                info.attachmentCount = attachments.size();
                info.pAttachments = attachments.size() ? attachments.data() : nullptr;
            }

            void setAttachments(const std::vector<VKFWColorBlendAttachmentState> &_states) {
                attachments.resize(_states.size());
                for (uint i = 0u; i < _states.size(); ++i)
                    attachments[i] = _states[i].attachment;
                info.attachmentCount = attachments.size();
                info.pAttachments = attachments.size() ? attachments.data() : nullptr;
            }

            const VkPipelineColorBlendStateCreateInfo *getInfo() { return &info; }
        }; // VKFWColorBlendState END


        struct VKFWDescriptorSetLayout {
            VkDescriptorSetLayout layout = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks *_destroyCb = nullptr;

            VKFWDescriptorSetLayout(VkDevice _logicDevice,
                                    const VkAllocationCallbacks *_dAlloc = nullptr) :
                        device{_logicDevice}, _destroyCb{_dAlloc} {}
            ~VKFWDescriptorSetLayout() {
                if (layout != VK_NULL_HANDLE) {
                    vkDestroyDescriptorSetLayout(device, layout, _destroyCb);
                }
            }

            void create(const std::vector<VkDescriptorSetLayoutBinding> &_bindings,
                        const VkAllocationCallbacks *_cAlloc = nullptr) {
                VkDescriptorSetLayoutCreateInfo info;
                info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                info.pNext = nullptr;
                info.flags = 0u;
                info.bindingCount = _bindings.size();
                info.pBindings = _bindings.size() ? _bindings.data() : nullptr;
                vkCreateDescriptorSetLayout(device, &info, _cAlloc, &layout);
            } 
        }; // VKFWDescriptorSetLayout END


        // Probably ANY pipeline
        // TODO!!!
        struct VKFWPipelineLayoutInfo {
            VkPipelineLayoutCreateInfo info;
            std::vector <VkDescriptorSetLayout> layouts;
            std::vector <VkPushConstantRange> constants;

            VKFWPipelineLayoutInfo() {
                info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
                info.pNext = nullptr;
                info.flags = 0u;
                info.setLayoutCount = 0u;
                info.pSetLayouts = nullptr; // struct
                info.pushConstantRangeCount = 0u;
                info.pPushConstantRanges = nullptr; // has create/destroy functions
            }
            VKFWPipelineLayoutInfo(const VKFWPipelineLayoutInfo &_info) :
                        info{_info.info}, layouts{_info.layouts}, constants{_info.constants} {
                info.setLayoutCount = layouts.size();
                info.pSetLayouts = layouts.size() ? layouts.data() : nullptr; // struct
                info.pushConstantRangeCount = constants.size();
                info.pPushConstantRanges = constants.size() ? constants.data() : nullptr; // has create/destroy functions
            }

            void setLayouts(const std::vector <VkDescriptorSetLayout> &_layouts) {
                layouts = _layouts;
                info.setLayoutCount = layouts.size();
                info.pSetLayouts = layouts.size() ? layouts.data() : nullptr;
            }

            void setConstRanges(const std::vector <VkPushConstantRange> &_constants) {
                constants = _constants;
                info.pushConstantRangeCount = constants.size();
                info.pPushConstantRanges = constants.size() ? constants.data() : nullptr;
            }

            // TODO: standartize getInfo, make it const and make getInfoUpd() - non-const, which updates structure members
            const VkPipelineLayoutCreateInfo *getInfo() const { return &info; }
        }; // VKFWPipelineLayoutInfo END


        struct VKFWPipelineLayout {
            VkPipelineLayout layout = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks *_destroyCb = nullptr;

            VKFWPipelineLayout(VkDevice _logicDevice,
                               const std::vector<VkDescriptorSetLayout> &_layouts,
                               const std::vector<VkPushConstantRange> &_constRanges,
                               const VkAllocationCallbacks *_cAlloc = nullptr,
                               const VkAllocationCallbacks *_dAlloc = nullptr) :
                            device{_logicDevice}, _destroyCb{_dAlloc} {
                VkPipelineLayoutCreateInfo __info;
                __info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
                __info.pNext = nullptr;
                __info.flags = 0u;
                __info.setLayoutCount = _layouts.size();
                __info.pSetLayouts = _layouts.size() ? _layouts.data() : nullptr; // struct
                __info.pushConstantRangeCount = _constRanges.size();
                __info.pPushConstantRanges = _constRanges.size() ? _constRanges.data() : nullptr; // has create/destroy functions
                if (vkCreatePipelineLayout(device, &__info, _cAlloc, &layout) != VK_SUCCESS)
                    throw std::runtime_error(SVKFW_WRAPERR("VKFWPipelineLayout 1", "could not create pipeline layout"));
            }
            VKFWPipelineLayout(VkDevice _logicDevice, const VKFWPipelineLayoutInfo &_info,
                               const VkAllocationCallbacks *_cAlloc = nullptr,
                               const VkAllocationCallbacks *_dAlloc = nullptr) :
                            device{_logicDevice} {
                if (vkCreatePipelineLayout(device, _info.getInfo(), _cAlloc, &layout) != VK_SUCCESS)
                    throw std::runtime_error(SVKFW_WRAPERR("VKFWPipelineLayout 2", "could not create pipeline layout"));
            }
            ~VKFWPipelineLayout() {
                if (layout != VK_NULL_HANDLE) {
                    vkDestroyPipelineLayout(device, layout, _destroyCb);
                    layout = VK_NULL_HANDLE;
                    device = VK_NULL_HANDLE;
                    _destroyCb = nullptr; // do this everywhere
                }
            }
        }; // VKFWPipelineLayout END


        struct VKFWAttachmentDescription {
            VkAttachmentDescription description;

            VKFWAttachmentDescription() {
                description.flags = 0u;
            }
            VKFWAttachmentDescription(const VkAttachmentDescription &_description) :
                        description{_description} {}


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
        }; // VKFWAttachmentDescription END


        struct VKFWSubpassDescription {
            VkSubpassDescription description;
            VkAttachmentReference depthStencilAttachment;
            std::vector <VkAttachmentReference> colorAttachments,
                                                inputAttachments,
                                                resolveAttachments;
            std::vector <uint32_t> preserveAttachments;

            VKFWSubpassDescription() {
                description.flags = 0u;
                description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_MAX_ENUM; // in VKFWGraphicsPipelineConfig namespace
                description.inputAttachmentCount = 0u;
                description.pInputAttachments    = nullptr;
                description.colorAttachmentCount = 0u;
                description.pColorAttachments    = nullptr;
                description.pResolveAttachments  = nullptr;
                description.pDepthStencilAttachment = nullptr;
                description.preserveAttachmentCount = 0u;
                description.pPreserveAttachments    = nullptr;
            }
            VKFWSubpassDescription(const VKFWSubpassDescription &_description) :
                            description{_description.description},
                            colorAttachments{_description.colorAttachments},
                            inputAttachments{_description.inputAttachments},
                            resolveAttachments{_description.resolveAttachments},
                            preserveAttachments{_description.preserveAttachments},
                            depthStencilAttachment{_description.depthStencilAttachment} {
                description.colorAttachmentCount = colorAttachments.size();
                description.pColorAttachments    = colorAttachments.data();
                description.inputAttachmentCount = inputAttachments.size();
                description.pInputAttachments    = inputAttachments.data();
                description.preserveAttachmentCount = preserveAttachments.size();
                description.pPreserveAttachments    = preserveAttachments.data();
                description.pResolveAttachments  = resolveAttachments.size() ? resolveAttachments.data() : nullptr;
            }


            void setColorAttachments(const std::vector <VkAttachmentReference> &_colAttachments) {
                colorAttachments = _colAttachments;
                description.colorAttachmentCount = colorAttachments.size();
                description.pColorAttachments    = colorAttachments.data();
            }

            void setInputAttachments(const std::vector <VkAttachmentReference> &_inAttachments) {
                inputAttachments = _inAttachments;
                description.inputAttachmentCount = inputAttachments.size();
                description.pInputAttachments    = inputAttachments.data();
            }

            void setResolveAttachments(const std::vector <VkAttachmentReference> &_resolveAttachments) {
                resolveAttachments = _resolveAttachments;
                // non-empty resolveAttachments array implies non-empty colorAttachments array,
                // so colorAttachmentCount is only set in setColorAttachments.
                // Also, this pointer can be set to NULL by providing an empty vector,
                // color and input attachments' ptrs are ignored(?) if their count == 0
                description.pResolveAttachments = resolveAttachments.size() ? resolveAttachments.data() : nullptr;
            }

            void setDepthStencilAttachment(VkAttachmentReference _dsAttachment) {
                depthStencilAttachment = _dsAttachment;
                description.pDepthStencilAttachment = &depthStencilAttachment;
            }

            void setDepthStencilAttachmentToNull() {
                description.pDepthStencilAttachment = nullptr;
            }

            void setPreservedAttachments(const std::vector <uint32_t> &_preserveAttachments) {
                preserveAttachments = _preserveAttachments;
                description.preserveAttachmentCount = preserveAttachments.size();
                description.pPreserveAttachments    = preserveAttachments.data();
            }

            void setBindingPoint(VkPipelineBindPoint _bindPoint) {
                description.pipelineBindPoint = _bindPoint;
            }

            // TODO: something like getDescription, but only after standartizing such methods.
            //       getInfo returns const pointer, for example, while this can return const reference - but this is bad
        }; // VKFWSubpassDescription END


        struct VKFWRenderPassInfo {
            VkRenderPassCreateInfo info;
            std::vector <VkAttachmentDescription> attachments;
            std::vector <VkSubpassDescription> subpasses;
            std::vector <VkSubpassDependency> dependencies;
            // TODO: for consistency such vectors should probably be called "aAttachments" (array) or "vAttachments" (vector)


            VKFWRenderPassInfo() {
                info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
                info.pNext = nullptr;
                info.flags = 0u;
                info.attachmentCount = 0u;
                info.pAttachments = nullptr;
                info.subpassCount = 0u;
                info.pSubpasses = nullptr;
                info.dependencyCount = 0u;
                info.pDependencies = nullptr;
            }
            VKFWRenderPassInfo(const VKFWRenderPassInfo &_info) :
                        info{_info.info}, attachments{_info.attachments},
                        subpasses{_info.subpasses}, dependencies{_info.dependencies} {
                info.attachmentCount = attachments.size();
                info.pAttachments = attachments.data();
                // copy subpasses' attachment arrays
                info.subpassCount = subpasses.size();
                info.pSubpasses = subpasses.data();
                info.dependencyCount = dependencies.size();
                info.pDependencies = dependencies.data();
            }
            ~VKFWRenderPassInfo() {
                for (auto& _subpass : subpasses) {
                    if (_subpass.colorAttachmentCount) {
                        delete [] _subpass.pColorAttachments;
                        if (_subpass.pResolveAttachments != nullptr)
                            delete [] _subpass.pResolveAttachments;
                    }
                    if (_subpass.inputAttachmentCount)
                        delete [] _subpass.pInputAttachments;
                    if (_subpass.pDepthStencilAttachment != nullptr)
                        delete _subpass.pDepthStencilAttachment;
                }
                subpasses.clear();
            }

            // TODO: all add* functions are probably unneccessary (uneffective compared to set*) and should be deleted
            // MAYBE there are use cases for other structures, but most likely not here
            void addAttachment(const VkAttachmentDescription &_attachment) {
                attachments.push_back(_attachment);
            }
            void addAttachments(const std::vector<VKFWAttachmentDescription> &_attachments) {
                attachments.reserve(attachments.size() + _attachments.size());
                for (const auto &__attach : _attachments)
                    attachments.push_back(__attach.description);
            }
            void setAttachments(const std::vector<VKFWAttachmentDescription> &_attachments) {
                attachments.resize(_attachments.size());
                for (uint i = 0u; i < _attachments.size(); ++i)
                    attachments[i] = _attachments[i].description;
                info.attachmentCount = attachments.size();
                info.pAttachments = attachments.data();
            }

            void setSubpasses(const std::vector<VKFWSubpassDescription> &_subpasses) {
                subpasses.resize(_subpasses.size());
                for (uint i = 0u; i < subpasses.size(); ++i) {
                    subpasses[i] = _subpasses[i].description;
                    subpasses[i].colorAttachmentCount = _subpasses[i].colorAttachments.size();
                    VkAttachmentReference *__tmpptr = new VkAttachmentReference[subpasses[i].colorAttachmentCount];
                    memcpy(__tmpptr, _subpasses[i].colorAttachments.data(), subpasses[i].colorAttachmentCount * sizeof(VkAttachmentReference));
                    subpasses[i].pColorAttachments = __tmpptr;

                    if (_subpasses[i].resolveAttachments.size() != 0u) {
                        __tmpptr = new VkAttachmentReference[subpasses[i].colorAttachmentCount];
                        memcpy(__tmpptr, _subpasses[i].resolveAttachments.data(), subpasses[i].colorAttachmentCount * sizeof(VkAttachmentReference));
                        subpasses[i].pResolveAttachments = __tmpptr;
                    }

                    subpasses[i].inputAttachmentCount = _subpasses[i].inputAttachments.size();
                    __tmpptr = new VkAttachmentReference[subpasses[i].inputAttachmentCount];
                    memcpy(__tmpptr, _subpasses[i].inputAttachments.data(), subpasses[i].inputAttachmentCount * sizeof(VkAttachmentReference));
                    subpasses[i].pInputAttachments = __tmpptr;

                    if (subpasses[i].pDepthStencilAttachment != nullptr) {
                        __tmpptr = new VkAttachmentReference{_subpasses[i].depthStencilAttachment};
                        subpasses[i].pDepthStencilAttachment = __tmpptr;
                    }
                }
                info.subpassCount = subpasses.size();
                info.pSubpasses = subpasses.data();
            }

            void setDependencies(const std::vector<VkSubpassDependency> &_dependencies) {
                dependencies.resize(_dependencies.size());
                for (uint i = 0u; i < dependencies.size(); ++i)
                    dependencies[i] = _dependencies[i];
                info.dependencyCount = dependencies.size();
                info.pDependencies = dependencies.data();
            }
        }; // VKFWRenderPassInfo END


        struct VKFWRenderPass {
            VkRenderPass renderPass = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            const VkAllocationCallbacks *_destroyCb = nullptr;

            VKFWRenderPass(VkDevice _logicDevice, const VKFWRenderPassInfo &_info,
                            const VkAllocationCallbacks *_cAlloc = nullptr,
                            const VkAllocationCallbacks *_dAlloc = nullptr) :
                        device{_logicDevice}, _destroyCb{_dAlloc} {
                if (vkCreateRenderPass(_logicDevice, &_info.info, _cAlloc, &renderPass) != VK_SUCCESS)
                    throw std::runtime_error(SVKFW_WRAPERR("VKFWRenderPass", "could not create render pass"));
            }
            ~VKFWRenderPass() {
                if (renderPass != VK_NULL_HANDLE) {
                    vkDestroyRenderPass(device, renderPass, _destroyCb);
                    renderPass = VK_NULL_HANDLE;
                    device = VK_NULL_HANDLE;
                    _destroyCb = nullptr;
                }
            }
        }; // VKFWRenderPass END
    }; // namespace VKFWGraphicsPipelineConfig END



// Pipeline handler/info
    struct VKFWGraphicsPipelineInfo {
        VkGraphicsPipelineCreateInfo info;
        VKFWGraphicsPipelineConfig::VKFWPipelineLayout layout;
        VKFWGraphicsPipelineConfig::VKFWRenderPass renderPass;
        VKFWGraphicsPipelineConfig::VKFWShaderStages shaderStages; // mandatory
        VKFWGraphicsPipelineConfig::VKFWVertexInputState   *   vertexInputState = nullptr;
        VKFWGraphicsPipelineConfig::VKFWInputAssemblyState * inputAssemblyState = nullptr;
        VKFWGraphicsPipelineConfig::VKFWTessellationState  *  tessellationState = nullptr;
        VKFWGraphicsPipelineConfig::VKFWViewportState      *      viewportState = nullptr;
        VKFWGraphicsPipelineConfig::VKFWRasterizationState * rasterizationState = nullptr;
        VKFWGraphicsPipelineConfig::VKFWMultisampleState   *   multisampleState = nullptr;
        VKFWGraphicsPipelineConfig::VKFWDepthStencilState  *  depthStencilState = nullptr;
        VKFWGraphicsPipelineConfig::VKFWColorBlendState    *    colorBlendState = nullptr;
        VKFWGraphicsPipelineConfig::VKFWDynamicState       *       dynamicState = nullptr;

        VKFWGraphicsPipelineInfo(VkDevice _logicDevice,
                                 const VKFWGraphicsPipelineConfig::VKFWPipelineLayoutInfo &_layoutInfo,
                                 const VKFWGraphicsPipelineConfig::VKFWRenderPassInfo &_renderPassInfo,
                                 uint32_t _subpass = 0u,
                                 const VkAllocationCallbacks *_cAlloc = nullptr, // TODO: allocation callbacks
                                 const VkAllocationCallbacks *_dAlloc = nullptr) :
                                    layout      { _logicDevice, _layoutInfo },
                                    renderPass  { _logicDevice, _renderPassInfo },
                                    shaderStages{ _logicDevice, _cAlloc, _dAlloc } {
            info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            info.pNext = nullptr;
            info.flags = 0u;
            info.layout = layout.layout;
            info.renderPass = renderPass.renderPass;
            info.subpass = _subpass;
            info.stageCount = 0u;
            info.pStages = nullptr;
            info.pDynamicState       = nullptr;
            info.pVertexInputState   = nullptr;
            info.pInputAssemblyState = nullptr;
            info.pTessellationState  = nullptr;
            info.pViewportState      = nullptr;
            info.pRasterizationState = nullptr;
            info.pMultisampleState   = nullptr;
            info.pDepthStencilState  = nullptr;
            info.pColorBlendState    = nullptr;
        }
        ~VKFWGraphicsPipelineInfo() {
            if (vertexInputState != nullptr) {
                delete vertexInputState;
                vertexInputState = nullptr;
                info.pVertexInputState = nullptr;
            }
            if (inputAssemblyState != nullptr) {
                delete inputAssemblyState;
                inputAssemblyState = nullptr;
                info.pInputAssemblyState = nullptr;
            }
            if (tessellationState != nullptr) {
                delete tessellationState;
                tessellationState = nullptr;
                info.pTessellationState = nullptr;
            }
            if (viewportState != nullptr) {
                delete viewportState;
                viewportState = nullptr;
                info.pViewportState = nullptr;
            }
            if (rasterizationState != nullptr) {
                delete rasterizationState;
                rasterizationState = nullptr;
                info.pRasterizationState = nullptr;
            }
            if (multisampleState != nullptr) {
                delete multisampleState;
                multisampleState = nullptr;
                info.pMultisampleState = nullptr;
            }
            if (depthStencilState != nullptr) {
                delete depthStencilState;
                depthStencilState = nullptr;
                info.pDepthStencilState = nullptr;
            }
            if (colorBlendState != nullptr) {
                delete colorBlendState;
                colorBlendState = nullptr;
                info.pColorBlendState = nullptr;
            }
            if (dynamicState != nullptr) {
                delete dynamicState;
                dynamicState = nullptr;
                info.pDynamicState = nullptr;
            }
        }


// Interfaces

    // VKFWShaderStages interface

        void addShaderFromString(const char *_sh_str, size_t _length,
                                VkShaderStageFlagBits _stage, const char *_name,
                                const VKFWSpecializationInfo &_spec_info = {}) {
            shaderStages.createShaderStageInfo(shaderStages.addFromString(_sh_str, _length),
                                                _stage, _name, _spec_info);
            info.stageCount = shaderStages.stage_infos.size();
            info.pStages = shaderStages.stage_infos.data();
        }

        void addShaderFromFile(const char *_fpath,
                                VkShaderStageFlagBits _stage, const char *_name,
                                const VKFWSpecializationInfo &_spec_info = {}) {
            shaderStages.createShaderStageInfo(shaderStages.addFromFile(_fpath),
                                                _stage, _name, _spec_info);
            info.stageCount = shaderStages.stage_infos.size();
            info.pStages = shaderStages.stage_infos.data();
        }


    // VKFWVertexInputState interface

        void allocateVertexInputState() {
            if (vertexInputState == nullptr)
                vertexInputState = new VKFWGraphicsPipelineConfig::VKFWVertexInputState{};
        }

    //// TODO


    // VKFWInputAssemblyState interface

        void allocateInputAssemblyState() {
            if (inputAssemblyState == nullptr)
                inputAssemblyState = new VKFWGraphicsPipelineConfig::VKFWInputAssemblyState;
        }
        void allocateInputAssemblyState(VkPrimitiveTopology _topology_type, bool _primitive_restart) {
            inputAssemblyState = new VKFWGraphicsPipelineConfig::VKFWInputAssemblyState
                                                { _topology_type, _primitive_restart };
        }

        void setInputAssemblyTopologyType(VkPrimitiveTopology _topology_type) {
            allocateInputAssemblyState();
            inputAssemblyState->setTopology(_topology_type);
        }
        void setInputAssemblyPrimitiveRestart(bool _primitive_restart) {
            allocateInputAssemblyState();
            inputAssemblyState->setRestart(_primitive_restart);
        }


    // VKFWTessellationState interface

        void allocateTessellationState() {
            if (tessellationState == nullptr)
                tessellationState = new VKFWGraphicsPipelineConfig::VKFWTessellationState;
        }

        void setPatchControlPoints(uint32_t _cPoints) {
            allocateTessellationState();
            tessellationState->setPatchControlPoints(_cPoints);
        }


    // VKFWViewportState interface

        void allocateViewportState() {
            if (viewportState == nullptr)
                viewportState = new VKFWGraphicsPipelineConfig::VKFWViewportState{};
        }

        void setViewportsDynamic(uint32_t _viewportCount) {
            allocateViewportState();
            viewportState->setViewportsDynamic(_viewportCount);
        }
        void setViewports(const std::vector<VKFWViewport> &_viewports) {
            allocateViewportState();
            viewportState->setViewports(_viewports);
        }
        void setScissorsDynamic(uint32_t _scissorCount) {
            allocateViewportState();
            viewportState->setScissorsDynamic(_scissorCount);
        }
        void setScissors(const std::vector<VKFWRect2D> &_scissors) {
            allocateViewportState();
            viewportState->setScissors(_scissors);
        }
        void setViewportStateFromFramebuffer(const VKFWSwapChain &_swapchain) {
            allocateViewportState();
            viewportState->setViewports({ {_swapchain.extent, 0.f, 0.f, 0.f, 1.f} });
            viewportState->setScissors ({ {_swapchain.extent, {0, 0}} });
        }


    // VKFWRasterizationState interface

        void allocateRasterizationState() {
            if (rasterizationState == nullptr)
                rasterizationState = new VKFWGraphicsPipelineConfig::VKFWRasterizationState;
        }
        void allocateRasterizationState(VkPolygonMode _poly_mode, VkCullModeFlags _cull_mode, VkFrontFace _front) {
            if (rasterizationState == nullptr)
                rasterizationState = new VKFWGraphicsPipelineConfig::VKFWRasterizationState
                                                { _poly_mode, _cull_mode, _front };
        }

        void setDepthClamp(bool _enable)
            { allocateRasterizationState(); rasterizationState->setDepthClamp(_enable); }
        void setRasterizerDiscard(bool _enable)
            { allocateRasterizationState(); rasterizationState->setRasterizerDiscard(_enable); }
        void setPolygonMode(VkPolygonMode _mode)
            { allocateRasterizationState(); rasterizationState->setPolygonMode(_mode); }
        void setCullMode(VkCullModeFlags _mode)
            { allocateRasterizationState(); rasterizationState->setCullMode(_mode); }
        void setFrontFace(VkFrontFace _front)
            { allocateRasterizationState(); rasterizationState->setFrontFace(_front); }
        void setLineWidth(float _width)
            { allocateRasterizationState(); rasterizationState->setLineWidth(_width); }
        void setDepthBias(bool _enable, float _constFactor, float _clamp, float _slopeFactor) {
            allocateRasterizationState();
            rasterizationState->setDepthBias(_enable, _constFactor, _clamp, _slopeFactor);
        }


    // VKFWMultisampleState interface

        void allocateMultisampleState() {
            if (multisampleState == nullptr)
                multisampleState = new VKFWGraphicsPipelineConfig::VKFWMultisampleState;
        }

    //// TODO


    // VKFWDepthStencilState interface

        void allocateDepthStencilState() {
            if (depthStencilState == nullptr)
                depthStencilState = new VKFWGraphicsPipelineConfig::VKFWDepthStencilState;
        }

    //// TODO !!!


    // VKFWColorBlendState interface

        void allocateColorBlendState() {
            if (colorBlendState == nullptr)
                colorBlendState = new VKFWGraphicsPipelineConfig::VKFWColorBlendState;
        }

        void setColorBlendLogicOp(bool _enable, VkLogicOp _op) {
            allocateColorBlendState();
            colorBlendState->setLogicOp(_enable, _op);
        }

        void setBlendConstants(const Vec4 &_constants) {
            allocateColorBlendState();
            colorBlendState->setBlendConstants(_constants);
        }

        void addColorBlendAttachments(const std::vector<VKFWGraphicsPipelineConfig::
                                        VKFWColorBlendAttachmentState> &_states) {
            allocateColorBlendState();
            colorBlendState->addAttachments(_states);
        }

        void setColorBlendAttachments(const std::vector<VKFWGraphicsPipelineConfig::
                                        VKFWColorBlendAttachmentState> &_states) {
            allocateColorBlendState();
            colorBlendState->setAttachments(_states);
        }


    // VKFWDynamicState interface

        void allocateDynamicState() {
            if (dynamicState == nullptr)
                dynamicState = new VKFWGraphicsPipelineConfig::VKFWDynamicState;
        }

        // returns false if state is already included in dynamicStates, true if successfully added
        bool addDynamicState(VkDynamicState _state) {
            allocateDynamicState();
            return dynamicState->addDynamicState(_state);
        }

        // returns false if state is not in dynamicStates, true if successfully removed first entry
        bool delDynamicState(VkDynamicState _state) {
            allocateDynamicState();
            return dynamicState->delDynamicState(_state);
        }

        // add input states if not present
        void addDynamicStates(const std::vector<VkDynamicState> &_states) {
            allocateDynamicState();
            dynamicState->addDynamicStates(_states);
        }

        // removes input states if present
        void delDynamicStates(const std::vector<VkDynamicState> &_states) {
            allocateDynamicState();
            dynamicState->delDynamicStates(_states);
        }


// get info pointer

        const VkGraphicsPipelineCreateInfo *getInfo() {
            if (dynamicState       != nullptr) info.pDynamicState       =       dynamicState->getInfo();
            if (vertexInputState   != nullptr) info.pVertexInputState   =   vertexInputState->getInfo();
            if (inputAssemblyState != nullptr) info.pInputAssemblyState = inputAssemblyState->getInfo();
            if (tessellationState  != nullptr) info.pTessellationState  =  tessellationState->getInfo();
            if (viewportState      != nullptr) info.pViewportState      =      viewportState->getInfo();
            if (rasterizationState != nullptr) info.pRasterizationState = rasterizationState->getInfo();
            if (multisampleState   != nullptr) info.pMultisampleState   =   multisampleState->getInfo();
            if (depthStencilState  != nullptr) info.pDepthStencilState  =  depthStencilState->getInfo();
            if (colorBlendState    != nullptr) info.pColorBlendState    =    colorBlendState->getInfo();

            info.stageCount = shaderStages.stage_infos.size();
            info.pStages = shaderStages.stage_infos.size() ? shaderStages.stage_infos.data() : nullptr;
            info.basePipelineHandle = VK_NULL_HANDLE;
            info.basePipelineIndex = -1;

            return &info;
        }
    }; // VKFWGraphicsPipelineInfo END


    struct VKFWGraphicsPipeline {
        VkPipeline pipeline = VK_NULL_HANDLE;
        VkDevice device = VK_NULL_HANDLE;
        const VkAllocationCallbacks *_destroyCb = nullptr;

        VKFWGraphicsPipeline(VkDevice _logicDevice, const VkGraphicsPipelineCreateInfo *_info,
                                const VkAllocationCallbacks *_cAlloc = nullptr,
                                const VkAllocationCallbacks *_dAlloc = nullptr) :
                        device{_logicDevice}, _destroyCb{_dAlloc} {
            if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, _info, _cAlloc, &pipeline) != VK_SUCCESS)
                throw std::runtime_error(SVKFW_WRAPERR("VKFWGraphicsPipeline", "failed to create graphics pipeline"));
        }
        ~VKFWGraphicsPipeline() {
            if (pipeline != VK_NULL_HANDLE) {
                vkDestroyPipeline(device, pipeline, _destroyCb);
                pipeline = VK_NULL_HANDLE;
            }
        }
    };
};

#endif