#include "debugwidget.h"
#include "ui_debugwidget.h"
#include "usettings.h"
#include "netutil.h"
#include "myjson.h"
#include "accountinfo.h"

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

void DebugWidget::test()
{
    QString path = "B:/Projects/CatlikeQQ_Run/release/data/cache/images/CBF1F5F43AECF94EDA67872CA9D54D41.amr";

    QFile file(path);
    file.open(QFile::ReadOnly);
    QByteArray ba = file.readAll();
    file.close();
    QByteArray base64 = ba.toBase64();

    MyJson json;
    json.insert("format", "pcm");
    json.insert("rate", 16000);
    json.insert("channel", 1);
    json.insert("cuid", snum(ac->myId));
    json.insert("token", us->baiduSpeechAccessToken);
    json.insert("dev_pid", 1537);
    json.insert("len", file.size());
    qDebug() << "参数：" << json;
    json.insert("speech", QString(base64));

    QByteArray rst = NetUtil::postJsonData("http://vop.baidu.com/server_api", json);
    qDebug() << "返回：" << rst;

    json = MyJson(rst);
    qDebug() << "回复：" << json;
}

void DebugWidget::on_showWidgetBorderCheck_clicked()
{
    us->set("debug/showWidgetBorder", us->showWidgetBorder = ui->showWidgetBorderCheck->isChecked());
}
