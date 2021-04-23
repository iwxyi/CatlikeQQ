#ifndef RUNTIME_H
#define RUNTIME_H

#include <QObject>

class Runtime : public QObject
{
    Q_OBJECT
public:
    explicit Runtime(QObject *parent = nullptr);

    QString imageCache(QString id);

public:
    QString APP_PATH;
    QString DATA_PATH;
    QString IMAGE_PATH;
    QString CACHE_PATH;
    QString CACHE_IMAGE_PATH;
};

#endif // RUNTIME_H
