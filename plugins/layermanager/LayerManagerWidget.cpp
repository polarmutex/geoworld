#include "LayerManagerWidget.h"
#include "DataProviderManager.h"
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QMimeData>
#include <QDrag>
#include <QDebug>

LayerManagerWidget::LayerManagerWidget(DataProviderManager* dataManager, QWidget *parent)
    : QWidget(parent)
    , m_dataManager(dataManager)
    , m_updating(false)
{
    setupUI();
    setupConnections();
    populateProviders();
}

LayerManagerWidget::~LayerManagerWidget()
{
}

void LayerManagerWidget::setupUI()
{
    setMinimumSize(300, 400);
    
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(5, 5, 5, 5);
    
    // Create splitter for tree and properties
    m_splitter = new QSplitter(Qt::Vertical, this);
    
    // === Data Tree Section ===
    m_treeWidget = new QWidget();
    m_treeLayout = new QVBoxLayout(m_treeWidget);
    m_treeLayout->setContentsMargins(0, 0, 0, 0);
    
    // Toolbar
    m_toolbarLayout = new QHBoxLayout();
    m_refreshButton = new QPushButton("Refresh");
    m_refreshButton->setToolTip("Refresh data providers");
    m_importButton = new QPushButton("Import");
    m_importButton->setToolTip("Import data file");
    
    m_exportButton = new QToolButton();
    m_exportButton->setText("Export");
    m_exportButton->setToolTip("Export selected layer");
    m_exportButton->setPopupMode(QToolButton::MenuButtonPopup);
    
    m_exportMenu = new QMenu(this);
    m_exportButton->setMenu(m_exportMenu);
    
    m_toolbarLayout->addWidget(m_refreshButton);
    m_toolbarLayout->addWidget(m_importButton);
    m_toolbarLayout->addWidget(m_exportButton);
    m_toolbarLayout->addStretch();
    
    // Data tree
    m_dataTree = new QTreeWidget();
    m_dataTree->setHeaderLabels(QStringList() << "Data Layers" << "Type" << "Status");
    m_dataTree->setColumnWidth(0, 200);
    m_dataTree->setColumnWidth(1, 80);
    m_dataTree->setAlternatingRowColors(true);
    m_dataTree->setSelectionMode(QAbstractItemView::SingleSelection);
    m_dataTree->setContextMenuPolicy(Qt::CustomContextMenu);
    m_dataTree->setDragDropMode(QAbstractItemView::NoDragDrop);
    
    // Status label
    m_statusLabel = new QLabel("No data providers loaded");
    m_statusLabel->setStyleSheet("QLabel { color: gray; font-style: italic; }");
    
    m_treeLayout->addLayout(m_toolbarLayout);
    m_treeLayout->addWidget(m_dataTree);
    m_treeLayout->addWidget(m_statusLabel);
    
    // === Properties Section ===
    m_propertiesWidget = new QWidget();
    m_propertiesLayout = new QVBoxLayout(m_propertiesWidget);
    m_propertiesLayout->setContentsMargins(0, 0, 0, 0);
    
    m_propertiesTitle = new QLabel("Layer Properties");
    m_propertiesTitle->setStyleSheet("QLabel { font-weight: bold; font-size: 12px; }");
    
    // Layer controls
    m_controlsGroup = new QGroupBox("Controls");
    QVBoxLayout* controlsLayout = new QVBoxLayout(m_controlsGroup);
    
    m_visibilityCheck = new QCheckBox("Visible");
    
    QHBoxLayout* opacityLayout = new QHBoxLayout();
    m_opacityLabel = new QLabel("Opacity: 100%");
    m_opacitySlider = new QSlider(Qt::Horizontal);
    m_opacitySlider->setRange(0, 100);
    m_opacitySlider->setValue(100);
    opacityLayout->addWidget(new QLabel("Opacity:"));
    opacityLayout->addWidget(m_opacitySlider);
    opacityLayout->addWidget(m_opacityLabel);
    
    // Layer ordering buttons
    QHBoxLayout* orderingLayout = new QHBoxLayout();
    m_moveUpButton = new QPushButton("↑");
    m_moveDownButton = new QPushButton("↓");
    m_moveUpButton->setToolTip("Move layer up");
    m_moveDownButton->setToolTip("Move layer down");
    m_moveUpButton->setMaximumWidth(30);
    m_moveDownButton->setMaximumWidth(30);
    orderingLayout->addWidget(new QLabel("Order:"));
    orderingLayout->addWidget(m_moveUpButton);
    orderingLayout->addWidget(m_moveDownButton);
    orderingLayout->addStretch();
    
    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    m_zoomToButton = new QPushButton("Zoom To");
    m_removeButton = new QPushButton("Remove");
    m_propertiesButton = new QPushButton("Properties...");
    buttonsLayout->addWidget(m_zoomToButton);
    buttonsLayout->addWidget(m_removeButton);
    buttonsLayout->addWidget(m_propertiesButton);
    
    controlsLayout->addWidget(m_visibilityCheck);
    controlsLayout->addLayout(opacityLayout);
    controlsLayout->addLayout(orderingLayout);
    controlsLayout->addLayout(buttonsLayout);
    
    // Layer information
    m_infoGroup = new QGroupBox("Information");
    QVBoxLayout* infoLayout = new QVBoxLayout(m_infoGroup);
    m_infoText = new QTextEdit();
    m_infoText->setMaximumHeight(100);
    m_infoText->setReadOnly(true);
    infoLayout->addWidget(m_infoText);
    
    m_propertiesLayout->addWidget(m_propertiesTitle);
    m_propertiesLayout->addWidget(m_controlsGroup);
    m_propertiesLayout->addWidget(m_infoGroup);
    m_propertiesLayout->addStretch();
    
    // Add to splitter
    m_splitter->addWidget(m_treeWidget);
    m_splitter->addWidget(m_propertiesWidget);
    m_splitter->setSizes(QList<int>() << 300 << 200);
    
    m_mainLayout->addWidget(m_splitter);
    
    // Setup context menu
    m_contextMenu = new QMenu(this);
    m_showPropertiesAction = m_contextMenu->addAction("Properties...");
    m_contextMenu->addSeparator();
    m_toggleVisibilityAction = m_contextMenu->addAction("Toggle Visibility");
    m_contextMenu->addSeparator();
    m_zoomToLayerAction = m_contextMenu->addAction("Zoom To Layer");
    m_contextMenu->addSeparator();
    m_exportLayerAction = m_contextMenu->addAction("Export Layer...");
    m_removeLayerAction = m_contextMenu->addAction("Remove Layer");
    
    // Initially disable controls
    clearLayerProperties();
}

void LayerManagerWidget::setupConnections()
{
    // Data manager signals
    if (m_dataManager) {
        connect(m_dataManager, &DataProviderManager::providerRegistered,
                this, &LayerManagerWidget::onProviderRegistered);
        connect(m_dataManager, &DataProviderManager::providerUnregistered,
                this, &LayerManagerWidget::onProviderUnregistered);
        connect(m_dataManager, &DataProviderManager::layerAdded,
                this, &LayerManagerWidget::onLayerAdded);
        connect(m_dataManager, &DataProviderManager::layerRemoved,
                this, &LayerManagerWidget::onLayerRemoved);
        connect(m_dataManager, &DataProviderManager::layerChanged,
                this, &LayerManagerWidget::onLayerChanged);
    }
    
    // Tree widget signals
    connect(m_dataTree, &QTreeWidget::itemSelectionChanged,
            this, &LayerManagerWidget::onItemSelectionChanged);
    connect(m_dataTree, &QTreeWidget::itemChanged,
            this, &LayerManagerWidget::onItemChanged);
    connect(m_dataTree, &QTreeWidget::itemDoubleClicked,
            this, &LayerManagerWidget::onItemDoubleClicked);
    connect(m_dataTree, &QTreeWidget::customContextMenuRequested,
            this, &LayerManagerWidget::onContextMenuRequested);
    
    // Toolbar buttons
    connect(m_refreshButton, &QPushButton::clicked,
            this, &LayerManagerWidget::refreshProviders);
    connect(m_importButton, &QPushButton::clicked,
            this, &LayerManagerWidget::importData);
    connect(m_exportButton, &QPushButton::clicked,
            this, &LayerManagerWidget::exportSelectedLayer);
    
    // Layer controls
    connect(m_visibilityCheck, &QCheckBox::toggled,
            this, &LayerManagerWidget::onVisibilityToggled);
    connect(m_opacitySlider, &QSlider::valueChanged,
            this, &LayerManagerWidget::onOpacityChanged);
    connect(m_zoomToButton, &QPushButton::clicked,
            this, &LayerManagerWidget::zoomToLayer);
    connect(m_removeButton, &QPushButton::clicked,
            this, &LayerManagerWidget::removeLayer);
    connect(m_propertiesButton, &QPushButton::clicked,
            this, &LayerManagerWidget::showLayerProperties);
    connect(m_moveUpButton, &QPushButton::clicked,
            this, &LayerManagerWidget::moveLayerUp);
    connect(m_moveDownButton, &QPushButton::clicked,
            this, &LayerManagerWidget::moveLayerDown);
    
    // Context menu actions
    connect(m_showPropertiesAction, &QAction::triggered,
            this, &LayerManagerWidget::showLayerProperties);
    connect(m_removeLayerAction, &QAction::triggered,
            this, &LayerManagerWidget::removeLayer);
    connect(m_exportLayerAction, &QAction::triggered,
            this, &LayerManagerWidget::exportLayer);
    connect(m_zoomToLayerAction, &QAction::triggered,
            this, &LayerManagerWidget::zoomToLayer);
    connect(m_toggleVisibilityAction, &QAction::triggered, [this]() {
        IDataLayer* layer = getSelectedLayer();
        if (layer) {
            layer->setVisible(!layer->isVisible());
        }
    });
}

void LayerManagerWidget::populateProviders()
{
    m_dataTree->clear();
    
    if (!m_dataManager) {
        m_statusLabel->setText("Data manager not available");
        return;
    }
    
    QList<IDataProvider*> providers = m_dataManager->getAllProviders();
    if (providers.isEmpty()) {
        m_statusLabel->setText("No data providers loaded");
        return;
    }
    
    for (IDataProvider* provider : providers) {
        QTreeWidgetItem* providerItem = createProviderItem(provider);
        m_dataTree->addTopLevelItem(providerItem);
        
        // Add layers
        for (IDataLayer* layer : provider->getAllLayers()) {
            QTreeWidgetItem* layerItem = createLayerItem(layer, provider->providerId());
            providerItem->addChild(layerItem);
        }
        
        providerItem->setExpanded(true);
    }
    
    m_statusLabel->setText(QString("%1 provider(s), %2 layer(s)")
                           .arg(providers.size())
                           .arg(m_dataManager->getAllLayers().size()));
}

void LayerManagerWidget::refreshProviders()
{
    populateProviders();
}

void LayerManagerWidget::importData()
{
    if (!m_dataManager) return;
    
    QStringList formats = m_dataManager->getSupportedImportFormats();
    QString filter = "All Supported (";
    for (const QString& format : formats) {
        filter += "*." + format + " ";
    }
    filter += ");;All Files (*.*)";
    
    QString filePath = QFileDialog::getOpenFileName(this, "Import Data", "", filter);
    if (!filePath.isEmpty()) {
        if (m_dataManager->importData(filePath)) {
            QMessageBox::information(this, "Import", "Data imported successfully");
            refreshProviders();
        } else {
            QMessageBox::warning(this, "Import Error", "Failed to import data");
        }
    }
}

void LayerManagerWidget::exportSelectedLayer()
{
    exportLayer();
}

QTreeWidgetItem* LayerManagerWidget::createProviderItem(IDataProvider* provider)
{
    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setText(0, provider->name());
    item->setText(1, "Provider");
    item->setText(2, provider->isRealTime() ? "Real-time" : "Static");
    item->setIcon(0, provider->icon());
    item->setData(0, TypeRole, ProviderItem);
    item->setData(0, ProviderIdRole, provider->providerId());
    item->setToolTip(0, provider->description());
    
    // Provider items are not checkable
    item->setFlags(item->flags() & ~Qt::ItemIsUserCheckable);
    
    return item;
}

QTreeWidgetItem* LayerManagerWidget::createLayerItem(IDataLayer* layer, const QString& providerId)
{
    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setText(0, layer->name());
    item->setText(1, layer->type());
    item->setText(2, layer->isVisible() ? "Visible" : "Hidden");
    item->setIcon(0, layer->icon());
    item->setData(0, TypeRole, LayerItem);
    item->setData(0, ProviderIdRole, providerId);
    item->setData(0, LayerIdRole, layer->id());
    item->setData(0, LayerObjectRole, QVariant::fromValue(static_cast<void*>(layer)));
    item->setToolTip(0, layer->description());
    
    // Make layer items checkable for visibility
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(0, layer->isVisible() ? Qt::Checked : Qt::Unchecked);
    
    return item;
}

void LayerManagerWidget::onProviderRegistered(const QString& providerId)
{
    Q_UNUSED(providerId)
    refreshProviders();
}

void LayerManagerWidget::onProviderUnregistered(const QString& providerId)
{
    Q_UNUSED(providerId)
    refreshProviders();
}

void LayerManagerWidget::onLayerAdded(const QString& providerId, const QString& layerId)
{
    QTreeWidgetItem* providerItem = findProviderItem(providerId);
    if (providerItem && m_dataManager) {
        IDataLayer* layer = m_dataManager->getLayer(providerId, layerId);
        if (layer) {
            QTreeWidgetItem* layerItem = createLayerItem(layer, providerId);
            providerItem->addChild(layerItem);
            providerItem->setExpanded(true);
        }
    }
}

void LayerManagerWidget::onLayerRemoved(const QString& providerId, const QString& layerId)
{
    QTreeWidgetItem* layerItem = findLayerItem(providerId, layerId);
    if (layerItem) {
        delete layerItem;
    }
}

void LayerManagerWidget::onLayerChanged(const QString& providerId, const QString& layerId)
{
    QTreeWidgetItem* layerItem = findLayerItem(providerId, layerId);
    if (layerItem && m_dataManager) {
        IDataLayer* layer = m_dataManager->getLayer(providerId, layerId);
        if (layer) {
            m_updating = true;
            layerItem->setText(0, layer->name());
            layerItem->setText(1, layer->type());
            layerItem->setText(2, layer->isVisible() ? "Visible" : "Hidden");
            layerItem->setCheckState(0, layer->isVisible() ? Qt::Checked : Qt::Unchecked);
            m_updating = false;
            
            // Update properties if this layer is selected
            if (layerItem->isSelected()) {
                updateLayerProperties(layer);
            }
        }
    }
}

void LayerManagerWidget::onItemSelectionChanged()
{
    IDataLayer* layer = getSelectedLayer();
    if (layer) {
        updateLayerProperties(layer);
        emit layerSelectionChanged(layer->id());
    } else {
        clearLayerProperties();
    }
}

void LayerManagerWidget::onItemChanged(QTreeWidgetItem* item, int column)
{
    if (m_updating || column != 0) return;
    
    if (item->data(0, TypeRole).toInt() == LayerItem) {
        bool visible = (item->checkState(0) == Qt::Checked);
        QString layerId = item->data(0, LayerIdRole).toString();
        
        IDataLayer* layer = static_cast<IDataLayer*>(item->data(0, LayerObjectRole).value<void*>());
        if (layer && layer->isVisible() != visible) {
            layer->setVisible(visible);
            item->setText(2, visible ? "Visible" : "Hidden");
            emit layerVisibilityChanged(layerId, visible);
        }
    }
}

void LayerManagerWidget::onItemDoubleClicked(QTreeWidgetItem* item, int column)
{
    Q_UNUSED(column)
    if (item->data(0, TypeRole).toInt() == LayerItem) {
        QString layerId = item->data(0, LayerIdRole).toString();
        emit zoomToLayerRequested(layerId);
    }
}

void LayerManagerWidget::onContextMenuRequested(const QPoint& pos)
{
    QTreeWidgetItem* item = m_dataTree->itemAt(pos);
    if (item && item->data(0, TypeRole).toInt() == LayerItem) {
        m_contextMenu->exec(m_dataTree->mapToGlobal(pos));
    }
}

void LayerManagerWidget::onOpacityChanged(int value)
{
    if (m_updating) return;
    
    IDataLayer* layer = getSelectedLayer();
    if (layer) {
        double opacity = value / 100.0;
        layer->setOpacity(opacity);
        m_opacityLabel->setText(QString("Opacity: %1%").arg(value));
        emit layerOpacityChanged(layer->id(), opacity);
    }
}

void LayerManagerWidget::onVisibilityToggled(bool visible)
{
    if (m_updating) return;
    
    IDataLayer* layer = getSelectedLayer();
    if (layer) {
        layer->setVisible(visible);
        emit layerVisibilityChanged(layer->id(), visible);
        
        // Update tree item
        QTreeWidgetItem* item = m_dataTree->currentItem();
        if (item) {
            m_updating = true;
            item->setCheckState(0, visible ? Qt::Checked : Qt::Unchecked);
            item->setText(2, visible ? "Visible" : "Hidden");
            m_updating = false;
        }
    }
}

void LayerManagerWidget::updateLayerProperties(IDataLayer* layer)
{
    if (!layer) {
        clearLayerProperties();
        return;
    }
    
    m_updating = true;
    
    // Enable controls
    m_controlsGroup->setEnabled(true);
    m_infoGroup->setEnabled(true);
    
    // Update controls
    m_visibilityCheck->setChecked(layer->isVisible());
    m_opacitySlider->setValue(static_cast<int>(layer->opacity() * 100));
    m_opacityLabel->setText(QString("Opacity: %1%").arg(static_cast<int>(layer->opacity() * 100)));
    
    // Update information
    QString info = QString("Name: %1\n").arg(layer->name());
    info += QString("Type: %1\n").arg(layer->type());
    info += QString("Description: %1\n").arg(layer->description());
    info += QString("Last Updated: %1\n").arg(layer->lastUpdated().toString());
    
    QVariantMap bbox = layer->boundingBox();
    if (!bbox.isEmpty()) {
        info += QString("Extent: (%1, %2) to (%3, %4)\n")
                .arg(bbox["minLon"].toDouble(), 0, 'f', 6)
                .arg(bbox["minLat"].toDouble(), 0, 'f', 6)
                .arg(bbox["maxLon"].toDouble(), 0, 'f', 6)
                .arg(bbox["maxLat"].toDouble(), 0, 'f', 6);
    }
    
    m_infoText->setText(info);
    
    // Enable/disable move buttons based on position
    QTreeWidgetItem* item = m_dataTree->currentItem();
    if (item && item->parent()) {
        int index = item->parent()->indexOfChild(item);
        int childCount = item->parent()->childCount();
        m_moveUpButton->setEnabled(index > 0);
        m_moveDownButton->setEnabled(index < childCount - 1);
    } else {
        m_moveUpButton->setEnabled(false);
        m_moveDownButton->setEnabled(false);
    }
    
    m_updating = false;
}

void LayerManagerWidget::clearLayerProperties()
{
    m_controlsGroup->setEnabled(false);
    m_infoGroup->setEnabled(false);
    m_infoText->clear();
    m_visibilityCheck->setChecked(false);
    m_opacitySlider->setValue(100);
    m_opacityLabel->setText("Opacity: 100%");
    m_moveUpButton->setEnabled(false);
    m_moveDownButton->setEnabled(false);
}

QTreeWidgetItem* LayerManagerWidget::findProviderItem(const QString& providerId)
{
    for (int i = 0; i < m_dataTree->topLevelItemCount(); ++i) {
        QTreeWidgetItem* item = m_dataTree->topLevelItem(i);
        if (item->data(0, ProviderIdRole).toString() == providerId) {
            return item;
        }
    }
    return nullptr;
}

QTreeWidgetItem* LayerManagerWidget::findLayerItem(const QString& providerId, const QString& layerId)
{
    QTreeWidgetItem* providerItem = findProviderItem(providerId);
    if (providerItem) {
        for (int i = 0; i < providerItem->childCount(); ++i) {
            QTreeWidgetItem* layerItem = providerItem->child(i);
            if (layerItem->data(0, LayerIdRole).toString() == layerId) {
                return layerItem;
            }
        }
    }
    return nullptr;
}

QString LayerManagerWidget::getSelectedLayerId() const
{
    QTreeWidgetItem* item = m_dataTree->currentItem();
    if (item && item->data(0, TypeRole).toInt() == LayerItem) {
        return item->data(0, LayerIdRole).toString();
    }
    return QString();
}

IDataLayer* LayerManagerWidget::getSelectedLayer() const
{
    QTreeWidgetItem* item = m_dataTree->currentItem();
    if (item && item->data(0, TypeRole).toInt() == LayerItem) {
        return static_cast<IDataLayer*>(item->data(0, LayerObjectRole).value<void*>());
    }
    return nullptr;
}

void LayerManagerWidget::showLayerProperties()
{
    IDataLayer* layer = getSelectedLayer();
    if (layer) {
        // TODO: Implement detailed properties dialog
        QMessageBox::information(this, "Layer Properties", 
                                QString("Properties for layer: %1").arg(layer->name()));
    }
}

void LayerManagerWidget::removeLayer()
{
    QString layerId = getSelectedLayerId();
    QTreeWidgetItem* item = m_dataTree->currentItem();
    if (!layerId.isEmpty() && item) {
        QString providerId = item->data(0, ProviderIdRole).toString();
        
        int ret = QMessageBox::question(this, "Remove Layer",
                                       QString("Remove layer '%1'?").arg(item->text(0)),
                                       QMessageBox::Yes | QMessageBox::No);
        
        if (ret == QMessageBox::Yes && m_dataManager) {
            IDataProvider* provider = m_dataManager->getProvider(providerId);
            if (provider) {
                provider->removeLayer(layerId);
            }
        }
    }
}

void LayerManagerWidget::exportLayer()
{
    IDataLayer* layer = getSelectedLayer();
    if (!layer || !m_dataManager) return;
    
    QStringList formats = m_dataManager->getSupportedExportFormats();
    QString filter = "All Supported (";
    for (const QString& format : formats) {
        filter += "*." + format + " ";
    }
    filter += ");;All Files (*.*)";
    
    QString filePath = QFileDialog::getSaveFileName(this, "Export Layer", 
                                                   layer->name(), filter);
    if (!filePath.isEmpty()) {
        // TODO: Implement export functionality
        QMessageBox::information(this, "Export", "Export functionality not yet implemented");
    }
}

void LayerManagerWidget::zoomToLayer()
{
    QString layerId = getSelectedLayerId();
    if (!layerId.isEmpty()) {
        emit zoomToLayerRequested(layerId);
    }
}

void LayerManagerWidget::moveLayerUp()
{
    QTreeWidgetItem* item = m_dataTree->currentItem();
    if (!item || item->data(0, TypeRole).toInt() != LayerItem) {
        return;
    }
    
    QTreeWidgetItem* parent = item->parent();
    if (!parent) return;
    
    int currentIndex = parent->indexOfChild(item);
    if (currentIndex > 0) {
        // Remove and reinsert at previous position
        parent->takeChild(currentIndex);
        parent->insertChild(currentIndex - 1, item);
        
        // Keep selection
        m_dataTree->setCurrentItem(item);
        
        // Emit signal for order change
        QString layerId = item->data(0, LayerIdRole).toString();
        emit layerOrderChanged(layerId, currentIndex - 1);
    }
}

void LayerManagerWidget::moveLayerDown()
{
    QTreeWidgetItem* item = m_dataTree->currentItem();
    if (!item || item->data(0, TypeRole).toInt() != LayerItem) {
        return;
    }
    
    QTreeWidgetItem* parent = item->parent();
    if (!parent) return;
    
    int currentIndex = parent->indexOfChild(item);
    if (currentIndex < parent->childCount() - 1) {
        // Remove and reinsert at next position
        parent->takeChild(currentIndex);
        parent->insertChild(currentIndex + 1, item);
        
        // Keep selection
        m_dataTree->setCurrentItem(item);
        
        // Emit signal for order change
        QString layerId = item->data(0, LayerIdRole).toString();
        emit layerOrderChanged(layerId, currentIndex + 1);
    }
}