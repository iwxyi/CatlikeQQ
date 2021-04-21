#ifndef ACCOUNTWIDGET_H
#define ACCOUNTWIDGET_H

#include <QWidget>
#include "global.h"

namespace Ui {
class AccountWidget;
}

class AccountWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AccountWidget(QWidget *parent = nullptr);
    ~AccountWidget();

    void resotreSettings();

private slots:
    void on_lineEdit_editingFinished();

private:
    Ui::AccountWidget *ui;
};

#endif // ACCOUNTWIDGET_H
