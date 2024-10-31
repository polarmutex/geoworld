#include "MainWindow.h"

#include "ui_MainWindow.h"

#include <QCalendarWidget>
#include <QFileDialog>
#include <QFileSystemModel>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSettings>
#include <QTableWidget>
#include <QToolBar>
#include <QTreeView>
#include <QWidgetAction>

#include "qt6advanceddocking/DockAreaTabBar.h"
#include "qt6advanceddocking/DockAreaTitleBar.h"
#include "qt6advanceddocking/DockAreaWidget.h"
#include "qt6advanceddocking/DockComponentsFactory.h"
#include "qt6advanceddocking/FloatingDockContainer.h"

using namespace ads;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::CMainWindow) {
  ui->setupUi(this);
  CDockManager::setConfigFlag(CDockManager::OpaqueSplitterResize, true);
  CDockManager::setConfigFlag(CDockManager::XmlCompressionEnabled, false);
  CDockManager::setConfigFlag(CDockManager::FocusHighlighting, true);
  DockManager = new CDockManager(this);

  // Set central widget
  QPlainTextEdit *w = new QPlainTextEdit();
  w->setPlaceholderText("This is the central editor. Enter your text here.");
  CDockWidget *CentralDockWidget = new CDockWidget("CentralWidget");
  CentralDockWidget->setWidget(w);
  auto *CentralDockArea = DockManager->setCentralWidget(CentralDockWidget);
  CentralDockArea->setAllowedAreas(DockWidgetArea::OuterDockAreas);

  // create other dock widgets
  QTableWidget *table = new QTableWidget();
  table->setColumnCount(3);
  table->setRowCount(10);
  CDockWidget *TableDockWidget = new CDockWidget("Table 1");
  TableDockWidget->setWidget(table);
  TableDockWidget->setMinimumSizeHintMode(
      CDockWidget::MinimumSizeHintFromDockWidget);
  TableDockWidget->resize(250, 150);
  TableDockWidget->setMinimumSize(200, 150);
  auto TableArea = DockManager->addDockWidget(
      DockWidgetArea::LeftDockWidgetArea, TableDockWidget);
  ui->menuView->addAction(TableDockWidget->toggleViewAction());

  table = new QTableWidget();
  table->setColumnCount(5);
  table->setRowCount(1020);
  TableDockWidget = new CDockWidget("Table 2");
  TableDockWidget->setWidget(table);
  TableDockWidget->setMinimumSizeHintMode(
      CDockWidget::MinimumSizeHintFromDockWidget);
  TableDockWidget->resize(250, 150);
  TableDockWidget->setMinimumSize(200, 150);
  DockManager->addDockWidget(DockWidgetArea::BottomDockWidgetArea,
                             TableDockWidget, TableArea);
  ui->menuView->addAction(TableDockWidget->toggleViewAction());

  QTableWidget *propertiesTable = new QTableWidget();
  propertiesTable->setColumnCount(3);
  propertiesTable->setRowCount(10);
  CDockWidget *PropertiesDockWidget = new CDockWidget("Properties");
  PropertiesDockWidget->setWidget(propertiesTable);
  PropertiesDockWidget->setMinimumSizeHintMode(
      CDockWidget::MinimumSizeHintFromDockWidget);
  PropertiesDockWidget->resize(250, 150);
  PropertiesDockWidget->setMinimumSize(200, 150);
  DockManager->addDockWidget(DockWidgetArea::RightDockWidgetArea,
                             PropertiesDockWidget, CentralDockArea);
  ui->menuView->addAction(PropertiesDockWidget->toggleViewAction());

  createPerspectiveUi();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::createPerspectiveUi() {
  SavePerspectiveAction = new QAction("Create Perspective", this);
  connect(SavePerspectiveAction, SIGNAL(triggered()), SLOT(savePerspective()));
  PerspectiveListAction = new QWidgetAction(this);
  PerspectiveComboBox = new QComboBox(this);
  PerspectiveComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  PerspectiveComboBox->setSizePolicy(QSizePolicy::Preferred,
                                     QSizePolicy::Preferred);
  connect(PerspectiveComboBox, SIGNAL(currentTextChanged(const QString &)),
          DockManager, SLOT(openPerspective(const QString &)));
  PerspectiveListAction->setDefaultWidget(PerspectiveComboBox);
  ui->toolBar->addSeparator();
  ui->toolBar->addAction(PerspectiveListAction);
  ui->toolBar->addAction(SavePerspectiveAction);
}

void MainWindow::savePerspective() {
  QString PerspectiveName =
      QInputDialog::getText(this, "Save Perspective", "Enter unique name:");
  if (PerspectiveName.isEmpty()) {
    return;
  }

  DockManager->addPerspective(PerspectiveName);
  QSignalBlocker Blocker(PerspectiveComboBox);
  PerspectiveComboBox->clear();
  PerspectiveComboBox->addItems(DockManager->perspectiveNames());
  PerspectiveComboBox->setCurrentText(PerspectiveName);
}

//============================================================================
void MainWindow::closeEvent(QCloseEvent *event) {
  // Delete dock manager here to delete all floating widgets. This ensures
  // that all top level windows of the dock manager are properly closed
  DockManager->deleteLater();
  QMainWindow::closeEvent(event);
}
