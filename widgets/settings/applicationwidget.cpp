#include <QDir>
#include "applicationwidget.h"
#include "ui_applicationwidget.h"
#include "usettings.h"

ApplicationWidget::ApplicationWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ApplicationWidget)
{
    ui->setupUi(this);

    ui->startOnPowerOnCheck->setChecked(us->startOnPowerOn);
}

ApplicationWidget::~ApplicationWidget()
{
    delete ui;
}

void ApplicationWidget::on_startOnPowerOnCheck_clicked()
{
    us->set("app/startOnPowerOn", us->startOnPowerOn = ui->startOnPowerOnCheck->isChecked());
    QString appName = QApplication::applicationName();
    QString appPath = QDir::toNativeSeparators(QApplication::applicationFilePath());
    QSettings *reg=new QSettings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    QString val = reg->value(appName).toString();// 如果此键不存在，则返回的是空字符串
    if (us->startOnPowerOn)
        reg->setValue(appName, appPath);// 如果移除的话
    else
        reg->remove(appName);
    reg->deleteLater();
}
