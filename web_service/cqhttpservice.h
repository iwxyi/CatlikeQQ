#ifndef CQHTTPSERVICE_H
#define CQHTTPSERVICE_H

#include <QObject>
#include <QtWebSockets/QWebSocket>
#include "global.h"
#include "usersettings.h"

#define SKT_DEB if (1) qDebug()

class MyJson;

class CqhttpService : public QObject
{
    Q_OBJECT
public:
    explicit CqhttpService(QObject *parent = nullptr);

private:
    void initWS();

signals:

public slots:
    void openHost(QString host);

    void messageReceived(const QString &message);

private:
    void receivedPrivate(const MyJson& json);

private:
    QWebSocket* socket = nullptr;
};

#endif // CQHTTPSERVICE_H
