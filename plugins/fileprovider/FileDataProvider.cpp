#include "FileDataProvider.h"
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTextStream>
#include <QDebug>
#include <QDir>

const QStringList FileDataProvider::s_supportedExtensions = {
    "geojson", "json", "csv", "kml"
};

FileDataProvider::FileDataProvider(QObject* parent)
    : QObject(parent)
    , m_initialized(false)
{
}

FileDataProvider::~FileDataProvider()
{
    shutdown();
}

QString FileDataProvider::providerId() const
{
    return "file-provider";
}

QString FileDataProvider::name() const
{
    return "File Data Provider";
}

QString FileDataProvider::description() const
{
    return "Loads geospatial data from local files including GeoJSON, CSV, and KML formats";
}

QIcon FileDataProvider::icon() const
{
    return QIcon(":/icons/file-provider.png");
}

QStringList FileDataProvider::supportedTypes() const
{
    return s_supportedExtensions;
}

bool FileDataProvider::canCreateLayers() const
{
    return false; // File provider only loads existing files
}

bool FileDataProvider::canImportData() const
{
    return true;
}

bool FileDataProvider::canExportData() const
{
    return true;
}

bool FileDataProvider::isRealTime() const
{
    return false;
}

QStringList FileDataProvider::layerIds() const
{
    return m_layers.keys();
}

IDataLayer* FileDataProvider::getLayer(const QString& layerId) const
{
    auto it = m_layers.find(layerId);
    return (it != m_layers.end()) ? it.value() : nullptr;
}

QList<IDataLayer*> FileDataProvider::getAllLayers() const
{
    QList<IDataLayer*> layers;
    for (auto it = m_layers.begin(); it != m_layers.end(); ++it) {
        layers.append(it.value());
    }
    return layers;
}

bool FileDataProvider::createLayer(const QString& name, const QString& type, const QVariantMap& parameters)
{
    Q_UNUSED(name)
    Q_UNUSED(type)
    Q_UNUSED(parameters)
    
    // File provider doesn't support creating new layers
    qWarning() << "File provider does not support creating new layers";
    return false;
}

bool FileDataProvider::removeLayer(const QString& layerId)
{
    auto it = m_layers.find(layerId);
    if (it == m_layers.end()) {
        qWarning() << "Layer not found:" << layerId;
        return false;
    }
    
    delete it.value();
    m_layers.erase(it);
    emit layerRemoved(layerId);
    
    qDebug() << "Removed layer:" << layerId;
    return true;
}

bool FileDataProvider::importData(const QString& filePath, const QVariantMap& options)
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        qWarning() << "File does not exist:" << filePath;
        return false;
    }
    
    QString extension = fileInfo.suffix().toLower();
    if (!s_supportedExtensions.contains(extension)) {
        qWarning() << "Unsupported file format:" << extension;
        return false;
    }
    
    // Generate unique layer ID
    QString layerId = generateLayerId();
    QString layerName = options.value("name", fileInfo.baseName()).toString();
    QString layerType = detectFileType(filePath);
    
    // Create the layer
    FileDataLayer* layer = new FileDataLayer(layerId, layerName, filePath, layerType);
    
    // Try to load the data
    if (!layer->loadFromFile()) {
        qWarning() << "Failed to load data from file:" << filePath;
        delete layer;
        return false;
    }
    
    // Store the layer
    m_layers[layerId] = layer;
    emit layerAdded(layerId);
    
    qDebug() << "Imported file as layer:" << layerId << "from" << filePath;
    return true;
}

bool FileDataProvider::exportLayer(const QString& layerId, const QString& filePath, const QVariantMap& options)
{
    auto it = m_layers.find(layerId);
    if (it == m_layers.end()) {
        qWarning() << "Layer not found:" << layerId;
        return false;
    }
    
    FileDataLayer* layer = it.value();
    QFileInfo fileInfo(filePath);
    QString extension = fileInfo.suffix().toLower();
    
    if (extension == "geojson" || extension == "json") {
        return exportGeoJSON(layer, filePath);
    } else if (extension == "csv") {
        return exportCSV(layer, filePath);
    } else {
        qWarning() << "Unsupported export format:" << extension;
        return false;
    }
}

bool FileDataProvider::initialize()
{
    if (m_initialized) {
        return true;
    }
    
    qDebug() << "Initializing File Data Provider";
    
    // No special initialization needed for file provider
    m_initialized = true;
    return true;
}

void FileDataProvider::shutdown()
{
    if (!m_initialized) {
        return;
    }
    
    qDebug() << "Shutting down File Data Provider";
    
    // Delete all layers
    for (auto it = m_layers.begin(); it != m_layers.end(); ++it) {
        delete it.value();
    }
    m_layers.clear();
    m_initialized = false;
}

QString FileDataProvider::detectFileType(const QString& filePath) const
{
    QFileInfo fileInfo(filePath);
    QString extension = fileInfo.suffix().toLower();
    
    if (extension == "geojson" || extension == "json" || extension == "kml") {
        return "vector";
    } else if (extension == "csv") {
        return "vector"; // CSV typically contains vector data
    }
    
    return "unknown";
}

QString FileDataProvider::generateLayerId() const
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

bool FileDataProvider::exportGeoJSON(FileDataLayer* layer, const QString& filePath) const
{
    if (!layer || !layer->isDataLoaded()) {
        qWarning() << "Layer has no data to export";
        return false;
    }
    
    QVariant data = layer->data();
    QJsonDocument doc = QJsonDocument::fromVariant(data);
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Cannot write to file:" << filePath;
        return false;
    }
    
    file.write(doc.toJson());
    qDebug() << "Exported layer to GeoJSON:" << filePath;
    return true;
}

bool FileDataProvider::exportCSV(FileDataLayer* layer, const QString& filePath) const
{
    if (!layer || !layer->isDataLoaded()) {
        qWarning() << "Layer has no data to export";
        return false;
    }
    
    QVariant data = layer->data();
    QVariantMap geoData = data.toMap();
    
    if (geoData["type"].toString() != "FeatureCollection") {
        qWarning() << "Cannot export non-FeatureCollection to CSV";
        return false;
    }
    
    QVariantList features = geoData["features"].toList();
    if (features.isEmpty()) {
        qWarning() << "No features to export";
        return false;
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Cannot write to file:" << filePath;
        return false;
    }
    
    QTextStream out(&file);
    
    // Write header from first feature
    QVariantMap firstFeature = features[0].toMap();
    QVariantMap properties = firstFeature["properties"].toMap();
    QStringList headers = properties.keys();
    out << headers.join(",") << "\n";
    
    // Write data rows
    for (const QVariant& featureVar : features) {
        QVariantMap feature = featureVar.toMap();
        QVariantMap props = feature["properties"].toMap();
        
        QStringList values;
        for (const QString& header : headers) {
            values.append(props.value(header).toString());
        }
        out << values.join(",") << "\n";
    }
    
    qDebug() << "Exported layer to CSV:" << filePath;
    return true;
}