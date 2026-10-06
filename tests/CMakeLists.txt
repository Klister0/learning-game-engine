add_executable(engine_tests
    test_main.cpp
    test_log.cpp
    test_fixed_timestep.cpp
    test_fps_counter.cpp)
target_link_libraries(engine_tests PRIVATE engine doctest::doctest)
lge_set_warnings(engine_tests)

add_test(NAME engine_tests COMMAND engine_tests)
