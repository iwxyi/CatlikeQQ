#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include "defines.h"
#include "cqhttpservice.h"
#include "notificationcard.h"
#include "notificationbubble.h"
#if defined(ENABLE_SHORTCUT)
#include "qxtglobalshortcut.h"
#endif

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
    void initKey();

    void startMessageLoop();

    QRect screenGeometry() const;

public slots:
    void showMessage(const MsgBean& msg, bool blockSelf = true);
    void createNotificationBanner(const MsgBean& msg);
    void adjustUnderCardsTop(int aboveIndex, int deltaHeight);
    void focusCardReply();
    void closeAllCard();

    void autoReplyMessage(const MsgBean& msg);
    void triggerAiReply(const MsgBean& msg, int retry = 0);

protected:
    void closeEvent(QCloseEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;

private:
    void returnToPrevWindow();

private:
    Ui::MainWindow *ui;
    InteractiveButtonBase* confirmButton = nullptr;
    CqhttpService* service;

    QList<NotificationCard*> notificationCards;
    QList<NotificationBubble*> notificationBubbles;
#if defined(ENABLE_SHORTCUT)
    QxtGlobalShortcut* editShortcut;
#endif
#ifdef Q_OS_WIN32
    HWND prevWindow = nullptr;
#endif
};
#endif // MAINWINDOW_H
