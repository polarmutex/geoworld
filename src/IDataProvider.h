#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QIcon>
#include <QDateTime>

class IDataLayer
{
public:
    virtual ~IDataLayer() = default;
    
    // Layer metadata
    virtual QString id() const = 0;
    virtual QString name() const = 0;
    virtual QString type() const = 0; // "vector", "raster", "realtime", etc.
    virtual QString description() const = 0;
    virtual QIcon icon() const = 0;
    
    // Layer state
    virtual bool isVisible() const = 0;
    virtual void setVisible(bool visible) = 0;
    virtual double opacity() const = 0;
    virtual void setOpacity(double opacity) = 0;
    
    // Layer data access
    virtual QVariantMap properties() const = 0;
    virtual QVariantMap style() const = 0;
    virtual void setStyle(const QVariantMap& style) = 0;
    
    // Spatial extent
    virtual QVariantMap boundingBox() const = 0; // {minLat, minLon, maxLat, maxLon}
    
    // Data access (returns format-specific data)
    virtual QVariant data() const = 0;
    virtual QDateTime lastUpdated() const = 0;
};

Q_DECLARE_INTERFACE(IDataLayer, "com.geoworld.IDataLayer/1.0")

class IDataProvider
{
public:
    virtual ~IDataProvider() = default;
    
    // Provider metadata
    virtual QString providerId() const = 0;
    virtual QString name() const = 0;
    virtual QString description() const = 0;
    virtual QIcon icon() const = 0;
    virtual QStringList supportedTypes() const = 0; // Data types this provider can handle
    
    // Provider capabilities
    virtual bool canCreateLayers() const = 0;
    virtual bool canImportData() const = 0;
    virtual bool canExportData() const = 0;
    virtual bool isRealTime() const = 0;
    
    // Layer management
    virtual QStringList layerIds() const = 0;
    virtual IDataLayer* getLayer(const QString& layerId) const = 0;
    virtual QList<IDataLayer*> getAllLayers() const = 0;
    
    // Layer operations
    virtual bool createLayer(const QString& name, const QString& type, const QVariantMap& parameters = QVariantMap()) = 0;
    virtual bool removeLayer(const QString& layerId) = 0;
    virtual bool importData(const QString& filePath, const QVariantMap& options = QVariantMap()) = 0;
    virtual bool exportLayer(const QString& layerId, const QString& filePath, const QVariantMap& options = QVariantMap()) = 0;
    
    // Provider lifecycle
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    
signals:
    virtual void layerAdded(const QString& layerId) = 0;
    virtual void layerRemoved(const QString& layerId) = 0;
    virtual void layerChanged(const QString& layerId) = 0;
    virtual void dataUpdated(const QString& layerId) = 0;
};

Q_DECLARE_INTERFACE(IDataProvider, "com.geoworld.IDataProvider/1.0")