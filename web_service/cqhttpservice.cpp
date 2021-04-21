#include <QSslConfiguration>
#include "cqhttpservice.h"
#include "myjson.h"

CqhttpService::CqhttpService(QObject *parent) : QObject(parent)
{
    initWS();

    connect(sig, SIGNAL(hostChanged(QString)), this, SLOT(openHost(QString)));
}

void CqhttpService::initWS()
{
    socket = new QWebSocket();

    connect(socket, SIGNAL(textMessageReceived(const QString&)), this, SLOT(messageReceived(const QString&)));

    connect(socket, &QWebSocket::connected, this, [=]{
        emit sig->socketStateChanged(true);
    });

    connect(socket, &QWebSocket::disconnected, this, [=]{
        emit sig->socketStateChanged(false);
    });

    /* connect(socket, &QWebSocket::binaryMessageReceived, this, [=](const QByteArray &message){
        SKT_DEB << "binaryMessageReceived";
    });

    connect(socket, &QWebSocket::textFrameReceived, this, [=](const QString &frame, bool isLastFrame){
        SKT_DEB << "textFrameReceived";
    });

    connect(socket, &QWebSocket::textMessageReceived, this, [=](const QString &message){
        SKT_DEB << "textMessageReceived";
    });

    connect(socket, &QWebSocket::stateChanged, this, [=](QAbstractSocket::SocketState state){
        SKT_DEB << "stateChanged" << state;
    }); */
}

void CqhttpService::openHost(QString host)
{
    // 设置安全套接字连接模式（不知道有啥用）
    QSslConfiguration config = socket->sslConfiguration();
    config.setPeerVerifyMode(QSslSocket::VerifyNone);
    config.setProtocol(QSsl::TlsV1SslV3);
    socket->setSslConfiguration(config);

    socket->open(host);
}

void CqhttpService::messageReceived(const QString &message)
{
    MyJson json(message.toUtf8());
    JS(json, post_type);
    if (post_type == "meta_event") // 心跳，忽略
    {
    }
    else if (post_type == "message") // 消息
    {
        JS(json, message_type);
        if (message_type == "private") // 私信
        {
            receivedPrivate(json);
        }
        else
        {
            qDebug() << "未处理类型的消息：" << json;
        }
    }
    else
    {
        qDebug() << "未处理类型的数据" << json;
    }
}

void CqhttpService::receivedPrivate(const MyJson &json)
{
    JS(json, message);
    JS(json, raw_message);
    JL(json, self_id); // 自己的QQ号
    JS(json, sub_type); // friend
    JS(json, time); // 10位时间戳

    JO(json, sender);
    JS(sender, nickname);
    JL(sender, user_id);

    qDebug() << "收到好友消息：" << nickname << user_id << message;
}
