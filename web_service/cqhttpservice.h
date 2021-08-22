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
    void openHost(QString host, QString token);
    void sendMessage(const QString& text);
    void messageReceived(const QString &message);

    void refreshFriends();
    void refreshGroups();
    void refreshGroupMembers(qint64 groupId);
    void sendUserMsg(qint64 userId, const QString &message);
    void sendGroupMsg(qint64 groupId, const QString &message);

private:
    void parseEchoMessage(const MyJson& json);
    void parsePrivateMessage(const MyJson& json);
    void parseGroupMessage(const MyJson& json);
    void parseGroupUpload(const MyJson& json);
    void parseOfflineFile(const MyJson& json);
    void parseMessageSent(const MyJson& json);
    void parseFriendRecall(const MyJson& json);
    void parseGroupRecall(const MyJson& json);
    void parseGroupIncrease(const MyJson& json);
    void parseGroupCard(const MyJson& json);
    void parseGroupBan(const MyJson& json);

    void ensureFriendExist(FriendInfo user);
    void ensureGroupExist(GroupInfo group);

private:
    QWebSocket* socket = nullptr;
    QString host;
    QString accessToken;
    QTimer* retryTimer = nullptr;
};

#endif // CQHTTPSERVICE_H
