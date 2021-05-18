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

    void on_aiReplyPrefixEdit_undoAvailable(bool);

    void on_aiReplySuffixEdit_undoAvailable(bool);

    void on_aiReplyDefaultEdit_undoAvailable(bool);

    void on_aiReplyIntervalSpin_editingFinished();

private:
    Ui::LeaveModeWidget *ui;
};

#endif // LEAVEMODEWIDGET_H
