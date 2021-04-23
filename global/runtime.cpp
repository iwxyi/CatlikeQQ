#include "runtime.h"

Runtime::Runtime(QObject *parent) : QObject(parent)
{

}

QString Runtime::imageCache(QString id)
{
    return CACHE_IMAGE_PATH + id + ".png";
}
