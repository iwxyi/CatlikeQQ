#include <QSslConfiguration>
#include <QRegularExpression>
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
        loopStarted();
        emit sig->socketStateChanged(true);
    });

    connect(socket, &QWebSocket::disconnected, this, [=]{
        emit sig->socketStateChanged(false);
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
    {
        MyJson json;
        json.insert("action", "get_friend_list");
        json.insert("echo", "get_friend_list");
        socket->sendTextMessage(json.toBa());
    }

    // 获取群列表
    {
        MyJson json;
        json.insert("action", "get_group_list");
        json.insert("echo", "get_group_list");
        socket->sendTextMessage(json.toBa());
    }
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
        parseEchoMessage(json);
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
            qDebug() << "未处理类型的消息：" << json;
        }
    }
    else if (post_type == "notice")
    {
        JS(json, notice_type);
        if (notice_type == "group_upload")
        {
            parseGroupUpload(json);
        }
        else
        {
            qDebug() << "未处理类型的通知：" << json;
        }
    }
    else
    {
        qDebug() << "未处理类型的数据" << json;
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
        myId = user_id;
        myNickname = nickname;
        qInfo() << "登录成功：" << nickname << user_id;
        emit sig->myAccount(user_id, nickname);
    }
    else if (echo == "get_friend_list")
    {
        qInfo() << "读取好友列表：" << json.a("data").size();
        json.each("data", [=](MyJson fri) {
            JS(fri, nickname);
            JS(fri, remark); // 备注，如果为空则默认为nickname
            JL(fri, user_id);
            userNames.insert(user_id, remark.isEmpty() ? nickname : remark);
        });
    }
    else if (echo == "get_group_list")
    {
        qInfo() << "读取群列表：" << json.a("data").size();
        json.each("data", [=](MyJson group) {
            JL(group, group_id);
            JS(group, group_name);
            groupNames.insert(group_id, group_name);
        });
    }
    else if (echo == "send_private_msg" || echo == "send_group_msg")
    {
        // 发送消息的回复，不做处理
    }
    else
    {
        qDebug() << "未处理类型的返回：" << json;
    }
    return ;
}

void CqhttpService::parsePrivateMessage(const MyJson &json)
{
    JS(json, sub_type); // 好友：friend，群临时会话：group，群里自己发送：group_self
    JS(json, message); // 消息内容
    JS(json, raw_message);
    JL(json, message_id);

    JO(json, sender); // 发送者，但不保证存在
    JL(sender, user_id); // 发送者用户QQ号
    JS(sender, nickname);

    MsgBean msg(user_id, nickname, message, message_id, sub_type);
    parseMsgDisplay(msg);
    emit signalMessage(msg);
    qInfo() << "收到好友消息：" << user_id << nickname << message << message_id;

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
    }

    MsgBean msg = MsgBean(user_id, nickname, message, message_id, sub_type).group(group_id, groupNames.value(group_id), card);
    parseMsgDisplay(msg);
    emit signalMessage(msg);
    qInfo() << "收到群消息：" << group_id << groupNames.value(group_id) << user_id << userNames.value(user_id) << message << message_id;
}

void CqhttpService::parseGroupUpload(const MyJson &json)
{
    JL(json, group_id); // QQ群号
    JL(json, user_id); // 发送者QQ号

    JO(json, file);
    JS(file, id); // 文件ID
    JS(file, name); // 文件名
    JL(file, size); // 文件大小（字节数）

    MsgBean msg = MsgBean(user_id, userNames.value(user_id))
                       .group(group_id, groupNames.value(group_id))
                       .file(id, name, size);
    parseMsgDisplay(msg);
    emit signalMessage(msg);
    qInfo() << "收到群文件消息：" << group_id << groupNames.value(group_id) << user_id << userNames.value(user_id) << name << size << id;
}

MsgBean& CqhttpService::parseMsgDisplay(MsgBean &msg) const
{
    QString text = msg.message;
    QRegularExpression re;
    QRegularExpressionMatch match;

    // 头像
    // 群头像API：https://p.qlogo.cn/gh/{group_id}/{group_id}/100


    // #替换CQ
    // 文件
    if (!msg.fileId.isEmpty())
    {
        text = "[文件] " + msg.fileName;
    }

    // 表情
    text.replace(QRegExp("\\[CQ:face,id=\\d+\\]"), "[表情]");

    // 图片
    // 图片格式：[CQ:image,file=e9f40e7fb43071e7471a2add0df33b32.image,url=http://gchat.qpic.cn/gchatpic_new/707049914/3934208404-2722739418-E9F40E7FB43071E7471A2ADD0DF33B32/0?term=3]
    text.replace(QRegExp("\\[CQ:image,.+\\]"), "[图片]");

    // 回复


    // 艾特
    text.replace(QRegExp("\\[CQ:at,qq=(\\d+)\\]"), "@\\1");


    // #处理长度
    if (text.length() > us->msgMaxLength)
        text = text.left(us->msgMaxLength) + "...";

    msg.display = text;
    return msg;
}
