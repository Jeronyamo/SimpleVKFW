cmake_minimum_required(VERSION 3.15)

project(SimpleVKFW CXX)


set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)


## Global CMake Variables

option(SVKFW_OVERRIDE_RES_LOCATION On)

if(SVKFW_OVERRIDE_RES_LOCATION)
    set(SVKFW_BUILD_DIR ${PROJECT_SOURCE_DIR}/build_${CMAKE_BUILD_TYPE})

    if(CMAKE_SYSTEM_NAME STREQUAL Windows)
        set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS On)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${SVKFW_BUILD_DIR}/win)
    elseif(CMAKE_SYSTEM_NAME STREQUAL Linux)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${PROJECT_SOURCE_DIR}/linux)
    elseif(CMAKE_SYSTEM_NAME STREQUAL Darwin)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${PROJECT_SOURCE_DIR}/mac)
    endif()

    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/lib)
    set(CMAKE_BINARY_DIR               ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/bin)
endif()


## SVKFW Executables

add_executable(svkfw ${PROJECT_SOURCE_DIR}/main.cpp)
add_executable(svkfw_test ${PROJECT_SOURCE_DIR}/tests/tests.cpp)


## SVKFW Target CMake Definitions

function(svkfw_set_definitions SVKFW_TARGET)
    if (CMAKE_BUILD_TYPE STREQUAL Debug)
        target_compile_definitions(${SVKFW_TARGET} PRIVATE SVKFW_DEBUG)
    endif()

    if(CMAKE_SYSTEM_NAME STREQUAL Windows)
        target_compile_definitions(${SVKFW_TARGET} PRIVATE SVKFW_CM_WIN)

        ## Get Windows build
        string(FIND ${CMAKE_SYSTEM_VERSION} "." WIN_VERSION_LEN REVERSE)
        math(EXPR WIN_VERSION_LEN "${WIN_VERSION_LEN} + 1")
        string(SUBSTRING ${CMAKE_SYSTEM_VERSION} ${WIN_VERSION_LEN} -1 WIN_BUILD_VER)
        math(EXPR WIN_BUILD_VER ${WIN_BUILD_VER})

        ## Anniversary update, ANSI supported since 10586 (November update), but this one expands functionality
        if(${WIN_BUILD_VER} GREATER_EQUAL 14393)
            target_compile_definitions(${SVKFW_TARGET} PRIVATE SVKFW_CM_ANSI_ESC)
        endif()
        # add_definitions(-DSVKFW_CM_WIN_BUILD=${WIN_BUILD_VER})
    elseif(CMAKE_SYSTEM_NAME STREQUAL Linux)
        target_compile_definitions(${SVKFW_TARGET} PRIVATE SVKFW_CM_LINUX PRIVATE SVKFW_CM_ANSI_ESC)
    elseif(CMAKE_SYSTEM_NAME STREQUAL Darwin)
        target_compile_definitions(${SVKFW_TARGET} PRIVATE SVKFW_CM_MAC PRIVATE SVKFW_CM_ANSI_ESC)
    endif()
    if (${RTAUDIO_API_ASIO} STREQUAL ON)
        target_compile_definitions(${SVKFW_TARGET} PRIVATE RTA_ASIO)
    endif()
endfunction()


## Directory Variables
set(SVKFW_SOURCE_DIR     ${PROJECT_SOURCE_DIR}/source)
set(SVKFW_EXTERN_LIB_DIR ${PROJECT_SOURCE_DIR}/ext_libs)
set(SVKFW_EXTERN_SRC_DIR ${PROJECT_SOURCE_DIR}/ext_sources)
set(SVKFW_EXTERN_INC_DIR ${PROJECT_SOURCE_DIR}/ext_includes)

### Includes
# set(EXTERN_INC_GLAD_DIR     ${SVKFW_EXTERN_INC_DIR}/glad)
set(EXTERN_INC_GLFW_DIR     ${SVKFW_EXTERN_SRC_DIR}/glfw/include)
set(EXTERN_INC_STBI_DIR     ${SVKFW_EXTERN_INC_DIR}/STBI)
set(EXTERN_INC_RTAUDIO_DIR  ${SVKFW_EXTERN_SRC_DIR}/rtaudio)

### Sources
set(EXTERN_SRC_IMGUI_DIR    ${SVKFW_EXTERN_SRC_DIR}/imgui)
set(EXTERN_SRC_VOLK_DIR     ${SVKFW_EXTERN_SRC_DIR}/volk)
set(EXTERN_SRC_GLFW_DIR     ${SVKFW_EXTERN_SRC_DIR}/glfw)
set(EXTERN_SRC_RTAUDIO_DIR  ${SVKFW_EXTERN_SRC_DIR}/rtaudio)

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
    ${EXTERN_INC_RTAUDIO_DIR}

    ${EXTERN_SRC_IMGUI_DIR}
    ${EXTERN_SRC_IMGUI_DIR}/backends

    ## SVKFW
    ${SVKFW_SOURCE_DIR}
)


##  ============  External Targets  ============  ##

## GLFW
set(GLFW_LIBRARY_TYPE STATIC)
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
set(RTAUDIO_API_DS ON)
set(RTAUDIO_API_ALSA OFF)
set(RTAUDIO_API_ASIO OFF)
set(RTAUDIO_API_PULSE OFF)
set(RTAUDIO_API_WASAPI OFF)
set(BUILD_TESTING Off)
set(RTAUDIO_TARGETNAME_UNINSTALL "rtaudio_uninstall")

add_subdirectory(${EXTERN_SRC_RTAUDIO_DIR})
set_target_properties(rtaudio PROPERTIES FOLDER "RtAudio")
set_target_properties(${RTAUDIO_TARGETNAME_UNINSTALL} PROPERTIES FOLDER "RtAudio")


##  ============  SVKFW Targets  ============  ##

svkfw_set_definitions(svkfw_test)
target_include_directories(svkfw_test PUBLIC ${SVKFW_ALL_INCULDES})
target_link_libraries(svkfw_test volk_headers glfw imgui rtaudio)

svkfw_set_definitions(svkfw)
target_include_directories(svkfw PRIVATE ${SVKFW_ALL_INCULDES})
target_link_libraries(svkfw volk_headers glfw imgui rtaudio)