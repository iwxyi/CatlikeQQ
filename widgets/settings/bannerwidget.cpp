#include <QColorDialog>
#include "bannerwidget.h"
#include "ui_bannerwidget.h"
#include "defines.h"
#include "usettings.h"

BannerWidget::BannerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BannerWidget)
{
    ui->setupUi(this);

    ui->displayDurationSpin->setValue(us->bannerDisplayDuration / 1000);
    ui->textReadSpeedSpin->setValue(us->bannerTextReadSpeed);
    ui->floatPixelSpin->setValue(us->bannerFloatPixel);
    ui->fixedWidthSpin->setValue(us->bannerFixedWidth);
    ui->contentMaxHeightSpin->setValue(us->bannerContentMaxHeight);
    ui->retentionDurationSpin->setValue(us->bannerRetentionDuration / 1000);
    ui->useHeaderColorCheck->setChecked(us->bannerUseHeaderColor);
    ui->useHeaderGradientCheck->setChecked(us->bannerUseHeaderGradient);
    ui->colorfulGroupMemberCheck->setChecked(us->bannerColorfulGroupMember);
    ui->frostedGlassBgCheck->setChecked(us->bannerFrostedGlassBg);
    ui->frostedGlassOpacitySpin->setValue(us->bannerFrostedGlassOpacity);
    ui->privateKeepShowingCheck->setChecked(us->bannerPrivateKeepShowing);
    ui->groupKeepShowingCheck->setChecked(us->bannerGroupKeepShowing);

    ui->bgColorButton->setBorderColor(Qt::gray);
    ui->titleColorButton->setBorderColor(Qt::gray);
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

void BannerWidget::on_colorfulGroupMemberCheck_clicked()
{
    us->set("banner/colorfulGroupMember", us->bannerColorfulGroupMember = ui->colorfulGroupMemberCheck->isChecked());
}

void BannerWidget::on_bgColorButton_clicked()
{
    QColor c = QColorDialog::getColor(us->bannerBgColor, this, "通知卡片背景颜色", QColorDialog::ShowAlphaChannel);
    us->set("banner/bgColor", us->bannerBgColor = c);
}

void BannerWidget::on_frostedGlassBgCheck_clicked()
{
    us->set("banner/frostedGlassBg", us->bannerFrostedGlassBg = ui->frostedGlassBgCheck->isChecked());
}

void BannerWidget::on_titleColorButton_clicked()
{
    QColor c = QColorDialog::getColor(us->bannerTitleColor, this, "通知卡片文字颜色", QColorDialog::ShowAlphaChannel);
    us->set("banner/titleColor", us->bannerTitleColor = c);
    us->set("banner/contentColor", us->bannerContentColor = c);
}

void BannerWidget::on_frostedGlassOpacitySpin_editingFinished()
{
    us->set("banner/frostedGlassOpacity", us->bannerFrostedGlassOpacity = ui->frostedGlassOpacitySpin->value());
}

void BannerWidget::on_privateKeepShowingCheck_clicked()
{
    us->set("banner/privateKeepShowing", us->bannerPrivateKeepShowing = ui->privateKeepShowingCheck->isChecked());
}

void BannerWidget::on_groupKeepShowingCheck_clicked()
{
    us->set("banner/groupKeepShowing", us->bannerGroupKeepShowing = ui->groupKeepShowingCheck->isChecked());
}

void BannerWidget::on_fixedWidthSpin_editingFinished()
{
    us->set("banner/fixedWidth", us->bannerFixedWidth = ui->fixedWidthSpin->value());
}

void BannerWidget::on_contentMaxHeightSpin_editingFinished()
{
    us->set("banner/contentMaxHeight", us->bannerContentMaxHeight = ui->contentMaxHeightSpin->value());
}
