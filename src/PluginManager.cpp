#include "PluginManager.h"
#include <QDir>
#include <QDebug>
#include <QCoreApplication>

PluginManager::PluginManager(QObject* parent)
    : QObject(parent)
{
}

PluginManager::~PluginManager()
{
    unloadPlugins();
}

void PluginManager::loadPlugins()
{
    // Load plugins from application directory
    QString appDir = QCoreApplication::applicationDirPath();
    loadPluginsFromDirectory(appDir + "/plugins");
    
    // Load plugins from installed location (relative to bin directory)
    loadPluginsFromDirectory(appDir + "/../lib/geoworld/plugins");
    
    // Load plugins from build directory (for development)
    loadPluginsFromDirectory("./plugins");
    loadPluginsFromDirectory("../plugins");
    loadPluginsFromDirectory("./build/plugins");
    
    // Load plugins from system directory
    loadPluginsFromDirectory("/usr/local/lib/geoworld/plugins");
    
    qDebug() << "Loaded" << m_plugins.size() << "plugins";
}

void PluginManager::loadPluginsFromDirectory(const QString& directory)
{
    QDir pluginDir(directory);
    if (!pluginDir.exists()) {
        qDebug() << "Plugin directory does not exist:" << directory;
        return;
    }
    
    qDebug() << "Searching for plugins in:" << directory;
    
    QStringList filters;
    filters << "*.so" << "*.dll" << "*.dylib";
    pluginDir.setNameFilters(filters);
    
    for (const QString& fileName : pluginDir.entryList(QDir::Files)) {
        QString filePath = pluginDir.absoluteFilePath(fileName);
        
        QPluginLoader* loader = new QPluginLoader(filePath);
        QObject* pluginObject = loader->instance();
        
        if (pluginObject) {
            IPlugin* plugin = qobject_cast<IPlugin*>(pluginObject);
            if (plugin) {
                QString name = plugin->name();
                qDebug() << "Loading plugin:" << name;
                
                if (plugin->initialize()) {
                    m_plugins[name] = plugin;
                    m_loaders[name] = loader;
                    emit pluginLoaded(name);
                } else {
                    qWarning() << "Failed to initialize plugin:" << name;
                    delete loader;
                }
            } else {
                qWarning() << "Plugin does not implement IPlugin interface:" << fileName;
                delete loader;
            }
        } else {
            qWarning() << "Failed to load plugin:" << fileName << loader->errorString();
            delete loader;
        }
    }
}

void PluginManager::unloadPlugins()
{
    for (auto it = m_plugins.begin(); it != m_plugins.end(); ++it) {
        it.value()->shutdown();
        emit pluginUnloaded(it.key());
    }
    
    // Delete loaders
    for (auto it = m_loaders.begin(); it != m_loaders.end(); ++it) {
        delete it.value();
    }
    
    m_plugins.clear();
    m_loaders.clear();
}

QStringList PluginManager::availablePlugins() const
{
    return m_plugins.keys();
}

IPlugin* PluginManager::getPlugin(const QString& name) const
{
    auto it = m_plugins.find(name);
    return (it != m_plugins.end()) ? it.value() : nullptr;
}

void PluginManager::registerPlugin(const QString& name, IPlugin* plugin)
{
    if (plugin && plugin->initialize()) {
        m_plugins[name] = plugin;
        emit pluginLoaded(name);
    }
}