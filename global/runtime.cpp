#include "runtime.h"

Runtime::Runtime(QObject *parent) : QObject(parent)
{

}

QString Runtime::userHeader(qint64 id) const
{
    return CACHE_USER_HEADER_PATH + QString::number(id) + ".png";
}

QString Runtime::groupHeader(qint64 id) const
{
    return CACHE_GROUP_HEADER_PATH + QString::number(id) + ".png";
}

QString Runtime::imageCache(QString id) const
{
    return CACHE_IMAGE_PATH + id + ".png";
}
