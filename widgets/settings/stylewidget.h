#ifndef STYLEWIDGET_H
#define STYLEWIDGET_H

#include <QWidget>

namespace Ui {
class StyleWidget;
}

class StyleWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StyleWidget(QWidget *parent = nullptr);
    ~StyleWidget();

private slots:
    void on_useHeaderGradientCheck_clicked();

    void on_colorfulGroupMemberCheck_clicked();

    void on_bgColorButton_clicked();

    void on_frostedGlassBgCheck_clicked();

    void on_titleColorButton_clicked();

    void on_frostedGlassOpacitySpin_editingFinished();

    void on_useHeaderColorCheck_clicked();

    void on_showBubbleCheck_clicked();

    void on_replyRecursionCheck_clicked();

    void on_bubbleMimeButton_clicked();

    void on_bubbleOppoButton_clicked();

    void on_bubbleReplyButton_clicked();

    void on_linkColorButton_clicked();

    void on_openLinkCheck_clicked();

    void on_historyMultiLineCheck_clicked();

private:
    Ui::StyleWidget *ui;
};

#endif // STYLEWIDGET_H
