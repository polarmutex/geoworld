#include "LayerManagerPlugin.h"
#include "LayerManagerWidget.h"
#include "DataProviderManager.h"
#include <QDebug>

LayerManagerPlugin::LayerManagerPlugin(QObject* parent)
    : QObject(parent)
    , m_initialized(false)
    , m_dataProviderManager(nullptr)
{
}

LayerManagerPlugin::~LayerManagerPlugin()
{
    shutdown();
}

QString LayerManagerPlugin::name() const
{
    return "Layer Manager";
}

QString LayerManagerPlugin::version() const
{
    return "1.0.0";
}

QString LayerManagerPlugin::description() const
{
    return "Layer management widget for controlling data layers, visibility, ordering, and properties";
}

QIcon LayerManagerPlugin::icon() const
{
    return QIcon(":/icons/layermanager.png");
}

bool LayerManagerPlugin::initialize()
{
    if (m_initialized) {
        return true;
    }
    
    qDebug() << "Initializing Layer Manager Plugin";
    m_initialized = true;
    return true;
}

void LayerManagerPlugin::shutdown()
{
    if (!m_initialized) {
        return;
    }
    
    qDebug() << "Shutting down Layer Manager Plugin";
    m_dataProviderManager = nullptr;
    m_initialized = false;
}

QWidget* LayerManagerPlugin::createWidget(QWidget* parent)
{
    if (!m_initialized) {
        qWarning() << "Layer Manager Plugin not initialized";
        return nullptr;
    }
    
    return new LayerManagerWidget(m_dataProviderManager, parent);
}

QStringList LayerManagerPlugin::capabilities() const
{
    return QStringList() << "layer-manager" << "layer-ordering" << "data-visualization";
}

void LayerManagerPlugin::setDataProviderManager(DataProviderManager* manager)
{
    m_dataProviderManager = manager;
}

DataProviderManager* LayerManagerPlugin::dataProviderManager() const
{
    return m_dataProviderManager;
}