#include <QFile>
#include <QUuid>
#include <QFileInfo>

#include "httpuploader.h"

HttpUploader::HttpUploader(const QString& url, QObject* parent)
    : QObject(parent)
    , url(url)
    , post_content(QByteArray())
{
    multi_part = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    net_manager = new QNetworkAccessManager(this);
     connect(net_manager, SIGNAL(finished(QNetworkReply*)), this, SIGNAL(finished(QNetworkReply*)));
}

HttpUploader::~HttpUploader()
{
    if (multi_part != nullptr)
    {
        delete multi_part;
    }

    if (net_manager != nullptr)
    {
        delete net_manager;
    }
}

bool HttpUploader::addTextField(const QString &key, const QByteArray &value)
{
    QHttpPart text_part;
    text_part.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"" + key + "\""));
    text_part.setBody(value);

    multi_part->append(text_part);

    return true;
}

bool HttpUploader::addFilePath(const QString &key, const QString& file_path)
{
    QHttpPart file_part;
    QFileInfo upload_file_info(file_path);
    file_part.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
    file_part.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"" + key + "\"; filename=\"" + upload_file_info.fileName() + "\""));

    QFile *file = new QFile(file_path);
    file->open(QIODevice::ReadOnly);
    file->setParent(multi_part);
    file_part.setBodyDevice(file);

    multi_part->append(file_part);

    return true;
}

bool HttpUploader::post()
{
    QNetworkRequest request = QNetworkRequest(QUrl(url));
    net_reply = net_manager->post(request, multi_part);
    multi_part->setParent(net_reply); // delete the multiPart with the reply
    connect(net_reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SIGNAL(errored(QNetworkReply::NetworkError)));
    connect(net_reply, SIGNAL(uploadProgress(qint64, qint64)), this, SIGNAL(progress(qint64, qint64)));

    return true;
}
