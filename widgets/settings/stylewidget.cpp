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
    ui->showBubbleCheck->setChecked(us->bannerShowBubble);
    ui->replyRecursionCheck->setChecked(us->bannerReplyRecursion);
    ui->historyMultiLineCheck->setChecked(us->showMultiMessageHistories);

    ui->bgColorButton->setBorderColor(Qt::gray);
    ui->titleColorButton->setBorderColor(Qt::gray);
    ui->linkColorButton->setBorderColor(Qt::gray);
    ui->bubbleMimeButton->setBorderColor(Qt::gray);
    ui->bubbleOppoButton->setBorderColor(Qt::gray);
    ui->bubbleReplyButton->setBorderColor(Qt::gray);
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
    if (!c.isValid())
        return ;
    us->set("banner/bgColor", us->bannerBgColor = c);
}

void StyleWidget::on_frostedGlassBgCheck_clicked()
{
    us->set("banner/frostedGlassBg", us->bannerFrostedGlassBg = ui->frostedGlassBgCheck->isChecked());
}

void StyleWidget::on_titleColorButton_clicked()
{
    QColor c = QColorDialog::getColor(us->bannerTitleColor, this, "通知卡片文字颜色", QColorDialog::ShowAlphaChannel);
    if (!c.isValid())
        return ;
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

void StyleWidget::on_showBubbleCheck_clicked()
{
    us->set("banner/showBubble", us->bannerShowBubble = ui->showBubbleCheck->isChecked());
}

void StyleWidget::on_replyRecursionCheck_clicked()
{
    us->set("banner/replyRecursion", us->bannerReplyRecursion = ui->replyRecursionCheck->isChecked());
}

void StyleWidget::on_bubbleMimeButton_clicked()
{
    QColor c = QColorDialog::getColor(us->bannerBubbleMime, this, "自己消息的气泡背景颜色", QColorDialog::ShowAlphaChannel);
    if (!c.isValid())
        return ;
    us->set("banner/bubbleMime", us->bannerBubbleMime = c);
}

void StyleWidget::on_bubbleOppoButton_clicked()
{
    QColor c = QColorDialog::getColor(us->bannerBubbleOppo, this, "他人消息的气泡背景颜色", QColorDialog::ShowAlphaChannel);
    if (!c.isValid())
        return ;
    us->set("banner/bubbleOppo", us->bannerBubbleOppo = c);
}

void StyleWidget::on_bubbleReplyButton_clicked()
{
    QColor c = QColorDialog::getColor(us->bannerBubbleReply, this, "回复消息的气泡背景颜色", QColorDialog::ShowAlphaChannel);
    if (!c.isValid())
        return ;
    us->set("banner/bubbleReply", us->bannerBubbleReply = c);
}

void StyleWidget::on_linkColorButton_clicked()
{
    QColor c = QColorDialog::getColor(us->bannerLinkColor, this, "超链接颜色", QColorDialog::ShowAlphaChannel);
    if (!c.isValid())
        return ;
    us->set("banner/linkColor", us->bannerLinkColor = c);
}

void StyleWidget::on_openLinkCheck_clicked()
{

}

void StyleWidget::on_historyMultiLineCheck_clicked()
{
    us->set("history/showMultiMessageHistories", us->showMultiMessageHistories = ui->historyMultiLineCheck->isChecked());
}
