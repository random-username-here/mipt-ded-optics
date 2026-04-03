project(canvas-example)

add_executable(canvas main.cpp)
target_link_libraries(canvas PRIVATE cum)
target_link_libraries(canvas PRIVATE dr4)
target_link_libraries(canvas PRIVATE pp)

