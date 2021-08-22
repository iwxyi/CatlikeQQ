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
    qint64 uid = 123456;
    int r = qrand() % 4;
    if (r == 1)
        uid = 234567;
    else if (r == 2)
        uid = 345678;
    else if (r == 3)
        uid = 456789;

    MsgBean msg(uid, "测试用户", ui->messageEdit->toPlainText(), -654321, "friend");
    if (ui->groupRadio->isChecked())
    {
        msg.group(uid, "测试群名", "测试昵称");
    }

    if (ui->msgRadio->isChecked())
    {
        if (msg.message.isEmpty())
            msg.message = "测试消息" + QString::number(uid);
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
