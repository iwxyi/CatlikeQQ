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
    ui->trayShowAllMessageIconCheck->setChecked(us->trayShowAllMessageIcon);
    ui->trayShowAllSlientMessageIconCheck->setChecked(us->trayShowAllSlientMessageIcon);
    ui->trayShowSlientPrivateMessageIconCheck->setChecked(us->trayShowSlientPrivateMessageIcon);
    ui->trayShowSlientSpecialMessageIconCheck->setChecked(us->trayShowSlientSpecialMessageIcon);
    ui->trayShowLowImportanceMessageIconCheck->setChecked(us->trayShowLowImportanceMessageIcon);
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
        reg->setValue(appName, appPath);
    else
        reg->remove(appName);
    reg->deleteLater();
}

void ApplicationWidget::on_trayShowAllMessageIconCheck_clicked()
{
    us->set("tray/allMessage", us->trayShowAllMessageIcon = ui->trayShowAllMessageIconCheck->isChecked());
}

void ApplicationWidget::on_trayShowAllSlientMessageIconCheck_clicked()
{
    us->set("tray/allSlientMessage", us->trayShowAllSlientMessageIcon = ui->trayShowAllSlientMessageIconCheck->isChecked());
}

void ApplicationWidget::on_trayShowSlientPrivateMessageIconCheck_clicked()
{
    us->set("tray/slientPrivate", us->trayShowSlientPrivateMessageIcon = ui->trayShowSlientPrivateMessageIconCheck->isChecked());
}

void ApplicationWidget::on_trayShowSlientSpecialMessageIconCheck_clicked()
{
    us->set("tray/slientSpecial", us->trayShowSlientSpecialMessageIcon = ui->trayShowSlientSpecialMessageIconCheck->isChecked());
}

void ApplicationWidget::on_trayShowLowImportanceMessageIconCheck_clicked()
{
us->set("tray/lowImportance", us->trayShowLowImportanceMessageIcon = ui->trayShowLowImportanceMessageIconCheck->isChecked());
}
