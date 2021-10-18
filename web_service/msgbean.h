#ifndef MSGBEAN_H
#define MSGBEAN_H

#include <QString>
#include <QPixmap>
#include <QDateTime>
#include <QMetaType>

struct FriendInfo
{
    qint64 userId = 0;
    QString nickname;
    QString remark; // 好友中就是备注，群组中是群备注
    qint64 lastMsgTime = 0;

    FriendInfo()
    {}

    FriendInfo(qint64 userId, QString nickname, QString remark)
    {
        this->userId = userId;
        this->nickname = nickname;
        this->remark = remark;
    }

    QString username() const
    {
        return remark.isEmpty() ? nickname : remark;
    }
};

struct GroupInfo
{
    qint64 groupId;
    QString name;
    QHash<qint64, FriendInfo> members;
    qint64 lastMsgTime = 0;

    GroupInfo()
    {}

    GroupInfo(qint64 groupId, QString name)
    {
        this->groupId = groupId;
        this->name = name;
    }
};

enum ActionType
{
    ActionMsg,
    ActionRecall,
    ActionJoin,
    ActionExit
};

struct MsgBean
{
    qint64 senderId = 0;
    QString nickname; // 优先群昵称/好友备注，才是原昵称
    QString message;
    QString rawMessage;
    qint64 messageId = 0;
    QString subType;
    QString remark; // 自己对TA的备注

    qint64 targetId = 0; // 准备发给谁，给对方还是给自己
    qint64 friendId = 0; // 如果是私聊消息，就是另一边的
    qint64 groupId = 0;
    QString groupName;
    QString groupCard; // 群昵称
    qint64 fromGroupId = 0; // 群私聊
    ActionType action = ActionMsg;

    QString fileId;
    QString fileName;
    qint64 fileSize;
    QString fileUrl; // 下载地址

    QString display; // 显示的纯文本
    QString imageId; // 显示唯一图片（不一定有）
    QColor bgColor; // 显示的背景（不一定有，除非开启动态背景）
    qint64 timestamp = 0; // 创建时间（毫秒级）

    MsgBean()
    {
        qRegisterMetaType<MsgBean>("MsgBean");
    }

    MsgBean(qint64 senderId, QString nickname, QString message, qint64 messageId, QString subType)
        : senderId(senderId), nickname(nickname), message(message), rawMessage(message), messageId(messageId), subType(subType)
    {
        qRegisterMetaType<MsgBean>("MsgBean");
        if (nickname.isEmpty())
            nickname = QString::number(senderId);
        timestamp = QDateTime::currentMSecsSinceEpoch();
    }

    MsgBean(qint64 senderId, QString nickname)
        : senderId(senderId), nickname(nickname)
    {
        qRegisterMetaType<MsgBean>("MsgBean");
        if (nickname.isEmpty())
            nickname = QString::number(senderId);
        timestamp = QDateTime::currentMSecsSinceEpoch();
    }

    MsgBean& privt(qint64 targetId, qint64 friendId)
    {
        if (targetId == 0)
            this->targetId = senderId;
        else
            this->targetId = targetId;
        this->friendId = friendId;
        return *this;
    }

    MsgBean& group(qint64 groupId, QString groupName, QString card = "")
    {
        this->groupId = groupId;
        this->groupName = groupName;
        if (!card.isEmpty())
            groupCard = card;
        if (groupName.isEmpty())
            groupName = QString::number(groupId);
        return *this;
    }

    MsgBean& file(QString fileId, QString fileName, qint64 fileSize, QString fileUrl = "")
    {
        this->fileId = fileId;
        this->fileName = fileName;
        this->fileSize = fileSize;
        this->fileUrl = fileUrl;
        return *this;
    }

    MsgBean& frind(QString remark)
    {
        this->remark = remark;
        return *this;
    }

    /**
     * @brief recall
     * @param messageId
     * @param userId 被撤回的发送者的ID
     * @param operatorId 操作者的ID
     * @param groupId
     * @return
     */
    MsgBean& recall(qint64 messageId, qint64 userId, qint64 operatorId, qint64 groupId = 0)
    {
        this->action = ActionRecall;
        this->messageId = messageId;
        this->senderId = operatorId;
        if (!groupId)
            this->friendId = userId;
        else
            this->groupId = groupId;
        return *this;
    }

    QString displayNickname() const
    {
        if (isGroup() && !groupCard.trimmed().isEmpty())
            return groupCard;
        if (!remark.trimmed().isEmpty())
            return remark;
        return nickname;
    }

    QString displayMessage() const
    {
        if (!display.isEmpty())
            return display;
        if (!fileId.isEmpty())
            return QString("[文件] %1").arg(fileName);
        return message;
    }

    bool isPrivate() const
    {
        return !groupId;
    }

    bool isGroup() const
    {
        return groupId;
    }

    bool isMsg() const
    {
        return action == ActionMsg;
    }

    bool is(ActionType action) const
    {
        return this->action == action;
    }

    bool isValid() const
    {
        return senderId != 0 || !message.isEmpty();
    }

    bool isSameObject(const MsgBean& m) const
    {
        return this->friendId == m.friendId
                && this->groupId == m.groupId;
    }

    bool hasAt(qint64 id) const
    {
        if (!id)
            return rawMessage.contains("[CQ:at,qq=all]");
        return rawMessage.contains("[CQ:at,qq=" + QString::number(id) + "]");
    }

    QString username() const
    {
        if (!groupCard.isEmpty())
            return groupCard;
        if (!remark.isEmpty())
            return remark;
        if (!nickname.isEmpty())
            return nickname;
        return QString::number(senderId);
    }

    QString titleName() const
    {
        if (isGroup())
            return groupName;
        return username();
    }

    bool is(const MsgBean& o) const
    {
        return this->senderId == o.senderId
                && this->targetId == o.targetId
                && this->groupId == o.groupId;
    }

    bool operator==(const MsgBean& o) const
    {
        return this->senderId == o.senderId
                && this->targetId == o.targetId
                && this->groupId == o.groupId
                && this->messageId == o.messageId;
    }

    QString debugString() const
    {
        if (isPrivate())
        {
            return QString("%1(%2):%3")
                    .arg(nickname)
                    .arg(senderId)
                    .arg(message);
        }
        else if (isGroup())
        {
            return QString("[%4(%5)]%1(%2):%3")
                    .arg(nickname)
                    .arg(senderId)
                    .arg(message)
                    .arg(groupName)
                    .arg(groupId);
        }
        else
        {
            return QString("%1(%2):%3")
                    .arg(nickname)
                    .arg(senderId)
                    .arg(message);
        }
    }
};

#endif // MSGBEAN_H
