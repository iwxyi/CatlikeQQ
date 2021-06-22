#ifndef VIDEOLABEL_H
#define VIDEOLABEL_H

#include <QLabel>
#include <QMediaPlayer>
#include "videosurface.h"

class VideoLabel : public QLabel
{
    Q_OBJECT
public:
    VideoLabel(QWidget* parent = nullptr);

    void setMedia(QString path);

    void setRadius(int r);

private slots:
    void positionChanged(qint64 position);

    void durationChanged(qint64 duration);

    void stateChanged(QMediaPlayer::State state);

public:
    QMediaPlayer* player;
    VideoSurface *videoSurface;
    QSize videoSize;
    int radius = 0;
};

#endif // VIDEOLABEL_H
