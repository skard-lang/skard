cmake_minimum_required(VERSION 3.20)
project(skard C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DSKARD_DEBUG -DSKARD_DEBUG_TRACE")

add_compile_options(-Wall -Wextra -Wpedantic -Werror)

file(GLOB SKARD_LIB_SOURCE_FILES skard-lib/src/*.h skard-lib/src/*.c)
add_library(skard-lib STATIC ${SKARD_LIB_SOURCE_FILES})
target_compile_definitions(skard-lib PRIVATE -D__USE_MINGW_ANSI_STDIO)

file(GLOB SKARD_RUNTIME_SOURCE_FILES skard-runtime/src/*.h skard-runtime/src/*.c)
add_executable(skard ${SKARD_RUNTIME_SOURCE_FILES})

set_target_properties(skard-lib skard PROPERTIES LINKER_LANGUAGE C)

target_include_directories(skard PRIVATE skard-lib/src)
target_link_libraries(skard skard-lib)