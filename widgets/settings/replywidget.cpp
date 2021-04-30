#include "replywidget.h"
#include "ui_replywidget.h"
#include "defines.h"
#include "usettings.h"

ReplyWidget::ReplyWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ReplyWidget)
{
    ui->setupUi(this);

    ui->autoShowReplyCheck->setChecked(us->bannerAutoShowReply);
    ui->autoFocusReplyCheck->setChecked(us->bannerAutoFocusReply);
    ui->closeAfterReplyCheck->setChecked(us->bannerCloseAfterReply);
}

ReplyWidget::~ReplyWidget()
{
    delete ui;
}

void ReplyWidget::on_autoShowReplyCheck_clicked()
{
    us->set("banner/autoShowReply", us->bannerAutoShowReply = ui->autoShowReplyCheck->isChecked());
}

void ReplyWidget::on_closeAfterReplyCheck_clicked()
{
    us->set("banner/closeAfterReply", us->bannerCloseAfterReply = ui->closeAfterReplyCheck->isChecked());
}

void ReplyWidget::on_autoFocusReplyCheck_clicked()
{
    us->set("banner/autoFocusReply", us->bannerAutoFocusReply = ui->autoFocusReplyCheck->isChecked());
}
