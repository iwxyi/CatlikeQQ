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

    void on_trayShowAllMessageIconCheck_clicked();

    void on_trayShowAllSlientMessageIconCheck_clicked();

    void on_trayShowSlientPrivateMessageIconCheck_clicked();

    void on_trayShowSlientSpecialMessageIconCheck_clicked();

    void on_trayShowLowImportanceMessageIconCheck_clicked();

    void on_unreadFlickerCheck_clicked();

private:
    Ui::ApplicationWidget *ui;
};

#endif // APPLICATIONWIDGET_H
