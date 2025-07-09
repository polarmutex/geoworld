cmake_minimum_required(VERSION 3.5)

project(app LANGUAGES CXX)

option(BUILD_UNIT_TESTS "Build unit tests (default OFF)" OFF)
option(CODE_COVERAGE "Enable coverage reporting (default OFF)" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Tell CMake to call moc automaticly
# set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTORCC ON)
# set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(PLUGIN_DIR ${CMAKE_BINARY_DIR}/plugins)

option(BUILD_SHARED_LIBS "Build all libraries as shared libraries instead of static" OFF)



# include(cmake/conan_provider.cmake)
# conan_cmake_run(REQUIRES
#     qt/6.2.1
#     libffi/3.4.2
#     BASIC_SETUP CMAKE_TARGETS
#     GENERATORS cmake_find_package cmake_paths
#     BUILD missing
#     OPTIONS qt:shared=True)
# include(${CMAKE_BINARY_DIR}/conan_paths.cmake)

# add_library(coverage_config INTERFACE)

# if(CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
#     # Add required flags (GCC & LLVM/Clang)
#     target_compile_options(coverage_config INTERFACE
#         -O0        # no optimization
#         -g         # generate debug info
#         --coverage # sets all required flags
#     )
#
#     target_link_options(coverage_config INTERFACE --coverage)
# endif(CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")

include(${CMAKE_SOURCE_DIR}/cmake/clang_format.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/cppcheck.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/clang_tidy.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/cpplint.cmake)

add_subdirectory(src)

# if (BUILD_UNIT_TESTS)
#     enable_testing()
#     add_subdirectory(test)
# endif()
