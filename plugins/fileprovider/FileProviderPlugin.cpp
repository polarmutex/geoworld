#include "FileProviderPlugin.h"
#include <QDebug>

FileProviderPlugin::FileProviderPlugin(QObject* parent)
    : QObject(parent)
    , m_dataProvider(nullptr)
    , m_initialized(false)
{
}

FileProviderPlugin::~FileProviderPlugin()
{
    shutdown();
}

QString FileProviderPlugin::name() const
{
    return "File Data Provider";
}

QString FileProviderPlugin::version() const
{
    return "1.0.0";
}

QString FileProviderPlugin::description() const
{
    return "Provides geospatial data from local files including GeoJSON, CSV, and KML";
}

QIcon FileProviderPlugin::icon() const
{
    return QIcon(":/icons/file-provider.png");
}

bool FileProviderPlugin::initialize()
{
    if (m_initialized) {
        return true;
    }
    
    qDebug() << "Initializing File Provider Plugin";
    
    // Create the data provider
    m_dataProvider = new FileDataProvider(this);
    
    if (!m_dataProvider->initialize()) {
        qWarning() << "Failed to initialize File Data Provider";
        delete m_dataProvider;
        m_dataProvider = nullptr;
        return false;
    }
    
    // TODO: Register with DataProviderManager
    // This would be done by the core application when the plugin is loaded
    
    m_initialized = true;
    qDebug() << "File Provider Plugin initialized successfully";
    return true;
}

void FileProviderPlugin::shutdown()
{
    if (!m_initialized) {
        return;
    }
    
    qDebug() << "Shutting down File Provider Plugin";
    
    if (m_dataProvider) {
        m_dataProvider->shutdown();
        delete m_dataProvider;
        m_dataProvider = nullptr;
    }
    
    m_initialized = false;
}

QWidget* FileProviderPlugin::createWidget(QWidget* parent)
{
    Q_UNUSED(parent)
    
    // File provider doesn't have a UI widget
    // Data management is handled by the Data Browser plugin
    return nullptr;
}

QStringList FileProviderPlugin::capabilities() const
{
    return QStringList() << "data-provider" << "import-export";
}