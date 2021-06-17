#ifndef REMOTECONTROLSERVIE_H
#define REMOTECONTROLSERVIE_H

#include <QObject>
#include <QDesktopServices>

class RemoteControlServie : public QObject
{
    Q_OBJECT
public:
    explicit RemoteControlServie(QObject *parent = nullptr);

    void execCmd(QString cmd);

signals:

public slots:
};

#endif // REMOTECONTROLSERVIE_H
