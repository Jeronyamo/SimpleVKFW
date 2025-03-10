SET BUILD_PATH=%~dp0
cmake -B"%BUILD_PATH%build\win\build" -S"%BUILD_PATH:~0,-1%"