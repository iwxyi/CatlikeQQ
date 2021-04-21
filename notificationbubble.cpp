#include "notificationbubble.h"
#include "ui_notificationbubble.h"

NotificationBubble::NotificationBubble(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NotificationBubble)
{
    ui->setupUi(this);
}

NotificationBubble::~NotificationBubble()
{
    delete ui;
}
