TODO

Compile (CMake 3.21+, presets from CMakePresets.json: win-debug, win-release, linux-debug, linux-release, mac-debug, mac-release):

cmake --preset linux-release
cmake --build --preset linux-release --target svkfw svkfw_test

Build files go to build_<BuildType>/<os>/build, binaries to build_<BuildType>/<os> (os: win, linux, mac).
VS Code (CMake Tools) picks the presets for the current OS, select Debug/Release in the status bar.
Windows uses MSYS2 MinGW-w64 (gcc, g++, mingw32-make must be in PATH). Local overrides go to CMakeUserPresets.json (not in git).

Or: ./build.sh [<C compiler> <CXX compiler>] [Debug|Release], build.bat [Debug|Release]

Linux packages (Debian/Ubuntu names):

- Vulkan + Python: libvulkan-dev (or the LunarG SDK, VULKAN_SDK must be set), python3, cmake, pkg-config
- GLFW, X11: libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev
- GLFW, Wayland: libwayland-dev wayland-protocols libxkbcommon-dev (or build with SVKFW_GLFW_WAYLAND=Off ./build.sh ...)
- Audio: libasound2-dev and/or libpulse-dev; without both the build works but has no sound

Run svkfw from the repository root, it loads fonts and shaders by relative paths.
