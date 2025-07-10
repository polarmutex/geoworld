#pragma once

#include "IPlugin.h"
#include "FileDataProvider.h"
#include <QObject>
#include <QtPlugin>

class FileProviderPlugin : public QObject, public IPlugin
{
    Q_OBJECT
    Q_INTERFACES(IPlugin)
    Q_PLUGIN_METADATA(IID "com.geoworld.IPlugin/1.0" FILE "fileprovider.json")

public:
    explicit FileProviderPlugin(QObject* parent = nullptr);
    ~FileProviderPlugin();

    // IPlugin interface
    QString name() const override;
    QString version() const override;
    QString description() const override;
    QIcon icon() const override;
    
    bool initialize() override;
    void shutdown() override;
    
    QWidget* createWidget(QWidget* parent = nullptr) override;
    
    QStringList capabilities() const override;
    
    // Access to the data provider
    FileDataProvider* getDataProvider() const { return m_dataProvider; }

private:
    FileDataProvider* m_dataProvider;
    bool m_initialized;
};