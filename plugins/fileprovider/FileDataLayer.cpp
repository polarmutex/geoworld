#include "FileDataLayer.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDebug>
#include <QUuid>

FileDataLayer::FileDataLayer(const QString& id, const QString& name, const QString& filePath, const QString& type)
    : m_id(id)
    , m_name(name)
    , m_type(type)
    , m_filePath(filePath)
    , m_visible(true)
    , m_opacity(1.0)
    , m_dataLoaded(false)
    , m_lastUpdated(QDateTime::currentDateTime())
{
    QFileInfo fileInfo(filePath);
    m_description = QString("File layer: %1").arg(fileInfo.fileName());
    
    // Set default style based on type
    if (m_type == "vector") {
        m_style["stroke"] = "#0000FF";
        m_style["strokeWidth"] = 2;
        m_style["fill"] = "#0000FF33";
    }
    
    // Extract basic properties
    m_properties["fileName"] = fileInfo.fileName();
    m_properties["filePath"] = filePath;
    m_properties["fileSize"] = fileInfo.size();
    m_properties["lastModified"] = fileInfo.lastModified().toString();
}

FileDataLayer::~FileDataLayer()
{
}

QIcon FileDataLayer::icon() const
{
    // Return different icons based on layer type
    if (m_type == "vector") {
        return QIcon(":/icons/vector-layer.png");
    } else if (m_type == "raster") {
        return QIcon(":/icons/raster-layer.png");
    }
    return QIcon(":/icons/file-layer.png");
}

void FileDataLayer::setVisible(bool visible)
{
    if (m_visible != visible) {
        m_visible = visible;
        // In a real implementation, this would trigger a repaint
    }
}

void FileDataLayer::setOpacity(double opacity)
{
    opacity = qBound(0.0, opacity, 1.0);
    if (m_opacity != opacity) {
        m_opacity = opacity;
        // In a real implementation, this would trigger a repaint
    }
}

QVariant FileDataLayer::data() const
{
    if (!m_dataLoaded) {
        const_cast<FileDataLayer*>(this)->loadFromFile();
    }
    return m_cachedData;
}

bool FileDataLayer::loadFromFile()
{
    if (m_dataLoaded) {
        return true;
    }
    
    QFileInfo fileInfo(m_filePath);
    QString extension = fileInfo.suffix().toLower();
    
    bool success = false;
    if (extension == "json" || extension == "geojson") {
        success = loadGeoJSON();
    } else if (extension == "csv") {
        success = loadCSV();
    } else if (extension == "kml") {
        success = loadKML();
    } else {
        qWarning() << "Unsupported file format:" << extension;
        return false;
    }
    
    if (success) {
        m_dataLoaded = true;
        calculateBoundingBox();
        extractProperties();
        m_lastUpdated = QDateTime::currentDateTime();
    }
    
    return success;
}

bool FileDataLayer::loadGeoJSON()
{
    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open file:" << m_filePath;
        return false;
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (doc.isNull()) {
        qWarning() << "Invalid JSON in file:" << m_filePath << "Error:" << error.errorString();
        return false;
    }
    
    m_cachedData = doc.object().toVariantMap();
    m_type = "vector";
    return true;
}

bool FileDataLayer::loadCSV()
{
    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open CSV file:" << m_filePath;
        return false;
    }
    
    QTextStream in(&file);
    QVariantList features;
    QStringList headers;
    bool firstLine = true;
    
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split(',');
        
        if (firstLine) {
            headers = fields;
            firstLine = false;
            continue;
        }
        
        // Create a simple feature from CSV row
        QVariantMap feature;
        QVariantMap properties;
        
        for (int i = 0; i < qMin(headers.size(), fields.size()); ++i) {
            properties[headers[i]] = fields[i];
        }
        
        feature["properties"] = properties;
        // For CSV, we'd need to detect lat/lon columns for geometry
        // This is a simplified implementation
        features.append(feature);
    }
    
    QVariantMap geoData;
    geoData["type"] = "FeatureCollection";
    geoData["features"] = features;
    
    m_cachedData = geoData;
    m_type = "vector";
    return true;
}

bool FileDataLayer::loadKML()
{
    // KML loading would require XML parsing
    // This is a placeholder implementation
    qWarning() << "KML loading not implemented yet";
    return false;
}

void FileDataLayer::calculateBoundingBox()
{
    if (!m_dataLoaded || m_cachedData.isNull()) {
        return;
    }
    
    QVariantMap data = m_cachedData.toMap();
    if (data["type"].toString() != "FeatureCollection") {
        return;
    }
    
    QVariantList features = data["features"].toList();
    if (features.isEmpty()) {
        return;
    }
    
    double minLat = 90.0, maxLat = -90.0;
    double minLon = 180.0, maxLon = -180.0;
    bool hasCoordinates = false;
    
    for (const QVariant& featureVar : features) {
        QVariantMap feature = featureVar.toMap();
        QVariantMap geometry = feature["geometry"].toMap();
        
        if (geometry["type"].toString() == "Point") {
            QVariantList coords = geometry["coordinates"].toList();
            if (coords.size() >= 2) {
                double lon = coords[0].toDouble();
                double lat = coords[1].toDouble();
                
                minLon = qMin(minLon, lon);
                maxLon = qMax(maxLon, lon);
                minLat = qMin(minLat, lat);
                maxLat = qMax(maxLat, lat);
                hasCoordinates = true;
            }
        }
        // TODO: Handle other geometry types
    }
    
    if (hasCoordinates) {
        m_boundingBox["minLat"] = minLat;
        m_boundingBox["maxLat"] = maxLat;
        m_boundingBox["minLon"] = minLon;
        m_boundingBox["maxLon"] = maxLon;
    }
}

void FileDataLayer::extractProperties()
{
    if (!m_dataLoaded || m_cachedData.isNull()) {
        return;
    }
    
    QVariantMap data = m_cachedData.toMap();
    if (data["type"].toString() == "FeatureCollection") {
        QVariantList features = data["features"].toList();
        m_properties["featureCount"] = features.size();
        
        // Extract data schema from first feature
        if (!features.isEmpty()) {
            QVariantMap firstFeature = features[0].toMap();
            QVariantMap props = firstFeature["properties"].toMap();
            QStringList fields = props.keys();
            m_properties["fields"] = fields;
        }
    }
}