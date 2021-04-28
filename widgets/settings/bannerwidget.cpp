#include "bannerwidget.h"
#include "ui_bannerwidget.h"
#include "global.h"

BannerWidget::BannerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BannerWidget)
{
    ui->setupUi(this);

    ui->useHeaderColorCheck->setChecked(us->bannerUseHeaderColor);
    ui->displayDurationSpin->setValue(us->bannerDisplayDuration / 1000);
    ui->textReadSpeedSpin->setValue(us->bannerTextReadSpeed);
}

BannerWidget::~BannerWidget()
{
    delete ui;
}

void BannerWidget::on_useHeaderColorCheck_clicked()
{
    us->set("banner/useHeaderColor", us->bannerUseHeaderColor = ui->useHeaderColorCheck->isChecked());
}

void BannerWidget::on_displayDurationSpin_editingFinished()
{
    us->set("banner/displayDuration", us->bannerDisplayDuration = ui->displayDurationSpin->value() * 1000);
}

void BannerWidget::on_textReadSpeedSpin_editingFinished()
{
    us->set("banner/textReadSpeed", us->bannerTextReadSpeed = ui->textReadSpeedSpin->value());
}
