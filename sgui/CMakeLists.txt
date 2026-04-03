project(sgui)

add_library(sgui SHARED)
target_link_libraries(sgui PUBLIC dr4 cum hui imm-dr4)
target_include_directories(sgui PUBLIC include)
target_sources(sgui PRIVATE
    src/sgui/root.cpp
    src/sgui/box/box.cpp
    src/sgui/box/scrollbox.cpp
    src/sgui/box/window.cpp
    src/sgui/input/button.cpp
    src/sgui/input/checkbox.cpp
    src/sgui/input/textedit.cpp
    src/sgui/view/text.cpp
    src/sgui/menu.cpp
)

add_executable(example.basic example/basic.cpp)
target_link_libraries(example.basic PRIVATE sgui)

