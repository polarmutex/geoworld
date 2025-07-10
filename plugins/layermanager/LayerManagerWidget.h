#pragma once

#include <QWidget>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QToolButton>
#include <QMenu>
#include <QAction>
#include <QLabel>
#include <QProgressBar>
#include <QSplitter>
#include <QTextEdit>
#include <QGroupBox>
#include <QSlider>
#include <QCheckBox>
#include "IDataProvider.h"

class DataProviderManager;

class LayerManagerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LayerManagerWidget(DataProviderManager* dataManager, QWidget *parent = nullptr);
    ~LayerManagerWidget();

public slots:
    void refreshProviders();
    void importData();
    void exportSelectedLayer();
    void moveLayerUp();
    void moveLayerDown();

private slots:
    void onProviderRegistered(const QString& providerId);
    void onProviderUnregistered(const QString& providerId);
    void onLayerAdded(const QString& providerId, const QString& layerId);
    void onLayerRemoved(const QString& providerId, const QString& layerId);
    void onLayerChanged(const QString& providerId, const QString& layerId);
    
    void onItemSelectionChanged();
    void onItemChanged(QTreeWidgetItem* item, int column);
    void onItemDoubleClicked(QTreeWidgetItem* item, int column);
    void onContextMenuRequested(const QPoint& pos);
    
    void onOpacityChanged(int value);
    void onVisibilityToggled(bool visible);
    
    void showLayerProperties();
    void removeLayer();
    void exportLayer();
    void zoomToLayer();

signals:
    void layerSelectionChanged(const QString& layerId);
    void layerVisibilityChanged(const QString& layerId, bool visible);
    void layerOpacityChanged(const QString& layerId, double opacity);
    void layerOrderChanged(const QString& layerId, int newPosition);
    void zoomToLayerRequested(const QString& layerId);

private:
    enum ItemType {
        ProviderItem = QTreeWidgetItem::UserType + 1,
        LayerItem = QTreeWidgetItem::UserType + 2
    };
    
    enum ItemDataRole {
        TypeRole = Qt::UserRole + 1,
        ProviderIdRole = Qt::UserRole + 2,
        LayerIdRole = Qt::UserRole + 3,
        LayerObjectRole = Qt::UserRole + 4
    };
    
    void setupUI();
    void setupConnections();
    void populateProviders();
    void updateLayerProperties(IDataLayer* layer);
    void clearLayerProperties();
    
    QTreeWidgetItem* findProviderItem(const QString& providerId);
    QTreeWidgetItem* findLayerItem(const QString& providerId, const QString& layerId);
    QTreeWidgetItem* createProviderItem(IDataProvider* provider);
    QTreeWidgetItem* createLayerItem(IDataLayer* layer, const QString& providerId);
    
    QString getSelectedLayerId() const;
    IDataLayer* getSelectedLayer() const;
    
    // UI Components
    QVBoxLayout* m_mainLayout;
    QSplitter* m_splitter;
    
    // Data tree
    QWidget* m_treeWidget;
    QVBoxLayout* m_treeLayout;
    QHBoxLayout* m_toolbarLayout;
    QTreeWidget* m_dataTree;
    QPushButton* m_refreshButton;
    QPushButton* m_importButton;
    QToolButton* m_exportButton;
    QMenu* m_exportMenu;
    QLabel* m_statusLabel;
    
    // Layer properties panel
    QWidget* m_propertiesWidget;
    QVBoxLayout* m_propertiesLayout;
    QLabel* m_propertiesTitle;
    
    // Layer controls
    QGroupBox* m_controlsGroup;
    QCheckBox* m_visibilityCheck;
    QLabel* m_opacityLabel;
    QSlider* m_opacitySlider;
    QPushButton* m_moveUpButton;
    QPushButton* m_moveDownButton;
    QPushButton* m_zoomToButton;
    QPushButton* m_removeButton;
    QPushButton* m_propertiesButton;
    
    // Layer information
    QGroupBox* m_infoGroup;
    QTextEdit* m_infoText;
    
    // Context menu
    QMenu* m_contextMenu;
    QAction* m_showPropertiesAction;
    QAction* m_removeLayerAction;
    QAction* m_exportLayerAction;
    QAction* m_zoomToLayerAction;
    QAction* m_toggleVisibilityAction;
    
    DataProviderManager* m_dataManager;
    bool m_updating; // Flag to prevent recursive updates
};