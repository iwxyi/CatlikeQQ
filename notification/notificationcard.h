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

    void showFrom(QPoint hi, QPoint sh);
    void setColors(QColor bg, QColor fg);
    void setMsg(const MsgBean& msg);
    bool append(const MsgBean& msg);
    void adjustTop(int delta);

    bool isPrivate() const;
    bool isGroup() const;
    bool isHidding() const;
    bool canMerge() const;
    void setFastFocus();

    const QList<MsgBean> &getMsgs() const;

signals:
    void signalHeightChanged(int delta);
    void signalToHide();
    void signalHided();
    void signalReplyPrivate(qint64 senderId, const QString& message);
    void signalReplyGroup(qint64 groupId, const QString& message);
    void signalCancelReply();

public slots:
    void showReplyEdit();
    void showReplyEdit(bool focus);
    void hideReplyEdit();

private slots:
    void mouseEnter();
    void mouseLeave();
    void focusIn();
    void focusOut();
    void sendReply();
    void toHide();
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
    void createMsgEdit(const MsgBean &msg, int index = -1);
    void createMsgBox(const MsgBean &msg, int index = -1);
    void createMsgBoxEdit(const MsgBean &msg, int index = -1);
    MessageView* newMsgView();
    int getReadDisplayDuration(QString text) const;

protected:
    void showEvent(QShowEvent *event) override;
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    Ui::NotificationCard *ui;

    qint64 senderId = 0;
    qint64 groupId = 0;
    QList<MsgBean> msgs; // 可能会合并多条消息
    QList<MessageView*> msgViews;

    InteractiveButtonBase* bg;
    QPoint showPoint;
    QPoint hidePoint;
    QTimer* displayTimer;
    bool focusing = false;
    bool hidding = false;
    bool single = false;
    AccountInfo::CardColor cardColor;

    bool fastFocus = false;
};

#endif // NOTIFICATIONCARD_H
