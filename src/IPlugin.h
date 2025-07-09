#pragma once

#include <QObject>
#include <QWidget>
#include <QString>
#include <QIcon>

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

Q_DECLARE_INTERFACE(IPlugin, "com.geoworld.IPlugin/1.0")