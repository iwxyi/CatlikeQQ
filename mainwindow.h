#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include "global.h"
#include "cqhttpservice.h"
#include "notificationcard.h"
#include "notificationbubble.h"

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
    void initView();
    void initTray();
    void initService();

    void startMessageLoop();

    QRect screenGeometry() const;

public slots:
    void showMessage(const MsgBean& msg);
    void createNotificationBanner(const MsgBean& msg);
    void adjustUnderCardsTop(int aboveIndex, int deltaHeight);

protected:
    void closeEvent(QCloseEvent* e) override;

private:
    Ui::MainWindow *ui;
    CqhttpService* service;

    QList<NotificationCard*> notificationCards;
    QList<NotificationBubble*> notificationBubbles;
};
#endif // MAINWINDOW_H
