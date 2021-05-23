#ifndef ACCOUNTWIDGET_H
#define ACCOUNTWIDGET_H

#include <QWidget>
#include "defines.h"
#include "cqhttpservice.h"

namespace Ui {
class AccountWidget;
}

class AccountWidget : public QWidget
{
    Q_OBJECT

public:
    AccountWidget(CqhttpService* service, QWidget *parent = nullptr);
    ~AccountWidget();

    void resotreSettings();

private slots:
    void on_hostEdit_editingFinished();

    void on_tokenEdit_editingFinished();

private:
    Ui::AccountWidget *ui;
    CqhttpService* service;
};

#endif // ACCOUNTWIDGET_H
