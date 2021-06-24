#ifndef APPLICATIONWIDGET_H
#define APPLICATIONWIDGET_H

#include <QWidget>

namespace Ui {
class ApplicationWidget;
}

class ApplicationWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ApplicationWidget(QWidget *parent = nullptr);
    ~ApplicationWidget();

private slots:
    void on_startOnPowerOnCheck_clicked();

private:
    Ui::ApplicationWidget *ui;
};

#endif // APPLICATIONWIDGET_H
