#include "MainWindow.h"
#include "IPlugin.h"
#include <QApplication>
#include <QAction>
#include <QMenu>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QMessageBox>
#include <QLabel>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_mapWidget(nullptr)
    , m_dockManager(nullptr)
    , m_pluginManager(nullptr)
    , m_coordLabel(nullptr)
    , m_zoomLabel(nullptr)
    , m_scaleLabel(nullptr)
    , m_mapDockWidget(nullptr)
{
    setWindowTitle("GeoWorld - Geospatial Data Visualization");
    setMinimumSize(1200, 800);
    
    setupPlugins();
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
    
    // Load map plugin
    loadMapPlugin();
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
    
    QAction *refreshAction = new QAction("&Refresh Plugins", this);
    refreshAction->setShortcut(QKeySequence("F5"));
    connect(refreshAction, &QAction::triggered, [this]() {
        if (m_pluginManager) {
            m_pluginManager->unloadPlugins();
            m_pluginManager->loadPlugins();
        }
    });
    viewMenu->addAction(refreshAction);
    
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
    
    // General application actions only
    QAction *aboutAction = new QAction("About", this);
    connect(aboutAction, &QAction::triggered, [this]() {
        QMessageBox::about(this, "About GeoWorld", 
            "GeoWorld v1.0.0\nGeospatial Data Visualization Platform\nPlugin-based Architecture");
    });
    mainToolBar->addAction(aboutAction);
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

void MainWindow::setupPlugins()
{
    m_pluginManager = new PluginManager(this);
    
    // Connect plugin manager signals
    connect(m_pluginManager, &PluginManager::pluginLoaded,
            this, &MainWindow::onPluginLoaded);
    connect(m_pluginManager, &PluginManager::pluginUnloaded,
            this, &MainWindow::onPluginUnloaded);
    
    // Load plugins from standard locations
    m_pluginManager->loadPlugins();
}

void MainWindow::loadMapPlugin()
{
    // Try to find any map plugin
    QStringList plugins = m_pluginManager->availablePlugins();
    IPlugin* mapPlugin = nullptr;
    
    for (const QString& pluginName : plugins) {
        IPlugin* plugin = m_pluginManager->getPlugin(pluginName);
        if (plugin && plugin->capabilities().contains("mapping")) {
            mapPlugin = plugin;
            break;
        }
    }
    
    if (mapPlugin) {
        QWidget* mapWidget = mapPlugin->createWidget(this);
        if (mapWidget) {
            // Try to connect signals using QObject::connect with string-based connection
            QObject::connect(mapWidget, SIGNAL(coordinateChanged(double, double)), 
                           this, SLOT(onCoordinateUpdate(double, double)));
            QObject::connect(mapWidget, SIGNAL(zoomChanged(int)), 
                           this, SLOT(onZoomChanged(int)));
            
            m_mapWidget = mapWidget;
        }
    }
    
    // Display error if no plugin found
    if (!m_mapWidget) {
        qCritical() << "No map plugin found! Please ensure map plugin is installed.";
        // Create a simple error label instead of fallback
        QLabel* errorLabel = new QLabel("No map plugin found!\nPlease install a map plugin.", this);
        errorLabel->setAlignment(Qt::AlignCenter);
        errorLabel->setStyleSheet("QLabel { color: red; font-size: 16px; }");
        m_mapWidget = errorLabel;
    }
}

void MainWindow::onPluginLoaded(const QString& name)
{
    qDebug() << "Plugin loaded:" << name;
    statusBar()->showMessage(QString("Plugin loaded: %1").arg(name), 3000);
}

void MainWindow::onPluginUnloaded(const QString& name)
{
    qDebug() << "Plugin unloaded:" << name;
    statusBar()->showMessage(QString("Plugin unloaded: %1").arg(name), 3000);
}