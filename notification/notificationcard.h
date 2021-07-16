#ifndef NOTIFICATIONCARD_H
#define NOTIFICATIONCARD_H

#include <QWidget>
#include <QTimer>
#include <QGraphicsDropShadowEffect>
#include <QListWidgetItem>
#include "interactivebuttonbase.h"
#include "msgbean.h"
#include "defines.h"
#include "messageview.h"
#include "accountinfo.h"
#include "usettings.h"
#include "runtime.h"
#include "signaltransfer.h"

#define CREATE_SHADOW(x)                                                  \
do {                                                                      \
    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(x); \
    effect->setOffset(3, 3);                                              \
    effect->setBlurRadius(12);                                            \
    effect->setColor(QColor(128, 128, 128, 128));                         \
    x->setGraphicsEffect(effect);                                         \
} while(0)

namespace Ui {
class NotificationCard;
}

class NotificationCard : public QWidget
{
    Q_OBJECT

public:
    explicit NotificationCard(QWidget *parent = nullptr);
    ~NotificationCard() override;

    void setShowPos(QPoint startPos, QPoint showPos);
    void showFrom();
    void setColors(QColor bg, QColor title, QColor content);
    void setMsg(const MsgBean& msg);
    bool append(const MsgBean& msg);
    void adjustTop(int delta);

    bool isPrivate() const;
    bool isGroup() const;
    bool isHidding() const;
    bool canMerge() const;
    bool isFixing() const;
    void setFastFocus();
    bool is(const MsgBean& msg) const;

    const QList<MsgBean> &getMsgs() const;
    int getImportance() const;
    static QString getValiableMessage(QString text);

signals:
    void signalHeightChanged(int delta);
    void signalToHide();
    void signalHided();
    void signalReplyPrivate(qint64 friendId, const QString& message);
    void signalReplyGroup(qint64 groupId, const QString& message);
    void signalCancelReply();
    void signalFocusPrevCard();
    void signalFocusNextCard();
    void signalFocusCard(int index);
    void signalCloseAllCards();

public slots:
    void showReplyEdit();
    void showReplyEdit(bool focus, bool selectAll = true);
    void hideReplyEdit();
    void toHide();
    void triggerAIReply(int retry = 0);
    void shallToHide();

    void showGrougInfo(qint64 groupId, QPoint pos = QPoint(-1, -1));
    void showUserInfo(qint64 friendId, QPoint pos = QPoint(-1, -1));

private slots:
    void mouseEnter();
    void mouseLeave();
    void displayTimeout();
    void focusIn();
    void focusOut();
    void sendReply();
    void cardClicked();
    void cardMenu();
    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);
    void loadMsgHistory();

private:
    void setPrivateMsg(const MsgBean& msg);
    void setGroupMsg(const MsgBean& msg);
    void appendPrivateMsg(const MsgBean& msg);
    void appendGroupMsg(const MsgBean& msg);
    void addSingleSenderMsg(const MsgBean& msg);
    void createPureMsgView(const MsgBean &msg, int index = -1);
    void createMsgBox(const MsgBean &msg, int index = -1);
    void createBlankMsgBox(const MsgBean &msg, int index = -1);
    MessageView* newMsgView();
    void connectGroupHeader(QLabel* label, const MsgBean &msg);
    void connectUserHeader(QLabel *label, const MsgBean &msg);
    void connectUserName(QLabel* label, const MsgBean &msg);
    int getReadDisplayDuration(QString text) const;
    void createFrostGlass();
    void suspendHide();

protected:
    void showEvent(QShowEvent *event) override;
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    Ui::NotificationCard *ui;

    qint64 friendId = 0;
    qint64 groupId = 0;
    QList<MsgBean> msgs; // 可能会合并多条消息
    QList<MessageView*> msgViews;

    InteractiveButtonBase* bg;
    QPoint showPoint;
    QPoint hidePoint;
    QTimer* displayTimer;
    bool focusing = false;
    bool hidding = false;
    bool fastFocus = false;
    bool fixing = false; // 固定不自动隐藏
    AccountInfo::CardColor cardColor;
    QLabel* frostGlassLabel = nullptr;
    QPixmap frostGlassPixmap;
};

#endif // NOTIFICATIONCARD_H
