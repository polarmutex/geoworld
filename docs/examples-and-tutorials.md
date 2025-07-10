# Examples and Tutorials

## Tutorial 1: Creating a Simple File Data Provider

This tutorial walks through creating a basic data provider that can load GeoJSON files.

### Step 1: Project Setup

Create the plugin directory structure:
```bash
mkdir -p plugins/geojson-provider
cd plugins/geojson-provider
```

### Step 2: Plugin Metadata

Create `geojson-provider.json`:
```json
{
    "name": "GeoJSON Provider",
    "version": "1.0.0",
    "description": "Loads GeoJSON files as data layers",
    "author": "Tutorial",
    "category": "data-provider",
    "capabilities": ["data-provider", "import-export"],
    "dependencies": ["QtCore", "QtWidgets"],
    "provides": {
        "services": ["geojson-loader"]
    }
}
```

### Step 3: Data Layer Implementation

Create `GeoJSONLayer.h`:
```cpp
#pragma once

#include "IDataProvider.h"
#include <QObject>
#include <QJsonObject>
#include <QVariantMap>

class GeoJSONLayer : public IDataLayer
{
public:
    GeoJSONLayer(const QString& id, const QString& name, const QJsonObject& geoJson);
    
    // IDataLayer interface
    QString id() const override { return m_id; }
    QString name() const override { return m_name; }
    QString type() const override { return "vector"; }
    QString description() const override { return m_description; }
    QIcon icon() const override;
    
    bool isVisible() const override { return m_visible; }
    void setVisible(bool visible) override;
    double opacity() const override { return m_opacity; }
    void setOpacity(double opacity) override;
    
    QVariantMap properties() const override { return m_properties; }
    QVariantMap style() const override { return m_style; }
    void setStyle(const QVariantMap& style) override { m_style = style; }
    
    QVariantMap boundingBox() const override { return m_boundingBox; }
    QVariant data() const override { return m_geoJsonData; }
    QDateTime lastUpdated() const override { return m_lastUpdated; }

private:
    void calculateBoundingBox();
    void extractProperties();
    
    QString m_id;
    QString m_name;
    QString m_description;
    bool m_visible;
    double m_opacity;
    
    QVariantMap m_properties;
    QVariantMap m_style;
    QVariantMap m_boundingBox;
    QVariant m_geoJsonData;
    QDateTime m_lastUpdated;
};
```

Create `GeoJSONLayer.cpp`:
```cpp
#include "GeoJSONLayer.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QDateTime>

GeoJSONLayer::GeoJSONLayer(const QString& id, const QString& name, const QJsonObject& geoJson)
    : m_id(id)
    , m_name(name)
    , m_visible(true)
    , m_opacity(1.0)
    , m_lastUpdated(QDateTime::currentDateTime())
{
    m_geoJsonData = QVariant(geoJson);
    m_description = QString("GeoJSON layer with %1 features").arg(geoJson["features"].toArray().size());
    
    calculateBoundingBox();
    extractProperties();
    
    // Default style
    m_style["fillColor"] = "#3388ff";
    m_style["strokeColor"] = "#3388ff";
    m_style["strokeWidth"] = 2;
    m_style["fillOpacity"] = 0.2;
}

QIcon GeoJSONLayer::icon() const
{
    // Return appropriate icon based on geometry type
    QJsonObject geoJson = m_geoJsonData.toJsonObject();
    QJsonArray features = geoJson["features"].toArray();
    
    if (!features.isEmpty()) {
        QString geometryType = features[0].toObject()["geometry"].toObject()["type"].toString();
        if (geometryType == "Point" || geometryType == "MultiPoint") {
            return QIcon(":/icons/point-layer.png");
        } else if (geometryType == "LineString" || geometryType == "MultiLineString") {
            return QIcon(":/icons/line-layer.png");
        } else if (geometryType == "Polygon" || geometryType == "MultiPolygon") {
            return QIcon(":/icons/polygon-layer.png");
        }
    }
    
    return QIcon(":/icons/vector-layer.png");
}

void GeoJSONLayer::setVisible(bool visible)
{
    if (m_visible != visible) {
        m_visible = visible;
        // Emit change signal if this was a QObject
    }
}

void GeoJSONLayer::setOpacity(double opacity)
{
    if (m_opacity != opacity) {
        m_opacity = qBound(0.0, opacity, 1.0);
        // Emit change signal if this was a QObject
    }
}

void GeoJSONLayer::calculateBoundingBox()
{
    QJsonObject geoJson = m_geoJsonData.toJsonObject();
    QJsonArray features = geoJson["features"].toArray();
    
    if (features.isEmpty()) {
        return;
    }
    
    double minLon = 180.0, maxLon = -180.0;
    double minLat = 90.0, maxLat = -90.0;
    
    for (const QJsonValue& featureValue : features) {
        QJsonObject feature = featureValue.toObject();
        QJsonObject geometry = feature["geometry"].toObject();
        QJsonArray coordinates = geometry["coordinates"].toArray();
        
        // Simple bbox calculation for Point geometries
        if (geometry["type"].toString() == "Point") {
            double lon = coordinates[0].toDouble();
            double lat = coordinates[1].toDouble();
            
            minLon = qMin(minLon, lon);
            maxLon = qMax(maxLon, lon);
            minLat = qMin(minLat, lat);
            maxLat = qMax(maxLat, lat);
        }
        // Add support for other geometry types as needed
    }
    
    m_boundingBox["minLon"] = minLon;
    m_boundingBox["maxLon"] = maxLon;
    m_boundingBox["minLat"] = minLat;
    m_boundingBox["maxLat"] = maxLat;
}

void GeoJSONLayer::extractProperties()
{
    QJsonObject geoJson = m_geoJsonData.toJsonObject();
    QJsonArray features = geoJson["features"].toArray();
    
    m_properties["featureCount"] = features.size();
    m_properties["layerType"] = "GeoJSON";
    m_properties["coordinateSystem"] = "WGS84";
    
    // Extract unique property names
    QStringList propertyNames;
    for (const QJsonValue& featureValue : features) {
        QJsonObject feature = featureValue.toObject();
        QJsonObject properties = feature["properties"].toObject();
        
        for (auto it = properties.begin(); it != properties.end(); ++it) {
            if (!propertyNames.contains(it.key())) {
                propertyNames.append(it.key());
            }
        }
    }
    
    m_properties["attributeNames"] = propertyNames;
}
```

### Step 4: Data Provider Implementation

Create `GeoJSONProvider.h`:
```cpp
#pragma once

#include "IDataProvider.h"
#include <QObject>
#include <QMap>

class GeoJSONLayer;

class GeoJSONProvider : public QObject, public IDataProvider
{
    Q_OBJECT

public:
    GeoJSONProvider(QObject* parent = nullptr);
    ~GeoJSONProvider();

    // IDataProvider interface
    QString providerId() const override { return "geojson-provider"; }
    QString name() const override { return "GeoJSON Provider"; }
    QString description() const override { return "Loads GeoJSON files"; }
    QIcon icon() const override;
    QStringList supportedTypes() const override { return QStringList() << "geojson" << "json"; }
    
    bool canCreateLayers() const override { return false; }
    bool canImportData() const override { return true; }
    bool canExportData() const override { return true; }
    bool isRealTime() const override { return false; }
    
    QStringList layerIds() const override;
    IDataLayer* getLayer(const QString& layerId) const override;
    QList<IDataLayer*> getAllLayers() const override;
    
    bool createLayer(const QString& name, const QString& type, const QVariantMap& parameters = QVariantMap()) override;
    bool removeLayer(const QString& layerId) override;
    bool importData(const QString& filePath, const QVariantMap& options = QVariantMap()) override;
    bool exportLayer(const QString& layerId, const QString& filePath, const QVariantMap& options = QVariantMap()) override;
    
    bool initialize() override;
    void shutdown() override;

signals:
    void layerAdded(const QString& layerId) override;
    void layerRemoved(const QString& layerId) override;
    void layerChanged(const QString& layerId) override;
    void dataUpdated(const QString& layerId) override;

private:
    bool loadGeoJSONFile(const QString& filePath);
    
    QMap<QString, GeoJSONLayer*> m_layers;
    bool m_initialized;
};
```

Create `GeoJSONProvider.cpp`:
```cpp
#include "GeoJSONProvider.h"
#include "GeoJSONLayer.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>
#include <QUuid>
#include <QDebug>

GeoJSONProvider::GeoJSONProvider(QObject* parent)
    : QObject(parent)
    , m_initialized(false)
{
}

GeoJSONProvider::~GeoJSONProvider()
{
    shutdown();
}

QIcon GeoJSONProvider::icon() const
{
    return QIcon(":/icons/geojson-provider.png");
}

QStringList GeoJSONProvider::layerIds() const
{
    return m_layers.keys();
}

IDataLayer* GeoJSONProvider::getLayer(const QString& layerId) const
{
    return m_layers.value(layerId, nullptr);
}

QList<IDataLayer*> GeoJSONProvider::getAllLayers() const
{
    QList<IDataLayer*> layers;
    for (GeoJSONLayer* layer : m_layers.values()) {
        layers.append(layer);
    }
    return layers;
}

bool GeoJSONProvider::createLayer(const QString& name, const QString& type, const QVariantMap& parameters)
{
    Q_UNUSED(name)
    Q_UNUSED(type)
    Q_UNUSED(parameters)
    
    // This provider doesn't support creating empty layers
    return false;
}

bool GeoJSONProvider::removeLayer(const QString& layerId)
{
    if (m_layers.contains(layerId)) {
        GeoJSONLayer* layer = m_layers.take(layerId);
        delete layer;
        emit layerRemoved(layerId);
        return true;
    }
    return false;
}

bool GeoJSONProvider::importData(const QString& filePath, const QVariantMap& options)
{
    Q_UNUSED(options)
    return loadGeoJSONFile(filePath);
}

bool GeoJSONProvider::exportLayer(const QString& layerId, const QString& filePath, const QVariantMap& options)
{
    Q_UNUSED(options)
    
    GeoJSONLayer* layer = m_layers.value(layerId);
    if (!layer) {
        qWarning() << "Layer not found:" << layerId;
        return false;
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Cannot write to file:" << filePath;
        return false;
    }
    
    QJsonObject geoJson = layer->data().toJsonObject();
    QJsonDocument doc(geoJson);
    file.write(doc.toJson());
    
    return true;
}

bool GeoJSONProvider::initialize()
{
    qDebug() << "Initializing GeoJSON Provider";
    m_initialized = true;
    return true;
}

void GeoJSONProvider::shutdown()
{
    if (!m_initialized) return;
    
    qDebug() << "Shutting down GeoJSON Provider";
    
    // Clean up all layers
    for (GeoJSONLayer* layer : m_layers.values()) {
        delete layer;
    }
    m_layers.clear();
    
    m_initialized = false;
}

bool GeoJSONProvider::loadGeoJSONFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot read file:" << filePath;
        return false;
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << error.errorString();
        return false;
    }
    
    QJsonObject geoJson = doc.object();
    
    // Validate GeoJSON structure
    if (geoJson["type"].toString() != "FeatureCollection") {
        qWarning() << "Invalid GeoJSON: not a FeatureCollection";
        return false;
    }
    
    // Create layer
    QString layerId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QString layerName = QFileInfo(filePath).baseName();
    
    GeoJSONLayer* layer = new GeoJSONLayer(layerId, layerName, geoJson);
    m_layers[layerId] = layer;
    
    emit layerAdded(layerId);
    qDebug() << "Loaded GeoJSON layer:" << layerName << "with" << geoJson["features"].toArray().size() << "features";
    
    return true;
}
```

### Step 5: Plugin Implementation

Create `GeoJSONProviderPlugin.h`:
```cpp
#pragma once

#include "IPlugin.h"
#include <QObject>
#include <QtPlugin>

class GeoJSONProvider;
class DataProviderManager;

class GeoJSONProviderPlugin : public QObject, public IPlugin
{
    Q_OBJECT
    Q_INTERFACES(IPlugin)
    Q_PLUGIN_METADATA(IID "com.geoworld.IPlugin/1.0" FILE "geojson-provider.json")

public:
    GeoJSONProviderPlugin(QObject* parent = nullptr);
    ~GeoJSONProviderPlugin();

    // IPlugin interface
    QString name() const override;
    QString version() const override;
    QString description() const override;
    QIcon icon() const override;
    
    bool initialize() override;
    void shutdown() override;
    
    QWidget* createWidget(QWidget* parent = nullptr) override;
    
    QStringList capabilities() const override;

private:
    bool m_initialized;
    GeoJSONProvider* m_provider;
};
```

Create `GeoJSONProviderPlugin.cpp`:
```cpp
#include "GeoJSONProviderPlugin.h"
#include "GeoJSONProvider.h"
#include "DataProviderManager.h"
#include <QDebug>

GeoJSONProviderPlugin::GeoJSONProviderPlugin(QObject* parent)
    : QObject(parent)
    , m_initialized(false)
    , m_provider(nullptr)
{
}

GeoJSONProviderPlugin::~GeoJSONProviderPlugin()
{
    shutdown();
}

QString GeoJSONProviderPlugin::name() const
{
    return "GeoJSON Provider";
}

QString GeoJSONProviderPlugin::version() const
{
    return "1.0.0";
}

QString GeoJSONProviderPlugin::description() const
{
    return "Loads GeoJSON files as data layers";
}

QIcon GeoJSONProviderPlugin::icon() const
{
    return QIcon(":/icons/geojson.png");
}

bool GeoJSONProviderPlugin::initialize()
{
    if (m_initialized) {
        return true;
    }
    
    qDebug() << "Initializing GeoJSON Provider Plugin";
    
    // Create provider instance
    m_provider = new GeoJSONProvider(this);
    if (!m_provider->initialize()) {
        delete m_provider;
        m_provider = nullptr;
        return false;
    }
    
    // Get data provider manager and register our provider
    // Note: In real implementation, you'd get this from the core app
    // DataProviderManager* dataManager = getCoreService<DataProviderManager>();
    // if (dataManager) {
    //     dataManager->registerProvider(m_provider);
    // }
    
    m_initialized = true;
    return true;
}

void GeoJSONProviderPlugin::shutdown()
{
    if (!m_initialized) {
        return;
    }
    
    qDebug() << "Shutting down GeoJSON Provider Plugin";
    
    // Unregister provider
    // if (dataManager) {
    //     dataManager->unregisterProvider(m_provider->providerId());
    // }
    
    if (m_provider) {
        m_provider->shutdown();
        delete m_provider;
        m_provider = nullptr;
    }
    
    m_initialized = false;
}

QWidget* GeoJSONProviderPlugin::createWidget(QWidget* parent)
{
    Q_UNUSED(parent)
    // This plugin doesn't provide UI widgets
    return nullptr;
}

QStringList GeoJSONProviderPlugin::capabilities() const
{
    return QStringList() << "data-provider" << "import-export";
}
```

### Step 6: Build Configuration

Create `CMakeLists.txt`:
```cmake
cmake_minimum_required(VERSION 3.25)
project(GeoJSONProviderPlugin VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

set(PLUGIN_SOURCES
    GeoJSONProviderPlugin.cpp
    GeoJSONProvider.cpp
    GeoJSONLayer.cpp
)

set(PLUGIN_HEADERS
    GeoJSONProviderPlugin.h
    GeoJSONProvider.h
    GeoJSONLayer.h
)

add_library(geojson-provider SHARED ${PLUGIN_SOURCES} ${PLUGIN_HEADERS})

qt_standard_project_setup()

target_link_libraries(geojson-provider PRIVATE
    Qt6::Core
    Qt6::Widgets
)

target_include_directories(geojson-provider PRIVATE
    ../../src
)

set_target_properties(geojson-provider PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    PREFIX ""
)

install(TARGETS geojson-provider
    LIBRARY DESTINATION lib/geoworld/plugins
    RUNTIME DESTINATION lib/geoworld/plugins
)
```

### Step 7: Testing

Create a test GeoJSON file `test-data.geojson`:
```json
{
    "type": "FeatureCollection",
    "features": [
        {
            "type": "Feature",
            "geometry": {
                "type": "Point",
                "coordinates": [-122.4194, 37.7749]
            },
            "properties": {
                "name": "San Francisco",
                "population": 883305
            }
        },
        {
            "type": "Feature",
            "geometry": {
                "type": "Point",
                "coordinates": [-74.0059, 40.7128]
            },
            "properties": {
                "name": "New York",
                "population": 8398748
            }
        }
    ]
}
```

Build and test:
```bash
# Build the plugin
cmake --build build --target geojson-provider

# The plugin will be available in build/plugins/geojson-provider.so
# Copy test data to accessible location
cp test-data.geojson build/
```

---

## Tutorial 2: Creating a Real-time Data Provider

This tutorial shows how to create a provider that handles streaming GPS data.

### GPS Data Provider Structure

```cpp
class GPSDataProvider : public QObject, public IDataProvider
{
    Q_OBJECT

public:
    GPSDataProvider(QObject* parent = nullptr);
    
    // IDataProvider interface
    bool isRealTime() const override { return true; }
    bool canCreateLayers() const override { return true; }
    
    bool initialize() override;
    bool createLayer(const QString& name, const QString& type, const QVariantMap& parameters) override;

private slots:
    void onNMEADataReceived();
    void onPositionUpdate(const QGeoPositionInfo& position);
    void updateTrackLayer();

private:
    void parseNMEAsentence(const QString& sentence);
    void addPositionToTrack(const QGeoCoordinate& coordinate);
    
    QGeoPositionInfoSource* m_positionSource;
    QTimer* m_updateTimer;
    
    // Track layer data
    QString m_trackLayerId;
    QList<QGeoCoordinate> m_trackPoints;
    QDateTime m_trackStartTime;
    
    // Current position layer
    QString m_currentPositionLayerId;
    QGeoCoordinate m_currentPosition;
};
```

### Real-time Layer Implementation

```cpp
class GPSTrackLayer : public QObject, public IDataLayer
{
    Q_OBJECT

public:
    GPSTrackLayer(const QString& id, QObject* parent = nullptr);
    
    // IDataLayer interface
    QString type() const override { return "realtime"; }
    QVariant data() const override;
    QDateTime lastUpdated() const override { return m_lastUpdate; }
    
    // GPS-specific methods
    void addTrackPoint(const QGeoCoordinate& coordinate, const QDateTime& timestamp);
    void clearTrack();
    QList<QGeoCoordinate> getTrackPoints() const { return m_trackPoints; }
    
signals:
    void trackUpdated();

private:
    void updateBoundingBox();
    
    QList<QGeoCoordinate> m_trackPoints;
    QList<QDateTime> m_timestamps;
    QDateTime m_lastUpdate;
    int m_maxPoints;
};

void GPSTrackLayer::addTrackPoint(const QGeoCoordinate& coordinate, const QDateTime& timestamp)
{
    m_trackPoints.append(coordinate);
    m_timestamps.append(timestamp);
    m_lastUpdate = timestamp;
    
    // Limit track length
    while (m_trackPoints.size() > m_maxPoints) {
        m_trackPoints.removeFirst();
        m_timestamps.removeFirst();
    }
    
    updateBoundingBox();
    emit trackUpdated();
}

QVariant GPSTrackLayer::data() const
{
    // Convert track to GeoJSON LineString
    QJsonObject geoJson;
    geoJson["type"] = "FeatureCollection";
    
    QJsonArray features;
    QJsonObject feature;
    feature["type"] = "Feature";
    
    QJsonObject geometry;
    geometry["type"] = "LineString";
    
    QJsonArray coordinates;
    for (const QGeoCoordinate& point : m_trackPoints) {
        QJsonArray coord;
        coord.append(point.longitude());
        coord.append(point.latitude());
        coordinates.append(coord);
    }
    
    geometry["coordinates"] = coordinates;
    feature["geometry"] = geometry;
    
    QJsonObject properties;
    properties["pointCount"] = m_trackPoints.size();
    properties["startTime"] = m_timestamps.isEmpty() ? "" : m_timestamps.first().toString(Qt::ISODate);
    properties["endTime"] = m_timestamps.isEmpty() ? "" : m_timestamps.last().toString(Qt::ISODate);
    feature["properties"] = properties;
    
    features.append(feature);
    geoJson["features"] = features;
    
    return QVariant(geoJson);
}
```

---

## Tutorial 3: Creating a Custom Analysis Tool Plugin

This tutorial demonstrates creating a measurement tool plugin.

### Measurement Tool Widget

```cpp
class MeasurementToolWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MeasurementToolWidget(QWidget *parent = nullptr);

public slots:
    void startDistanceMeasurement();
    void startAreaMeasurement();
    void clearMeasurements();

signals:
    void measurementModeChanged(const QString& mode);
    void measurementCompleted(const QString& type, double value, const QString& units);

private slots:
    void onMapClicked(double latitude, double longitude);
    void onMeasurementFinished();

private:
    void setupUI();
    void addMeasurementPoint(const QPointF& point);
    double calculateDistance(const QList<QPointF>& points);
    double calculateArea(const QList<QPointF>& points);
    QString formatDistance(double meters);
    QString formatArea(double squareMeters);
    
    QPushButton* m_distanceButton;
    QPushButton* m_areaButton;
    QPushButton* m_clearButton;
    QLabel* m_resultLabel;
    QLabel* m_instructionLabel;
    
    QString m_currentMode; // "distance", "area", or "none"
    QList<QPointF> m_measurementPoints;
    QStringList m_measurementResults;
};

void MeasurementToolWidget::setupUI()
{
    setWindowTitle("Measurement Tools");
    setMinimumSize(200, 150);
    
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    // Tool buttons
    m_distanceButton = new QPushButton("Measure Distance");
    m_areaButton = new QPushButton("Measure Area");
    m_clearButton = new QPushButton("Clear All");
    
    connect(m_distanceButton, &QPushButton::clicked, this, &MeasurementToolWidget::startDistanceMeasurement);
    connect(m_areaButton, &QPushButton::clicked, this, &MeasurementToolWidget::startAreaMeasurement);
    connect(m_clearButton, &QPushButton::clicked, this, &MeasurementToolWidget::clearMeasurements);
    
    // Status labels
    m_instructionLabel = new QLabel("Select a measurement tool");
    m_instructionLabel->setWordWrap(true);
    
    m_resultLabel = new QLabel("No measurements");
    m_resultLabel->setStyleSheet("QLabel { font-weight: bold; }");
    
    // Layout
    layout->addWidget(m_distanceButton);
    layout->addWidget(m_areaButton);
    layout->addWidget(m_clearButton);
    layout->addWidget(m_instructionLabel);
    layout->addWidget(m_resultLabel);
    layout->addStretch();
}

void MeasurementToolWidget::startDistanceMeasurement()
{
    m_currentMode = "distance";
    m_measurementPoints.clear();
    m_instructionLabel->setText("Click on map to add points. Double-click to finish.");
    emit measurementModeChanged("distance");
}

void MeasurementToolWidget::onMapClicked(double latitude, double longitude)
{
    if (m_currentMode == "none") return;
    
    QPointF point(longitude, latitude);
    addMeasurementPoint(point);
    
    if (m_currentMode == "distance" && m_measurementPoints.size() >= 2) {
        double distance = calculateDistance(m_measurementPoints);
        m_resultLabel->setText(QString("Distance: %1").arg(formatDistance(distance)));
    } else if (m_currentMode == "area" && m_measurementPoints.size() >= 3) {
        double area = calculateArea(m_measurementPoints);
        m_resultLabel->setText(QString("Area: %1").arg(formatArea(area)));
    }
    
    m_instructionLabel->setText(QString("Points: %1. %2")
                               .arg(m_measurementPoints.size())
                               .arg(m_currentMode == "distance" ? "Double-click to finish." : "Click to add more points."));
}

double MeasurementToolWidget::calculateDistance(const QList<QPointF>& points)
{
    if (points.size() < 2) return 0.0;
    
    double totalDistance = 0.0;
    for (int i = 1; i < points.size(); ++i) {
        QGeoCoordinate coord1(points[i-1].y(), points[i-1].x());
        QGeoCoordinate coord2(points[i].y(), points[i].x());
        totalDistance += coord1.distanceTo(coord2);
    }
    
    return totalDistance;
}

QString MeasurementToolWidget::formatDistance(double meters)
{
    if (meters < 1000) {
        return QString("%1 m").arg(meters, 0, 'f', 1);
    } else {
        return QString("%1 km").arg(meters / 1000.0, 0, 'f', 2);
    }
}
```

---

## Example: Complete Plugin Integration

This example shows how all components work together in the main application.

### Main Application Integration

```cpp
// MainWindow.cpp - Setting up the plugin ecosystem

void MainWindow::setupPlugins()
{
    // Create core services
    m_pluginManager = new PluginManager(this);
    m_dataProviderManager = new DataProviderManager(this);
    
    // Connect plugin manager signals
    connect(m_pluginManager, &PluginManager::pluginLoaded,
            this, &MainWindow::onPluginLoaded);
    
    // Load plugins
    m_pluginManager->loadPlugins();
    
    // Set up data browser
    setupDataBrowser();
    
    // Set up map view
    setupMapView();
}

void MainWindow::setupDataBrowser()
{
    // Get data browser plugin
    IPlugin* dataBrowserPlugin = m_pluginManager->getPlugin("Data Browser");
    if (dataBrowserPlugin) {
        // Create data browser widget
        DataBrowserWidget* dataBrowser = qobject_cast<DataBrowserWidget*>(
            dataBrowserPlugin->createWidget(this));
        
        if (dataBrowser) {
            // Connect data browser signals
            connect(dataBrowser, &DataBrowserWidget::layerSelectionChanged,
                    this, &MainWindow::onLayerSelected);
            connect(dataBrowser, &DataBrowserWidget::zoomToLayerRequested,
                    this, &MainWindow::onZoomToLayer);
            
            // Add to docking system
            ads::CDockWidget* dockWidget = new ads::CDockWidget("Data Browser");
            dockWidget->setWidget(dataBrowser);
            m_dockManager->addDockWidget(ads::LeftDockWidgetArea, dockWidget);
        }
    }
}

void MainWindow::onPluginLoaded(const QString& name)
{
    qDebug() << "Plugin loaded:" << name;
    
    // If it's a data provider plugin, register the provider
    IPlugin* plugin = m_pluginManager->getPlugin(name);
    if (plugin && plugin->capabilities().contains("data-provider")) {
        // Plugin should register its provider with DataProviderManager
        // This is typically done in the plugin's initialize() method
    }
    
    // Update UI to reflect new capabilities
    updatePluginMenus();
}

void MainWindow::updatePluginMenus()
{
    // Clear existing plugin menus
    if (m_pluginMenu) {
        m_pluginMenu->clear();
    } else {
        m_pluginMenu = menuBar()->addMenu("&Plugins");
    }
    
    // Add menu items for each loaded plugin
    QStringList pluginNames = m_pluginManager->availablePlugins();
    for (const QString& name : pluginNames) {
        IPlugin* plugin = m_pluginManager->getPlugin(name);
        if (plugin) {
            QAction* action = m_pluginMenu->addAction(plugin->icon(), name);
            connect(action, &QAction::triggered, [this, name]() {
                showPluginWidget(name);
            });
        }
    }
}

void MainWindow::showPluginWidget(const QString& pluginName)
{
    IPlugin* plugin = m_pluginManager->getPlugin(pluginName);
    if (plugin) {
        QWidget* widget = plugin->createWidget(this);
        if (widget) {
            // Add to docking system or show as dialog
            ads::CDockWidget* dockWidget = new ads::CDockWidget(pluginName);
            dockWidget->setWidget(widget);
            m_dockManager->addDockWidget(ads::RightDockWidgetArea, dockWidget);
        }
    }
}
```

### Data Flow Example

```cpp
// Example: User imports a GeoJSON file

// 1. User clicks "Import" in Data Browser
void DataBrowserWidget::importData()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Import Data");
    if (!filePath.isEmpty()) {
        // 2. Data Browser asks DataProviderManager to import
        bool success = m_dataManager->importData(filePath);
        if (success) {
            // 3. DataProviderManager found suitable provider and imported data
            refreshProviders(); // Update UI
        }
    }
}

// 4. GeoJSON Provider handles the import
bool GeoJSONProvider::importData(const QString& filePath, const QVariantMap& options)
{
    // Load and parse GeoJSON
    bool success = loadGeoJSONFile(filePath);
    if (success) {
        // 5. Provider emits signal that new layer was added
        emit layerAdded(newLayerId);
    }
    return success;
}

// 6. DataProviderManager forwards the signal
void DataProviderManager::onProviderLayerAdded(const QString& layerId)
{
    // Forward to all interested parties
    emit layerAdded(providerId, layerId);
    emit layersChanged();
}

// 7. Data Browser updates its UI
void DataBrowserWidget::onLayerAdded(const QString& providerId, const QString& layerId)
{
    // Add new layer to tree widget
    refreshProviders();
}

// 8. Map Plugin automatically renders the new layer
void MapWidget::onLayersChanged()
{
    // Get all visible layers and render them
    QList<IDataLayer*> visibleLayers = m_dataManager->getVisibleLayers();
    updateMapDisplay(visibleLayers);
}
```

---

## Performance Optimization Examples

### Lazy Loading Pattern

```cpp
class OptimizedDataLayer : public IDataLayer
{
private:
    mutable QVariant m_cachedData;
    mutable bool m_dataLoaded;
    QString m_dataSource;

public:
    QVariant data() const override {
        if (!m_dataLoaded) {
            // Load data only when first accessed
            m_cachedData = loadDataFromSource(m_dataSource);
            m_dataLoaded = true;
        }
        return m_cachedData;
    }
    
    void invalidateCache() {
        m_dataLoaded = false;
        m_cachedData = QVariant();
    }
};
```

### Spatial Indexing

```cpp
class SpatialIndexedLayer : public IDataLayer
{
private:
    struct SpatialIndex {
        QMap<QString, QRectF> featureBounds;
        QMultiMap<int, QString> gridIndex; // Grid cell -> feature IDs
        
        void buildIndex(const QList<Feature>& features) {
            const int gridSize = 100; // 100x100 grid
            
            for (const Feature& feature : features) {
                QRectF bounds = calculateBounds(feature.geometry);
                featureBounds[feature.id] = bounds;
                
                // Add to grid cells
                int minX = static_cast<int>(bounds.left() * gridSize);
                int maxX = static_cast<int>(bounds.right() * gridSize);
                int minY = static_cast<int>(bounds.top() * gridSize);
                int maxY = static_cast<int>(bounds.bottom() * gridSize);
                
                for (int x = minX; x <= maxX; ++x) {
                    for (int y = minY; y <= maxY; ++y) {
                        int cellId = x * 10000 + y; // Combine x,y into single key
                        gridIndex.insert(cellId, feature.id);
                    }
                }
            }
        }
        
        QStringList queryBounds(const QRectF& queryRect) const {
            QSet<QString> candidates;
            const int gridSize = 100;
            
            int minX = static_cast<int>(queryRect.left() * gridSize);
            int maxX = static_cast<int>(queryRect.right() * gridSize);
            int minY = static_cast<int>(queryRect.top() * gridSize);
            int maxY = static_cast<int>(queryRect.bottom() * gridSize);
            
            for (int x = minX; x <= maxX; ++x) {
                for (int y = minY; y <= maxY; ++y) {
                    int cellId = x * 10000 + y;
                    QList<QString> cellFeatures = gridIndex.values(cellId);
                    for (const QString& featureId : cellFeatures) {
                        candidates.insert(featureId);
                    }
                }
            }
            
            return candidates.values();
        }
    };
    
    SpatialIndex m_spatialIndex;

public:
    QList<Feature> getFeaturesInBounds(const QRectF& bounds) const {
        // Use spatial index for fast initial filtering
        QStringList candidates = m_spatialIndex.queryBounds(bounds);
        
        QList<Feature> results;
        for (const QString& featureId : candidates) {
            Feature feature = getFeature(featureId);
            if (bounds.intersects(m_spatialIndex.featureBounds[featureId])) {
                results.append(feature);
            }
        }
        
        return results;
    }
};
```

This comprehensive documentation and examples provide a solid foundation for understanding and extending the GeoWorld plugin architecture!