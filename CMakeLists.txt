cmake_minimum_required(VERSION 3.10)
project(optics)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # for lsp-s
set(CMAKE_BUILD_TYPE Debug)

add_subdirectory(mipt-ded-zemax)
add_subdirectory(plugin-inspector)
add_subdirectory(dr4-sdl2)
add_subdirectory(imm-dr4)
add_subdirectory(canvas-example)
add_subdirectory(pp-toolbox)
add_subdirectory(sgui)
add_subdirectory(resfile)
add_subdirectory(ln)
