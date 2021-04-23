#ifndef NOTIFICATIONCARD_H
#define NOTIFICATIONCARD_H

#include <QWidget>
#include <QTimer>
#include <QGraphicsDropShadowEffect>
#include "interactivebuttonbase.h"
#include "msgbean.h"
#include "global.h"

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

    void setMsg(const MsgBean& msg);
    bool append(const MsgBean& msg, int& delta);

    bool isHidding() const;
    bool canMerge() const;

signals:
    void signalToHide();
    void signalHided();
    void signalReplyPrivate(qint64 userId, const QString& message);
    void signalReplyGroup(qint64 groupId, const QString& message);

private slots:
    void focusIn();
    void focusOut();
    void showReplyEdit();
    void sendReply();
    void toHide();

protected:
    void showEvent(QShowEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    Ui::NotificationCard *ui;

    qint64 userId = 0;
    qint64 groupId = 0;
    QList<MsgBean> msgs; // 可能会合并多条消息
    QString showText;

    InteractiveButtonBase* bg;
    QPoint hidePoint;
    QTimer* displayTimer;
    bool focusing = false;
    bool hidding = false;
    bool single = false;
};

#endif // NOTIFICATIONCARD_H
