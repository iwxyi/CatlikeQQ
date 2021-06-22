#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QObject>
#include <QVideoWidget>
#include <QMediaPlayer>

class VideoWidget : public QVideoWidget
{
    Q_OBJECT
public:
    VideoWidget(QWidget* parent = nullptr);

    void setMedia(QString path);

private slots:
    void positionChanged(qint64 position);

    void durationChanged(qint64 duration);

    void stateChanged(QMediaPlayer::State state);

public:
    QMediaPlayer* player;
};

#endif // VIDEOWIDGET_H
