#include <QDialog>
#include <QListWidget>
#include "interactivebuttonbase.h"
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
    us->set("group/mainEnabled", us->enableGroupNotification = enabled);
}

void GroupWidget::on_enabledGroupButton_clicked()
{
    // 设置对话框
    QDialog* dialog = new QDialog(this);
    QListWidget* view = new QListWidget(dialog);
    QVBoxLayout* layout = new QVBoxLayout(dialog);
    layout->addWidget(view);
    view->setFocusPolicy(Qt::NoFocus);

    // 添加按钮
    QPushButton* selectAllButton = new InteractiveButtonBase("全选", dialog);
    QPushButton* selectRevButton = new InteractiveButtonBase("反选", dialog);
    QHBoxLayout* btnLayout = new QHBoxLayout;
    btnLayout->addWidget(selectAllButton);
    btnLayout->addWidget(selectRevButton);
    layout->addLayout(btnLayout);

    // 生成model
    const auto& groups = ac->groupNames;
    auto& enables = us->enabledGroups;
    for (auto i = groups.begin(); i != groups.end(); i++)
    {
        QListWidgetItem* item = new QListWidgetItem(i.value(), view);
        item->setData( Qt::UserRole, i.key());
        item->setData(Qt::CheckStateRole, enables.contains(i.key()) ? Qt::Checked : Qt::Unchecked);
    }

    // 设置事件
    connect(view, &QListWidget::currentRowChanged, this, [=](int currentRow){
        auto item = view->item(currentRow);
        item->setCheckState(item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked);
    });
    connect(selectAllButton, &QPushButton::clicked, this, [=]{
        // 全选
        for (int i = 0; i < view->count(); i++)
        {
            auto item = view->item(i);
            item->setCheckState(Qt::Checked);
        }
    });
    connect(selectRevButton, &QPushButton::clicked, this, [=]{
        // 反选
        for (int i = 0; i < view->count(); i++)
        {
            auto item = view->item(i);
            item->setCheckState(item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked);
        }
    });
    dialog->exec();

    // 获取选中项
    enables.clear();
    for (int i = 0; i < view->count(); i++)
    {
        auto item = view->item(i);
        if (item->checkState() == Qt::Checked)
            enables.append(item->data(Qt::UserRole).toLongLong());
    }
    us->set("groups/enabled", enables);

    dialog->deleteLater();
}
