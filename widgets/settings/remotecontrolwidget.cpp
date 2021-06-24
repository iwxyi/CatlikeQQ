#include "remotecontrolwidget.h"
#include "ui_remotecontrolwidget.h"
#include "usettings.h"

RemoteControlWidget::RemoteControlWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RemoteControlWidget)
{
    ui->setupUi(this);

    ui->prefixEdit->setText(us->remoteControlPrefixs.join(" "));
}

RemoteControlWidget::~RemoteControlWidget()
{
    delete ui;
}

void RemoteControlWidget::on_prefixEdit_editingFinished()
{
    us->set("remoteControl/prefixs", us->remoteControlPrefixs = ui->prefixEdit->text().split(" ", QString::SkipEmptyParts));
}
