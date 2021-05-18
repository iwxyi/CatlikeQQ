#include "leavemodewidget.h"
#include "ui_leavemodewidget.h"
#include "usettings.h"

LeaveModeWidget::LeaveModeWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LeaveModeWidget)
{
    ui->setupUi(this);

    ui->leaveModelCheck->setChecked(us->leaveMode);
    ui->aiReplyPrivateCheck->setChecked(us->aiReplyPrivate);
    ui->aiReplyPrefixEdit->setPlainText(us->aiReplyPrefix);
    ui->aiReplySuffixEdit->setPlainText(us->aiReplySuffix);
    ui->aiReplyDefaultEdit->setPlainText(us->aiReplyDefault);
    ui->aiReplyIntervalSpin->setValue(us->aiReplyInterval / 1000);
}

LeaveModeWidget::~LeaveModeWidget()
{
    delete ui;
}

void LeaveModeWidget::on_leaveModelCheck_clicked()
{
    us->set("leave/leaveMode", us->leaveMode = ui->leaveModelCheck->isChecked());
}

void LeaveModeWidget::on_aiReplyPrivateCheck_clicked()
{
    us->set("leave/aiReplyPrivate", us->aiReplyPrivate = ui->aiReplyPrivateCheck->isChecked());
}

void LeaveModeWidget::on_aiReplyPrefixEdit_undoAvailable(bool)
{
    us->set("leave/aiReplyPrefix", us->aiReplyPrefix = ui->aiReplyPrefixEdit->toPlainText());
}

void LeaveModeWidget::on_aiReplySuffixEdit_undoAvailable(bool)
{
    us->set("leave/aiReplySuffix", us->aiReplySuffix = ui->aiReplySuffixEdit->toPlainText());
}

void LeaveModeWidget::on_aiReplyDefaultEdit_undoAvailable(bool)
{
    us->set("leave/aiReplyDefault", us->aiReplyDefault = ui->aiReplyDefaultEdit->toPlainText());
}

void LeaveModeWidget::on_aiReplyIntervalSpin_editingFinished()
{
    us->set("leave/aiReplyInterval", us->aiReplyInterval = ui->aiReplyIntervalSpin->value() * 10);
}
