#include <QStandardItemModel>
#include <QHashIterator>
#include "contactpage.h"
#include "ui_contactpage.h"
#include "usettings.h"
#include "signaltransfer.h"

ContactPage* ContactPage::page = nullptr;

ContactPage *ContactPage::getInstance()
{
    if (page == nullptr)
    {
        page = new ContactPage;
    }
    return page;
}

ContactPage::ContactPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ContactPage)
{
    setFocusPolicy(Qt::StrongFocus); // 获取焦点，允许按键点击
    setWindowTitle("搜索");

    ui->setupUi(this);

    // 上一行
    connect(ui->lineEdit, &SearchEdit::signalKeyUp, this, [=]{
        if (!model || !results.size())
            return ;

        QModelIndex index = ui->listView->currentIndex();
        int row = index.row() - 1;
        if (row < 0)
            row = results.size() - 1;
        index = model->index(row, 0);
        if (row >= 0)
            ui->listView->setCurrentIndex(index);
    });

    // 下一行
    connect(ui->lineEdit, &SearchEdit::signalKeyDown, this, [=]{
        if (!model || !results.size())
            return ;

        QModelIndex index = ui->listView->currentIndex();
        int row = index.row() + 1;
        if (row >= results.size())
            row = 0;
        index = model->index(row, 0);
        if (row >= 0)
            ui->listView->setCurrentIndex(index);
    });

    // 退出
    connect(ui->lineEdit, &SearchEdit::signalKeyESC, this, [=]{
        this->close();
    });
}

ContactPage::~ContactPage()
{
    delete ui;
}

void ContactPage::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    static bool first = true;
    if (first)
    {
        first = false;
        restoreGeometry(us->value("searchpage/geometry").toByteArray());
    }

    ui->lineEdit->selectAll();
    ui->lineEdit->setFocus();
}

void ContactPage::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    us->setValue("searchpage/geometry", this->saveGeometry());
}

void ContactPage::search(QString key)
{
    QStringList sl;
    results.clear();

    // 搜索好友
    QHashIterator<qint64, FriendInfo> it(ac->friendList);
    while (it.hasNext())
    {
        it.next();
        if (it.value().nickname.contains(key) || it.value().remark.contains(key))
        {
            results.append(SearchResultBean{it.key(), it.value().username(), SearchResultType::Friend});
            sl.append(it.value().username() + " (" + QString::number(it.key()) + ")");
        }
    }

    // 搜索群组
    QHashIterator<qint64, GroupInfo> it2(ac->groupList);
    while (it2.hasNext())
    {
        it2.next();
        if (it2.value().name.contains(key))
        {
            results.append(SearchResultBean{it2.key(), it2.value().name, SearchResultType::Group});
            sl.append(it2.value().name + " (" + QString::number(it2.key()) + ")");
        }
    }

    // TODO: 搜索聊天记录

    // 设置model
    if (model)
        delete model;
    model = new QStringListModel(sl);
    ui->listView->setModel(model);
    if (sl.size() &&
            (!ui->listView->currentIndex().isValid() || ui->listView->currentIndex().row() == -1)) {
        ui->listView->setCurrentIndex(model->index(0, 0));
    }
}

void ContactPage::on_lineEdit_textChanged(const QString &arg1)
{
    search(arg1);
}

void ContactPage::on_lineEdit_returnPressed()
{
    if (!results.size())
        return ;

    // 打开这个选中项
    int row = ui->listView->currentIndex().row();
    if (row < 0 || row >= results.size())
        return ;

    SearchResultBean result = results.at(row);
    switch (result.type)
    {
    case SearchResultType::Friend:
        this->close();
        emit sig->openUserCard(result.id, result.name, "");
        break;
    case SearchResultType::Group:
        this->close();
        emit sig->openGroupCard(result.id, "");
        break;
    case SearchResultType::Chat:
        // TODO: 打开聊天记录
        break;
    }
}
