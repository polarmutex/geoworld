#include "MapViewPlugin.h"
#include "QtLocationMapWidget.h"
#include <QDebug>

MapViewPlugin::MapViewPlugin(QObject* parent)
    : QObject(parent)
    , m_initialized(false)
{
}

MapViewPlugin::~MapViewPlugin()
{
    shutdown();
}

QString MapViewPlugin::name() const
{
    return "Map View";
}

QString MapViewPlugin::version() const
{
    return "1.0.0";
}

QString MapViewPlugin::description() const
{
    return "Interactive map widget using Qt Location with OpenStreetMap tiles";
}

QIcon MapViewPlugin::icon() const
{
    return QIcon(":/icons/map.png"); // We'll use a default icon for now
}

bool MapViewPlugin::initialize()
{
    if (m_initialized) {
        return true;
    }
    
    qDebug() << "Initializing Map View Plugin";
    
    // Any initialization code here
    m_initialized = true;
    return true;
}

void MapViewPlugin::shutdown()
{
    if (!m_initialized) {
        return;
    }
    
    qDebug() << "Shutting down Map View Plugin";
    
    // Any cleanup code here
    m_initialized = false;
}

QWidget* MapViewPlugin::createWidget(QWidget* parent)
{
    if (!m_initialized) {
        qWarning() << "Map View Plugin not initialized";
        return nullptr;
    }
    
    return new QtLocationMapWidget(parent);
}

QStringList MapViewPlugin::capabilities() const
{
    return QStringList() << "mapping" << "geolocation" << "tile-rendering" << "coordinate-display";
}