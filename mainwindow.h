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
#include "devcoderunner.h"

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
    NotificationCard *showMessageCard(const MsgBean& msg, bool blockSelf = false, bool showHistory = false);
    NotificationCard *createNotificationCard(const MsgBean& msg, bool showHistory = false);
    NotificationCard *focusOrShowMessageCard(const MsgBean& msg, bool focusEdit, const QString& insertText = "", bool showHistory = false);
    void slotCardHeightChanged(NotificationCard* card, int deltaHeight);
    void slotCardHeightChanged(int index, int deltaHeight);
    void focusCardReply();
    void closeAllCard();
    void setLastOpenMsg(const MsgBean& msg);

    void autoReplyMessage(const MsgBean& msg);
    void triggerAiReply(const MsgBean& msg, int retry = 0);
    void showTrayIcon(const MsgBean& msg) const;

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
    mutable MsgBean currentTrayMsg; // 当前头像
    mutable QPixmap trayFlashPixmap; // 正在闪烁的图像
    mutable bool trayHiding = false;
    QTimer* trayRestoreTimer; // 显示结束后回复
    QTimer* trayHideTimer; // 闪烁一下表示有消息

    DevCodeRunner codeRunner;
};
#endif // MAINWINDOW_H
