#ifndef CQHTTPSERVICE_H
#define CQHTTPSERVICE_H

#include <QObject>
#include <QtWebSockets/QWebSocket>
#include "global.h"
#include "usersettings.h"
#include "msgbean.h"

#define SKT_DEB if (1) qDebug()

class MyJson;

class CqhttpService : public QObject
{
    Q_OBJECT
public:
    explicit CqhttpService(QObject *parent = nullptr);

private:
    void initWS();
    void initLoopData();

signals:
    void signalMessage(MsgBean msg);
    void signalReceivedGroupMessage(MsgBean msg);

public slots:
    void openHost(QString host);

    void messageReceived(const QString &message);

private:
    void parsePrivateMessage(const MyJson& json);
    void parseGroupMessage(const MyJson& json);
    void parseGroupUpload(const MyJson& json);

private:
    QWebSocket* socket = nullptr;
};

#endif // CQHTTPSERVICE_H
