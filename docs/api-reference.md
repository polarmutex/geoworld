# API Reference

## Core Interfaces

### IPlugin

Base interface that all plugins must implement.

```cpp
class IPlugin
{
public:
    virtual ~IPlugin() = default;
    
    // Plugin metadata
    virtual QString name() const = 0;
    virtual QString version() const = 0;
    virtual QString description() const = 0;
    virtual QIcon icon() const = 0;
    
    // Plugin lifecycle
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    
    // Widget creation
    virtual QWidget* createWidget(QWidget* parent = nullptr) = 0;
    
    // Plugin capabilities
    virtual QStringList capabilities() const = 0;
};
```

#### Methods

##### `QString name() const`
Returns the human-readable name of the plugin.

**Returns:** Plugin display name

##### `QString version() const`
Returns the plugin version string.

**Returns:** Version in semantic versioning format (e.g., "1.2.3")

##### `QString description() const`
Returns a detailed description of the plugin's functionality.

**Returns:** Plugin description text

##### `QIcon icon() const`
Returns an icon representing the plugin.

**Returns:** QIcon object for display in UI

##### `bool initialize()`
Called when the plugin is loaded. Perform initialization tasks here.

**Returns:** `true` if initialization successful, `false` otherwise

##### `void shutdown()`
Called when the plugin is unloaded. Cleanup resources here.

##### `QWidget* createWidget(QWidget* parent = nullptr)`
Creates a widget instance for this plugin.

**Parameters:**
- `parent`: Parent widget (optional)

**Returns:** New widget instance or `nullptr` if not applicable

##### `QStringList capabilities() const`
Returns a list of capabilities this plugin provides.

**Returns:** List of capability strings (e.g., "mapping", "data-provider")

---

### IDataProvider

Interface for plugins that provide geospatial data.

```cpp
class IDataProvider
{
public:
    virtual ~IDataProvider() = default;
    
    // Provider metadata
    virtual QString providerId() const = 0;
    virtual QString name() const = 0;
    virtual QString description() const = 0;
    virtual QIcon icon() const = 0;
    virtual QStringList supportedTypes() const = 0;
    
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
```

#### Methods

##### `QString providerId() const`
Returns unique identifier for this provider.

**Returns:** Unique provider ID string

##### `QStringList supportedTypes() const`
Returns list of data types this provider can handle.

**Returns:** List of supported format strings (e.g., "geojson", "shp", "kml")

##### `bool canCreateLayers() const`
Indicates if this provider can create new layers programmatically.

**Returns:** `true` if layer creation is supported

##### `bool canImportData() const`
Indicates if this provider can import data from files.

**Returns:** `true` if data import is supported

##### `bool canExportData() const`
Indicates if this provider can export layers to files.

**Returns:** `true` if data export is supported

##### `bool isRealTime() const`
Indicates if this provider handles real-time/streaming data.

**Returns:** `true` if real-time data is supported

##### `QStringList layerIds() const`
Returns list of all layer IDs managed by this provider.

**Returns:** List of layer ID strings

##### `IDataLayer* getLayer(const QString& layerId) const`
Retrieves a specific layer by ID.

**Parameters:**
- `layerId`: Unique layer identifier

**Returns:** Layer object or `nullptr` if not found

##### `QList<IDataLayer*> getAllLayers() const`
Returns all layers managed by this provider.

**Returns:** List of layer objects

##### `bool createLayer(const QString& name, const QString& type, const QVariantMap& parameters)`
Creates a new layer.

**Parameters:**
- `name`: Layer display name
- `type`: Layer type (e.g., "vector", "raster")
- `parameters`: Additional creation parameters

**Returns:** `true` if layer created successfully

##### `bool removeLayer(const QString& layerId)`
Removes a layer from this provider.

**Parameters:**
- `layerId`: Layer to remove

**Returns:** `true` if layer removed successfully

##### `bool importData(const QString& filePath, const QVariantMap& options)`
Imports data from a file and creates appropriate layers.

**Parameters:**
- `filePath`: Path to data file
- `options`: Import configuration options

**Returns:** `true` if import successful

##### `bool exportLayer(const QString& layerId, const QString& filePath, const QVariantMap& options)`
Exports a layer to a file.

**Parameters:**
- `layerId`: Layer to export
- `filePath`: Output file path
- `options`: Export configuration options

**Returns:** `true` if export successful

#### Signals

##### `void layerAdded(const QString& layerId)`
Emitted when a new layer is added to this provider.

##### `void layerRemoved(const QString& layerId)`
Emitted when a layer is removed from this provider.

##### `void layerChanged(const QString& layerId)`
Emitted when layer properties change.

##### `void dataUpdated(const QString& layerId)`
Emitted when layer data is updated (especially for real-time providers).

---

### IDataLayer

Interface representing an individual data layer.

```cpp
class IDataLayer
{
public:
    virtual ~IDataLayer() = default;
    
    // Layer metadata
    virtual QString id() const = 0;
    virtual QString name() const = 0;
    virtual QString type() const = 0;
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
    virtual QVariantMap boundingBox() const = 0;
    
    // Data access
    virtual QVariant data() const = 0;
    virtual QDateTime lastUpdated() const = 0;
};
```

#### Methods

##### `QString id() const`
Returns unique identifier for this layer.

**Returns:** Unique layer ID string

##### `QString name() const`
Returns human-readable name for this layer.

**Returns:** Layer display name

##### `QString type() const`
Returns the type of this layer.

**Returns:** Layer type string (e.g., "vector", "raster", "tile")

##### `QString description() const`
Returns detailed description of this layer.

**Returns:** Layer description text

##### `QIcon icon() const`
Returns icon representing this layer.

**Returns:** QIcon for UI display

##### `bool isVisible() const`
Returns current visibility state.

**Returns:** `true` if layer is visible

##### `void setVisible(bool visible)`
Sets layer visibility.

**Parameters:**
- `visible`: New visibility state

##### `double opacity() const`
Returns current opacity level.

**Returns:** Opacity value between 0.0 (transparent) and 1.0 (opaque)

##### `void setOpacity(double opacity)`
Sets layer opacity.

**Parameters:**
- `opacity`: New opacity value (0.0 to 1.0)

##### `QVariantMap properties() const`
Returns layer metadata properties.

**Returns:** Map of property name to value

##### `QVariantMap style() const`
Returns current layer styling information.

**Returns:** Map of style parameters

##### `void setStyle(const QVariantMap& style)`
Sets layer styling.

**Parameters:**
- `style`: Map of style parameters

##### `QVariantMap boundingBox() const`
Returns spatial extent of this layer.

**Returns:** Map with keys: "minLat", "minLon", "maxLat", "maxLon"

##### `QVariant data() const`
Returns the actual layer data.

**Returns:** Format-specific data (e.g., GeoJSON object, pixel array)

##### `QDateTime lastUpdated() const`
Returns when this layer was last updated.

**Returns:** Last update timestamp

---

## Core Services

### PluginManager

Manages plugin loading and lifecycle.

```cpp
class PluginManager : public QObject
{
    Q_OBJECT

public:
    explicit PluginManager(QObject* parent = nullptr);
    ~PluginManager();
    
    // Plugin management
    void loadPlugins();
    void unloadPlugins();
    
    // Plugin access
    QStringList availablePlugins() const;
    IPlugin* getPlugin(const QString& name) const;
    
    // Plugin registration
    void registerPlugin(const QString& name, IPlugin* plugin);

signals:
    void pluginLoaded(const QString& name);
    void pluginUnloaded(const QString& name);
};
```

#### Methods

##### `void loadPlugins()`
Discovers and loads all available plugins from standard locations.

##### `void unloadPlugins()`
Unloads all currently loaded plugins.

##### `QStringList availablePlugins() const`
Returns list of all loaded plugin names.

**Returns:** List of plugin names

##### `IPlugin* getPlugin(const QString& name) const`
Retrieves a plugin by name.

**Parameters:**
- `name`: Plugin name

**Returns:** Plugin instance or `nullptr` if not found

##### `void registerPlugin(const QString& name, IPlugin* plugin)`
Registers a plugin instance (for built-in plugins).

**Parameters:**
- `name`: Plugin name
- `plugin`: Plugin instance

#### Signals

##### `void pluginLoaded(const QString& name)`
Emitted when a plugin is successfully loaded.

##### `void pluginUnloaded(const QString& name)`
Emitted when a plugin is unloaded.

---

### DataProviderManager

Central coordinator for all data providers.

```cpp
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
    
    // Layer discovery
    QStringList allLayerIds() const;
    IDataLayer* getLayer(const QString& layerId) const;
    IDataLayer* getLayer(const QString& providerId, const QString& layerId) const;
    QList<IDataLayer*> getAllLayers() const;
    QList<IDataLayer*> getLayersByType(const QString& type) const;
    QList<IDataLayer*> getVisibleLayers() const;
    
    // Global layer operations
    void setLayerVisible(const QString& layerId, bool visible);
    void setLayerOpacity(const QString& layerId, double opacity);
    
    // Data import/export
    QStringList getSupportedImportFormats() const;
    QStringList getSupportedExportFormats() const;
    bool importData(const QString& filePath, const QString& preferredProviderId = QString());

signals:
    void providerRegistered(const QString& providerId);
    void providerUnregistered(const QString& providerId);
    void layerAdded(const QString& providerId, const QString& layerId);
    void layerRemoved(const QString& providerId, const QString& layerId);
    void layerChanged(const QString& providerId, const QString& layerId);
    void layerVisibilityChanged(const QString& layerId, bool visible);
    void dataUpdated(const QString& providerId, const QString& layerId);
    void layersChanged();
};
```

#### Methods

##### `bool registerProvider(IDataProvider* provider)`
Registers a data provider with the manager.

**Parameters:**
- `provider`: Provider instance to register

**Returns:** `true` if registration successful

##### `bool unregisterProvider(const QString& providerId)`
Unregisters a data provider.

**Parameters:**
- `providerId`: Provider to unregister

**Returns:** `true` if unregistration successful

##### `QStringList providerIds() const`
Returns list of all registered provider IDs.

**Returns:** List of provider ID strings

##### `IDataProvider* getProvider(const QString& providerId) const`
Retrieves a provider by ID.

**Parameters:**
- `providerId`: Provider identifier

**Returns:** Provider instance or `nullptr` if not found

##### `QList<IDataProvider*> getAllProviders() const`
Returns all registered providers.

**Returns:** List of provider instances

##### `QStringList allLayerIds() const`
Returns global list of all layer IDs across all providers.

**Returns:** List of global layer IDs (format: "providerId::layerId")

##### `IDataLayer* getLayer(const QString& layerId) const`
Retrieves a layer by global layer ID.

**Parameters:**
- `layerId`: Global layer ID

**Returns:** Layer instance or `nullptr` if not found

##### `IDataLayer* getLayer(const QString& providerId, const QString& layerId) const`
Retrieves a layer by provider ID and layer ID.

**Parameters:**
- `providerId`: Provider identifier
- `layerId`: Layer identifier within provider

**Returns:** Layer instance or `nullptr` if not found

##### `QList<IDataLayer*> getAllLayers() const`
Returns all layers from all providers.

**Returns:** List of all layer instances

##### `QList<IDataLayer*> getLayersByType(const QString& type) const`
Returns layers of a specific type.

**Parameters:**
- `type`: Layer type to filter by

**Returns:** List of matching layers

##### `QList<IDataLayer*> getVisibleLayers() const`
Returns all currently visible layers.

**Returns:** List of visible layer instances

##### `void setLayerVisible(const QString& layerId, bool visible)`
Sets visibility for a layer.

**Parameters:**
- `layerId`: Global layer ID
- `visible`: New visibility state

##### `void setLayerOpacity(const QString& layerId, double opacity)`
Sets opacity for a layer.

**Parameters:**
- `layerId`: Global layer ID
- `opacity`: New opacity value (0.0 to 1.0)

##### `QStringList getSupportedImportFormats() const`
Returns all import formats supported by registered providers.

**Returns:** List of supported format strings

##### `QStringList getSupportedExportFormats() const`
Returns all export formats supported by registered providers.

**Returns:** List of supported format strings

##### `bool importData(const QString& filePath, const QString& preferredProviderId)`
Imports data file using appropriate provider.

**Parameters:**
- `filePath`: Path to data file
- `preferredProviderId`: Preferred provider (optional)

**Returns:** `true` if import successful

#### Signals

##### `void providerRegistered(const QString& providerId)`
Emitted when a provider is registered.

##### `void providerUnregistered(const QString& providerId)`
Emitted when a provider is unregistered.

##### `void layerAdded(const QString& providerId, const QString& layerId)`
Emitted when a layer is added to any provider.

##### `void layerRemoved(const QString& providerId, const QString& layerId)`
Emitted when a layer is removed from any provider.

##### `void layerChanged(const QString& providerId, const QString& layerId)`
Emitted when layer properties change.

##### `void layerVisibilityChanged(const QString& layerId, bool visible)`
Emitted when layer visibility changes.

##### `void dataUpdated(const QString& providerId, const QString& layerId)`
Emitted when layer data is updated.

##### `void layersChanged()`
Emitted when any layer-related change occurs.

---

## UI Components

### DataBrowserWidget

Widget for browsing and managing data layers.

```cpp
class DataBrowserWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DataBrowserWidget(DataProviderManager* dataManager, QWidget *parent = nullptr);
    ~DataBrowserWidget();

public slots:
    void refreshProviders();
    void importData();
    void exportSelectedLayer();

signals:
    void layerSelectionChanged(const QString& layerId);
    void layerVisibilityChanged(const QString& layerId, bool visible);
    void layerOpacityChanged(const QString& layerId, double opacity);
    void zoomToLayerRequested(const QString& layerId);
};
```

#### Methods

##### `void refreshProviders()`
Refreshes the provider and layer tree.

##### `void importData()`
Opens file dialog to import data.

##### `void exportSelectedLayer()`
Exports the currently selected layer.

#### Signals

##### `void layerSelectionChanged(const QString& layerId)`
Emitted when user selects a different layer.

##### `void layerVisibilityChanged(const QString& layerId, bool visible)`
Emitted when user changes layer visibility.

##### `void layerOpacityChanged(const QString& layerId, double opacity)`
Emitted when user changes layer opacity.

##### `void zoomToLayerRequested(const QString& layerId)`
Emitted when user requests to zoom to a layer.

---

## Plugin Metadata Format

### Plugin JSON Schema

```json
{
    "name": "Plugin Display Name",
    "version": "1.0.0",
    "description": "Detailed plugin description",
    "author": "Author Name",
    "category": "plugin-category",
    "capabilities": ["capability1", "capability2"],
    "dependencies": ["QtCore", "QtWidgets", "other-dependencies"],
    "provides": {
        "services": ["service-name"],
        "widgets": ["WidgetClassName"]
    },
    "requires": {
        "services": ["required-service"],
        "plugins": ["required-plugin"]
    },
    "configuration": {
        "configurable": true,
        "settings": {
            "setting-name": {
                "type": "string|int|bool|double",
                "default": "default-value",
                "description": "Setting description"
            }
        }
    }
}
```

#### Schema Fields

##### `name` (required)
Human-readable plugin name for display in UI.

##### `version` (required)
Plugin version in semantic versioning format.

##### `description` (required)
Detailed description of plugin functionality.

##### `author` (optional)
Plugin author or organization name.

##### `category` (optional)
Plugin category for organization:
- `"visualization"` - Map viewers, charts, 3D displays
- `"data-provider"` - Data source plugins
- `"tools"` - Analysis and processing tools
- `"ui"` - User interface components

##### `capabilities` (required)
List of capabilities this plugin provides:
- `"mapping"` - Map visualization
- `"data-provider"` - Provides geospatial data
- `"data-browser"` - Data management UI
- `"analysis"` - Spatial analysis tools
- `"import-export"` - Data import/export
- `"real-time"` - Real-time data handling

##### `dependencies` (optional)
List of required Qt modules and system dependencies.

##### `provides` (optional)
Services and widgets provided by this plugin.

##### `requires` (optional)
Services and plugins required by this plugin.

##### `configuration` (optional)
Plugin configuration schema for user settings.

---

## Error Codes and Messages

### Plugin Loading Errors

| Code | Message | Description |
|------|---------|-------------|
| `PLUGIN_NOT_FOUND` | "Plugin file not found" | Plugin library file missing |
| `PLUGIN_LOAD_FAILED` | "Failed to load plugin library" | Library loading error |
| `PLUGIN_NO_INTERFACE` | "Plugin does not implement required interface" | Missing IPlugin implementation |
| `PLUGIN_INIT_FAILED` | "Plugin initialization failed" | Plugin.initialize() returned false |
| `PLUGIN_METADATA_INVALID` | "Invalid plugin metadata" | Malformed JSON metadata |

### Data Provider Errors

| Code | Message | Description |
|------|---------|-------------|
| `PROVIDER_NOT_FOUND` | "Data provider not found" | Provider ID not registered |
| `PROVIDER_INIT_FAILED` | "Provider initialization failed" | Provider setup error |
| `LAYER_NOT_FOUND` | "Layer not found" | Layer ID not found in provider |
| `IMPORT_FAILED` | "Data import failed" | File import error |
| `EXPORT_FAILED` | "Data export failed" | File export error |
| `UNSUPPORTED_FORMAT` | "Unsupported data format" | File format not supported |

### UI Component Errors

| Code | Message | Description |
|------|---------|-------------|
| `WIDGET_CREATE_FAILED` | "Failed to create widget" | Widget creation error |
| `SERVICE_UNAVAILABLE` | "Required service not available" | Dependency not found |
| `INVALID_PARAMETERS` | "Invalid parameters provided" | Bad method arguments |

---

## Constants and Enumerations

### Layer Types
```cpp
namespace LayerTypes {
    const QString Vector = "vector";
    const QString Raster = "raster";
    const QString Tile = "tile";
    const QString RealTime = "realtime";
    const QString ThreeD = "3d";
}
```

### Capabilities
```cpp
namespace Capabilities {
    const QString Mapping = "mapping";
    const QString DataProvider = "data-provider";
    const QString DataBrowser = "data-browser";
    const QString Analysis = "analysis";
    const QString ImportExport = "import-export";
    const QString RealTime = "real-time";
}
```

### File Formats
```cpp
namespace FileFormats {
    // Vector formats
    const QString GeoJSON = "geojson";
    const QString Shapefile = "shp";
    const QString KML = "kml";
    const QString GPX = "gpx";
    
    // Raster formats
    const QString GeoTIFF = "tiff";
    const QString PNG = "png";
    const QString JPEG = "jpg";
    
    // Tabular formats
    const QString CSV = "csv";
    const QString Excel = "xlsx";
}
```

---

## Version Compatibility

### API Version Matrix

| GeoWorld Version | API Version | Compatibility |
|------------------|-------------|---------------|
| 1.0.x | 1.0 | Full compatibility |
| 1.1.x | 1.1 | Backward compatible with 1.0 |
| 2.0.x | 2.0 | Breaking changes from 1.x |

### Migration Guide

#### From API 1.0 to 1.1
- No breaking changes
- New optional methods in interfaces
- Additional signal parameters (backward compatible)

#### From API 1.x to 2.0
- Interface method signature changes
- New required interface methods
- Updated plugin metadata schema