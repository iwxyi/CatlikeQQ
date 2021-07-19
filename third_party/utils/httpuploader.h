#ifndef HTTPUPLOADER_H
#define HTTPUPLOADER_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QtNetwork/QHttpPart>
#include <QtNetwork/QHttpMultiPart>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkAccessManager>

class HttpUploader : public QObject
{
    Q_OBJECT
public:
    HttpUploader(const QString& url, QObject* parent = nullptr);
    ~HttpUploader();

    void setUrl(const QString& url) { this->url = url; }
    QString getUrl() { return url; }

    bool addTextField(const QString& key, const QByteArray& value);
    bool addFilePath(const QString& key, const QString& file_path);
    bool post();
    QNetworkAccessManager* getNetworkManager() { return net_manager; }

signals:
    void finished(QNetworkReply* reply);
    void errored(QNetworkReply::NetworkError code);
    void progress(qint64 sent, qint64 total);

private:
    QString url;
    QByteArray post_content;

    QNetworkAccessManager*  net_manager = nullptr;
    QNetworkReply*          net_reply = nullptr;
    QHttpMultiPart*         multi_part = nullptr;
};

#endif // HTTPUPLOADER_H
