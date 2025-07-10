#include "DataProviderManager.h"
#include <QDebug>
#include <QFileInfo>

DataProviderManager::DataProviderManager(QObject* parent)
    : QObject(parent)
{
}

DataProviderManager::~DataProviderManager()
{
    // Disconnect and shutdown all providers
    for (auto it = m_providers.begin(); it != m_providers.end(); ++it) {
        disconnectProvider(it.value());
        it.value()->shutdown();
    }
    m_providers.clear();
    m_layerToProvider.clear();
}

bool DataProviderManager::registerProvider(IDataProvider* provider)
{
    if (!provider) {
        qWarning() << "Cannot register null data provider";
        return false;
    }
    
    QString providerId = provider->providerId();
    if (m_providers.contains(providerId)) {
        qWarning() << "Data provider already registered:" << providerId;
        return false;
    }
    
    if (!provider->initialize()) {
        qWarning() << "Failed to initialize data provider:" << providerId;
        return false;
    }
    
    m_providers[providerId] = provider;
    connectProvider(provider);
    
    qDebug() << "Registered data provider:" << providerId;
    emit providerRegistered(providerId);
    
    // Register existing layers
    for (const QString& layerId : provider->layerIds()) {
        QString globalId = makeGlobalLayerId(providerId, layerId);
        m_layerToProvider[globalId] = providerId;
        emit layerAdded(providerId, layerId);
    }
    
    return true;
}

bool DataProviderManager::unregisterProvider(const QString& providerId)
{
    auto it = m_providers.find(providerId);
    if (it == m_providers.end()) {
        qWarning() << "Data provider not found:" << providerId;
        return false;
    }
    
    IDataProvider* provider = it.value();
    
    // Remove all layers from this provider
    QStringList layersToRemove;
    for (auto layerIt = m_layerToProvider.begin(); layerIt != m_layerToProvider.end(); ++layerIt) {
        if (layerIt.value() == providerId) {
            layersToRemove.append(layerIt.key());
        }
    }
    
    for (const QString& globalLayerId : layersToRemove) {
        QPair<QString, QString> ids = parseGlobalLayerId(globalLayerId);
        emit layerRemoved(providerId, ids.second);
        m_layerToProvider.remove(globalLayerId);
    }
    
    disconnectProvider(provider);
    provider->shutdown();
    m_providers.erase(it);
    
    qDebug() << "Unregistered data provider:" << providerId;
    emit providerUnregistered(providerId);
    
    return true;
}

QStringList DataProviderManager::providerIds() const
{
    return m_providers.keys();
}

IDataProvider* DataProviderManager::getProvider(const QString& providerId) const
{
    return m_providers.value(providerId, nullptr);
}

QList<IDataProvider*> DataProviderManager::getAllProviders() const
{
    return m_providers.values();
}

QStringList DataProviderManager::allLayerIds() const
{
    return m_layerToProvider.keys();
}

IDataLayer* DataProviderManager::getLayer(const QString& layerId) const
{
    auto it = m_layerToProvider.find(layerId);
    if (it == m_layerToProvider.end()) {
        return nullptr;
    }
    
    QString providerId = it.value();
    QPair<QString, QString> ids = parseGlobalLayerId(layerId);
    
    IDataProvider* provider = m_providers.value(providerId);
    return provider ? provider->getLayer(ids.second) : nullptr;
}

IDataLayer* DataProviderManager::getLayer(const QString& providerId, const QString& layerId) const
{
    IDataProvider* provider = m_providers.value(providerId);
    return provider ? provider->getLayer(layerId) : nullptr;
}

QList<IDataLayer*> DataProviderManager::getAllLayers() const
{
    QList<IDataLayer*> layers;
    for (IDataProvider* provider : m_providers.values()) {
        layers.append(provider->getAllLayers());
    }
    return layers;
}

QList<IDataLayer*> DataProviderManager::getLayersByType(const QString& type) const
{
    QList<IDataLayer*> layers;
    for (IDataLayer* layer : getAllLayers()) {
        if (layer->type() == type) {
            layers.append(layer);
        }
    }
    return layers;
}

QList<IDataLayer*> DataProviderManager::getVisibleLayers() const
{
    QList<IDataLayer*> layers;
    for (IDataLayer* layer : getAllLayers()) {
        if (layer->isVisible()) {
            layers.append(layer);
        }
    }
    return layers;
}

QList<IDataProvider*> DataProviderManager::getProvidersByType(const QString& type) const
{
    QList<IDataProvider*> providers;
    for (IDataProvider* provider : m_providers.values()) {
        if (provider->supportedTypes().contains(type)) {
            providers.append(provider);
        }
    }
    return providers;
}

QList<IDataProvider*> DataProviderManager::getRealTimeProviders() const
{
    QList<IDataProvider*> providers;
    for (IDataProvider* provider : m_providers.values()) {
        if (provider->isRealTime()) {
            providers.append(provider);
        }
    }
    return providers;
}

void DataProviderManager::setLayerVisible(const QString& layerId, bool visible)
{
    IDataLayer* layer = getLayer(layerId);
    if (layer) {
        layer->setVisible(visible);
        emit layerVisibilityChanged(layerId, visible);
        emit layersChanged();
    }
}

void DataProviderManager::setLayerOpacity(const QString& layerId, double opacity)
{
    IDataLayer* layer = getLayer(layerId);
    if (layer) {
        layer->setOpacity(opacity);
        emit layersChanged();
    }
}

QStringList DataProviderManager::getSupportedImportFormats() const
{
    QStringList formats;
    for (IDataProvider* provider : m_providers.values()) {
        if (provider->canImportData()) {
            formats.append(provider->supportedTypes());
        }
    }
    formats.removeDuplicates();
    return formats;
}

QStringList DataProviderManager::getSupportedExportFormats() const
{
    QStringList formats;
    for (IDataProvider* provider : m_providers.values()) {
        if (provider->canExportData()) {
            formats.append(provider->supportedTypes());
        }
    }
    formats.removeDuplicates();
    return formats;
}

bool DataProviderManager::importData(const QString& filePath, const QString& preferredProviderId)
{
    QFileInfo fileInfo(filePath);
    QString extension = fileInfo.suffix().toLower();
    
    // Try preferred provider first
    if (!preferredProviderId.isEmpty()) {
        IDataProvider* provider = getProvider(preferredProviderId);
        if (provider && provider->canImportData()) {
            if (provider->importData(filePath)) {
                qDebug() << "Data imported successfully by provider:" << preferredProviderId;
                return true;
            }
        }
    }
    
    // Try all providers that can import this type
    for (IDataProvider* provider : m_providers.values()) {
        if (provider->canImportData() && provider->supportedTypes().contains(extension)) {
            if (provider->importData(filePath)) {
                qDebug() << "Data imported successfully by provider:" << provider->providerId();
                return true;
            }
        }
    }
    
    qWarning() << "No provider could import file:" << filePath;
    return false;
}

void DataProviderManager::onProviderLayerAdded(const QString& layerId)
{
    IDataProvider* provider = qobject_cast<IDataProvider*>(sender());
    if (provider) {
        QString providerId = provider->providerId();
        QString globalId = makeGlobalLayerId(providerId, layerId);
        m_layerToProvider[globalId] = providerId;
        emit layerAdded(providerId, layerId);
        emit layersChanged();
    }
}

void DataProviderManager::onProviderLayerRemoved(const QString& layerId)
{
    IDataProvider* provider = qobject_cast<IDataProvider*>(sender());
    if (provider) {
        QString providerId = provider->providerId();
        QString globalId = makeGlobalLayerId(providerId, layerId);
        m_layerToProvider.remove(globalId);
        emit layerRemoved(providerId, layerId);
        emit layersChanged();
    }
}

void DataProviderManager::onProviderLayerChanged(const QString& layerId)
{
    IDataProvider* provider = qobject_cast<IDataProvider*>(sender());
    if (provider) {
        QString providerId = provider->providerId();
        emit layerChanged(providerId, layerId);
        emit layersChanged();
    }
}

void DataProviderManager::onProviderDataUpdated(const QString& layerId)
{
    IDataProvider* provider = qobject_cast<IDataProvider*>(sender());
    if (provider) {
        QString providerId = provider->providerId();
        emit dataUpdated(providerId, layerId);
    }
}

QString DataProviderManager::makeGlobalLayerId(const QString& providerId, const QString& layerId) const
{
    return QString("%1::%2").arg(providerId, layerId);
}

QPair<QString, QString> DataProviderManager::parseGlobalLayerId(const QString& globalLayerId) const
{
    QStringList parts = globalLayerId.split("::");
    if (parts.size() == 2) {
        return qMakePair(parts[0], parts[1]);
    }
    return qMakePair(QString(), globalLayerId);
}

void DataProviderManager::connectProvider(IDataProvider* provider)
{
    QObject* providerObj = dynamic_cast<QObject*>(provider);
    if (providerObj) {
        connect(providerObj, SIGNAL(layerAdded(QString)), 
                this, SLOT(onProviderLayerAdded(QString)));
        connect(providerObj, SIGNAL(layerRemoved(QString)), 
                this, SLOT(onProviderLayerRemoved(QString)));
        connect(providerObj, SIGNAL(layerChanged(QString)), 
                this, SLOT(onProviderLayerChanged(QString)));
        connect(providerObj, SIGNAL(dataUpdated(QString)), 
                this, SLOT(onProviderDataUpdated(QString)));
    }
}

void DataProviderManager::disconnectProvider(IDataProvider* provider)
{
    QObject* providerObj = dynamic_cast<QObject*>(provider);
    if (providerObj) {
        disconnect(providerObj, nullptr, this, nullptr);
    }
}