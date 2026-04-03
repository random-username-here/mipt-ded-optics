project(resfile)

add_library(resfile STATIC)
target_include_directories(resfile PUBLIC include)
target_sources(resfile PRIVATE resfile.cpp)
set_property(TARGET imm-dr4 PROPERTY POSITION_INDEPENDENT_CODE ON)
