#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    // Set Qt platform to xcb for proper mouse handling
    qputenv("QT_QPA_PLATFORM", "xcb");
    
    QApplication app(argc, argv);
    
    app.setApplicationName("GeoWorld");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("GeoWorld");
    app.setOrganizationDomain("geoworld.dev");
    
    MainWindow window;
    window.show();
    
    return app.exec();
}