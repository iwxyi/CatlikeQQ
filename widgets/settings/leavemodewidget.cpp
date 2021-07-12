#include <QDebug>
#include <QDialog>
#include <QListWidget>
#include <QLineEdit>
#include "leavemodewidget.h"
#include "ui_leavemodewidget.h"
#include "usettings.h"
#include "accountinfo.h"
#include "interactivebuttonbase.h"

LeaveModeWidget::LeaveModeWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LeaveModeWidget)
{
    ui->setupUi(this);

    ui->leaveModelCheck->setChecked(us->leaveMode);
    ui->aiReplyPrivateCheck->setChecked(us->aiReplyPrivate);
    ui->aiReplyPrefixEdit->setPlainText(us->aiReplyPrefix);
    ui->aiReplySuffixEdit->setPlainText(us->aiReplySuffix);
    ui->aiReplyDefaultEdit->setPlainText(us->aiReplyDefault);
    ui->aiReplyIntervalSpin->setValue(us->aiReplyInterval / 1000);

    opening = false;
    ui->aiReplyAllowButton->setBorderColor(Qt::gray);
}

LeaveModeWidget::~LeaveModeWidget()
{
    delete ui;
}

void LeaveModeWidget::on_leaveModelCheck_clicked()
{
    us->set("leave/leaveMode", us->leaveMode = ui->leaveModelCheck->isChecked());
}

void LeaveModeWidget::on_aiReplyPrivateCheck_clicked()
{
    us->set("leave/aiReplyPrivate", us->aiReplyPrivate = ui->aiReplyPrivateCheck->isChecked());
}

void LeaveModeWidget::on_aiReplyIntervalSpin_editingFinished()
{
    us->set("leave/aiReplyInterval", us->aiReplyInterval = ui->aiReplyIntervalSpin->value() * 10);
}

void LeaveModeWidget::on_aiReplyPrefixEdit_textChanged()
{
    if (opening)
        return ;
    us->set("leave/aiReplyPrefix", us->aiReplyPrefix = ui->aiReplyPrefixEdit->toPlainText());
}

void LeaveModeWidget::on_aiReplySuffixEdit_textChanged()
{
    if (opening)
        return ;
    us->set("leave/aiReplySuffix", us->aiReplySuffix = ui->aiReplySuffixEdit->toPlainText());
}

void LeaveModeWidget::on_aiReplyDefaultEdit_textChanged()
{
    if (opening)
        return ;
    us->set("leave/aiReplyDefault", us->aiReplyDefault = ui->aiReplyDefaultEdit->toPlainText());
}

void LeaveModeWidget::on_aiReplyAllowButton_clicked()
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
    const auto& users = ac->friendList;
    auto& enables = us->aiReplyUsers;
    for (auto i = users.begin(); i != users.end(); i++)
    {
        QListWidgetItem* item = new QListWidgetItem(i.value().nickname, view);
        item->setData( Qt::UserRole, i.key());
        item->setData(Qt::CheckStateRole, enables.contains(i.key()) ? Qt::Checked : Qt::Unchecked);
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
    us->set("leave/aiReplyUsers", enables);

    dialog->deleteLater();
}
