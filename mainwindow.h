#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
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
    ~MainWindow();

private slots:
    void on_sideButtons_currentRowChanged(int currentRow);

private:
    void loadSettingsTabs();
    void loadAuxiliaryTabs();
    void loadDataTabs();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
