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
    void on_displayDurationSpin_editingFinished();

    void on_textReadSpeedSpin_editingFinished();

    void on_floatPixelSpin_editingFinished();

    void on_retentionDurationSpin_editingFinished();

    void on_privateKeepShowingCheck_clicked();

    void on_groupKeepShowingCheck_clicked();

    void on_fixedWidthSpin_editingFinished();

    void on_contentMaxHeightSpin_editingFinished();

    void on_thumbnailPropSpin_editingFinished();

    void on_positionCombo_activated(int index);

private:
    Ui::BannerWidget *ui;
};

#endif // BANNERWIDGET_H
