cmake_minimum_required(VERSION 3.2.7)
project(strstrbench VERSION 0.1.0 LANGUAGES C)

include(FetchContent)

FetchContent_Declare(stringzilla
    GIT_REPOSITORY https://github.com/ashvardanian/stringzilla.git
    GIT_TAG v3.8.4)
FetchContent_MakeAvailable(stringzilla)

add_executable(strstrbench src/strstrbench.c)
target_include_directories(strstrbench PRIVATE third-party/ubench/include
    stringzilla)
target_link_libraries(strstrbench m stringzilla)
