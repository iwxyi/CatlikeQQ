#include "remotecontrolwidget.h"
#include "ui_remotecontrolwidget.h"

RemoteControlWidget::RemoteControlWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RemoteControlWidget)
{
    ui->setupUi(this);
}

RemoteControlWidget::~RemoteControlWidget()
{
    delete ui;
}
