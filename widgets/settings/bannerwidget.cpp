#include "bannerwidget.h"
#include "ui_bannerwidget.h"
#include "defines.h"
#include "usettings.h"

BannerWidget::BannerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BannerWidget)
{
    ui->setupUi(this);

    ui->useHeaderColorCheck->setChecked(us->bannerUseHeaderColor);
    ui->useHeaderGradientCheck->setChecked(us->bannerUseHeaderGradient);
    ui->displayDurationSpin->setValue(us->bannerDisplayDuration / 1000);
    ui->textReadSpeedSpin->setValue(us->bannerTextReadSpeed);
    ui->floatPixelSpin->setValue(us->bannerFloatPixel);
    ui->retentionDurationSpin->setValue(us->bannerRetentionDuration / 1000);
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

void BannerWidget::on_floatPixelSpin_editingFinished()
{
    us->set("banner/floatPixel", us->bannerFloatPixel = ui->floatPixelSpin->value());
}

void BannerWidget::on_retentionDurationSpin_editingFinished()
{
    us->set("banner/retentionDuration", us->bannerRetentionDuration = ui->retentionDurationSpin->value() * 1000);
}

void BannerWidget::on_useHeaderGradientCheck_clicked()
{
    us->set("banner/useHeaderGradient", us->bannerUseHeaderGradient = ui->useHeaderGradientCheck->isChecked());
}
