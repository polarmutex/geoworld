#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    app.setApplicationName("GeoWorld");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("GeoWorld");
    app.setOrganizationDomain("geoworld.dev");
    
    MainWindow window;
    window.show();
    
    return app.exec();
}