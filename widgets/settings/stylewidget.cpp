#include <QColorDialog>
#include "stylewidget.h"
#include "ui_stylewidget.h"
#include "defines.h"
#include "usettings.h"

StyleWidget::StyleWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StyleWidget)
{
    ui->setupUi(this);

    ui->useHeaderColorCheck->setChecked(us->bannerUseHeaderColor);
    ui->useHeaderGradientCheck->setChecked(us->bannerUseHeaderGradient);
    ui->colorfulGroupMemberCheck->setChecked(us->bannerColorfulGroupMember);
    ui->frostedGlassBgCheck->setChecked(us->bannerFrostedGlassBg);
    ui->frostedGlassOpacitySpin->setValue(us->bannerFrostedGlassOpacity);

    ui->bgColorButton->setBorderColor(Qt::gray);
    ui->titleColorButton->setBorderColor(Qt::gray);
}

StyleWidget::~StyleWidget()
{
    delete ui;
}

void StyleWidget::on_useHeaderGradientCheck_clicked()
{
    us->set("banner/useHeaderGradient", us->bannerUseHeaderGradient = ui->useHeaderGradientCheck->isChecked());
}

void StyleWidget::on_colorfulGroupMemberCheck_clicked()
{
    us->set("banner/colorfulGroupMember", us->bannerColorfulGroupMember = ui->colorfulGroupMemberCheck->isChecked());
}

void StyleWidget::on_bgColorButton_clicked()
{
    QColor c = QColorDialog::getColor(us->bannerBgColor, this, "通知卡片背景颜色", QColorDialog::ShowAlphaChannel);
    us->set("banner/bgColor", us->bannerBgColor = c);
}

void StyleWidget::on_frostedGlassBgCheck_clicked()
{
    us->set("banner/frostedGlassBg", us->bannerFrostedGlassBg = ui->frostedGlassBgCheck->isChecked());
}

void StyleWidget::on_titleColorButton_clicked()
{
    QColor c = QColorDialog::getColor(us->bannerTitleColor, this, "通知卡片文字颜色", QColorDialog::ShowAlphaChannel);
    us->set("banner/titleColor", us->bannerTitleColor = c);
    us->set("banner/contentColor", us->bannerContentColor = c);
}

void StyleWidget::on_frostedGlassOpacitySpin_editingFinished()
{
    us->set("banner/frostedGlassOpacity", us->bannerFrostedGlassOpacity = ui->frostedGlassOpacitySpin->value());
}

void StyleWidget::on_useHeaderColorCheck_clicked()
{
    us->set("banner/useHeaderColor", us->bannerUseHeaderColor = ui->useHeaderColorCheck->isChecked());
}
