project(dr4-sdl2)

find_package(PkgConfig)
pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET GLOBAL sdl2)
pkg_check_modules(SDL2_TTF REQUIRED IMPORTED_TARGET GLOBAL SDL2_ttf)
pkg_check_modules(SDL2_GFX REQUIRED IMPORTED_TARGET GLOBAL SDL2_gfx)

add_library(dr4-sdl2 MODULE drawables.cpp window.cpp plugin.cpp)
target_link_libraries(dr4-sdl2 PRIVATE dr4 cum PkgConfig::SDL2 PkgConfig::SDL2_TTF PkgConfig::SDL2_GFX)
set_target_properties(dr4-sdl2 PROPERTIES PREFIX "")
set_target_properties(dr4-sdl2 PROPERTIES OUTPUT_NAME "isd.dr4-sdl2-backend")

