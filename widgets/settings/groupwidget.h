#ifndef GROUPWIDGET_H
#define GROUPWIDGET_H

#include <QWidget>
#include "global.h"

namespace Ui {
class GroupWidget;
}

class GroupWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GroupWidget(QWidget *parent = nullptr);
    ~GroupWidget();

private slots:
    void on_mainCheck_clicked();

private:
    Ui::GroupWidget *ui;
};

#endif // GROUPWIDGET_H
