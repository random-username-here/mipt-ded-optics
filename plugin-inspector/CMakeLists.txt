project(plugin-inspector)

add_executable(plugin-inspector inspector.cpp)
target_link_libraries(plugin-inspector PRIVATE cum)

add_library(sample-plugin SHARED sample-plugin.cpp)
target_link_libraries(sample-plugin PRIVATE cum)
set_target_properties(sample-plugin PROPERTIES PREFIX "")
set_target_properties(sample-plugin PROPERTIES OUTPUT_NAME "isd.sample-plugin")


