#ifndef CQHTTPSERVICE_H
#define CQHTTPSERVICE_H

#include <QObject>
#include <QtWebSockets/QWebSocket>
#include <QHash>
#include "defines.h"
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
    void loopStarted();

signals:
    void signalMessage(const MsgBean& msg);

public slots:
    void openHost(QString host);
    void sendMessage(const QString& text);
    void messageReceived(const QString &message);

private:
    void parseEchoMessage(const MyJson& json);
    void parsePrivateMessage(const MyJson& json);
    void parseGroupMessage(const MyJson& json);
    void parseGroupUpload(const MyJson& json);

private:
    QWebSocket* socket = nullptr;
};

#endif // CQHTTPSERVICE_H
