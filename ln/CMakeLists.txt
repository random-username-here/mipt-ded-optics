project(lenses)
set (CMAKE_CXX_STANDARD 20)

add_executable(lenses)
target_link_libraries(lenses PUBLIC dr4 cum hui imm-dr4 sgui resfile)
target_include_directories(lenses PUBLIC include)
target_compile_options(lenses PUBLIC -msse4.1 -O3)
target_sources(lenses PRIVATE
    src/lenses/screenshot.cpp
    src/lenses/file_picker.cpp
    src/lenses/tools/gnd.cpp
    src/lenses/tools/object.cpp
    src/lenses/tools/sphere.cpp
    src/lenses/tools/mesh.cpp
    src/lenses/ui/scene_tree.cpp
    src/lenses/ui/tool.cpp
    src/lenses/ui/toolbox.cpp
    src/math/gfxcommon.cpp
    src/rtr/obj/mesh.cpp
    src/rtr/obj/gnd.cpp
    src/rtr/obj/obj.cpp
    src/rtr/obj/sphere.cpp
    src/rtr/obj/sun.cpp
    src/rtr/renderer.cpp
    src/rtr/sgui-view.cpp
    src/main.cpp
)

