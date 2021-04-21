#ifndef SIGNALTRANSFER_H
#define SIGNALTRANSFER_H

#include <QObject>

class SignalTransfer : public QObject
{
    Q_OBJECT
signals:
    void hostChanged(QString host);
    void socketStateChanged(bool connected);
};

#endif // SIGNALTRANSFER_H
