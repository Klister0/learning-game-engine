cmake_minimum_required(VERSION 3.21)
project(LearningGameEngine LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Kaynak dosyalar UTF-8 (Türkçe yorumlar/string'ler); MSVC'ye bunu açıkça söylüyoruz.
if(MSVC)
    add_compile_options(/utf-8)
endif()

# Kendi kodumuz için sıkı uyarılar. Üçüncü parti kütüphanelere uygulamıyoruz.
function(lge_set_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

# --- Dış bağımlılıklar: ilk configure sırasında otomatik indirilir ---
include(FetchContent)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE)

FetchContent_Declare(doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG v2.4.12
    GIT_SHALLOW TRUE)

FetchContent_MakeAvailable(glfw doctest)

add_subdirectory(external/glad)

# --- Kendi kodumuz ---
add_subdirectory(engine)
add_subdirectory(examples)

enable_testing()
add_subdirectory(tests)
