#include "runtime.h"

Runtime::Runtime(QObject *parent) : QObject(parent)
{

}

QString Runtime::userHeader(qint64 id) const
{
    return CACHE_USER_HEADER_PATH + QString::number(id) + ".png";
}

QString Runtime::userHeader(QString id) const
{
    return CACHE_USER_HEADER_PATH + id + ".png";
}

QString Runtime::groupHeader(qint64 id) const
{
    return CACHE_GROUP_HEADER_PATH + QString::number(id) + ".png";
}

QString Runtime::groupHeader(QString id) const
{
    return CACHE_GROUP_HEADER_PATH + id + ".png";
}

QString Runtime::imageCache(QString id) const
{
    return CACHE_IMAGE_PATH + id + imageSuffix;
}

QString Runtime::imageSCache(QString id) const
{
    return CACHE_IMAGE_S_PATH + id + imageSuffix;
}

QString Runtime::faceCache(QString id) const
{
    return CACHE_FACE_PATH + id + imageSuffix;
}
