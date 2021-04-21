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

void NotificationCard::set(const MsgBean &msg)
{
    this->userId = msg.senderId;
    this->groupId = msg.groupId;
    msgs.append(msg);

    // 显示
    showText = msg.message;
    showText.replace("<", "&lt;").replace(">", "&gt;");
    ui->messageLabel->setText(showText);
}

bool NotificationCard::append(const MsgBean &msg)
{
    if (this->userId != msg.senderId || this->groupId != msg.groupId)
        return false;

    // 插入到自己的消息
    msgs.append(msg);

    // 段落显示
    QString s = msg.message;
    s.replace("<", "&lt;").replace(">", "&gt;");
    showText.append("<p>" + s + "</p>");
    ui->messageLabel->setText(showText);
    return true;
}
