project(imm-dr4)

add_library(imm-dr4 STATIC)
target_include_directories(imm-dr4 PUBLIC include)
target_link_libraries(imm-dr4 PUBLIC dr4)
target_sources(imm-dr4 PRIVATE imm_dr4.cpp)
set_property(TARGET imm-dr4 PROPERTY POSITION_INDEPENDENT_CODE ON)
