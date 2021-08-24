#include "countwidget.h"
#include "ui_countwidget.h"
#include "usettings.h"
#include <QDebug>

CountWidget::CountWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CountWidget)
{
    ui->setupUi(this);

    refresh();
}

CountWidget::~CountWidget()
{
    delete ui;
}

void CountWidget::refresh()
{
    ui->countReceiveAllLabel->setNum(us->countReceiveAll);
    ui->countReceivePrivateLabel->setNum(us->countReceivePrivate);
    ui->countReceiveGroupLabel->setNum(us->countReceiveGroup);
    ui->countShowBannerLabel->setNum(us->countShowBanner);
    ui->countSendAllLabel->setNum(us->countSendAll);
    ui->countSendPrivateLabel->setNum(us->countSendPrivate);
    ui->countSendGroupLabel->setNum(us->countSendGroup);
}

void CountWidget::showEvent(QShowEvent *event)
{
    refresh();
    return QWidget::showEvent(event);
}
