#ifndef REPLYWIDGET_H
#define REPLYWIDGET_H

#include <QWidget>

namespace Ui {
class ReplyWidget;
}

class ReplyWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ReplyWidget(QWidget *parent = nullptr);
    ~ReplyWidget();

private slots:
    void on_autoShowReplyCheck_clicked();

    void on_closeAfterReplyCheck_clicked();

    void on_autoFocusReplyCheck_clicked();

    void on_replyKeyEdit_editingFinished();

    void on_AIReplyCheck_clicked();

    void on_showMySendCheck_clicked();

private:
    Ui::ReplyWidget *ui;
};

#endif // REPLYWIDGET_H
