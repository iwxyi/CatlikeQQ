#ifndef DEBUGWIDGET_H
#define DEBUGWIDGET_H

#include <QWidget>
#include "defines.h"
#include "msgbean.h"
#include "cqhttpservice.h"

namespace Ui {
class DebugWidget;
}

class DebugWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DebugWidget(CqhttpService* service, QWidget *parent = nullptr);
    ~DebugWidget();

private slots:
    void on_sendButton_clicked();

private:
    Ui::DebugWidget *ui;
    CqhttpService* service;
};

#endif // DEBUGWIDGET_H
