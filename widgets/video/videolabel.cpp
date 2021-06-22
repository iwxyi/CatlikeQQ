#include <QMediaPlaylist>
#include <QPainter>
#include <QBitmap>
#include "videolabel.h"

VideoLabel::VideoLabel(QWidget *parent) : QLabel(parent)
{
    setStyleSheet("QLabel{background: transparent;}");

    player = new QMediaPlayer(this);

    connect(player, SIGNAL(positionChanged(qint64)), this, SLOT(positionChanged(qint64)));
    connect(player, SIGNAL(durationChanged(qint64)), this, SLOT(durationChanged(qint64)));
    connect(player, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(stateChanged(QMediaPlayer::State)));

    videoSurface = new VideoSurface(this);
    player->setVideoOutput(videoSurface);
    connect(videoSurface, &VideoSurface::frameAvailable, this, [=](QVideoFrame &frame){
        QVideoFrame cloneFrame(frame);
        cloneFrame.map(QAbstractVideoBuffer::ReadOnly);
        videoSize = cloneFrame.size();
        auto format = QVideoFrame::imageFormatFromPixelFormat(cloneFrame.pixelFormat());
        QImage recvImage(cloneFrame.bits(), videoSize.width(), videoSize.height(), format);
        cloneFrame.unmap();
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
    });
}

void VideoLabel::setMedia(QString path)
{
    QMediaPlaylist* list = new QMediaPlaylist;
    list->addMedia(QUrl::fromLocalFile(path));
    list->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);
    player->setPlaylist(list);
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
}
