#ifndef SPEECHWIDGET_H
#define SPEECHWIDGET_H

#include <QWidget>

namespace Ui {
class SpeechWidget;
}

class SpeechWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SpeechWidget(QWidget *parent = nullptr);
    ~SpeechWidget();

public:
    static void refreshToken();

private slots:
    void on_autoPlaySpeechCheck_clicked();

    void on_autoTransSpeechCheck_clicked();

    void on_baiduSpeechApiKey_editingFinished();

    void on_baiduSpeechSecretKey_editingFinished();

    void on_baiduSpeechAccessTokenEdit_editingFinished();

    void on_refreshAccessTokenButton_clicked();

protected:
    void showEvent(QShowEvent *event) override;

private:
    Ui::SpeechWidget *ui;
};

#endif // SPEECHWIDGET_H
