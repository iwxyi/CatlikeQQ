#include "headerutil.h"
#include "fileutil.h"
#include "netimageutil.h"

// 用户头像API：http://q1.qlogo.cn/g?b=qq&nk=QQ号&s=100&t=
QPixmap HeaderUtil::userHeader(qint64 id)
{
    if (isFileExist(rt->userHeader(id)))
    {
        return QPixmap(rt->userHeader(id));
    }
    else // 没有头像，联网获取
    {
        QString url = "http://q1.qlogo.cn/g?b=qq&nk=" + QString::number(id) + "&s=100&t=";
        QPixmap pixmap = NetImageUtil::loadNetPixmap(url);
        pixmap.save(rt->userHeader(id));
        return pixmap;
    }
}

QPixmap HeaderUtil::groupHeader(qint64 id)
{
    if (isFileExist(rt->groupHeader(id)))
    {
        return QPixmap(rt->groupHeader(id));
    }
    else // 没有头像，联网获取
    {
        QString url = "http://p.qlogo.cn/gh/" + QString::number(id) + "/" + QString::number(id) + "/100";
        QPixmap pixmap = NetImageUtil::loadNetPixmap(url);
        pixmap.save(rt->groupHeader(id));
        return pixmap;
    }
}
