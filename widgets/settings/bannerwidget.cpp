#include <QDesktopWidget>
#include "bannerwidget.h"
#include "ui_bannerwidget.h"
#include "defines.h"
#include "usettings.h"

BannerWidget::BannerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BannerWidget)
{
    ui->setupUi(this);

    ui->bannerScreenIndexSpin->setValue(us->bannerScreenIndex);
    ui->displayDurationSpin->setValue(us->bannerDisplayDuration / 1000);
    ui->textReadSpeedSpin->setValue(us->bannerTextReadSpeed);
    ui->floatPixelSpin->setValue(us->bannerFloatPixel);
    ui->fixedWidthSpin->setValue(us->bannerFixedWidth);
    ui->contentMaxHeightSpin->setValue(us->bannerContentMaxHeight);
    ui->thumbnailPropSpin->setValue(us->bannerThumbnailProp);
    ui->retentionDurationSpin->setValue(us->bannerRetentionDuration / 1000);

    ui->privateKeepShowingCheck->setChecked(us->bannerPrivateKeepShowing);
    ui->groupKeepShowingCheck->setChecked(us->bannerGroupKeepShowing);

    if (us->bannerFloatSide == SideRight)
    {
        if (us->bannerFloatDirection == TopToBottom)
            ui->positionCombo->setCurrentIndex(0);
        else
            ui->positionCombo->setCurrentIndex(1);
    }
    else if (us->bannerFloatSide == SideLeft)
    {
        if (us->bannerFloatDirection == TopToBottom)
            ui->positionCombo->setCurrentIndex(2);
        else
            ui->positionCombo->setCurrentIndex(3);
    }

    // 多显示器
    auto screens = QGuiApplication::screens();
    int screenNum = screens.size();
    ui->bannerScreenIndexSpin->setMaximum(screenNum - 1);
    if (us->bannerScreenIndex > screens.size())
        us->bannerScreenIndex = 0;
}

BannerWidget::~BannerWidget()
{
    delete ui;
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
    us->bannerContentWidth = us->bannerFixedWidth - us->bannerContentWidthDelta;
}

void BannerWidget::on_contentMaxHeightSpin_editingFinished()
{
    us->set("banner/contentMaxHeight", us->bannerContentMaxHeight = ui->contentMaxHeightSpin->value());
}

void BannerWidget::on_thumbnailPropSpin_editingFinished()
{
    us->set("banner/thumbnailProp", us->bannerThumbnailProp = ui->thumbnailPropSpin->value());
}

void BannerWidget::on_positionCombo_activated(int index)
{
    if (index == 0) // 右上
    {
        us->bannerFloatSide = SideRight;
        us->bannerFloatDirection = TopToBottom;
    }
    else if (index == 1) // 右下
    {
        us->bannerFloatSide = SideRight;
        us->bannerFloatDirection = BottomToTop;
    }
    else if (index == 2) // 左上
    {
        us->bannerFloatSide = SideLeft;
        us->bannerFloatDirection = TopToBottom;
    }
    else if (index == 3) // 左下
    {
        us->bannerFloatSide = SideLeft;
        us->bannerFloatDirection = BottomToTop;
    }

    us->set("banner/floatSide", us->bannerFloatSide);
    us->set("banner/floatDirection", us->bannerFloatDirection);
}

void BannerWidget::on_bannerScreenIndexSpin_editingFinished()
{
    us->set("banner/screenIndex", us->bannerScreenIndex = ui->bannerScreenIndexSpin->value());
}
