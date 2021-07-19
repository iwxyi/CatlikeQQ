#include <QRegularExpression>
#include <QIcon>
#include "accountwidget.h"
#include "ui_accountwidget.h"
#include "stringutil.h"
#include "signaltransfer.h"
#include "usettings.h"
#include "netutil.h"
#include "netimageutil.h"
#include "accountinfo.h"

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
        updateHeader(id);
    });

    connect(sig, &SignalTransfer::myFriendsLoaded, this, [=] {
        ui->friendCountLabel->setText(snum(ac->friendList.count()) + "好友");
    });

    connect(sig, &SignalTransfer::myGroupsLoaded, this, [=] {
        ui->groupCountLabel->setText(snum(ac->groupList.count()) + "群组");
    });
}

AccountWidget::~AccountWidget()
{
    delete ui;
}

void AccountWidget::resotreSettings()
{
    ui->hostEdit->setText(us->host);
    ui->tokenEdit->setText(us->accessToken);
    ui->fileHostEdit->setText(us->fileHost);
}

void AccountWidget::on_hostEdit_editingFinished()
{
    QString host = ui->hostEdit->text().trimmed();
    if (host == us->host || host.contains("*"))
        return ;

    // 自动填充前缀
    if (!host.isEmpty() && !host.startsWith("ws") && !host.contains("://"))
        host = "ws://" + host;

    us->set("net/host", us->host = host);
    emit sig->hostChanged(us->host, us->accessToken);
}

void AccountWidget::on_tokenEdit_editingFinished()
{
    QString token = ui->tokenEdit->text();
    if (token == us->accessToken)
        return ;

    us->set("net/accessToken", us->accessToken = token);
    emit sig->hostChanged(us->host, us->accessToken);
}

/// 下载头像
void AccountWidget::updateHeader(qint64 id)
{
    QString url = "http://q1.qlogo.cn/g?b=qq&nk=" + snum(id) + "&s=100&t=";
    QByteArray ba = NetUtil::getWebFile(url);
    QPixmap pixmap;
    pixmap.loadFromData(ba);
    pixmap = NetImageUtil::toRoundedPixmap(pixmap);
    ui->headerLabel->setPixmap(pixmap);
    emit sig->myHeader(pixmap);
}

void AccountWidget::on_fileHostEdit_editingFinished()
{
    QString host = ui->fileHostEdit->text().trimmed();
    if (host == us->fileHost || host.contains("*"))
        return ;

    // 自动填充前缀
    if (!host.isEmpty() && !host.startsWith("http") && !host.contains("://"))
        host = "http://" + host;
    if (host.endsWith("/"))
        host = host.left(host.length() - 1);

    us->set("net/fileHost", us->fileHost = host);
}
