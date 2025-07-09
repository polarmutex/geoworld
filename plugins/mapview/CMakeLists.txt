cmake_minimum_required(VERSION 3.25)
project(MapViewPlugin VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Location Positioning)

# Plugin sources
set(PLUGIN_SOURCES
    QtLocationMapWidget.cpp
    MapViewPlugin.cpp
)

set(PLUGIN_HEADERS
    QtLocationMapWidget.h
    MapViewPlugin.h
)

# Create plugin library
add_library(mapview SHARED ${PLUGIN_SOURCES} ${PLUGIN_HEADERS})

# Set up Qt
qt_standard_project_setup()

# Link Qt libraries
target_link_libraries(mapview PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::Location
    Qt6::Positioning
)

# Include directories
target_include_directories(mapview PRIVATE
    ../../src  # For IPlugin.h
)

# Plugin properties
set_target_properties(mapview PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    PREFIX ""  # Remove lib prefix on Linux
)

# Install the plugin
install(TARGETS mapview
    LIBRARY DESTINATION lib/geoworld/plugins
    RUNTIME DESTINATION lib/geoworld/plugins
)