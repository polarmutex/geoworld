# GeoWorld - Geospatial Data Visualization Platform

## Project Overview
GeoWorld is a C++ application built with Qt6 that provides a map-based interface for visualizing streaming geospatial data. The application features both desktop and web interfaces with a plugin architecture for extensible functionality including mapping tools, data source integrations, and geospatial calculations. The desktop interface uses Qt Advanced Docking System for flexible window management.

## Technology Stack
- **Language**: C++
- **UI Framework**: Qt6 (Desktop and Web)
- **Docking System**: Qt Advanced Docking System
- **Package Manager**: Conan 2
- **Build System**: CMake
- **Development Environment**: Nix flake with flake-parts
- **Architecture**: Plugin-based system

## Core Features
- **Map Interface**: Interactive map display for geospatial data visualization
- **Streaming Data API**: Real-time data ingestion from various sources
- **Advanced Docking**: Flexible, customizable window layout system
- **Plugin System**: Modular architecture for extending functionality
- **Geospatial Tools**: Distance calculations, area measurements, coordinate transformations
- **Data Sources**: Support for multiple streaming data providers
- **Dual Interface**: Desktop application and web interface

## Project Structure
- Desktop application using Qt6 with Qt Advanced Docking System
- Web interface for browser-based access
- Plugin system for data sources and tools
- Streaming API for real-time data updates
- Geospatial calculation utilities
- Dockable panels for tools, data sources, and map layers

## Development Commands

### Setup Development Environment
```bash
# Enter development shell
nix develop

# Install dependencies with Conan
conan install . --build=missing
```

### Build Commands
```bash
# Configure build
cmake --preset=default

# Build project
cmake --build --preset=default

# Run tests
ctest --preset=default
```

### Code Quality
```bash
# Format code
clang-format -i src/**/*.cpp src/**/*.h

# Static analysis
clang-tidy src/

# Additional checks
cppcheck src/
```

### Plugin Development
- Data source plugins implement streaming data interface
- Tool plugins provide geospatial calculation functions
- Plugin widgets can be docked in the advanced docking system
- Place plugin source files in `plugins/` directory
- Use CMake to build plugins as shared libraries
- Plugin loading/unloading handled by main application

## Key Components
- **Map Engine**: Core mapping and rendering system
- **Data Stream Manager**: Handles real-time data ingestion
- **Plugin Manager**: Dynamic loading/unloading of functionality
- **Docking Manager**: Qt Advanced Docking System integration
- **Geospatial Tools**: Distance, area, coordinate utilities
- **API Layer**: RESTful API for external data sources
- **Web Interface**: Browser-based map visualization

## Docking System Features
- Dockable tool panels
- Data source management panels
- Map layer controls
- Plugin-specific widgets
- Customizable workspace layouts
- Persistent layout configuration

## Dependencies
- Qt6 (Core, Widgets, Quick, WebEngine, Network, Positioning)
- Qt Advanced Docking System
- Mapping libraries (OpenStreetMap, MapBox, etc.)
- Geospatial libraries (GDAL, PROJ, GEOS)
- Streaming/networking libraries
- Conan packages as specified in conanfile.py

## Notes
- Use flake-parts for modular Nix configuration
- Conan profiles may need adjustment for different platforms
- Plugin API documentation for data sources and tools
- Web interface serves Qt Quick/QML mapping content
- Real-time data streaming requires efficient memory management
- Docking system layouts saved/restored between sessions