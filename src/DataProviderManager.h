#pragma once

#include <QObject>
#include <QMap>
#include <QStringList>
#include "IDataProvider.h"

class DataProviderManager : public QObject
{
    Q_OBJECT

public:
    explicit DataProviderManager(QObject* parent = nullptr);
    ~DataProviderManager();
    
    // Provider registration
    bool registerProvider(IDataProvider* provider);
    bool unregisterProvider(const QString& providerId);
    
    // Provider access
    QStringList providerIds() const;
    IDataProvider* getProvider(const QString& providerId) const;
    QList<IDataProvider*> getAllProviders() const;
    
    // Layer discovery across all providers
    QStringList allLayerIds() const;
    IDataLayer* getLayer(const QString& layerId) const;
    IDataLayer* getLayer(const QString& providerId, const QString& layerId) const;
    QList<IDataLayer*> getAllLayers() const;
    QList<IDataLayer*> getLayersByType(const QString& type) const;
    QList<IDataLayer*> getVisibleLayers() const;
    
    // Provider filtering
    QList<IDataProvider*> getProvidersByType(const QString& type) const;
    QList<IDataProvider*> getRealTimeProviders() const;
    
    // Global layer operations
    void setLayerVisible(const QString& layerId, bool visible);
    void setLayerOpacity(const QString& layerId, double opacity);
    
    // Data import/export coordination
    QStringList getSupportedImportFormats() const;
    QStringList getSupportedExportFormats() const;
    bool importData(const QString& filePath, const QString& preferredProviderId = QString());
    
signals:
    // Provider events
    void providerRegistered(const QString& providerId);
    void providerUnregistered(const QString& providerId);
    
    // Layer events (forwarded from providers)
    void layerAdded(const QString& providerId, const QString& layerId);
    void layerRemoved(const QString& providerId, const QString& layerId);
    void layerChanged(const QString& providerId, const QString& layerId);
    void layerVisibilityChanged(const QString& layerId, bool visible);
    void dataUpdated(const QString& providerId, const QString& layerId);
    
    // Global events
    void layersChanged(); // Any layer was added/removed/modified

private slots:
    void onProviderLayerAdded(const QString& layerId);
    void onProviderLayerRemoved(const QString& layerId);
    void onProviderLayerChanged(const QString& layerId);
    void onProviderDataUpdated(const QString& layerId);

private:
    QString makeGlobalLayerId(const QString& providerId, const QString& layerId) const;
    QPair<QString, QString> parseGlobalLayerId(const QString& globalLayerId) const;
    void connectProvider(IDataProvider* provider);
    void disconnectProvider(IDataProvider* provider);
    
    QMap<QString, IDataProvider*> m_providers;
    QMap<QString, QString> m_layerToProvider; // globalLayerId -> providerId
};