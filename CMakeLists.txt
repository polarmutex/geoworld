cmake_minimum_required(VERSION 3.25)
project(GeoWorld VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Quick QuickWidgets Network Positioning)
find_package(ads REQUIRED)

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

qt_add_qml_module(geoworld
    URI GeoWorld
    VERSION 1.0
    QML_FILES
        qml/MapView.qml
)

target_link_libraries(geoworld
    Qt6::Core
    Qt6::Widgets
    Qt6::Quick
    Qt6::QuickWidgets
    Qt6::Network
    Qt6::Positioning
    ads::qtadvanceddocking
)

# Install target
install(TARGETS geoworld
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

target_include_directories(geoworld PRIVATE src)