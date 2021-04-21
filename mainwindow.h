#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include "global.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QListWidgetItem;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void trayAction(QSystemTrayIcon::ActivationReason reason);
    void on_sideButtons_currentRowChanged(int currentRow);

private:
    void loadSettingsTabs();
    void loadAuxiliaryTabs();
    void loadDataTabs();
    void initTray();

protected:
    void closeEvent(QCloseEvent* e) override;

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
