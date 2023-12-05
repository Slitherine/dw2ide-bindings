cmake_minimum_required(VERSION 3.26)
cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0042 NEW)
cmake_policy(SET CMP0135 NEW)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(InstallRequiredSystemLibraries)

include(FetchContent)

FetchContent_Declare(googletest URL https://github.com/google/googletest/archive/master.zip)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)