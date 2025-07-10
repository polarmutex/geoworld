#pragma once

#include "IDataProvider.h"
#include <QObject>
#include <QJsonObject>
#include <QVariantMap>
#include <QDateTime>
#include <QIcon>

class FileDataLayer : public IDataLayer
{
public:
    FileDataLayer(const QString& id, const QString& name, const QString& filePath, 
                  const QString& type = "vector");
    ~FileDataLayer();
    
    // IDataLayer interface
    QString id() const override { return m_id; }
    QString name() const override { return m_name; }
    QString type() const override { return m_type; }
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
    QVariant data() const override;
    QDateTime lastUpdated() const override { return m_lastUpdated; }
    
    // File-specific methods
    QString filePath() const { return m_filePath; }
    bool loadFromFile();
    bool isDataLoaded() const { return m_dataLoaded; }

private:
    void calculateBoundingBox();
    void extractProperties();
    bool loadGeoJSON();
    bool loadCSV();
    bool loadKML();
    
    QString m_id;
    QString m_name;
    QString m_type;
    QString m_description;
    QString m_filePath;
    bool m_visible;
    double m_opacity;
    
    QVariantMap m_properties;
    QVariantMap m_style;
    QVariantMap m_boundingBox;
    mutable QVariant m_cachedData;
    mutable bool m_dataLoaded;
    QDateTime m_lastUpdated;
};