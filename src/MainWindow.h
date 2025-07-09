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
#include "PluginManager.h"

class IPlugin;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onCoordinateUpdate(double latitude, double longitude);
    void onZoomChanged(int zoom);
    void onPluginLoaded(const QString& name);
    void onPluginUnloaded(const QString& name);

private:
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupDockingSystem();
    void setupPlugins();
    void loadMapPlugin();
    
    // UI Components
    QWidget *m_mapWidget;
    ads::CDockManager *m_dockManager;
    PluginManager *m_pluginManager;
    
    // Status bar components
    QLabel *m_coordLabel;
    QLabel *m_zoomLabel;
    QLabel *m_scaleLabel;
    
    // Dock widgets
    ads::CDockWidget *m_mapDockWidget;
};