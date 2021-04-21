#ifndef NOTIFICATIONCARD_H
#define NOTIFICATIONCARD_H

#include <QWidget>
#include "msgbean.h"

namespace Ui {
class NotificationCard;
}

class NotificationCard : public QWidget
{
    Q_OBJECT

public:
    explicit NotificationCard(QWidget *parent = nullptr);
    ~NotificationCard();

    void set(const MsgBean& msg);
    bool append(const MsgBean& msg);

private:
    Ui::NotificationCard *ui;

    qint64 userId = 0;
    qint64 groupId = 0;
    QList<MsgBean> msgs; // 可能会合并多条消息
    QString showText;
};

#endif // NOTIFICATIONCARD_H
