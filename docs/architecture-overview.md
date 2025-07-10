# GeoWorld Architecture Overview

## Introduction

GeoWorld is a plugin-based geospatial data visualization platform built with Qt6. The architecture is designed around a modular plugin system where functionality is provided by independent, dynamically-loaded plugins.

## Core Architectural Principles

### 1. Plugin-First Design
- **Core App**: Minimal kernel providing essential services
- **Plugins**: Self-contained modules providing specific functionality
- **Loose Coupling**: Plugins communicate through well-defined interfaces
- **Dynamic Loading**: Plugins can be loaded/unloaded at runtime

### 2. Service-Oriented Architecture
- **Service Registry**: Core app provides service discovery and registration
- **Service Interfaces**: Standardized contracts for inter-plugin communication
- **Event-Driven**: Services communicate through signals/slots
- **Dependency Injection**: Services are injected into plugins that need them

### 3. Data Provider Ecosystem
- **Data Providers**: Plugins that supply geospatial data
- **Data Consumers**: Plugins that visualize or process data
- **Data Layers**: Individual datasets with metadata and styling
- **Centralized Management**: Core app coordinates data flow

## System Components

```
┌─────────────────────────────────────────────────────────────┐
│                        Core Application                     │
├─────────────────────────────────────────────────────────────┤
│  Plugin Manager  │  Data Provider Manager  │  UI Manager   │
├─────────────────────────────────────────────────────────────┤
│                    Service Registry                         │
└─────────────────────────────────────────────────────────────┘
                                │
            ┌───────────────────┼───────────────────┐
            │                   │                   │
    ┌───────▼────────┐ ┌────────▼────────┐ ┌───────▼────────┐
    │  Map Plugin    │ │ Data Browser    │ │File Provider   │
    │   (Consumer)   │ │    Plugin       │ │    Plugin      │
    │                │ │    (UI)         │ │  (Provider)    │
    └────────────────┘ └─────────────────┘ └────────────────┘
```

### Core Application
- **Minimal Core**: Framework for plugin loading and service management
- **Plugin Manager**: Discovers, loads, and manages plugin lifecycle
- **Data Provider Manager**: Central registry for data providers and layers
- **UI Manager**: Advanced docking system for plugin UI components

### Plugin Types

#### 1. Data Provider Plugins
- **Purpose**: Supply geospatial data from various sources
- **Examples**: File Provider, Database Provider, Web Service Provider, GPS Provider
- **Responsibilities**: 
  - Data loading and parsing
  - Layer management
  - Data caching and optimization
  - Real-time data streaming

#### 2. Visualization Plugins
- **Purpose**: Display and interact with geospatial data
- **Examples**: Map Plugin, 3D Viewer Plugin, Chart Plugin
- **Responsibilities**:
  - Rendering data layers
  - User interaction handling
  - Styling and symbolization
  - Performance optimization

#### 3. Tool Plugins
- **Purpose**: Provide analysis and processing capabilities
- **Examples**: Measurement Tools, Spatial Analysis, Data Export
- **Responsibilities**:
  - Data processing algorithms
  - User interface for tool parameters
  - Result visualization
  - Progress reporting

#### 4. UI Plugins
- **Purpose**: Provide user interface components
- **Examples**: Data Browser, Layer Properties, Tool Palettes
- **Responsibilities**:
  - User interface widgets
  - Data binding and visualization
  - User interaction handling
  - Configuration management

## Data Flow Architecture

### Data Provider Registration
```
1. Plugin Loaded → 2. Provider Registered → 3. Layers Discovered → 4. UI Updated
```

### Layer Visibility Control
```
1. User Action → 2. Data Browser → 3. Provider Manager → 4. Map Plugin → 5. Render Update
```

### Real-time Data Updates
```
1. Data Source → 2. Provider Plugin → 3. Layer Updated → 4. Event Broadcast → 5. Consumers Notified
```

## Service Architecture

### Core Services

#### 1. Plugin Manager (`PluginManager`)
- **Interface**: `IPlugin`
- **Purpose**: Plugin lifecycle management
- **Capabilities**:
  - Plugin discovery and loading
  - Dependency resolution
  - Plugin metadata management
  - Error handling and recovery

#### 2. Data Provider Manager (`DataProviderManager`)
- **Interface**: `IDataProvider`, `IDataLayer`
- **Purpose**: Data provider coordination
- **Capabilities**:
  - Provider registration and discovery
  - Layer aggregation across providers
  - Data import/export coordination
  - Layer visibility and styling management

#### 3. UI Manager (Qt Advanced Docking System)
- **Interface**: Qt's docking framework
- **Purpose**: Window and widget management
- **Capabilities**:
  - Dockable widget management
  - Layout persistence
  - Multi-monitor support
  - Flexible workspace configuration

### Plugin Services

#### 1. Layer Rendering Service
- **Provided by**: Map Plugin
- **Consumers**: Data Browser, Analysis Tools
- **Capabilities**:
  - Layer rendering coordination
  - Spatial extent management
  - Zoom and pan operations
  - Style application

#### 2. Data Access Service
- **Provided by**: Data Provider Plugins
- **Consumers**: Map Plugin, Analysis Tools
- **Capabilities**:
  - Data loading and caching
  - Spatial filtering
  - Attribute querying
  - Format conversion

## Plugin Communication Patterns

### 1. Service Discovery Pattern
```cpp
// Plugin requests a service from core app
IDataProviderManager* dataManager = serviceRegistry->getService<IDataProviderManager>();
```

### 2. Event Broadcasting Pattern
```cpp
// Provider notifies about data changes
emit layerDataUpdated(layerId);
// All interested consumers receive notification
```

### 3. Direct Interface Pattern
```cpp
// Plugin provides interface to other plugins
IDataProvider* provider = dataManager->getProvider("file-provider");
provider->createLayer("new-layer", "vector");
```

## Configuration and Persistence

### Plugin Configuration
- **Plugin Metadata**: JSON files describing plugin capabilities
- **User Settings**: QSettings-based configuration storage
- **Workspace Layouts**: Docking system layout persistence
- **Layer Styles**: Style definitions stored with layers

### Data Persistence
- **Provider-Managed**: Each provider handles its own data persistence
- **Metadata Storage**: Layer metadata stored in provider-specific formats
- **Cache Management**: Configurable caching strategies per provider
- **Session Restore**: Ability to restore previous session state

## Performance Considerations

### 1. Lazy Loading
- Plugins loaded only when needed
- Data loaded on-demand
- Progressive layer rendering

### 2. Caching Strategy
- Multi-level caching (memory, disk, provider-specific)
- Configurable cache sizes and policies
- Automatic cache invalidation

### 3. Asynchronous Operations
- Non-blocking data loading
- Background processing
- Progress reporting for long operations

### 4. Spatial Indexing
- Provider-level spatial indexing
- Efficient spatial queries
- Level-of-detail rendering

## Security Model

### 1. Plugin Sandboxing
- Plugins run in application process (trusted)
- File system access through Qt's security model
- Network access controlled by Qt's network stack

### 2. Data Access Control
- Provider-level access control
- User authentication for remote data sources
- Secure credential storage

### 3. Configuration Security
- Settings encryption for sensitive data
- Secure storage of authentication tokens
- Audit logging for data access

## Extension Points

### 1. New Plugin Types
- Implement `IPlugin` interface
- Register with Plugin Manager
- Provide required metadata

### 2. New Data Providers
- Implement `IDataProvider` and `IDataLayer` interfaces
- Register with Data Provider Manager
- Handle data-specific operations

### 3. Custom Services
- Define service interface
- Register with Service Registry
- Provide service to other plugins

### 4. Custom UI Components
- Create Qt widgets
- Integrate with docking system
- Handle user interactions

## Development Workflow

### 1. Plugin Development
```bash
1. Create plugin directory
2. Implement plugin interfaces
3. Create plugin metadata
4. Build as shared library
5. Deploy to plugin directory
```

### 2. Testing and Debugging
- Unit tests for plugin logic
- Integration tests with core app
- Debug support through Qt's debugging tools
- Plugin hot-reloading for development

### 3. Deployment
- Plugin packaging
- Version management
- Dependency resolution
- Update mechanisms

## Future Considerations

### 1. Remote Plugins
- Plugin loading from remote sources
- Plugin marketplace
- Digital signing and verification

### 2. Distributed Computing
- Remote data processing
- Cloud-based providers
- Distributed rendering

### 3. Mobile Support
- Touch-optimized UI
- Mobile-specific plugins
- Offline data synchronization

### 4. Web Integration
- WebAssembly plugin support
- Web-based data sources
- Browser embedding