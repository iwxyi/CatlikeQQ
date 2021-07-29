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
