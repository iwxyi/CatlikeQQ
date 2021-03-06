#ifndef REMOTECONTROLSERVIE_H
#define REMOTECONTROLSERVIE_H

#include <QObject>
#include <QDesktopServices>

class RemoteControlServie : public QObject
{
    Q_OBJECT
public:
    explicit RemoteControlServie(QObject *parent = nullptr);

    bool execCmd(QString cmd);

    void simulateKeys(QString seq, bool delay = false);

signals:
    void reply(QString text);

public slots:
};

#endif // REMOTECONTROLSERVIE_H
