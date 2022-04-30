
cmake_minimum_required(VERSION 3.0)

project(qwq LANGUAGES C)

find_package(SDL2 REQUIRED)

add_executable(qwq main.c)

target_link_libraries(qwq SDL2)

install(TARGETS qwq RUNTIME DESTINATION bin)
