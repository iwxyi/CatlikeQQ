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
#include "remotecontrolservie.h"

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
    void showHistoryListMenu();
    bool canNewCardShow(const MsgBean& msg) const;

    QRect screenGeometry() const;
    void adjustUnderCardsTop(int aboveIndex, int deltaHeight);
    void adjustAboveCardsTop(int underIndex, int deltaHeight);

public slots:
    void messageReceived(const MsgBean& msg, bool blockSelf = true);
    NotificationCard *showMessageCard(const MsgBean& msg, bool blockSelf = false);
    NotificationCard *createNotificationCard(const MsgBean& msg);
    void focusOrShowMessageCard(const MsgBean& msg, bool focusEdit, const QString& insertText = "");
    void slotCardHeightChanged(NotificationCard* card, int deltaHeight);
    void slotCardHeightChanged(int index, int deltaHeight);
    void focusCardReply();
    void closeAllCard();

    void autoReplyMessage(const MsgBean& msg);
    void triggerAiReply(const MsgBean& msg, int retry = 0);

protected:
    void showEvent(QShowEvent* e) override;
    void closeEvent(QCloseEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;

private:
    void returnToPrevWindow();

private:
    Ui::MainWindow *ui;
    InteractiveButtonBase* confirmButton = nullptr;
    CqhttpService* cqhttpService;
    RemoteControlServie* remoteControlService;

    QList<NotificationCard*> notificationCards;
    QList<NotificationBubble*> notificationBubbles;
#if defined(ENABLE_SHORTCUT)
    QxtGlobalShortcut* editShortcut;
#endif
#ifdef Q_OS_WIN32
    HWND prevWindow = nullptr;
#endif
    QSystemTrayIcon* tray;
};
#endif // MAINWINDOW_H
