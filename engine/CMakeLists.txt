add_library(engine STATIC
    core/Log.cpp)
target_include_directories(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(engine PUBLIC glfw glad)
lge_set_warnings(engine)
