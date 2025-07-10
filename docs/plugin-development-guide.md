# Plugin Development Guide

## Introduction

This guide covers how to develop plugins for GeoWorld, including data providers, visualization components, and UI widgets. GeoWorld's plugin architecture allows for extensive customization and functionality extension.

## Plugin Types

### 1. Data Provider Plugins
Provide geospatial data from various sources (files, databases, web services, sensors).

### 2. Visualization Plugins
Display and interact with geospatial data (maps, charts, 3D viewers).

### 3. Tool Plugins
Provide analysis and processing capabilities (measurement, spatial analysis, export).

### 4. UI Plugins
Provide user interface components (data browsers, property panels, tool palettes).

## Development Environment Setup

### Prerequisites
```bash
# Required tools
- Qt6 (6.9.0 or later)
- CMake (3.25 or later)
- C++20 compatible compiler
- Git

# GeoWorld development environment
nix develop  # If using Nix
# or set up Qt6 development environment manually
```

### Project Structure
```
geoworld/
├── src/                    # Core application
│   ├── IPlugin.h          # Base plugin interface
│   ├── IDataProvider.h    # Data provider interfaces
│   └── ...
├── plugins/               # Plugin implementations
│   ├── mapview/          # Map visualization plugin
│   ├── databrowser/      # Data browser UI plugin
│   └── yourplugin/       # Your custom plugin
└── docs/                 # Documentation
```

## Creating a Basic Plugin

### 1. Plugin Directory Structure
```
plugins/yourplugin/
├── CMakeLists.txt         # Build configuration
├── YourPlugin.h           # Plugin header
├── YourPlugin.cpp         # Plugin implementation
├── YourWidget.h           # Widget header (if UI plugin)
├── YourWidget.cpp         # Widget implementation
└── yourplugin.json        # Plugin metadata
```

### 2. Plugin Metadata (yourplugin.json)
```json
{
    "name": "Your Plugin",
    "version": "1.0.0",
    "description": "Description of your plugin functionality",
    "author": "Your Name",
    "category": "tools",
    "capabilities": ["your-capability", "another-capability"],
    "dependencies": ["QtCore", "QtWidgets"],
    "provides": {
        "services": ["your-service"],
        "widgets": ["YourWidget"]
    }
}
```

### 3. Basic Plugin Implementation

#### Plugin Header (YourPlugin.h)
```cpp
#pragma once

#include "IPlugin.h"
#include <QObject>
#include <QIcon>
#include <QtPlugin>

class YourPlugin : public QObject, public IPlugin
{
    Q_OBJECT
    Q_INTERFACES(IPlugin)
    Q_PLUGIN_METADATA(IID "com.geoworld.IPlugin/1.0" FILE "yourplugin.json")

public:
    YourPlugin(QObject* parent = nullptr);
    ~YourPlugin();

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
};
```

#### Plugin Implementation (YourPlugin.cpp)
```cpp
#include "YourPlugin.h"
#include "YourWidget.h"
#include <QDebug>

YourPlugin::YourPlugin(QObject* parent)
    : QObject(parent)
    , m_initialized(false)
{
}

YourPlugin::~YourPlugin()
{
    shutdown();
}

QString YourPlugin::name() const
{
    return "Your Plugin";
}

QString YourPlugin::version() const
{
    return "1.0.0";
}

QString YourPlugin::description() const
{
    return "Description of your plugin functionality";
}

QIcon YourPlugin::icon() const
{
    return QIcon(":/icons/yourplugin.png");
}

bool YourPlugin::initialize()
{
    if (m_initialized) {
        return true;
    }
    
    qDebug() << "Initializing Your Plugin";
    
    // Perform initialization tasks
    // - Set up resources
    // - Initialize services
    // - Connect to external systems
    
    m_initialized = true;
    return true;
}

void YourPlugin::shutdown()
{
    if (!m_initialized) {
        return;
    }
    
    qDebug() << "Shutting down Your Plugin";
    
    // Perform cleanup tasks
    // - Release resources
    // - Disconnect from external systems
    // - Save state
    
    m_initialized = false;
}

QWidget* YourPlugin::createWidget(QWidget* parent)
{
    if (!m_initialized) {
        qWarning() << "Your Plugin not initialized";
        return nullptr;
    }
    
    return new YourWidget(parent);
}

QStringList YourPlugin::capabilities() const
{
    return QStringList() << "your-capability" << "another-capability";
}
```

### 4. CMakeLists.txt Configuration
```cmake
cmake_minimum_required(VERSION 3.25)
project(YourPlugin VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Plugin sources
set(PLUGIN_SOURCES
    YourPlugin.cpp
    YourWidget.cpp
)

set(PLUGIN_HEADERS
    YourPlugin.h
    YourWidget.h
)

# Create plugin library
add_library(yourplugin SHARED ${PLUGIN_SOURCES} ${PLUGIN_HEADERS})

# Set up Qt
qt_standard_project_setup()

# Link Qt libraries
target_link_libraries(yourplugin PRIVATE
    Qt6::Core
    Qt6::Widgets
)

# Include directories
target_include_directories(yourplugin PRIVATE
    ../../src  # For core interfaces
)

# Plugin properties
set_target_properties(yourplugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    PREFIX ""  # Remove lib prefix on Linux
)

# Install the plugin
install(TARGETS yourplugin
    LIBRARY DESTINATION lib/geoworld/plugins
    RUNTIME DESTINATION lib/geoworld/plugins
)
```

## Developing Data Provider Plugins

### 1. Data Provider Interface Implementation

#### Provider Header
```cpp
#pragma once

#include "IDataProvider.h"
#include <QObject>
#include <QNetworkAccessManager>

class YourDataProvider : public QObject, public IDataProvider
{
    Q_OBJECT

public:
    YourDataProvider(QObject* parent = nullptr);
    ~YourDataProvider();

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

private slots:
    void onDataReceived();
    void onNetworkError();

private:
    void loadConfiguration();
    void setupNetworkManager();
    IDataLayer* createLayerFromData(const QVariantMap& data);
    
    QMap<QString, IDataLayer*> m_layers;
    QNetworkAccessManager* m_networkManager;
    QString m_configFile;
    bool m_initialized;
};
```

#### Layer Implementation Example
```cpp
class YourDataLayer : public IDataLayer
{
public:
    YourDataLayer(const QString& id, const QString& name, QObject* parent = nullptr);
    
    // IDataLayer interface
    QString id() const override { return m_id; }
    QString name() const override { return m_name; }
    QString type() const override { return m_type; }
    QString description() const override { return m_description; }
    QIcon icon() const override { return m_icon; }
    
    bool isVisible() const override { return m_visible; }
    void setVisible(bool visible) override;
    double opacity() const override { return m_opacity; }
    void setOpacity(double opacity) override;
    
    QVariantMap properties() const override { return m_properties; }
    QVariantMap style() const override { return m_style; }
    void setStyle(const QVariantMap& style) override;
    
    QVariantMap boundingBox() const override { return m_boundingBox; }
    QVariant data() const override;
    QDateTime lastUpdated() const override { return m_lastUpdated; }

private:
    void calculateBoundingBox();
    void loadData();
    
    QString m_id;
    QString m_name;
    QString m_type;
    QString m_description;
    QIcon m_icon;
    
    bool m_visible;
    double m_opacity;
    
    QVariantMap m_properties;
    QVariantMap m_style;
    QVariantMap m_boundingBox;
    
    mutable QVariant m_cachedData;
    mutable bool m_dataLoaded;
    QDateTime m_lastUpdated;
};
```

### 2. Implementing Specific Provider Types

#### File Provider Example
```cpp
class FileDataProvider : public QObject, public IDataProvider
{
public:
    bool importData(const QString& filePath, const QVariantMap& options = QVariantMap()) override {
        QFileInfo fileInfo(filePath);
        QString extension = fileInfo.suffix().toLower();
        
        if (extension == "json" || extension == "geojson") {
            return importGeoJSON(filePath);
        } else if (extension == "shp") {
            return importShapefile(filePath);
        } else if (extension == "kml") {
            return importKML(filePath);
        }
        
        qWarning() << "Unsupported file format:" << extension;
        return false;
    }

private:
    bool importGeoJSON(const QString& filePath) {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "Cannot open file:" << filePath;
            return false;
        }
        
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (doc.isNull()) {
            qWarning() << "Invalid JSON in file:" << filePath;
            return false;
        }
        
        // Parse GeoJSON and create layer
        QString layerId = QUuid::createUuid().toString();
        QString layerName = QFileInfo(filePath).baseName();
        
        auto layer = new GeoJSONLayer(layerId, layerName, doc.object());
        m_layers[layerId] = layer;
        
        emit layerAdded(layerId);
        return true;
    }
};
```

#### Web Service Provider Example
```cpp
class WebServiceDataProvider : public QObject, public IDataProvider
{
public:
    bool initialize() override {
        m_networkManager = new QNetworkAccessManager(this);
        
        // Discover service capabilities
        QString capabilitiesUrl = m_serviceUrl + "?service=WFS&request=GetCapabilities";
        QNetworkRequest request(capabilitiesUrl);
        QNetworkReply* reply = m_networkManager->get(request);
        
        connect(reply, &QNetworkReply::finished, this, &WebServiceDataProvider::onCapabilitiesReceived);
        
        return true;
    }

private slots:
    void onCapabilitiesReceived() {
        QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
        if (!reply) return;
        
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            parseCapabilities(data);
        } else {
            qWarning() << "Failed to get capabilities:" << reply->errorString();
        }
        
        reply->deleteLater();
    }
    
    void parseCapabilities(const QByteArray& xml) {
        // Parse WFS capabilities XML
        // Create layers for each available feature type
        QXmlStreamReader reader(xml);
        
        while (!reader.atEnd()) {
            reader.readNext();
            if (reader.isStartElement() && reader.name() == "FeatureType") {
                QString typeName = reader.attributes().value("name").toString();
                createRemoteLayer(typeName);
            }
        }
    }
};
```

## Developing UI Plugins

### 1. Widget-Based UI Plugin

#### Widget Header
```cpp
#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QPushButton>
#include <QLabel>

class YourWidget : public QWidget
{
    Q_OBJECT

public:
    explicit YourWidget(QWidget *parent = nullptr);
    ~YourWidget();

public slots:
    void refreshData();
    void processSelectedItem();

signals:
    void itemSelected(const QString& itemId);
    void actionRequested(const QString& action, const QVariantMap& parameters);

private slots:
    void onItemSelectionChanged();
    void onActionButtonClicked();

private:
    void setupUI();
    void populateData();
    void updateStatus(const QString& message);
    
    QVBoxLayout* m_layout;
    QTreeWidget* m_treeWidget;
    QPushButton* m_actionButton;
    QPushButton* m_refreshButton;
    QLabel* m_statusLabel;
};
```

#### Widget Implementation
```cpp
#include "YourWidget.h"
#include <QHeaderView>
#include <QDebug>

YourWidget::YourWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    populateData();
}

void YourWidget::setupUI()
{
    setMinimumSize(300, 400);
    
    m_layout = new QVBoxLayout(this);
    
    // Tree widget for data display
    m_treeWidget = new QTreeWidget();
    m_treeWidget->setHeaderLabels(QStringList() << "Name" << "Type" << "Status");
    m_treeWidget->setAlternatingRowColors(true);
    m_treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    
    // Action buttons
    m_refreshButton = new QPushButton("Refresh");
    m_actionButton = new QPushButton("Process");
    m_actionButton->setEnabled(false);
    
    // Status label
    m_statusLabel = new QLabel("Ready");
    m_statusLabel->setStyleSheet("QLabel { color: gray; font-style: italic; }");
    
    // Layout
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(m_refreshButton);
    buttonLayout->addWidget(m_actionButton);
    buttonLayout->addStretch();
    
    m_layout->addWidget(m_treeWidget);
    m_layout->addLayout(buttonLayout);
    m_layout->addWidget(m_statusLabel);
    
    // Connections
    connect(m_treeWidget, &QTreeWidget::itemSelectionChanged,
            this, &YourWidget::onItemSelectionChanged);
    connect(m_refreshButton, &QPushButton::clicked,
            this, &YourWidget::refreshData);
    connect(m_actionButton, &QPushButton::clicked,
            this, &YourWidget::onActionButtonClicked);
}

void YourWidget::populateData()
{
    m_treeWidget->clear();
    
    // Add sample data
    for (int i = 0; i < 5; ++i) {
        QTreeWidgetItem* item = new QTreeWidgetItem();
        item->setText(0, QString("Item %1").arg(i + 1));
        item->setText(1, "Type A");
        item->setText(2, "Active");
        item->setData(0, Qt::UserRole, QString("item_%1").arg(i));
        m_treeWidget->addTopLevelItem(item);
    }
    
    updateStatus(QString("Loaded %1 items").arg(m_treeWidget->topLevelItemCount()));
}

void YourWidget::onItemSelectionChanged()
{
    QTreeWidgetItem* item = m_treeWidget->currentItem();
    bool hasSelection = (item != nullptr);
    
    m_actionButton->setEnabled(hasSelection);
    
    if (hasSelection) {
        QString itemId = item->data(0, Qt::UserRole).toString();
        emit itemSelected(itemId);
    }
}

void YourWidget::onActionButtonClicked()
{
    QTreeWidgetItem* item = m_treeWidget->currentItem();
    if (item) {
        QString itemId = item->data(0, Qt::UserRole).toString();
        QVariantMap parameters;
        parameters["itemId"] = itemId;
        parameters["action"] = "process";
        
        emit actionRequested("process_item", parameters);
        updateStatus(QString("Processing item: %1").arg(item->text(0)));
    }
}

void YourWidget::refreshData()
{
    updateStatus("Refreshing data...");
    populateData();
}

void YourWidget::updateStatus(const QString& message)
{
    m_statusLabel->setText(message);
}
```

## Service Communication

### 1. Using Data Provider Manager
```cpp
// In your plugin initialization
bool YourPlugin::initialize()
{
    // Get data provider manager from core app
    DataProviderManager* dataManager = getService<DataProviderManager>();
    if (dataManager) {
        // Connect to data manager signals
        connect(dataManager, &DataProviderManager::layerAdded,
                this, &YourPlugin::onLayerAdded);
        
        // Register your data provider
        YourDataProvider* provider = new YourDataProvider(this);
        dataManager->registerProvider(provider);
        
        return true;
    }
    
    qWarning() << "Data Provider Manager not available";
    return false;
}
```

### 2. Inter-Plugin Communication
```cpp
// Plugin A provides a service
class ServiceProvider : public QObject
{
    Q_OBJECT
    
public:
    Q_INVOKABLE void performAction(const QString& action, const QVariantMap& params);
    
signals:
    void actionCompleted(const QString& action, const QVariant& result);
};

// Plugin B uses the service
void ConsumerPlugin::useService()
{
    QObject* service = getService("ServiceProvider");
    if (service) {
        // Call service method
        QMetaObject::invokeMethod(service, "performAction",
                                 Q_ARG(QString, "myAction"),
                                 Q_ARG(QVariantMap, parameters));
        
        // Connect to service signals
        connect(service, SIGNAL(actionCompleted(QString, QVariant)),
                this, SLOT(onActionCompleted(QString, QVariant)));
    }
}
```

## Building and Deployment

### 1. Build Process
```bash
# Configure with plugin support
cmake -S . -B build -DBUILD_PLUGINS=ON

# Build everything (core app + plugins)
cmake --build build --parallel

# Build specific plugin
cmake --build build --target yourplugin
```

### 2. Plugin Installation
```bash
# Install to system location
cmake --install build

# Or copy manually to plugin directory
cp build/plugins/yourplugin.so ~/.local/lib/geoworld/plugins/
```

### 3. Plugin Distribution
```cmake
# Create plugin package
set(CPACK_GENERATOR "TGZ")
set(CPACK_PACKAGE_NAME "geoworld-yourplugin")
set(CPACK_PACKAGE_VERSION "1.0.0")
include(CPack)
```

## Testing Plugins

### 1. Unit Testing
```cpp
#include <QtTest/QtTest>
#include "YourPlugin.h"

class YourPluginTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void testPluginInitialization();
    void testWidgetCreation();
    void testPluginCapabilities();
    void cleanupTestCase();

private:
    YourPlugin* m_plugin;
};

void YourPluginTest::initTestCase()
{
    m_plugin = new YourPlugin();
}

void YourPluginTest::testPluginInitialization()
{
    QVERIFY(m_plugin->initialize());
    QCOMPARE(m_plugin->name(), QString("Your Plugin"));
    QCOMPARE(m_plugin->version(), QString("1.0.0"));
}

void YourPluginTest::testWidgetCreation()
{
    QWidget* widget = m_plugin->createWidget();
    QVERIFY(widget != nullptr);
    QVERIFY(widget->isWidgetType());
    delete widget;
}

void YourPluginTest::cleanupTestCase()
{
    m_plugin->shutdown();
    delete m_plugin;
}

QTEST_MAIN(YourPluginTest)
#include "YourPluginTest.moc"
```

### 2. Integration Testing
```cpp
class PluginIntegrationTest : public QObject
{
    Q_OBJECT

private slots:
    void testPluginLoading();
    void testServiceIntegration();
    void testDataProviderRegistration();

private:
    PluginManager* m_pluginManager;
    DataProviderManager* m_dataManager;
};

void PluginIntegrationTest::testPluginLoading()
{
    // Test that plugin can be loaded by plugin manager
    bool loaded = m_pluginManager->loadPlugin("yourplugin");
    QVERIFY(loaded);
    
    IPlugin* plugin = m_pluginManager->getPlugin("YourPlugin");
    QVERIFY(plugin != nullptr);
}
```

## Debugging and Troubleshooting

### 1. Common Issues

#### Plugin Not Loading
```cpp
// Check plugin metadata
qDebug() << "Plugin metadata file exists:" << QFile::exists("yourplugin.json");

// Check interface implementation
qDebug() << "Plugin implements IPlugin:" << (qobject_cast<IPlugin*>(plugin) != nullptr);

// Check library dependencies
qDebug() << "Qt version:" << QT_VERSION_STR;
```

#### Service Communication Issues
```cpp
// Verify service availability
QObject* service = getService("ServiceName");
if (!service) {
    qWarning() << "Service not available: ServiceName";
    return;
}

// Check signal/slot connections
bool connected = connect(service, SIGNAL(serviceSignal()), 
                        this, SLOT(serviceSlot()));
qDebug() << "Signal connection successful:" << connected;
```

### 2. Debug Output
```cpp
// Enable Qt logging categories
QLoggingCategory::setFilterRules("geoworld.plugins.debug=true");

// Use structured logging
qCDebug(pluginCategory) << "Plugin operation:" << operation << "parameters:" << parameters;
```

### 3. Performance Profiling
```cpp
// Measure operation performance
QElapsedTimer timer;
timer.start();

performOperation();

qDebug() << "Operation completed in" << timer.elapsed() << "ms";
```

## Best Practices

### 1. Resource Management
- Use smart pointers for automatic memory management
- Clean up resources in plugin shutdown
- Avoid memory leaks in widget creation

### 2. Error Handling
- Always check return values and handle errors gracefully
- Provide meaningful error messages to users
- Log errors for debugging purposes

### 3. User Experience
- Provide progress feedback for long operations
- Use appropriate icons and tooltips
- Follow Qt's UI guidelines and conventions

### 4. Performance
- Use lazy loading for expensive operations
- Cache frequently accessed data
- Minimize UI updates during data processing

### 5. Compatibility
- Test with different Qt versions
- Handle API changes gracefully
- Provide backward compatibility when possible

## Plugin Examples

The GeoWorld repository includes several example plugins:

- **Map View Plugin**: Complete map visualization implementation
- **Data Browser Plugin**: Data layer management UI
- **File Provider Plugin**: File-based data provider example

Study these examples to understand real-world plugin implementation patterns and best practices.