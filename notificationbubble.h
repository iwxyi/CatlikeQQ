#ifndef NOTIFICATIONBUBBLE_H
#define NOTIFICATIONBUBBLE_H

#include <QWidget>

namespace Ui {
class NotificationBubble;
}

class NotificationBubble : public QWidget
{
    Q_OBJECT

public:
    explicit NotificationBubble(QWidget *parent = nullptr);
    ~NotificationBubble();

private:
    Ui::NotificationBubble *ui;
};

#endif // NOTIFICATIONBUBBLE_H
