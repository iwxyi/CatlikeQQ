#ifndef REMOTECONTROLWIDGET_H
#define REMOTECONTROLWIDGET_H

#include <QWidget>

namespace Ui {
class RemoteControlWidget;
}

class RemoteControlWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RemoteControlWidget(QWidget *parent = nullptr);
    ~RemoteControlWidget();

private slots:
    void on_prefixEdit_editingFinished();

private:
    Ui::RemoteControlWidget *ui;
};

#endif // REMOTECONTROLWIDGET_H
