#ifndef CQHTTPSERVICE_H
#define CQHTTPSERVICE_H

#include <QObject>
#include <QtWebSockets/QWebSocket>
#include <QHash>
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
    MsgBean &parseMsgDisplay(MsgBean& msg) const;

private:
    QWebSocket* socket = nullptr;

    qint64 myId = 0; // 自己的QQ号
    QString myNickname; // 自己的昵称
    QHash<qint64, QString> userNames; // 好友列表
    QHash<qint64, QString> groupNames; // 群列表
    QHash<qint64, QHash<qint64, QString>> groupMemberNames; // 群成员名字
    QHash<qint64, QPixmap> userHeads;
    QHash<qint64, QPixmap> groupHeads;
};

#endif // CQHTTPSERVICE_H
