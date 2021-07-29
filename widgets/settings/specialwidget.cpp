#include "specialwidget.h"
#include "ui_specialwidget.h"
#include "usettings.h"

SpecialWidget::SpecialWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SpecialWidget)
{
    ui->setupUi(this);

    ui->groupUseFriendImportanceCheck->setChecked(us->groupUseFriendImportance);
}

SpecialWidget::~SpecialWidget()
{
    delete ui;
}

void SpecialWidget::on_groupUseFriendImportanceCheck_clicked()
{
    us->set("importance/groupUseFriendImportance", us->groupUseFriendImportance = ui->groupUseFriendImportanceCheck->isChecked());
}
