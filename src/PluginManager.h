#pragma once

#include <QObject>
#include <QMap>
#include <QStringList>
#include <QPluginLoader>
#include <memory>
#include "IPlugin.h"

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
    
    // Plugin registration (for built-in plugins)
    void registerPlugin(const QString& name, IPlugin* plugin);

signals:
    void pluginLoaded(const QString& name);
    void pluginUnloaded(const QString& name);

private:
    void loadPluginsFromDirectory(const QString& directory);
    
    QMap<QString, IPlugin*> m_plugins;
    QMap<QString, QPluginLoader*> m_loaders;
};