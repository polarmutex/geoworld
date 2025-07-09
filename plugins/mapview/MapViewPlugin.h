#pragma once

#include "IPlugin.h"
#include <QObject>
#include <QIcon>
#include <QtPlugin>

class MapViewPlugin : public QObject, public IPlugin
{
    Q_OBJECT
    Q_INTERFACES(IPlugin)
    Q_PLUGIN_METADATA(IID "com.geoworld.IPlugin/1.0" FILE "mapview.json")

public:
    MapViewPlugin(QObject* parent = nullptr);
    ~MapViewPlugin();

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