#include "speechwidget.h"
#include "ui_speechwidget.h"
#include "usettings.h"
#include "netutil.h"
#include "myjson.h"

SpeechWidget::SpeechWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SpeechWidget)
{
    ui->setupUi(this);

    ui->autoPlaySpeechCheck->setChecked(us->autoPlaySpeech);
    ui->autoTransSpeechCheck->setChecked(us->autoTransSpeech);
    ui->baiduSpeechApiKey->setText(us->baiduSpeechApiKey);
    ui->baiduSpeechSecretKey->setText(us->baiduSpeechSecretKey);
    ui->baiduSpeechAccessTokenEdit->setText(us->baiduSpeechAccessToken);
}

SpeechWidget::~SpeechWidget()
{
    delete ui;
}

void SpeechWidget::refreshToken()
{
    if (us->baiduSpeechApiKey.isEmpty() || us->baiduSpeechSecretKey.isEmpty())
    {
        qWarning() << "未设置百度AI的 API Key 或者 Secret Key";
        return ;
    }
    QString s = NetUtil::getWebData("https://aip.baidubce.com/oauth/2.0/token?grant_type=client_credentials&client_id="
                                    + us->baiduSpeechApiKey + "&client_secret=" + us->baiduSpeechSecretKey);
    MyJson json = MyJson::from(s.toUtf8());
    us->baiduSpeechAccessToken = json.s("access_token");
    us->set("baiduSpeech/accessToken", us->baiduSpeechAccessToken);
    us->set("baiduSpeech/tokenRefreshTime", QDateTime::currentSecsSinceEpoch()); // 30天（30*24*3600）过期
}

void SpeechWidget::on_autoPlaySpeechCheck_clicked()
{
    us->set("banner/autoPlaySpeech", us->autoPlaySpeech = ui->autoPlaySpeechCheck->isChecked());
}

void SpeechWidget::on_autoTransSpeechCheck_clicked()
{
    us->set("banner/autoTransSpeech", us->autoTransSpeech = ui->autoTransSpeechCheck->isChecked());
}

void SpeechWidget::on_baiduSpeechApiKey_editingFinished()
{
    us->set("baiduSpeech/apiKey", us->baiduSpeechApiKey = ui->baiduSpeechApiKey->text().trimmed());
}

void SpeechWidget::on_baiduSpeechSecretKey_editingFinished()
{
    us->set("baiduSpeech/secretKey", us->baiduSpeechSecretKey = ui->baiduSpeechSecretKey->text().trimmed());
}

void SpeechWidget::on_baiduSpeechAccessTokenEdit_editingFinished()
{
    us->set("baiduSpeech/accessToken", us->baiduSpeechAccessToken = ui->baiduSpeechAccessTokenEdit->text().trimmed());
}

void SpeechWidget::showEvent(QShowEvent *event)
{
    ui->baiduSpeechAccessTokenEdit->setText(us->baiduSpeechAccessToken);

    return QWidget::showEvent(event);
}

void SpeechWidget::on_refreshAccessTokenButton_clicked()
{
    refreshToken();
    ui->baiduSpeechAccessTokenEdit->setText(us->baiduSpeechAccessToken);
}
