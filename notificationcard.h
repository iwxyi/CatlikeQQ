#ifndef NOTIFICATIONCARD_H
#define NOTIFICATIONCARD_H

#include <QWidget>

namespace Ui {
class NotificationCard;
}

class NotificationCard : public QWidget
{
    Q_OBJECT

public:
    explicit NotificationCard(QWidget *parent = nullptr);
    ~NotificationCard();

private:
    Ui::NotificationCard *ui;
};

#endif // NOTIFICATIONCARD_H
