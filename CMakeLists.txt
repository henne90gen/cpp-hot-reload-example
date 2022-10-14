cmake_minimum_required(VERSION 3.20)

project(hot-reload-test)

include(FetchContent)
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 7482de6071d21db77a7236155da44c172a7f6c9e # 3.3.8
)
FetchContent_MakeAvailable(glfw)

find_package(OpenGL REQUIRED)

add_library(glad SHARED glad/src/glad.c)
target_include_directories(glad PUBLIC glad/include)
target_compile_definitions(glad PRIVATE GLAD_GLAPI_EXPORT GLAD_GLAPI_EXPORT_BUILD)

add_library(test_library SHARED test_library.cpp bla.cpp)
set_property(TARGET test_library PROPERTY CXX_STANDARD 20)
set_target_properties(test_library
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/temp"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/temp"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/temp"
        )

if (WIN32)
    target_compile_definitions(test_library PRIVATE GLAD_GLAPI_EXPORT)
    target_link_libraries(test_library PUBLIC OpenGL::GL glad)
    add_custom_command(TARGET test_library POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:test_library> ${CMAKE_BINARY_DIR}/test_library.dll.new && ${CMAKE_COMMAND} -E rename ${CMAKE_BINARY_DIR}/test_library.dll.new ${CMAKE_BINARY_DIR}/test_library.dll
            )
else ()
    target_include_directories(test_library PUBLIC glad/include)
    add_custom_command(TARGET test_library POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:test_library> ${CMAKE_BINARY_DIR}/libtest_library.so.new && ${CMAKE_COMMAND} -E rename ${CMAKE_BINARY_DIR}/libtest_library.so.new ${CMAKE_BINARY_DIR}/libtest_library.so
            )
endif ()

add_executable(test_executable test_executable.cpp)
target_link_libraries(test_executable glfw OpenGL::GL glad)
set_property(TARGET test_executable PROPERTY CXX_STANDARD 20)
add_dependencies(test_executable test_library)
target_compile_definitions(test_executable PRIVATE GLAD_GLAPI_EXPORT)
