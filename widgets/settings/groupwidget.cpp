#include <QDialog>
#include <QListWidget>
#include <QLineEdit>
#include "interactivebuttonbase.h"
#include "groupwidget.h"
#include "ui_groupwidget.h"
#include "usettings.h"
#include "accountinfo.h"

GroupWidget::GroupWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GroupWidget)
{
    ui->setupUi(this);
    ui->enabledGroupButton->setBorderWidth(1);
    ui->enabledGroupButton->setBorderColor(Qt::gray);

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

void GroupWidget::on_enabledGroupButton_clicked()
{
    // 设置对话框
    QDialog* dialog = new QDialog(this);
    QListWidget* view = new QListWidget(dialog);
    QVBoxLayout* layout = new QVBoxLayout(dialog);
    dialog->setWindowTitle("开启群组通知");
    dialog->setWindowFlags(dialog->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    view->setFocusPolicy(Qt::NoFocus);

    // 添加过滤
    QLineEdit* edit = new QLineEdit(this);
    edit->setPlaceholderText("过滤...");
    connect(edit, &QLineEdit::textChanged, this, [=](const QString& text) {
        for (int i = 0; i < view->count(); i++)
        {
            auto item = view->item(i);
            item->setHidden(!item->text().contains(text));
        }
    });

    // 添加按钮
    QPushButton* selectAllButton = new InteractiveButtonBase("全选", dialog);
    QPushButton* selectRevButton = new InteractiveButtonBase("反选", dialog);
    QHBoxLayout* btnLayout = new QHBoxLayout;
    btnLayout->addWidget(selectAllButton);
    btnLayout->addWidget(selectRevButton);

    layout->addWidget(edit);
    layout->addWidget(view);
    layout->addLayout(btnLayout);

    // 生成model
    const auto& groupHashs = ac->groupList;
    auto groupList = groupHashs.values();
    std::sort(groupList.begin(), groupList.end(), [=](GroupInfo a, GroupInfo b) {
        return a.lastMsgTime > b.lastMsgTime;
    });
    auto& enables = us->enabledGroups;
    for (auto i = groupList.begin(); i != groupList.end(); i++)
    {
        QListWidgetItem* item = new QListWidgetItem(i->name, view);
        item->setData(Qt::UserRole, i->groupId);
        item->setData(Qt::CheckStateRole, enables.contains(i->groupId) ? Qt::Checked : Qt::Unchecked);
    }

    // 设置事件
    connect(view, &QListWidget::itemClicked, this, [=](QListWidgetItem *item){
        item->setCheckState(item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked);
    });
    connect(selectAllButton, &QPushButton::clicked, this, [=]{
        // 全选
        for (int i = 0; i < view->count(); i++)
        {
            auto item = view->item(i);
            if (item->isHidden())
                continue;
            item->setCheckState(Qt::Checked);
        }
    });
    connect(selectRevButton, &QPushButton::clicked, this, [=]{
        // 反选
        for (int i = 0; i < view->count(); i++)
        {
            auto item = view->item(i);
            if (item->isHidden())
                continue;
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
    us->set("group/enables", enables);

    dialog->deleteLater();
}
