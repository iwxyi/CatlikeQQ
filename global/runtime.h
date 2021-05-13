#ifndef RUNTIME_H
#define RUNTIME_H

#include <QObject>

class Runtime : public QObject
{
    Q_OBJECT
public:
    explicit Runtime(QObject *parent = nullptr);

    QString userHeader(qint64 id) const;
    QString userHeader(QString id) const;
    QString groupHeader(qint64 id) const;
    QString groupHeader(QString id) const;
    QString imageCache(QString id) const;
    QString imageSCache(QString id) const;
    QString faceCache(QString id) const;

public:
    QString APP_PATH;
    QString DATA_PATH;
    QString IMAGE_PATH;

    QString CACHE_PATH;
    QString CACHE_IMAGE_PATH;
    QString CACHE_IMAGE_S_PATH;
    QString CACHE_FACE_PATH;
    QString CACHE_USER_HEADER_PATH;
    QString CACHE_GROUP_HEADER_PATH;

    bool notificationSlient = false;

private:
    const QString imageSuffix = ".png";
};

extern Runtime* rt;

#endif // RUNTIME_H
