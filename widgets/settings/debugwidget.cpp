#include "debugwidget.h"
#include "ui_debugwidget.h"

DebugWidget::DebugWidget(CqhttpService *service, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DebugWidget), service(service)
{
    ui->setupUi(this);
}

DebugWidget::~DebugWidget()
{
    delete ui;
}

void DebugWidget::on_sendButton_clicked()
{
    MsgBean msg(123456, "测试用户", ui->messageEdit->toPlainText(), -654321, "friend");
    if (ui->groupRadio->isChecked())
    {
        msg.group(123456, "测试群名", "测试昵称");
    }

    if (ui->msgRadio->isChecked())
    {

    }
    else if (ui->imgRadio->isChecked())
    {

    }
    else if (ui->fileRadio->isChecked())
    {
        msg.file("123/qwe/456/asd/789/zxc", "测试文件名", 8888);
    }

    emit service->signalMessage(msg);
}
