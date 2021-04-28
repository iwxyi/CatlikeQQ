#include "groupwidget.h"
#include "ui_groupwidget.h"

GroupWidget::GroupWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GroupWidget)
{
    ui->setupUi(this);

    ui->mainCheck->setChecked(us->enableGroupNotification);
}

GroupWidget::~GroupWidget()
{
    delete ui;
}

void GroupWidget::on_mainCheck_clicked()
{
    bool enabled = ui->mainCheck->isChecked();
    us->set("group/enabled", us->enableGroupNotification = enabled);
}
