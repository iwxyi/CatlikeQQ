#include <QMediaPlaylist>
#include <QPainter>
#include <QBitmap>
#include <QVideoSurfaceFormat>
#include <QFileInfo>
#include "videolabel.h"

VideoLabel::VideoLabel(QWidget *parent) : ClickLabel(parent)
{
    setStyleSheet("QLabel{background: transparent;}");

    player = new QMediaPlayer(this);

    connect(player, SIGNAL(positionChanged(qint64)), this, SLOT(positionChanged(qint64)));
    connect(player, SIGNAL(durationChanged(qint64)), this, SLOT(durationChanged(qint64)));
    connect(player, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(stateChanged(QMediaPlayer::State)));

    videoSurface = new VideoSurface(this);
    player->setVideoOutput(videoSurface);
    connect(videoSurface, &VideoSurface::frameAvailable, this, [=](QVideoFrame &frame){
        frame.map(QAbstractVideoBuffer::ReadOnly);
        videoSize = frame.size();
        auto format = QVideoFrame::imageFormatFromPixelFormat(frame.pixelFormat()); // QImage::Format_RGB32
        if (!outputed && format == QImage::Format_Invalid)
            qWarning() << "不支持的视频格式";
        QImage recvImage(frame.bits(), videoSize.width(), videoSize.height(), format);
        frame.unmap();
        QPixmap pixmap = QPixmap::fromImage(recvImage);
        pixmap = pixmap.scaled(this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        this->setPixmap(pixmap);

        static bool first = true;
        if (first)
        {
            first = false;

            // 视频圆角
            QPixmap maskPix(videoSize);
            maskPix.fill(Qt::transparent);
            QPainter painter(&maskPix);
            QPainterPath path;
            path.addRoundedRect(0, 0, pixmap.width(), pixmap.height(), 5, 5);
            painter.fillPath(path, Qt::white);
            setMask(maskPix.mask());
        }
        outputed = true;
    });
}

void VideoLabel::setMedia(QString path)
{
    if (!QFileInfo(path).exists())
    {
        qWarning() << "Video File Not Exist:" << path;
        return ;
    }
    if (QFileInfo(path).size() < 1024)
    {
        qWarning() << "Video File Not Intact:" << path << "  size=" << QFileInfo(path).size();
        return ;
    }
    QMediaPlaylist* list = new QMediaPlaylist;
    list->addMedia(QUrl::fromLocalFile(path));
    list->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);
    player->setPlaylist(list);
    qInfo() << "play video:" << path;
    player->play();
}

void VideoLabel::setRadius(int r)
{
    this->radius = r;
}

void VideoLabel::positionChanged(qint64 position)
{
}

void VideoLabel::durationChanged(qint64 duration)
{
}

void VideoLabel::stateChanged(QMediaPlayer::State state)
{
    if (state == QMediaPlayer::PlayingState && player->duration() == 0)
    {
        qWarning() << "Force Stop Wrong Player: duration = 0";
        player->stop();
    }
}
