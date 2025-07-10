#pragma once

#include "IDataProvider.h"
#include "FileDataLayer.h"
#include <QObject>
#include <QMap>
#include <QUuid>
#include <memory>

class FileDataProvider : public QObject, public IDataProvider
{
    Q_OBJECT

public:
    explicit FileDataProvider(QObject* parent = nullptr);
    ~FileDataProvider();

    // IDataProvider interface
    QString providerId() const override;
    QString name() const override;
    QString description() const override;
    QIcon icon() const override;
    QStringList supportedTypes() const override;
    
    bool canCreateLayers() const override;
    bool canImportData() const override;
    bool canExportData() const override;
    bool isRealTime() const override;
    
    QStringList layerIds() const override;
    IDataLayer* getLayer(const QString& layerId) const override;
    QList<IDataLayer*> getAllLayers() const override;
    
    bool createLayer(const QString& name, const QString& type, 
                     const QVariantMap& parameters = QVariantMap()) override;
    bool removeLayer(const QString& layerId) override;
    bool importData(const QString& filePath, 
                    const QVariantMap& options = QVariantMap()) override;
    bool exportLayer(const QString& layerId, const QString& filePath, 
                     const QVariantMap& options = QVariantMap()) override;
    
    bool initialize() override;
    void shutdown() override;

signals:
    void layerAdded(const QString& layerId) override;
    void layerRemoved(const QString& layerId) override;
    void layerChanged(const QString& layerId) override;
    void dataUpdated(const QString& layerId) override;

private:
    QString detectFileType(const QString& filePath) const;
    QString generateLayerId() const;
    bool exportGeoJSON(FileDataLayer* layer, const QString& filePath) const;
    bool exportCSV(FileDataLayer* layer, const QString& filePath) const;
    
    QMap<QString, FileDataLayer*> m_layers;
    bool m_initialized;
    
    static const QStringList s_supportedExtensions;
};