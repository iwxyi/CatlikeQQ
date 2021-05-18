#ifndef LEAVEMODEWIDGET_H
#define LEAVEMODEWIDGET_H

#include <QWidget>

namespace Ui {
class LeaveModeWidget;
}

class LeaveModeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LeaveModeWidget(QWidget *parent = nullptr);
    ~LeaveModeWidget();

private slots:
    void on_leaveModelCheck_clicked();

    void on_aiReplyPrivateCheck_clicked();

    void on_aiReplyIntervalSpin_editingFinished();

    void on_aiReplyPrefixEdit_textChanged();

    void on_aiReplySuffixEdit_textChanged();

    void on_aiReplyDefaultEdit_textChanged();

    void on_aiReplyAllowButton_clicked();

private:
    Ui::LeaveModeWidget *ui;
    bool opening = true;
};

#endif // LEAVEMODEWIDGET_H
