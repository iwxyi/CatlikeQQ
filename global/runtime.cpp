#include "runtime.h"

Runtime::Runtime(QString path, QObject *parent) : QObject(parent)
{
    APP_PATH = path;
    DATA_PATH = APP_PATH + "/data/";
    IMAGE_PATH = DATA_PATH + "images/";
    CACHE_PATH = DATA_PATH + "cache/";
    CACHE_IMAGE_PATH = CACHE_PATH + "images/";
    CACHE_IMAGE_S_PATH = CACHE_PATH + "thumbnails/";
    CACHE_FACE_PATH = CACHE_PATH + "faces/";
    CACHE_USER_HEADER_PATH = CACHE_PATH + "user_headers/";
    CACHE_GROUP_HEADER_PATH = CACHE_PATH + "group_headers/";
    CACHE_FILE_PATH = CACHE_PATH + "files/";
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

QString Runtime::videoCache(QString id) const
{
    return CACHE_IMAGE_PATH + id + ".mp4";
}

QString Runtime::audioCache(QString id) const
{
    return CACHE_IMAGE_PATH + id + ".amr";
}

QString Runtime::faceCache(QString id) const
{
    return CACHE_FACE_PATH + id + imageSuffix;
}

QString Runtime::fileCache(QString name) const
{
    return CACHE_FILE_PATH + name;
}

QString Runtime::localImageCache(QString id) const
{
    return CACHE_FILE_PATH + "paste_" + id + ".png";
}
