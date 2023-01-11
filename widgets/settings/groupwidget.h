#ifndef GROUPWIDGET_H
#define GROUPWIDGET_H

#include <QWidget>
#include "defines.h"

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

    void on_enabledGroupButton_clicked();

    void on_autoPauseCheck_clicked();

    void on_isPausingCheck_clicked();

    void on_showDisabledGroupsCheck_clicked();

protected:
    void showEvent(QShowEvent *event) override;

private:
    Ui::GroupWidget *ui;
};

#endif // GROUPWIDGET_H
