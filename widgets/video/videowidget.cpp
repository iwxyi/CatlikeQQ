#include <QMediaPlaylist>
#include "videowidget.h"

VideoWidget::VideoWidget(QWidget *parent) : QVideoWidget(parent)
{
    player = new QMediaPlayer(this);
    player->setVideoOutput(this);

    connect(player, SIGNAL(positionChanged(qint64)), this, SLOT(positionChanged(qint64)));
    connect(player, SIGNAL(durationChanged(qint64)), this, SLOT(durationChanged(qint64)));
    connect(player, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(stateChanged(QMediaPlayer::State)));
}

void VideoWidget::setMedia(QString path)
{
    QMediaPlaylist* list = new QMediaPlaylist;
    list->addMedia(QUrl::fromLocalFile(path));
    list->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);
    player->setPlaylist(list);
    player->play();
}

void VideoWidget::positionChanged(qint64 position)
{
}

void VideoWidget::durationChanged(qint64 duration)
{

}

void VideoWidget::stateChanged(QMediaPlayer::State state)
{
}
