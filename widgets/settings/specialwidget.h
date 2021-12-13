#ifndef SPECIALWIDGET_H
#define SPECIALWIDGET_H

#include <QWidget>

namespace Ui {
class SpecialWidget;
}

class SpecialWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SpecialWidget(QWidget *parent = nullptr);
    ~SpecialWidget();

private slots:
    void on_groupUseFriendImportanceCheck_clicked();

    void on_improveAtMeImportanceCheck_clicked();

    void on_improveAtAllImportanceCheck_clicked();

    void on_keepImportantMessageCheck_clicked();

    void on_globalRemindWordsEdit_textChanged();

    void on_remindOverlayCheck_clicked();

    void on_dynamicImportanceCheck_clicked();

    void on_smartFocusCheck_clicked();

private:
    Ui::SpecialWidget *ui;
};

#endif // SPECIALWIDGET_H
