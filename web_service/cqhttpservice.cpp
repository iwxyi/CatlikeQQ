#include <QSslConfiguration>
#include <QRegularExpression>
#include <QNetworkAccessManager>
#include <QEventLoop>
#include <QNetworkReply>
#include <QPainter>
#include <QtConcurrent/QtConcurrent>
#include "cqhttpservice.h"
#include "myjson.h"
#include "fileutil.h"
#include "netimageutil.h"
#include "signaltransfer.h"
#include "accountinfo.h"

CqhttpService::CqhttpService(QObject *parent) : QObject(parent)
{
    initWS();

    connect(sig, SIGNAL(hostChanged(QString, QString)), this, SLOT(openHost(QString, QString)));
}

void CqhttpService::initWS()
{
    socket = new QWebSocket();

    connect(socket, SIGNAL(textMessageReceived(const QString&)), this, SLOT(messageReceived(const QString&)));

    connect(socket, &QWebSocket::connected, this, [=]{
        loopStarted();
        emit sig->socketStateChanged(true);

        if (retryTimer)
            retryTimer->stop();
    });

    connect(socket, &QWebSocket::disconnected, this, [=]{
        emit sig->socketStateChanged(false);

        if (!retryTimer)
        {
            retryTimer = new QTimer(this);
            connect(retryTimer, &QTimer::timeout, this, [=]{
                openHost(this->host, this->accessToken);
            });
        }
        retryTimer->setInterval(30000);
        retryTimer->start();
    });
}

/// 连接socket时初始化一些数据
void CqhttpService::loopStarted()
{
    // 获取登录号信息
    {
        MyJson json;
        json.insert("action", "get_login_info");
        json.insert("echo", "get_login_info");
        socket->sendTextMessage(json.toBa());
    }
    // 获取好友列表
    refreshFriends();

    // 获取群列表
    refreshGroups();
}

void CqhttpService::openHost(QString host, QString token)
{
    if (host.isEmpty())
        return ;

    // 设置安全套接字连接模式（不知道有啥用）
    QSslConfiguration config = socket->sslConfiguration();
    config.setPeerVerifyMode(QSslSocket::VerifyNone);
    config.setProtocol(QSsl::TlsV1SslV3);
    socket->setSslConfiguration(config);

    this->host = host;
    this->accessToken = token;

    QNetworkRequest req(host);
    if (!token.isEmpty())
        req.setRawHeader("Authorization", ("Bearer " + token).toUtf8());
    socket->open(req);
}

void CqhttpService::sendMessage(const QString &text)
{
    socket->sendTextMessage(text);
}

/// 收到消息的根入口
/// 字段说明：https://docs.go-cqhttp.org/event
void CqhttpService::messageReceived(const QString &message)
{
    MyJson json(message.toUtf8());

    // 先判断是不是主动发消息过去，回复的
    if (json.contains("echo"))
    {
        // 因为可能会收到很长很长的数据（例如群成员），所以用了多线程
        QtConcurrent::run(this, &CqhttpService::parseEchoMessage, json);
        return ;
    }

    JS(json, post_type);
    if (post_type == "meta_event") // 心跳，忽略
    {
    }
    else if (post_type == "message") // 消息
    {
        JS(json, message_type);
        if (message_type == "private") // 私信
        {
            parsePrivateMessage(json);
        }
        else if (message_type == "group") // 群消息
        {
            parseGroupMessage(json);
        }
        else
        {
            qWarning() << "未处理类型的消息：" << json;
        }
    }
    else if (post_type == "notice")
    {
        JS(json, notice_type);
        if (notice_type == "group_upload")
        {
            parseGroupUpload(json);
        }
        else if (notice_type == "offline_file")
        {
            parseOfflineFile(json);
        }
        else
        {
            qWarning() << "未处理类型的通知：" << json;
        }
    }
    else if (post_type == "message_sent") // 自己发送的
    {
        JS(json, sub_type);
        parseMessageSent(json);
    }
    else
    {
        qWarning() << "未处理类型的数据" << json;
    }
}

void CqhttpService::parseEchoMessage(const MyJson &json)
{
    if (json.i("retcode") != 0)
    {
        qWarning() << "返回错误：" << json;
        return ;
    }
    JS(json, echo);
    if (echo == "get_login_info")
    {
        JO(json, data);
        JL(data, user_id);
        JS(data, nickname);
        ac->myId = user_id;
        ac->myNickname = nickname;
        qInfo() << "登录成功：" << nickname << user_id;
        emit sig->myAccount(user_id, nickname);
    }
    else if (echo == "get_friend_list")
    {
        qInfo() << "读取好友列表：" << json.a("data").size();
        ac->friendNames.clear();
        json.each("data", [=](MyJson fri) {
            JS(fri, nickname);
            JS(fri, remark); // 备注，如果为空则默认为nickname
            JL(fri, user_id);
            ac->friendNames.insert(user_id, remark.isEmpty() ? nickname : remark);
        });
    }
    else if (echo == "get_group_list")
    {
        qInfo() << "读取群列表：" << json.a("data").size();
        ac->groupNames.clear();
        json.each("data", [=](MyJson group) {
            JL(group, group_id);
            JS(group, group_name);
            ac->groupNames.insert(group_id, group_name);
        });
    }
    else if (echo == "send_private_msg" || echo == "send_group_msg")
    {
        // 发送消息的回复，不做处理
    }
    else if (echo.startsWith("get_group_member_list"))
    {
        QRegularExpression re("^get_group_member_list:(\\d+)$");
        QRegularExpressionMatch match;
        if (echo.indexOf(re, 0, &match) == -1)
        {
            qWarning() << "无法识别的群成员echo：" << echo;
            return ;
        }

        qint64 groupId = match.captured(1).toLongLong();
        if (!ac->groupMemberNames.contains(groupId))
            ac->groupMemberNames.insert(groupId, QHash<qint64, QString>());

        QHash<qint64, QString>* names = &ac->groupMemberNames[groupId];
        names->clear();
        json.each("data", [=](MyJson member) {
            JL(member, user_id);
            JS(member, card);
            JS(member, nickname);
            QString name = card;
            if (card.isEmpty()) // 没有群备注
            {
                // 优先使用好友备注
                if (ac->friendNames.contains(user_id))
                    name = ac->friendNames.value(user_id);
                if (name.isEmpty()) // 好友备注还是空的（应该不会，空的就已经用 nickname 了）
                    name = nickname;
            }
            names->insert(user_id, name);
        });
        qInfo() << "加载群成员：" << groupId << names->size();
        emit sig->groupMembersLoaded(groupId);
    }
    else
    {
        qWarning() << "未处理类型的返回：" << json;
    }
    return ;
}

void CqhttpService::parsePrivateMessage(const MyJson &json)
{
    /* {
        "font": 0,
        "message": "。。。",
        "message_id": -546429301,
        "message_type": "private",
        "post_type": "message",
        "raw_message": "。。。",
        "self_id": 1600631528,
        "sender": {
            "age": 0,
            "nickname": "三界仙霖之雨",
            "sex": "unknown",
            "user_id": 3308218798 // 发出的，自己发给别人，或者别人发给自己，或者自己发给自己
        },
        "sub_type": "friend",
        "target_id": 1600631528, // 发向的，同上，三种情况
        "time": 1623898608,
        "user_id": 3308218798
    }*/

    JS(json, sub_type); // 好友：friend，群临时会话：group，群里自己发送：group_self
    JS(json, message); // 消息内容
    JS(json, raw_message);
    JL(json, message_id);
    JL(json, target_id);

    JO(json, sender); // 发送者，但不保证存在
    JL(sender, user_id); // 发送者用户QQ号
    JS(sender, nickname);

    qInfo() << "收到私聊消息：" << user_id << "->" << target_id << nickname << message << message_id;

    MsgBean msg = MsgBean(user_id, nickname, message, message_id, sub_type)
            .frind(ac->friendNames.value(user_id, ""))
            .privt(target_id);
    emit signalMessage(msg);

    qint64 oppo_id = user_id == ac->myId ? target_id : user_id;
    if (!ac->userMsgHistory.contains(oppo_id))
        ac->userMsgHistory.insert(oppo_id, QList<MsgBean>());
    ac->userMsgHistory[oppo_id].append(msg);

    // 图片消息：文字1\r\n[CQ:image,file=8f84df65ee005b52f7f798697765a81b.image,url=http://c2cpicdw.qpic.cn/offpic_new/1600631528//1600631528-3839913603-8F84DF65EE005B52F7F798697765A81B/0?term=3]\r\n文字二……
}

void CqhttpService::parseGroupMessage(const MyJson &json)
{
    JS(json, sub_type); // 正常：normal，匿名：anonymous，系统：notice
    JS(json, message);
    JS(json, raw_message);
    JL(json, group_id); // 群号
    JL(json, message_id);

    JO(json, sender); // 发送者，但不保证存在
    JL(sender, user_id); // 发送者QQ号
    JS(sender, nickname);
    JS(sender, card); // 群名片/备注，可能为空
    JS(sender, role); // 角色：owner/admin/member

    if (sub_type == "anonymous" && !json.value("anonymous").isNull()) // 是匿名消息
    {
        JO(json, anonymous);
        JL(anonymous, id); // 匿名用户ID
        JS(anonymous, name); // 匿名用户名称
        JS(anonymous, flag); // 匿名用户flag，在调用禁言API时需要传入
        Q_UNUSED(id)
    }

    qInfo() << "收到群消息：" << group_id << ac->groupNames.value(group_id)
            << user_id << nickname << ac->friendNames.value(user_id)
            << message << message_id;

    MsgBean msg = MsgBean(user_id, nickname, message, message_id, sub_type)
            .frind(ac->friendNames.value(user_id, ""))
            .group(group_id, ac->groupNames.value(group_id), card);
    emit signalMessage(msg);

    if (!ac->groupMsgHistory.contains(group_id))
        ac->groupMsgHistory.insert(group_id, QList<MsgBean>());
    ac->groupMsgHistory[group_id].append(msg);
}

void CqhttpService::parseGroupUpload(const MyJson &json)
{
    JL(json, group_id); // QQ群号
    JL(json, user_id); // 发送者QQ号

    JO(json, file);
    JS(file, id); // 文件ID
    JS(file, name); // 文件名
    JL(file, size); // 文件大小（字节数）

    qInfo() << "收到群文件消息：" << group_id << ac->groupNames.value(group_id) << user_id << ac->friendNames.value(user_id) << name << size << id;

    MsgBean msg = MsgBean(user_id, ac->friendNames.value(user_id))
                       .group(group_id, ac->groupNames.value(group_id))
                       .file(id, name, size);
    emit signalMessage(msg);
}

void CqhttpService::parseOfflineFile(const MyJson &json)
{
    /*{
        "file": {
            "name": "hwpe_eda.vsdx",
            "size": 724289,
            "url": "http://183.247.242.34/ftn_handler/ed5fde809720294dced4364198424b4e17f5adf6fa7234a73de6dd012ec6cad620996e9c99c919b750ea9857b1f5247c28beed0195fa667faad3761e3a375f4e"
        },
        "notice_type": "offline_file",
        "post_type": "notice",
        "self_id": 1600631528,
        "time": 1624352580,
        "user_id": 3308218798
    }*/

    JL(json, user_id);

    JO(json, file);
    JS(file, name);
    JL(file, size);
    JS(file, url);

    qInfo() << "收到离线文件消息：" << user_id << ac->friendNames.value(user_id) << name << size;

    QString fileId = name + "_" + snum(size);
    MsgBean msg = MsgBean(user_id, ac->friendNames.value(user_id))
                       .file(fileId, name, size, url);
    emit signalMessage(msg);
}

void CqhttpService::parseMessageSent(const MyJson &json)
{
    JS(json, message_type);
    JS(json, sub_type);
    if (message_type == "group") // 自己发送的群聊消息
    {
        /*{
            "anonymous": null,
            "font": 0,
            "group_id": 461245039,
            "message": "还以为开的瓜是那个瓜",
            "message_id": 1508336484,
            "message_seq": 149753,
            "message_type": "group",
            "post_type": "message_sent",
            "raw_message": "还以为开的瓜是那个瓜",
            "self_id": 1600631528,
            "sender": {
                "age": 0,
                "area": "",
                "card": "懒一夕智能科技",
                "level": "",
                "nickname": "小乂",
                "role": "admin",
                "sex": "unknown",
                "title": "",
                "user_id": 1600631528
            },
            "sub_type": "normal",
            "time": 1623896873,
            "user_id": 1600631528
        }*/
        parseGroupMessage(json);
    }
    else if (message_type == "private") // 自己发送的私聊消息
    {
        /*{
            "font": 0,
            "message": "唉",
            "message_id": 462908353,
            "message_type": "private",
            "post_type": "message_sent",
            "raw_message": "唉",
            "self_id": 1600631528,
            "sender": {
                "age": 0,
                "nickname": "小乂",
                "sex": "unknown",
                "user_id": 1600631528
            },
            "sub_type": "friend",
            "target_id": 3308218798,
            "time": 1623897521,
            "user_id": 1600631528
        }*/
        parsePrivateMessage(json);
    }
}

void CqhttpService::refreshFriends()
{
    MyJson json;
    json.insert("action", "get_friend_list");
    json.insert("echo", "get_friend_list");
    socket->sendTextMessage(json.toBa());
}

void CqhttpService::refreshGroups()
{
    MyJson json;
    json.insert("action", "get_group_list");
    json.insert("echo", "get_group_list");
    socket->sendTextMessage(json.toBa());
}

void CqhttpService::refreshGroupMembers(qint64 groupId)
{
    MyJson json;
    json.insert("action", "get_group_member_list");
    MyJson params;
    params.insert("group_id", groupId);
    json.insert("params", params);
    json.insert("echo", "get_group_member_list:" + snum(groupId));
    sendMessage(json.toBa());
}

void CqhttpService::sendUserMsg(qint64 userId, const QString& message)
{
    MyJson json;
    json.insert("action", "send_private_msg");
    MyJson params;
    params.insert("user_id", userId);
    params.insert("message", message);
    json.insert("params", params);
    json.insert("echo", "send_private_msg");
    sendMessage(json.toBa());
    emit sig->myReplyUser(userId, message);
}

void CqhttpService::sendGroupMsg(qint64 groupId, const QString& message)
{
    MyJson json;
    json.insert("action", "send_group_msg");
    MyJson params;
    params.insert("group_id", groupId);
    params.insert("message", message);
    json.insert("params", params);
    json.insert("echo", "send_group_msg");
    sendMessage(json.toBa());
    emit sig->myReplyGroup(groupId, message);
}
