cmake_minimum_required(VERSION 3.15)

project(SimpleVKFW CXX)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)


## Global CMake Variables

if(CMAKE_SYSTEM_NAME STREQUAL Windows)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${PROJECT_SOURCE_DIR}/build/win)
elseif(CMAKE_SYSTEM_NAME STREQUAL Linux)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${PROJECT_SOURCE_DIR}/build/linux)
elseif(CMAKE_SYSTEM_NAME STREQUAL Darwin)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${PROJECT_SOURCE_DIR}/build/mac)
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/lib)
set(CMAKE_BINARY_DIR               ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/bin)


## SVKFW Executable

add_executable(svkfw_test ${PROJECT_SOURCE_DIR}/main.cpp)


## SVKFW CMake Variables

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    target_compile_definitions(svkfw_test PRIVATE SVKFW_DEBUG)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL Windows)
    target_compile_definitions(svkfw_test PRIVATE SVKFW_CM_WIN)

    ## Get Windows build
    string(FIND ${CMAKE_SYSTEM_VERSION} "." WIN_VERSION_LEN REVERSE)
    math(EXPR WIN_VERSION_LEN "${WIN_VERSION_LEN} + 1")
    string(SUBSTRING ${CMAKE_SYSTEM_VERSION} ${WIN_VERSION_LEN} -1 WIN_BUILD_VER)
    math(EXPR WIN_BUILD_VER ${WIN_BUILD_VER})

    ## Anniversary update, ANSI supported since 10586 (November update), but this one expands functionality
    if(${WIN_BUILD_VER} GREATER_EQUAL 14393)
        target_compile_definitions(svkfw_test PRIVATE SVKFW_CM_ANSI_ESC)
    endif()
    # add_definitions(-DSVKFW_CM_WIN_BUILD=${WIN_BUILD_VER})
elseif(CMAKE_SYSTEM_NAME STREQUAL Linux)
    target_compile_definitions(svkfw_test PRIVATE SVKFW_CM_LINUX PRIVATE SVKFW_CM_ANSI_ESC)
elseif(CMAKE_SYSTEM_NAME STREQUAL Darwin)
    target_compile_definitions(svkfw_test PRIVATE SVKFW_CM_MAC PRIVATE SVKFW_CM_ANSI_ESC)
endif()


## Directory Variables

set(SVKFW_SOURCE_DIR     ${PROJECT_SOURCE_DIR}/source)
set(SVKFW_EXTERN_LIB_DIR ${PROJECT_SOURCE_DIR}/ext_libs)
set(SVKFW_EXTERN_SRC_DIR ${PROJECT_SOURCE_DIR}/ext_sources)
set(SVKFW_EXTERN_INC_DIR ${PROJECT_SOURCE_DIR}/ext_includes)

### Includes
# set(EXTERN_INC_GLAD_DIR  ${SVKFW_EXTERN_INC_DIR}/glad)
set(EXTERN_INC_GLFW_DIR  ${SVKFW_EXTERN_SRC_DIR}/glfw/include)
set(EXTERN_INC_STBI_DIR  ${SVKFW_EXTERN_INC_DIR}/STBI)

### Sources
set(EXTERN_SRC_IMGUI_DIR ${SVKFW_EXTERN_SRC_DIR}/imgui)
set(EXTERN_SRC_VOLK_DIR  ${SVKFW_EXTERN_SRC_DIR}/volk)
set(EXTERN_SRC_GLFW_DIR  ${SVKFW_EXTERN_SRC_DIR}/glfw)
### Libraries
# set(EXTERN_LIB_GLFW_DIR   ${SVKFW_EXTERN_LIB_DIR}/glfw_340/lib-mingw-w64)
# set(EXTERN_LIB_CSOUND_DIR ${SVKFW_EXTERN_LIB_DIR}/csound_6_18_1/Release)


##  ============  External  ============  ##

## CSound TODO: not sure it places files where it should be (ext_includes/csound)
# set(HEADER_INSTALL_DIR ${EXTERN_INC_CSOUND_DIR})
# set(TMP_CURR_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
# set(CMAKE_CURRENT_SOURCE_DIR ${EXTERN_INC_CSOUND_DIR})
# include(${EXTERN_INC_CSOUND_DIR}/CMakeLists.txt)
# set(CMAKE_CURRENT_SOURCE_DIR ${TMP_CURR_SRC_DIR})

# set(CMAKE_PREFIX_PATH ${EXTERN_LIB_CSOUND_DIR} ${CMAKE_PREFIX_PATH})
# find_library(EXTERN_LIB_CSND6_PATH csnd6)

# target_link_directories(svkfw_test PRIVATE ${EXTERN_LIB_CSOUND_DIR})

# add_library(csound64 SHARED IMPORTED)
# set_target_properties(csound64 PROPERTIES IMPORTED_LOCATION ${EXTERN_LIB_CSOUND_DIR}/csound64.dll
#                                           IMPORTED_IMPLIB   ${EXTERN_LIB_CSOUND_DIR}/csound64.lib)
# add_library(csnd6 SHARED IMPORTED)
# set_property(TARGET csnd6 APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
# set_target_properties(csnd6 PROPERTIES IMPORTED_LOCATION "${EXTERN_LIB_CSOUND_DIR}/csnd6.dll")
# set_target_properties(csnd6 PROPERTIES MAP_IMPORTED_CONFIG_DEBUG Debug)


## GLFW
set(GLFW_LIBRARY_TYPE SHARED)
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

target_include_directories(imgui
PRIVATE
    ${EXTERN_INC_GLFW_DIR}
    ${EXTERN_SRC_IMGUI_DIR}
    ${EXTERN_SRC_IMGUI_DIR}/backends
    ${Vulkan_INCLUDE_DIR}
)


##  ============  SVKFW  ============  ##

target_include_directories(svkfw_test
PUBLIC
    ${Vulkan_INCLUDE_DIR}

PRIVATE
    ## External
    # GLAD not needed for vulkan/volk (SOGLFW legacy):
    # ${EXTERN_INC_GLAD_DIR}
    # ${SVKFW_EXTERN_INC_DIR}/csound
    ${EXTERN_INC_STBI_DIR}
    ${EXTERN_INC_GLFW_DIR}
    ${EXTERN_INC_CSOUND_DIR}

    ${EXTERN_SRC_IMGUI_DIR}
    ${EXTERN_SRC_IMGUI_DIR}/backends

    ## SVKFW
    ${SVKFW_SOURCE_DIR}
    ${SVKFW_SOURCE_DIR}/common
    ${SVKFW_SOURCE_DIR}/math
    ${SVKFW_SOURCE_DIR}/interface
    ${SVKFW_SOURCE_DIR}/main
)

target_link_libraries(svkfw_test volk_headers glfw imgui)
