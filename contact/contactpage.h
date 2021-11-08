#ifndef CONTACTPAGE_H
#define CONTACTPAGE_H

#include <QWidget>
#include <QStringListModel>
#include "accountinfo.h"

namespace Ui {
class ContactPage;
}

enum SearchResultType
{
    Friend, // 0 好友
    Group,  // 1 群组
    Chat    // 2 聊天记录
};

struct SearchResultBean
{
    qint64 id;
    QString name;
    SearchResultType type; // 0好友，1群组，2聊天记录
};

class ContactPage : public QWidget
{
    Q_OBJECT
public:
    static ContactPage* getInstance();

private:
    explicit ContactPage(QWidget *parent = nullptr);
    ~ContactPage() override;

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void search(QString key);

private slots:
    void on_lineEdit_textChanged(const QString &arg1);

    void on_lineEdit_returnPressed();

private:
    Ui::ContactPage *ui;
    static ContactPage* page;

    QStringListModel* model = nullptr;
    QList<SearchResultBean> results;
};

#endif // CONTACTPAGE_H
