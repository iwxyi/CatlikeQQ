#include <QDebug>
#include "replywidget.h"
#include "ui_replywidget.h"
#include "defines.h"
#include "usettings.h"
#include "signaltransfer.h"

ReplyWidget::ReplyWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ReplyWidget)
{
    ui->setupUi(this);

    ui->autoShowReplyCheck->setChecked(us->bannerAutoShowReply);
    ui->autoFocusReplyCheck->setChecked(us->bannerAutoFocusReply);
    ui->closeAfterReplyCheck->setChecked(us->bannerCloseAfterReply);
    ui->replyKeyEdit->setKeySequence(QKeySequence(us->s("banner/replyKey", "shift+alt+x")));
    ui->AIReplyCheck->setChecked(us->bannerAIReply);
    ui->showMySendCheck->setChecked(us->bannerShowMySend);
    ui->groupEmojiToImageCheck->setChecked(us->groupEmojiToImage);
    ui->sendImagesDirectlyCheck->setChecked(us->sendImagesDirectly);
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

void ReplyWidget::on_replyKeyEdit_editingFinished()
{
    QString key = ui->replyKeyEdit->keySequence().toString();
    qInfo() << "设置回复快捷键：" << key;
    us->set("banner/replyKey", key);
    emit sig->setReplyKey(key);
}

void ReplyWidget::on_AIReplyCheck_clicked()
{
    us->set("banner/AIReply", us->bannerAIReply = ui->AIReplyCheck->isChecked());
}

void ReplyWidget::on_showMySendCheck_clicked()
{
    us->set("banner/showMySend", us->bannerShowMySend = ui->showMySendCheck->isChecked());
}

void ReplyWidget::on_groupEmojiToImageCheck_clicked()
{
    us->set("send/groupEmojiToImage", us->groupEmojiToImage = ui->groupEmojiToImageCheck->isChecked());
}

void ReplyWidget::on_sendImagesDirectlyCheck_clicked()
{
    us->set("send/sendImagesDirectly", us->sendImagesDirectly = ui->sendImagesDirectlyCheck->isChecked());
}
