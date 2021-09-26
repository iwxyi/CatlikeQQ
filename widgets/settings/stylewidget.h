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

private:
    Ui::StyleWidget *ui;
};

#endif // STYLEWIDGET_H
