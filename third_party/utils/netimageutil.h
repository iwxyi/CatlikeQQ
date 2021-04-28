#ifndef NETIMAGEUTIL_H
#define NETIMAGEUTIL_H

#include <QNetworkAccessManager>
#include <QEventLoop>
#include <QNetworkReply>
#include <QPainter>
#include <QFile>

class NetImageUtil : QObject
{
public:
    static QPixmap loadNetPixmap(QString url)
    {
        QNetworkAccessManager manager;
        QEventLoop loop;
        QNetworkReply *reply = manager.get(QNetworkRequest(url));
        //请求结束并下载完成后，退出子事件循环
        connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        //开启子事件循环
        loop.exec();
        QByteArray jpegData = reply->readAll();
        QPixmap pixmap;
        pixmap.loadFromData(jpegData);
        return pixmap;
    }

    /// 保存网络图片
    /// 因为不一定是静态图片，所以没法使用 QPixmap
    static void saveNetImage(QString url, QString path)
    {
        QNetworkAccessManager manager;
        QEventLoop loop;
        QNetworkReply *reply = manager.get(QNetworkRequest(url));
        //请求结束并下载完成后，退出子事件循环
        connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        //开启子事件循环
        loop.exec();
        QByteArray data = reply->readAll();
        QFile file(path);
        file.open(QIODevice::WriteOnly);
        file.write(data);
        file.close();
    }

    /// 转换为圆角头像
    /// radius: 半径，-1 时为圆形
    static QPixmap toRoundedPixmap(const QPixmap &pixmap, int radius = -1)
    {
        QPixmap dest(pixmap.size());
        dest.fill(Qt::transparent);
        QPainter painter(&dest);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
        QRect rect = QRect(0, 0, pixmap.width(), pixmap.height());
        if (radius == -1)
            radius = qMin(rect.width(), rect.height())/2;
        QPainterPath path;
        path.addRoundedRect(rect, radius, radius);
        painter.setClipPath(path);
        painter.drawPixmap(rect, pixmap);
        return dest;
    }
};

#endif // NETIMAGEUTIL_H
