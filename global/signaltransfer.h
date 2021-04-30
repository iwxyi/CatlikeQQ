#ifndef SIGNALTRANSFER_H
#define SIGNALTRANSFER_H

#include <QObject>

class SignalTransfer : public QObject
{
    Q_OBJECT
signals:
    void hostChanged(QString host);
    void socketStateChanged(bool connected);
    void myAccount(qint64 id, QString nickname);

    void imgLoaded(QString url, const QPixmap& pixmap);
    void faceLoaded(int id, const QPixmap& pixmap);
    void userHeadLoaded(qint64 id, const QPixmap& pixmap);
    void groupHeadLoaded(qint64 id, const QPixmap& pixmap);
};

extern SignalTransfer* sig;

#endif // SIGNALTRANSFER_H
