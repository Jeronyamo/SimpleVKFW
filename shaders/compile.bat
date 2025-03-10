SET SHADERS_PATH=%~dp0
%VULKAN_SDK%\Bin\glslc.exe %SHADERS_PATH%simple_shader.vert -o %SHADERS_PATH%simple_shader_vert.spv
%VULKAN_SDK%\Bin\glslc.exe %SHADERS_PATH%simple_shader.frag -o %SHADERS_PATH%simple_shader_frag.spv
pause