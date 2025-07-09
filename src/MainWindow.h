#pragma once

#include <QMainWindow>
#include <QStatusBar>
#include <QLabel>
#include <QComboBox>
#include <QToolBar>
#include <QMenuBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <qt6advanceddocking/DockManager.h>
#include <qt6advanceddocking/DockWidget.h>
#include <qt6advanceddocking/DockAreaWidget.h>

class MapWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onMapLayerChanged(int index);
    void onCoordinateUpdate(double latitude, double longitude);
    void onZoomChanged(int zoom);

private:
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupDockingSystem();
    
    // UI Components
    MapWidget *m_mapWidget;
    ads::CDockManager *m_dockManager;
    
    // Status bar components
    QLabel *m_coordLabel;
    QLabel *m_zoomLabel;
    QLabel *m_scaleLabel;
    
    // Toolbar components
    QComboBox *m_layerCombo;
    
    // Dock widgets
    ads::CDockWidget *m_mapDockWidget;
};