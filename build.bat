@ECHO OFF
REM Use: 'build.bat [Debug|Release]', default: Debug
REM Note: the presets are in CMakePresets.json, MSYS2 MinGW-w64 bin directory must be in PATH
SET BUILD_TYPE=debug
IF /I "%~1"=="Release" SET BUILD_TYPE=release
CD /D "%~dp0"
cmake --preset win-%BUILD_TYPE% || EXIT /B 1
cmake --build --preset win-%BUILD_TYPE% --target svkfw svkfw_test
