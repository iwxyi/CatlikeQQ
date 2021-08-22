#include <QRegularExpression>
#include "specialwidget.h"
#include "ui_specialwidget.h"
#include "usettings.h"

SpecialWidget::SpecialWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SpecialWidget)
{
    ui->setupUi(this);

    ui->groupUseFriendImportanceCheck->setChecked(us->groupUseFriendImportance);
    ui->improveAtMeImportanceCheck->setChecked(us->improveAtMeImportance);
    ui->improveAtAllImportanceCheck->setChecked(us->improveAtAllImportance);
    ui->keepImportantMessageCheck->setChecked(us->keepImportantMessage == VeryImportant);
    ui->globalRemindWordsEdit->setPlainText(us->globalRemindWords.join(" "));
    ui->remindOverlayCheck->setChecked(us->remindOverlay);
}

SpecialWidget::~SpecialWidget()
{
    delete ui;
}

void SpecialWidget::on_groupUseFriendImportanceCheck_clicked()
{
    us->set("importance/groupUseFriendImportance", us->groupUseFriendImportance = ui->groupUseFriendImportanceCheck->isChecked());
}

void SpecialWidget::on_improveAtMeImportanceCheck_clicked()
{
    us->set("importance/improveAtMeImportance", us->improveAtMeImportance = ui->improveAtMeImportanceCheck->isChecked());
}

void SpecialWidget::on_improveAtAllImportanceCheck_clicked()
{
    us->set("importance/improveAtAllImportance", us->improveAtAllImportance = ui->improveAtAllImportanceCheck->isChecked());
}

void SpecialWidget::on_keepImportantMessageCheck_clicked()
{
    us->set("importance/keepImportantMessage", us->keepImportantMessage =
            (ui->keepImportantMessageCheck->isChecked()
             ? VeryImportant : VeryImportant + 1));
}

void SpecialWidget::on_globalRemindWordsEdit_textChanged()
{
    us->globalRemindWords = ui->globalRemindWordsEdit->toPlainText().split(QRegularExpression("\\s+"), QString::SkipEmptyParts);
    us->set("special/globalRemindWords", us->globalRemindWords);
}

void SpecialWidget::on_remindOverlayCheck_clicked()
{
    us->set("special/remindOverlay", us->remindOverlay = ui->remindOverlayCheck->isChecked());
}
