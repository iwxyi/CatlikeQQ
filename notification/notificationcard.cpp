#include "notificationcard.h"
#include "ui_notificationcard.h"

NotificationCard::NotificationCard(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NotificationCard)
{
    ui->setupUi(this);
}

NotificationCard::~NotificationCard()
{
    delete ui;
}
