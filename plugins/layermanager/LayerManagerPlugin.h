#pragma once

#include "IPlugin.h"
#include <QObject>
#include <QIcon>
#include <QtPlugin>

class DataProviderManager;

class LayerManagerPlugin : public QObject, public IPlugin
{
    Q_OBJECT
    Q_INTERFACES(IPlugin)
    Q_PLUGIN_METADATA(IID "com.geoworld.IPlugin/1.0" FILE "layermanager.json")

public:
    LayerManagerPlugin(QObject* parent = nullptr);
    ~LayerManagerPlugin();

    // IPlugin interface
    QString name() const override;
    QString version() const override;
    QString description() const override;
    QIcon icon() const override;
    
    bool initialize() override;
    void shutdown() override;
    
    QWidget* createWidget(QWidget* parent = nullptr) override;
    
    QStringList capabilities() const override;

    // Additional methods for layer manager specific functionality
    void setDataProviderManager(DataProviderManager* manager);
    DataProviderManager* dataProviderManager() const;

private:
    bool m_initialized;
    DataProviderManager* m_dataProviderManager;
};