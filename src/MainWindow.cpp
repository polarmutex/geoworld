#include "MainWindow.h"
#include "MapWidget.h"
#include <QApplication>
#include <QAction>
#include <QMenu>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_mapWidget(nullptr)
    , m_dockManager(nullptr)
    , m_coordLabel(nullptr)
    , m_zoomLabel(nullptr)
    , m_scaleLabel(nullptr)
    , m_layerCombo(nullptr)
    , m_mapDockWidget(nullptr)
{
    setWindowTitle("GeoWorld - Geospatial Data Visualization");
    setMinimumSize(1200, 800);
    
    setupUI();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    setupDockingSystem();
}

MainWindow::~MainWindow()
{
    if (m_dockManager) {
        m_dockManager->deleteLater();
    }
}

void MainWindow::setupUI()
{
    // Create central widget container
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // Create main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // Create map widget
    m_mapWidget = new MapWidget(this);
    
    // Connect map widget signals
    connect(m_mapWidget, &MapWidget::coordinateChanged, 
            this, &MainWindow::onCoordinateUpdate);
    connect(m_mapWidget, &MapWidget::zoomChanged, 
            this, &MainWindow::onZoomChanged);
}

void MainWindow::setupMenuBar()
{
    // File menu
    QMenu *fileMenu = menuBar()->addMenu("&File");
    
    QAction *exitAction = new QAction("E&xit", this);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    fileMenu->addAction(exitAction);
    
    // View menu
    QMenu *viewMenu = menuBar()->addMenu("&View");
    
    QAction *resetViewAction = new QAction("Reset &View", this);
    resetViewAction->setShortcut(QKeySequence("Ctrl+R"));
    connect(resetViewAction, &QAction::triggered, m_mapWidget, &MapWidget::resetView);
    viewMenu->addAction(resetViewAction);
    
    // Tools menu
    QMenu *toolsMenu = menuBar()->addMenu("&Tools");
    
    QAction *measureAction = new QAction("&Measure Distance", this);
    measureAction->setShortcut(QKeySequence("Ctrl+M"));
    measureAction->setCheckable(true);
    toolsMenu->addAction(measureAction);
    
    // Help menu
    QMenu *helpMenu = menuBar()->addMenu("&Help");
    
    QAction *aboutAction = new QAction("&About", this);
    connect(aboutAction, &QAction::triggered, [this]() {
        QMessageBox::about(this, "About GeoWorld", 
            "GeoWorld v1.0.0\nGeospatial Data Visualization Platform");
    });
    helpMenu->addAction(aboutAction);
}

void MainWindow::setupToolBar()
{
    QToolBar *mainToolBar = addToolBar("Main");
    mainToolBar->setObjectName("MainToolBar");
    
    // Map layer selector
    mainToolBar->addWidget(new QLabel("Map Layer:"));
    m_layerCombo = new QComboBox(this);
    m_layerCombo->addItem("OpenStreetMap", "osm");
    m_layerCombo->addItem("Satellite", "satellite");
    m_layerCombo->addItem("Terrain", "terrain");
    m_layerCombo->setCurrentIndex(0);
    
    connect(m_layerCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onMapLayerChanged);
    
    mainToolBar->addWidget(m_layerCombo);
    mainToolBar->addSeparator();
    
    // Zoom controls
    QAction *zoomInAction = new QAction("Zoom In", this);
    zoomInAction->setShortcut(QKeySequence::ZoomIn);
    connect(zoomInAction, &QAction::triggered, m_mapWidget, &MapWidget::zoomIn);
    mainToolBar->addAction(zoomInAction);
    
    QAction *zoomOutAction = new QAction("Zoom Out", this);
    zoomOutAction->setShortcut(QKeySequence::ZoomOut);
    connect(zoomOutAction, &QAction::triggered, m_mapWidget, &MapWidget::zoomOut);
    mainToolBar->addAction(zoomOutAction);
    
    mainToolBar->addSeparator();
    
    // Reset view
    QAction *resetAction = new QAction("Reset View", this);
    connect(resetAction, &QAction::triggered, m_mapWidget, &MapWidget::resetView);
    mainToolBar->addAction(resetAction);
}

void MainWindow::setupStatusBar()
{
    // Coordinate display
    m_coordLabel = new QLabel("Lat: 0.000000, Lon: 0.000000", this);
    m_coordLabel->setMinimumWidth(200);
    statusBar()->addWidget(m_coordLabel);
    
    statusBar()->addPermanentWidget(new QLabel(" | "));
    
    // Zoom level display
    m_zoomLabel = new QLabel("Zoom: 1", this);
    m_zoomLabel->setMinimumWidth(80);
    statusBar()->addPermanentWidget(m_zoomLabel);
    
    statusBar()->addPermanentWidget(new QLabel(" | "));
    
    // Scale indicator
    m_scaleLabel = new QLabel("Scale: 1:1000000", this);
    m_scaleLabel->setMinimumWidth(120);
    statusBar()->addPermanentWidget(m_scaleLabel);
}

void MainWindow::setupDockingSystem()
{
    // Create dock manager
    m_dockManager = new ads::CDockManager(this);
    
    // Create map dock widget
    m_mapDockWidget = new ads::CDockWidget("Map View");
    m_mapDockWidget->setWidget(m_mapWidget);
    m_mapDockWidget->setFeature(ads::CDockWidget::DockWidgetClosable, false);
    
    // Add map as central widget
    m_dockManager->addDockWidget(ads::CenterDockWidgetArea, m_mapDockWidget);
}

void MainWindow::onMapLayerChanged(int index)
{
    QString layerType = m_layerCombo->itemData(index).toString();
    m_mapWidget->setMapLayer(layerType);
}

void MainWindow::onCoordinateUpdate(double latitude, double longitude)
{
    m_coordLabel->setText(QString("Lat: %1, Lon: %2")
                         .arg(latitude, 0, 'f', 6)
                         .arg(longitude, 0, 'f', 6));
}

void MainWindow::onZoomChanged(int zoom)
{
    m_zoomLabel->setText(QString("Zoom: %1").arg(zoom));
    
    // Update scale (approximate)
    double scale = 591657527.591555 / qPow(2, zoom);
    m_scaleLabel->setText(QString("Scale: 1:%1").arg(qRound(scale)));
}

#include "MainWindow.moc"