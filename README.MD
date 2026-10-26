TODO

Compile:

cmake -DCMAKE_BUILD_TYPE=Release .
cmake --build . --target svkfw svkfw_test

Or: ./build.sh <C compiler> <CXX compiler> [Debug|Release]

Linux packages (Debian/Ubuntu names):

- Vulkan + Python: libvulkan-dev (or the LunarG SDK, VULKAN_SDK must be set), python3, cmake, pkg-config
- GLFW, X11: libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev
- GLFW, Wayland: libwayland-dev wayland-protocols libxkbcommon-dev (or build with SVKFW_GLFW_WAYLAND=Off ./build.sh ...)
- Audio: libasound2-dev and/or libpulse-dev; without both the build works but has no sound

Run svkfw from the repository root, it loads fonts and shaders by relative paths.
