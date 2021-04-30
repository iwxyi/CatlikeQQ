#ifndef RUNTIME_H
#define RUNTIME_H

#include <QObject>

class Runtime : public QObject
{
    Q_OBJECT
public:
    explicit Runtime(QObject *parent = nullptr);

    QString userHeader(qint64 id) const;
    QString groupHeader(qint64 id) const;
    QString imageCache(QString id) const;
    QString faceCache(QString id) const;

public:
    QString APP_PATH;
    QString DATA_PATH;
    QString IMAGE_PATH;

    QString CACHE_PATH;
    QString CACHE_IMAGE_PATH;
    QString CACHE_FACE_PATH;
    QString CACHE_USER_HEADER_PATH;
    QString CACHE_GROUP_HEADER_PATH;
};

extern Runtime* rt;

#endif // RUNTIME_H
