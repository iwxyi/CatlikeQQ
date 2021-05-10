#ifndef BANNERWIDGET_H
#define BANNERWIDGET_H

#include <QWidget>

namespace Ui {
class BannerWidget;
}

class BannerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BannerWidget(QWidget *parent = nullptr);
    ~BannerWidget();

private slots:
    void on_useHeaderColorCheck_clicked();

    void on_displayDurationSpin_editingFinished();

    void on_textReadSpeedSpin_editingFinished();

    void on_floatPixelSpin_editingFinished();

    void on_retentionDurationSpin_editingFinished();

    void on_useHeaderGradientCheck_clicked();

    void on_colorfulGroupMemberCheck_clicked();

private:
    Ui::BannerWidget *ui;
};

#endif // BANNERWIDGET_H
