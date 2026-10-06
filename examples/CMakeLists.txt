# Her bölümün örnek programı buraya eklenir.
add_executable(example_01_window 01_window/main.cpp)
target_link_libraries(example_01_window PRIVATE engine)
lge_set_warnings(example_01_window)
