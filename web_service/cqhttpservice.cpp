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

void CqhttpService::sendTextMessage(const QString &text)
{
    socket->sendTextMessage(text);
}

void CqhttpService::sendJsonMessage(const MyJson &json)
{
    socket->sendTextMessage(json.toBa());
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
        if (notice_type == "group_upload") // 群文件
        {
            parseGroupUpload(json);
        }
        else if (notice_type == "offline_file") // 私聊文件
        {
            parseOfflineFile(json);
        }
        else if (notice_type == "group_increase") // 群成员加入
        {
            parseGroupIncrease(json);
        }
        else if (notice_type == "group_recall") // 撤销群消息
        {
            parseGroupRecall(json);
        }
        else if (notice_type == "friend_recall") // 撤销私聊消息
        {
            parseFriendRecall(json);
        }
        else if (notice_type == "group_card") // 修改群名片
        {
            parseGroupCard(json);
        }
        else if (notice_type == "group_ban") // 禁言群成员
        {
            parseGroupBan(json);
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
        ac->friendList.clear();
        qInfo() << "读取好友列表：" << json.a("data").size();
        json.each("data", [=](MyJson fri) {
            JS(fri, nickname);
            JS(fri, remark); // 备注，如果为空则默认为nickname
            JL(fri, user_id);
            ac->friendList.insert(user_id, FriendInfo(user_id, nickname, remark));
        });
        emit sig->myFriendsLoaded();
    }
    else if (echo == "get_group_list")
    {
        ac->groupList.clear();
        qInfo() << "读取群列表：" << json.a("data").size();
        json.each("data", [=](MyJson group) {
            JL(group, group_id);
            JS(group, group_name);
            ac->groupList.insert(group_id, GroupInfo(group_id, group_name));
        });
        emit sig->myGroupsLoaded();
    }
    else if (echo == "send_private_msg" || echo == "send_group_msg")
    {
        // 发送消息的回复，不做处理
    }
    else if (echo.startsWith("get_group_member_list")) // 加载群成员
    {
        QRegularExpression re("^get_group_member_list:(\\d+)$");
        QRegularExpressionMatch match;
        if (echo.indexOf(re, 0, &match) == -1)
        {
            qWarning() << "无法识别的群成员echo：" << echo;
            return ;
        }

        // 可能是新加入的群组，但是没有刷新
        qint64 groupId = match.captured(1).toLongLong();
        if (!ac->groupList.contains(groupId))
            ac->groupList.insert(groupId, GroupInfo());

        auto& members = ac->groupList[groupId].members;
        members.clear();
        json.each("data", [&](MyJson member) {
            JL(member, user_id);
            JS(member, card);
            JS(member, nickname);
            QString name = card;
            members[user_id] = FriendInfo(user_id, nickname, card);
        });
        qInfo() << "加载群成员：" << groupId << members.size();
        emit sig->groupMembersLoaded(groupId);
        ac->gettingGroupMembers.remove(groupId);
    }
    else if (echo.startsWith("msg_recall_private"))
    {
        QRegularExpression re("^msg_recall_private:(\\d+)_(-?\\w+)$");
        QRegularExpressionMatch match;
        if (echo.indexOf(re, 0, &match) == -1)
        {
            qWarning() << "无法识别的撤回echo：" << echo;
            return ;
        }
        qint64 friendId = match.captured(1).toLongLong();
        qint64 messageId = match.captured(2).toLongLong();
        MsgBean msg = MsgBean().recall(messageId, friendId, ac->myId);
        emit signalMessage(msg);
    }
    else if (echo.startsWith("msg_recall_group"))
    {
        // 群消息撤回会从ws发过来，这里不需要处理
    }
    else if (echo.startsWith("get_msg"))
    {
        if (echo.startsWith("get_msg:get_group_msg_history"))
        {
            QRegularExpression re("^get_msg:get_group_msg_history:(\\d+)_(-?\\w+)$");
            QRegularExpressionMatch match;
            if (echo.indexOf(re, 0, &match) == -1)
            {
                qWarning() << "无法识别的获取真实消息echo：" << echo;
                return ;
            }
            qint64 groupId = match.captured(1).toLongLong();
            qint64 messageId = match.captured(2).toLongLong();

            // 获取到真实ID
            JO(json, data);
            JL(data, real_id);
            // qDebug() << "get_msg:" << messageId << "->" << real_id;

            // 发送获取历史记录
            getGroupMsgHistory(groupId, messageId, real_id);
        }
    }
    else if (echo.startsWith("get_group_msg_history"))
    {
        QRegularExpression re("^get_group_msg_history:(\\d+)_(-?\\w+)$");
        QRegularExpressionMatch match;
        if (echo.indexOf(re, 0, &match) == -1)
        {
            qWarning() << "无法识别的获取群消息历史echo：" << echo;
            return ;
        }
        qint64 groupId = match.captured(1).toLongLong();
        qint64 messageId = match.captured(2).toLongLong();

        // 解析列表
        JO(json, data);
        JA(data, messages);
        // qDebug() << "get_group_msg_history" << messages.size();
        parseGetGroupMsgHistory(groupId, messageId, messages);
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

    /*{
        "font": 0,
        "message": "。。。",
        "message_id": -539978662,
        "message_type": "private",
        "post_type": "message",
        "raw_message": "。。。",
        "self_id": 1600631528,
        "sender": {
            "age": 0,
            "group_id": 647637553, // 群号
            "nickname": "用户昵称",
            "sex": "unknown",
            "user_id": 1057332539
        },
        "sub_type": "group", // 群私聊消息
        "temp_source": 0,
        "time": 1632823173,
        "user_id": 1057332539
    }*/

    JS(json, sub_type); // 好友：friend，群临时会话：group，群里自己发送：group_self(?)
    JS(json, message); // 消息内容
    JS(json, raw_message);
    JL(json, message_id);

    JL(json, target_id);
    JO(json, sender); // 发送者，但不保证存在
    JL(sender, user_id); // 发送者用户QQ号
    JS(sender, nickname);

    if (target_id == 0) // 非好友私聊发过来的，没有targetId
        target_id = ac->myId;

    qint64 friendId = target_id == ac->myId ? user_id : target_id;
    ensureFriendExist(FriendInfo(friendId, nickname, ""));
    qInfo() << "收到私聊消息：" << user_id << "->" << target_id << nickname << message << message_id;

    MsgBean msg = MsgBean(user_id, nickname, message, message_id, sub_type)
            .frind(ac->friendList.contains(friendId) ? ac->friendList[friendId].remark : "")
            .privt(target_id, friendId);

    if (sub_type == "group")
    {
        // 群消息，带一个ID号的
        // 如果要回复，也需要带这个ID号（还需要是管理员/群主）
        msg.fromGroupId = sender.l("group_id");
    }

    emit signalMessage(msg);

    // 图片消息：文字1\r\n[CQ:image,file=8f84df65ee005b52f7f798697765a81b.image,url=http://c2cpicdw.qpic.cn/offpic_new/1600631528//1600631528-3839913603-8F84DF65EE005B52F7F798697765A81B/0?term=3]\r\n文字二……
}

void CqhttpService::parseGroupMessage(const MyJson &json)
{
    JS(json, sub_type); // 正常：normal，匿名：anonymous，系统：notice
    JS(json, message);
    JS(json, raw_message);
    JL(json, group_id); // 群号
    JL(json, message_id);
    JL(json, time); // 秒，要转换为毫秒

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
        user_id = id; // 设置为匿名ID
        card = name;  // 设置为匿名名称
    }

    ensureGroupExist(GroupInfo(group_id, ""));
    qInfo() << "收到群消息：" << group_id << ac->groupList.value(group_id).name
            << user_id << nickname << ac->friendName(user_id)
            << message << message_id;

    MsgBean msg = MsgBean(user_id, nickname, message, message_id, sub_type)
            .frind(ac->friendList.contains(user_id) ? ac->friendList.value(user_id).remark : "")
            .group(group_id, ac->groupList.value(group_id).name, card);
    msg.timestamp = time * 1000;
    emit signalMessage(msg);
}

void CqhttpService::parseGetGroupMsgHistory(qint64 groupId, qint64 messageId, const QJsonArray &array)
{
    if (array.size() == 0)
    {
        qInfo() << "没有可获取的云端历史记录";
        return ;
    }

    // 解析 QJsonArray
    QList<MsgBean> hiss;
    for (int i = 0; i < array.size(); i++)
    {
        MyJson json = array.at(i).toObject();

        JS(json, sub_type); // 正常：normal，匿名：anonymous，系统：notice
        JS(json, message);
        JS(json, raw_message);
        JL(json, group_id); // 群号
        JL(json, message_id);
        JL(json, time); // 秒，要转换为毫秒

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
            user_id = id; // 设置为匿名ID
            card = name;  // 设置为匿名名称
        }

        qInfo() << "群消息历史：" << group_id << ac->groupList.value(group_id).name
                << user_id << nickname << ac->friendName(user_id)
                << message << message_id << time;

        MsgBean msg = MsgBean(user_id, nickname, message, message_id, sub_type)
                .frind(ac->friendList.contains(user_id) ? ac->friendList.value(user_id).remark : "")
                .group(group_id, ac->groupList.value(group_id).name, card);
        msg.timestamp = time * 1000;
        hiss.append(msg);
    }

    // 添加到历史中
    if (!ac->groupList.contains(groupId))
    {
        // TODO: 刷新群聊列表
        qWarning() << "群聊列表中未包括该群组，若新入群请刷新群聊";
        return ;
    }

    if (!ac->groupMsgHistory.contains(groupId))
        ac->groupMsgHistory.insert(groupId, QList<MsgBean>());
    auto all = &(ac->groupMsgHistory[groupId]);

    // 去除重复消息
    if (hiss.last().messageId == messageId)
    {
        hiss.removeLast();
    }

    // 添加到开头
    for (int i = hiss.size() - 1; i >= 0; --i)
    {
        all->insert(0, hiss.at(i));
    }

    // 发送信号，通知卡片控件
    emit sig->groupMsgHistoryLoaded(groupId, messageId, hiss.size());
    ac->gettingGroupMsgHistories.remove(groupId);
}

void CqhttpService::parseGroupUpload(const MyJson &json)
{
    JL(json, group_id); // QQ群号
    JL(json, user_id); // 发送者QQ号

    JO(json, file);
    JS(file, id); // 文件ID
    JS(file, name); // 文件名
    JL(file, size); // 文件大小（字节数）

    ensureGroupExist(GroupInfo(group_id, ""));
    qInfo() << "收到群文件消息：" << group_id << ac->groupList.value(group_id).name << user_id << ac->friendName(user_id) << name << size << id;

    MsgBean msg = MsgBean(user_id, ac->friendName(user_id))
                       .group(group_id, ac->groupList.value(group_id).name)
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
    JL(json, self_id);

    JO(json, file);
    JS(file, name);
    JL(file, size);
    JS(file, url);

    ensureFriendExist(FriendInfo(user_id, "", ""));
    qInfo() << "收到离线文件消息：" << user_id << ac->friendName(user_id) << name << size;

    QString fileId = name + "_" + snum(size);
    MsgBean msg = MsgBean(user_id, ac->friendName(user_id))
                       .file(fileId, name, size, url)
                       .privt(self_id, user_id);
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
    else
    {
        qWarning() << "未处理类型的sent数据" << json;
    }
}

void CqhttpService::parseFriendRecall(const MyJson &json)
{
    /* {
        "message_id": -1001132594,
        "notice_type": "friend_recall",
        "post_type": "notice",
        "self_id": 1600631528,
        "time": 1628479912,
        "user_id": 3308218798
    } */

    JL(json, user_id); // 这是好友消息撤回，自己撤回的接收不到，只能监听撤回自己的回调
    JL(json, message_id);

    qInfo() << "用户撤回：" << user_id << ac->friendName(user_id) << message_id;
    qint64 friendId = user_id;
    MsgBean msg = MsgBean().recall(message_id, friendId, user_id);
    emit signalMessage(msg);
}

void CqhttpService::parseGroupRecall(const MyJson &json)
{
    /* {
        "group_id": 647637553,
        "message_id": -2062705571,
        "notice_type": "group_recall",
        "operator_id": 1749535518,
        "post_type": "notice",
        "self_id": 1600631528,
        "time": 1628479447,
        "user_id": 1749535518
    } */

    JL(json, group_id);
    JL(json, message_id);
    JL(json, user_id); // 发送者ID
    JL(json, operator_id); // 操作者ID（发送者或者群管理员）

    qInfo() << "群消息撤回：" << group_id << ac->groupList.value(group_id).name << message_id;
    MsgBean msg = MsgBean().recall(message_id, user_id, operator_id, group_id);
    emit signalMessage(msg);
}

void CqhttpService::parseGroupIncrease(const MyJson &json)
{
    /* {
        "group_id": 647637553,
        "notice_type": "group_increase",
        "operator_id": 0,
        "post_type": "notice",
        "self_id": 1600631528,
        "sub_type": "approve",
        "time": 1628479399,
        "user_id": 2645311486
    } */

    JS(json, sub_type);
    if (sub_type == "approve")
    {
        JL(json, group_id);
        JL(json, user_id);

        qInfo() << "用户加入群组" << group_id << ac->groupList.value(group_id).name << " --> " << user_id;
    }
    else
    {
        qWarning() << "未处理类型的数据" << json;
    }
}

void CqhttpService::parseGroupCard(const MyJson &json)
{
    /* {
        "card_new": "深圳--龙岗区-Assembly line worker",
        "card_old": "深圳--龙岗区--嵌入式软件工程师",
        "group_id": 647637553,
        "notice_type": "group_card",
        "post_type": "notice",
        "self_id": 1600631528,
        "time": 1628482914,
        "user_id": 1749535518
    } */

    JS(json, card_new);
    JS(json, card_old);
    JL(json, group_id);
    JL(json, user_id);

    qInfo() << "禁言群成员："<< group_id << user_id << card_old << card_new;
}

void CqhttpService::parseGroupBan(const MyJson &json)
{
    /* {
        "duration": 600,
        "group_id": 647637553,
        "notice_type": "group_ban",
        "operator_id": 1466877104,
        "post_type": "notice",
        "self_id": 1600631528,
        "sub_type": "ban",
        "time": 1628482931,
        "user_id": 1749535518
    } */

    JL(json, group_id);
    JL(json, user_id);
    JL(json, operator_id);
    JL(json, duration); // 秒

    qInfo() << "群成员禁言：" << group_id << user_id << duration << operator_id;
}

void CqhttpService::ensureFriendExist(FriendInfo user)
{
    if (ac->friendList.contains(user.userId))
        return ;
    ac->friendList.insert(user.userId, user);
    qInfo() << "补充好友：" << user.userId << user.username();
}

void CqhttpService::ensureGroupExist(GroupInfo group)
{
    if (ac->groupList.contains(group.groupId))
        return ;
    ac->groupList.insert(group.groupId, group);
    qInfo() << "补充群组：" << group.groupId << group.name;
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
    if (ac->gettingGroupMembers.contains(groupId))
        return ;
    ac->gettingGroupMembers[groupId] = true;
    MyJson json;
    json.insert("action", "get_group_member_list");
    MyJson params;
    params.insert("group_id", groupId);
    json.insert("params", params);
    json.insert("echo", "get_group_member_list:" + snum(groupId));
    sendTextMessage(json.toBa());
}

void CqhttpService::sendPrivateMsg(qint64 userId, const QString& message, qint64 fromGroupId)
{
    MyJson json;
    json.insert("action", "send_private_msg");
    MyJson params;
    params.insert("user_id", userId);
    params.insert("message", message);
    if (fromGroupId)
        params.insert("group_id", fromGroupId);
    json.insert("params", params);
    json.insert("echo", "send_private_msg");
    sendTextMessage(json.toBa());
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
    sendTextMessage(json.toBa());
    emit sig->myReplyGroup(groupId, message);
}

/// 获取历史消息，这里分两步：
/// get_message: message_id -> real_id (message_seq)
/// get_group_msg_history: real_id -> messages[19] (最后一条是传参的，加载前面18条)
void CqhttpService::getGroupMsgHistory(qint64 groupId, qint64 messageId, qint64 realId)
{
    if (!realId) // 第一步
    {
        if (ac->gettingGroupMsgHistories.contains(groupId))
            return ;
        ac->gettingGroupMsgHistories[groupId] = true;
    }
    MyJson json;
    if (!realId && messageId)
    {
        json.insert("action", "get_msg");
        MyJson params;
        params.insert("group_id", groupId);
        params.insert("message_id", messageId);
        json.insert("params", params);
        json.insert("echo", "get_msg:get_group_msg_history:" + snum(groupId) + "_" + snum(messageId));
    }
    else
    {
        json.insert("action", "get_group_msg_history");
        MyJson params;
        params.insert("group_id", groupId);
        params.insert("message_seq", realId);
        json.insert("params", params);
        json.insert("echo", "get_group_msg_history:" + snum(groupId) + "_" + snum(messageId));
    }
    sendTextMessage(json.toBa());
}
