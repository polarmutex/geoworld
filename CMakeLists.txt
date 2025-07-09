cmake_minimum_required(VERSION 3.25)
project(GeoWorld VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)

# Find qt-advanced-docking-system - try different library names
find_library(ADS_LIBRARIES 
  NAMES qtadvanceddocking qt6advanceddocking advanceddocking ads qtads
  PATHS ${CMAKE_PREFIX_PATH}/lib
  NO_DEFAULT_PATH
)
find_path(ADS_INCLUDE_DIRS DockManager.h
  PATHS ${CMAKE_PREFIX_PATH}/include
  PATH_SUFFIXES ads advanceddocking qtadvanceddocking qt6advanceddocking "" .
)

# Verify we found the library
if(NOT ADS_LIBRARIES)
    message(FATAL_ERROR "Could not find qtadvanceddocking library")
endif()

if(NOT ADS_INCLUDE_DIRS)
    message(FATAL_ERROR "Could not find qtadvanceddocking headers")
endif()

qt_standard_project_setup()

set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/MapWidget.cpp
)

set(HEADERS
    src/MainWindow.h
    src/MapWidget.h
)

qt_add_executable(geoworld ${SOURCES} ${HEADERS})

# No QML resources needed anymore

target_link_libraries(geoworld PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    ${ADS_LIBRARIES}
)

target_include_directories(geoworld PRIVATE ${ADS_INCLUDE_DIRS})
target_compile_options(geoworld PRIVATE ${ADS_CFLAGS_OTHER})

# Install target
install(TARGETS geoworld
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

target_include_directories(geoworld PRIVATE src)