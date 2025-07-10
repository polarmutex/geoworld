cmake_minimum_required(VERSION 3.25)
project(FileProviderPlugin VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Plugin sources
set(PLUGIN_SOURCES
    FileProviderPlugin.cpp
    FileDataProvider.cpp
    FileDataLayer.cpp
)

set(PLUGIN_HEADERS
    FileProviderPlugin.h
    FileDataProvider.h
    FileDataLayer.h
)

# Create plugin library
add_library(fileprovider SHARED ${PLUGIN_SOURCES} ${PLUGIN_HEADERS})

# Set up Qt
qt_standard_project_setup()

# Link Qt libraries
target_link_libraries(fileprovider PRIVATE
    Qt6::Core
    Qt6::Widgets
)

# Include directories
target_include_directories(fileprovider PRIVATE
    ../../src  # For core interfaces
)

# Plugin properties
set_target_properties(fileprovider PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    PREFIX ""  # Remove lib prefix on Linux
)

# Install the plugin
install(TARGETS fileprovider
    LIBRARY DESTINATION lib/geoworld/plugins
    RUNTIME DESTINATION lib/geoworld/plugins
)