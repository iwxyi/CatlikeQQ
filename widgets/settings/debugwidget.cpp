#include "debugwidget.h"
#include "ui_debugwidget.h"
#include "usettings.h"

DebugWidget::DebugWidget(CqhttpService *service, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DebugWidget), service(service)
{
    ui->setupUi(this);

    ui->showWidgetBorderCheck->setChecked(us->showWidgetBorder);
    ui->sendButton->setBorderColor(Qt::gray);
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
        if (msg.message.isEmpty())
            msg.message = "测试消息123456";
    }
    else if (ui->imgRadio->isChecked())
    {

    }
    else if (ui->fileRadio->isChecked())
    {
        msg.file("123/qwe/456/asd/789/zxc", "测试文件名", 8888);
    }

    // service-parseMsgDisplay(msg);
    emit service->signalMessage(msg);
}

void DebugWidget::on_showWidgetBorderCheck_clicked()
{
    us->set("debug/showWidgetBorder", us->showWidgetBorder = ui->showWidgetBorderCheck->isChecked());
}
