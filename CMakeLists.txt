cmake_minimum_required(VERSION 3.15)

project(SimpleVKFW CXX)


set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(SVKFW_MSG_PREFIX ">>>  SimpleVKFW: ")


## Global CMake Variables

option(SVKFW_OVERRIDE_RES_LOCATION "Place build results into build_<type>/<os>" ON)

if(SVKFW_OVERRIDE_RES_LOCATION)
    set(SVKFW_BUILD_DIR ${PROJECT_SOURCE_DIR}/build_${CMAKE_BUILD_TYPE})

    if(CMAKE_SYSTEM_NAME STREQUAL Windows)
        set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS On)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${SVKFW_BUILD_DIR}/win)
    elseif(CMAKE_SYSTEM_NAME STREQUAL Linux)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${SVKFW_BUILD_DIR}/linux)
    elseif(CMAKE_SYSTEM_NAME STREQUAL Darwin)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${SVKFW_BUILD_DIR}/mac)
    endif()

    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/lib)
    set(CMAKE_BINARY_DIR               ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/bin)
endif()


## SVKFW Target CMake Definitions

# Note: SVKFW_SCOPE is PUBLIC for the library - the headers depend on these definitions too
function(svkfw_set_definitions SVKFW_TARGET SVKFW_SCOPE)
    if (CMAKE_BUILD_TYPE STREQUAL Debug)
        target_compile_definitions(${SVKFW_TARGET} ${SVKFW_SCOPE} SVKFW_DEBUG)
    endif()

    if(CMAKE_SYSTEM_NAME STREQUAL Windows)
        target_compile_definitions(${SVKFW_TARGET} ${SVKFW_SCOPE} SVKFW_CM_WIN)
        target_compile_definitions(${SVKFW_TARGET} ${SVKFW_SCOPE} NOMINMAX)

        ## Get Windows build
        string(FIND ${CMAKE_SYSTEM_VERSION} "." WIN_VERSION_LEN REVERSE)
        math(EXPR WIN_VERSION_LEN "${WIN_VERSION_LEN} + 1")
        string(SUBSTRING ${CMAKE_SYSTEM_VERSION} ${WIN_VERSION_LEN} -1 WIN_BUILD_VER)
        math(EXPR WIN_BUILD_VER ${WIN_BUILD_VER})

        ## Anniversary update, ANSI supported since 10586 (November update), but this one expands functionality
        if(${WIN_BUILD_VER} GREATER_EQUAL 14393)
            target_compile_definitions(${SVKFW_TARGET} ${SVKFW_SCOPE} SVKFW_CM_ANSI_ESC)
        endif()
        # add_definitions(-DSVKFW_CM_WIN_BUILD=${WIN_BUILD_VER})
    elseif(CMAKE_SYSTEM_NAME STREQUAL Linux)
        target_compile_definitions(${SVKFW_TARGET} ${SVKFW_SCOPE} SVKFW_CM_LINUX ${SVKFW_SCOPE} SVKFW_CM_ANSI_ESC)
    elseif(CMAKE_SYSTEM_NAME STREQUAL Darwin)
        target_compile_definitions(${SVKFW_TARGET} ${SVKFW_SCOPE} SVKFW_CM_MAC ${SVKFW_SCOPE} SVKFW_CM_ANSI_ESC)
    endif()
    if (RTAUDIO_API_ASIO)
        target_compile_definitions(${SVKFW_TARGET} ${SVKFW_SCOPE} RTA_ASIO)
    endif()
endfunction()

function(svkfw_set_link_options SVKFW_TARGET)
    if(MINGW)
        message(${SVKFW_MSG_PREFIX} "Linking MinGW static libraries (" ${SVKFW_TARGET} ")")
        target_link_options(${SVKFW_TARGET} PRIVATE -static-libgcc -static-libstdc++ -static)
    endif()
endfunction()


## Directory Variables
set(SVKFW_INCLUDE_DIR    ${PROJECT_SOURCE_DIR}/include)
set(SVKFW_SOURCE_DIR     ${PROJECT_SOURCE_DIR}/source)
set(SVKFW_EXTERN_LIB_DIR ${PROJECT_SOURCE_DIR}/ext_libs)
set(SVKFW_EXTERN_SRC_DIR ${PROJECT_SOURCE_DIR}/ext_sources)
set(SVKFW_EXTERN_INC_DIR ${PROJECT_SOURCE_DIR}/ext_includes)

### Includes
# set(EXTERN_INC_GLAD_DIR     ${SVKFW_EXTERN_INC_DIR}/glad)
set(EXTERN_INC_GLFW_DIR     ${SVKFW_EXTERN_SRC_DIR}/glfw/include)
set(EXTERN_INC_OPUS_DIR     ${SVKFW_EXTERN_SRC_DIR}/opus/include)
set(EXTERN_INC_STBI_DIR     ${SVKFW_EXTERN_INC_DIR}/STBI)
set(EXTERN_INC_RTAUDIO_DIR  ${SVKFW_EXTERN_SRC_DIR}/rtaudio)

### Sources
set(EXTERN_SRC_IMGUI_DIR    ${SVKFW_EXTERN_SRC_DIR}/imgui)
set(EXTERN_SRC_VOLK_DIR     ${SVKFW_EXTERN_SRC_DIR}/volk)
set(EXTERN_SRC_GLFW_DIR     ${SVKFW_EXTERN_SRC_DIR}/glfw)
set(EXTERN_SRC_RTAUDIO_DIR  ${SVKFW_EXTERN_SRC_DIR}/rtaudio)
set(EXTERN_SRC_OPUS_DIR     ${SVKFW_EXTERN_SRC_DIR}/opus)

### Libraries
# Note: includes from ext_libs should be declared here


set(SVKFW_ALL_INCULDES
    ${Vulkan_INCLUDE_DIR}

    ## External
    ${SVKFW_EXTERN_INC_DIR}
    ${SVKFW_EXTERN_LIB_DIR}
    ${SVKFW_EXTERN_SRC_DIR}
    # Note: GLAD not needed for vulkan/volk (SOGLFW legacy):
    # ${EXTERN_INC_GLAD_DIR}
    ${EXTERN_INC_STBI_DIR}
    ${EXTERN_INC_GLFW_DIR}
    ${EXTERN_INC_OPUS_DIR}
    ${EXTERN_INC_RTAUDIO_DIR}

    ${EXTERN_SRC_IMGUI_DIR}
    ${EXTERN_SRC_IMGUI_DIR}/backends

    ## SVKFW
    ${SVKFW_INCLUDE_DIR}
)


##  ============  External Targets  ============  ##

## GLFW
set(GLFW_LIBRARY_TYPE STATIC)
# Note: GLFW install/uninstall targets aren't needed. GLFW's "uninstall" target conflicts with RtAudio's one,
#       which is also named "uninstall" on the first configuration: RtAudio declares RTAUDIO_TARGETNAME_UNINSTALL
#       as a cache entry, so a normal variable set here is ignored until the cache entry exists.
set(GLFW_INSTALL OFF CACHE BOOL "Generate installation target" FORCE)
add_subdirectory(${EXTERN_SRC_GLFW_DIR})


## Vulkan & Volk
find_package(Vulkan REQUIRED)

if(CMAKE_SYSTEM_NAME STREQUAL Windows)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
elseif(CMAKE_SYSTEM_NAME STREQUAL Linux)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_XLIB_KHR)
elseif(CMAKE_SYSTEM_NAME STREQUAL Darwin)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_MACOS_MVK)
endif()

add_compile_definitions(USE_VOLK)
add_compile_definitions(USE_VULKAN)

add_subdirectory(${EXTERN_SRC_VOLK_DIR})


## Dear ImGUI
add_library(imgui
    ${EXTERN_SRC_IMGUI_DIR}/imgui.cpp
    ${EXTERN_SRC_IMGUI_DIR}/imgui_draw.cpp
    ${EXTERN_SRC_IMGUI_DIR}/imgui_widgets.cpp
    ${EXTERN_SRC_IMGUI_DIR}/imgui_tables.cpp
    ${EXTERN_SRC_IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${EXTERN_SRC_IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
)

target_compile_definitions(imgui PRIVATE IMGUI_IMPL_VULKAN_USE_VOLK)
target_include_directories(imgui
PRIVATE
    ${EXTERN_INC_GLFW_DIR}
    ${EXTERN_SRC_IMGUI_DIR}
    ${EXTERN_SRC_IMGUI_DIR}/backends
    ${Vulkan_INCLUDE_DIR}
    ${EXTERN_SRC_VOLK_DIR}
)


## RTAudio
# Note: RtAudio requires CMake 3.10, so its option() calls would ignore (and on the first configuration clear)
#       the normal variables set here. CMP0077 makes option() keep them.
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(RTAUDIO_API_DS     OFF)
set(RTAUDIO_API_ASIO   OFF)
set(RTAUDIO_API_WASAPI OFF)
set(RTAUDIO_API_ALSA   OFF)
set(RTAUDIO_API_PULSE  OFF)
set(RTAUDIO_API_JACK   OFF)
set(RTAUDIO_API_OSS    OFF)
set(RTAUDIO_API_CORE   OFF)
if(CMAKE_SYSTEM_NAME STREQUAL Windows)
    set(RTAUDIO_API_DS    ON)
elseif(CMAKE_SYSTEM_NAME STREQUAL Linux)
    # Each API is built when its dev package is installed (libasound2-dev, libpulse-dev).
    # PipeWire serves both, so either one plays sound; without both RtAudio builds a silent dummy API.
    find_package(ALSA QUIET)
    if(ALSA_FOUND)
        set(RTAUDIO_API_ALSA  ON)
    endif()
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(SVKFW_PULSE QUIET libpulse-simple)
    endif()
    if(SVKFW_PULSE_FOUND)
        set(RTAUDIO_API_PULSE ON)
    endif()
    if(NOT RTAUDIO_API_ALSA AND NOT RTAUDIO_API_PULSE)
        message(WARNING ${SVKFW_MSG_PREFIX} "Neither ALSA nor PulseAudio dev libraries found, there will be no sound. "
                        "Install libasound2-dev and/or libpulse-dev (Debian/Ubuntu names) and reconfigure.")
    endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL Darwin)
    set(RTAUDIO_API_CORE  ON)
endif()
message(${SVKFW_MSG_PREFIX} "RtAudio APIs: DS=${RTAUDIO_API_DS} ALSA=${RTAUDIO_API_ALSA} PULSE=${RTAUDIO_API_PULSE} CORE=${RTAUDIO_API_CORE}")
set(BUILD_TESTING OFF)
set(RTAUDIO_BUILD_TESTING OFF)
set(RTAUDIO_TARGETNAME_UNINSTALL "rtaudio_uninstall")

add_subdirectory(${EXTERN_SRC_RTAUDIO_DIR})
set_target_properties(rtaudio PROPERTIES FOLDER "RtAudio")
# Note: the actual target name is stored in the cache (see the GLFW note above)
if(TARGET $CACHE{RTAUDIO_TARGETNAME_UNINSTALL})
    set_target_properties($CACHE{RTAUDIO_TARGETNAME_UNINSTALL} PROPERTIES FOLDER "RtAudio")
endif()


## Opus
set(OPUS_STACK_PROTECTOR ON )
set(OPUS_FIXED_POINT     OFF)
set(OPUS_HARDENING       ON )

add_subdirectory(${EXTERN_SRC_OPUS_DIR})


## Python3 (generate files)

set(Python3_FIND_REGISTRY LAST) 
find_package(Python3 REQUIRED COMPONENTS Interpreter)
message(${SVKFW_MSG_PREFIX} "Looking for Python at: '${Python3_EXECUTABLE}'")
# Note: the generated files are not stored in git ('.gitignore'), they are made from the installed Vulkan SDK on configure
#   include/interface/vkfw_enum.h          + source/interface/vkfw_enum.cpp - enum names        (vulkan_core.h)
#   include/interface/vkfw_struct_traits.h                                  - pNext chain rules (vk.xml)
#   include/interface/vkfw_func.h          + source/interface/vkfw_func.cpp - Func wrappers     (vk.xml, ext_sources/volk/volk.h)
# Note: the scripts get the SDK path from the environment. IDEs started from a desktop menu (VS Code) don't read
#       ~/.bashrc, so if VULKAN_SDK isn't set, it is taken from the Vulkan headers CMake found (/usr for distro packages).
if(DEFINED ENV{VULKAN_SDK})
    set(SVKFW_VULKAN_SDK "$ENV{VULKAN_SDK}")
elseif(DEFINED ENV{VK_SDK_PATH})
    set(SVKFW_VULKAN_SDK "$ENV{VK_SDK_PATH}")
else()
    get_filename_component(SVKFW_VULKAN_SDK "${Vulkan_INCLUDE_DIR}" DIRECTORY)
endif()
message(${SVKFW_MSG_PREFIX} "Vulkan SDK for generated headers: '${SVKFW_VULKAN_SDK}'")

foreach(SVKFW_GEN_SCRIPT IN ITEMS vkfw_enum_gen.py vkfw_struct_gen.py vkfw_func_gen.py)
    message(${SVKFW_MSG_PREFIX} "Running script: '${CMAKE_SOURCE_DIR}/source/meta/${SVKFW_GEN_SCRIPT}'")
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E env "VULKAN_SDK=${SVKFW_VULKAN_SDK}" ${Python3_EXECUTABLE} "source/meta/${SVKFW_GEN_SCRIPT}"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        RESULT_VARIABLE SCRIPT_RES
        OUTPUT_VARIABLE SCRIPT_OUT
         ERROR_VARIABLE SCRIPT_ERR
    )
    if(NOT SCRIPT_RES EQUAL 0)
        message(FATAL_ERROR ${SVKFW_MSG_PREFIX} "'${SVKFW_GEN_SCRIPT}' failed: ${SCRIPT_RES}\n${SCRIPT_OUT}${SCRIPT_ERR}")
    endif()
endforeach()


##  ============  SVKFW Targets  ============  ##

# if(${CMAKE_BUILD_TYPE} EQUAL "Debug")
    # target_compile_options(rtaudio    PRIVATE "-fno-omit-frame-pointer" "-O0" "-g")
    # target_compile_options(svkfw      PRIVATE "-fno-omit-frame-pointer" "-O0" "-g")
    # target_compile_options(svkfw_test PRIVATE "-fno-omit-frame-pointer" "-O0" "-g")
    # target_link_options   (rtaudio    PRIVATE "-fno-omit-frame-pointer" "-O0" "-g")
    # target_link_options   (svkfw      PRIVATE "-fno-omit-frame-pointer" "-O0" "-g")
    # target_link_options   (svkfw_test PRIVATE "-fno-omit-frame-pointer" "-O0" "-g")
# endif()

## SVKFW Library
# Note: all sources except 'source/meta' (generator scripts), including the generated ones - so the glob is after the scripts
file(GLOB_RECURSE SVKFW_LIB_SOURCES CONFIGURE_DEPENDS ${SVKFW_SOURCE_DIR}/*.cpp)
list(FILTER SVKFW_LIB_SOURCES EXCLUDE REGEX "^${SVKFW_SOURCE_DIR}/meta/")

add_library(svkfw_lib STATIC ${SVKFW_LIB_SOURCES})
svkfw_set_definitions(svkfw_lib PUBLIC)
target_include_directories(svkfw_lib PUBLIC ${SVKFW_ALL_INCULDES})
target_link_libraries(svkfw_lib PUBLIC volk_headers glfw imgui rtaudio opus)


## SVKFW Executables

add_executable(svkfw      ${PROJECT_SOURCE_DIR}/main.cpp)
add_executable(svkfw_test ${PROJECT_SOURCE_DIR}/tests/tests.cpp)

target_link_libraries(svkfw      PRIVATE svkfw_lib)
target_link_libraries(svkfw_test PRIVATE svkfw_lib)
svkfw_set_link_options(svkfw)
svkfw_set_link_options(svkfw_test)
