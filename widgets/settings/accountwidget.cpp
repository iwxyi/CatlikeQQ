#include "accountwidget.h"
#include "ui_accountwidget.h"

AccountWidget::AccountWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AccountWidget)
{
    ui->setupUi(this);
    resotreSettings();

    connect(sig, &SignalTransfer::socketStateChanged, this, [=](bool connected) {
        ui->connectStateLabel->setText(connected ? "已连接" : "已断开");
    });
}

AccountWidget::~AccountWidget()
{
    delete ui;
}

void AccountWidget::resotreSettings()
{
    ui->lineEdit->setText(us->host);
}

void AccountWidget::on_lineEdit_editingFinished()
{
    QString host = ui->lineEdit->text().trimmed();
    if (host == us->host)
        return ;
    us->set("net/host", host);
    emit sig->hostChanged(host);
}
