project(pp-toolbox)

add_library(pp-toolbox MODULE base.cpp plugin.cpp)
target_link_libraries(pp-toolbox PRIVATE imm-dr4 dr4 cum pp)
set_target_properties(pp-toolbox PROPERTIES PREFIX "")
set_target_properties(pp-toolbox PROPERTIES OUTPUT_NAME "isd.pp-toolbox")

