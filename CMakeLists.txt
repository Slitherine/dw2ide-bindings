cmake_minimum_required(VERSION 3.26)
cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0042 NEW)
cmake_policy(SET CMP0135 NEW)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(InstallRequiredSystemLibraries)

project(dw2ide_runtime)

include(CMakeJS.cmake)
setup_cmake_js()
if (NOT CMAKE_JS_VERSION)
    message(FATAL_ERROR "CMake.js is missing or not set up.")
endif ()

add_definitions(-DNAPI_VERSION=8)

include_directories(${CMAKE_JS_INC})

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS LIST_DIRECTORIES false
        "src/*.cxx"
        "src/*.cpp"
        "src/*.c"
        "src/*.hpp"
        "src/*.h"
        "include/*.hpp"
        "include/*.h")

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${CMAKE_JS_SRC})
target_include_directories(${PROJECT_NAME} PRIVATE include)
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB})

if (MSVC AND CMAKE_JS_NODELIB_DEF AND CMAKE_JS_NODELIB_TARGET)
    # touch these files to make sure they exist
    file(TOUCH ${CMAKE_JS_NODELIB_DEF})
    file(TOUCH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/node-lib.exp)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_AR} /def:${CMAKE_JS_NODELIB_DEF} /out:${CMAKE_JS_NODELIB_TARGET} ${CMAKE_STATIC_LINKER_FLAGS}
            COMMENT "Generating node.lib")
endif ()


option(dw2ide_runtime_SKIP_TESTS "Skip building and running the tests." ON)
if (dw2ide_runtime_SKIP_TESTS)
    MESSAGE(WARNING "Tests skipped!")
else ()
    enable_testing()
    add_subdirectory(tests)
endif ()
