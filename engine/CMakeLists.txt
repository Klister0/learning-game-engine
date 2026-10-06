add_library(engine STATIC
    core/Log.cpp
    core/FixedTimestep.cpp
    core/FpsCounter.cpp
    core/Window.cpp
    core/Application.cpp
    renderer/RenderCommand.cpp)
target_include_directories(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(engine PUBLIC glfw glad)
lge_set_warnings(engine)
