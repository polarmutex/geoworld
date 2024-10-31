#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QComboBox>
#include <QMainWindow>
#include <QWidgetAction>

#include "qt6advanceddocking/DockAreaWidget.h"
#include "qt6advanceddocking/DockManager.h"
#include "qt6advanceddocking/DockWidget.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class CMainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

protected:
  virtual void closeEvent(QCloseEvent *event) override;

private:
  QAction *SavePerspectiveAction = nullptr;
  QWidgetAction *PerspectiveListAction = nullptr;
  QComboBox *PerspectiveComboBox = nullptr;

  Ui::CMainWindow *ui;

  ads::CDockManager *DockManager;
  ads::CDockAreaWidget *StatusDockArea;
  ads::CDockWidget *TimelineDockWidget;

  void createPerspectiveUi();

private slots:
  void savePerspective();
};
#endif // MAINWINDOW_H
