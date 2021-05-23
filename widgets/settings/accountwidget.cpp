#include <QRegularExpression>
#include "accountwidget.h"
#include "ui_accountwidget.h"
#include "stringutil.h"
#include "signaltransfer.h"
#include "usettings.h"

AccountWidget::AccountWidget(CqhttpService *service, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AccountWidget), service(service)
{
    ui->setupUi(this);
    resotreSettings();

    connect(sig, &SignalTransfer::socketStateChanged, this, [=](bool connected) {
        ui->connectStateLabel->setText(connected ? "已连接" : "已断开");
    });

    connect(sig, &SignalTransfer::myAccount, this, [=](qint64 id, QString nickname) {
        ui->accountLabel->setText(nickname + " (" + QString::number(id) + ")");
    });
}

AccountWidget::~AccountWidget()
{
    delete ui;
}

void AccountWidget::resotreSettings()
{
    QString host = us->host;
    // 加密
    QRegularExpressionMatch match;
    if (host.indexOf(QRegularExpression(":(\\d+)"), 0, &match) > -1)
        host.replace(match.captured(1), repeatString("*", match.captured(1).length()));
    if (host.indexOf(QRegularExpression("://(\\w+)"), 0, &match) > -1)
        host.replace(match.captured(1), repeatString("*", match.captured(1).length()));
    ui->hostEdit->setText(host);

    QString token = us->accessToken;
    ui->tokenEdit->setText(token.replace(QRegExp("\\w"), "*"));
}

void AccountWidget::on_hostEdit_editingFinished()
{
    QString host = ui->hostEdit->text().trimmed();
    if (host == us->host || host.contains("*"))
        return ;

    // 自动填充前缀
    if (!host.startsWith("ws"))
        host = "ws://" + host;

    us->set("net/host", us->host = host);
    emit sig->hostChanged(us->host, us->accessToken);
}

void AccountWidget::on_tokenEdit_editingFinished()
{
    QString token = ui->tokenEdit->text();
    if (token == us->accessToken || token.contains("*"))
        return ;

    us->set("net/accessToken", us->accessToken = token);
    emit sig->hostChanged(us->host, us->accessToken);
}
