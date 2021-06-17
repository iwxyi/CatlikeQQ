#ifndef MSGBEAN_H
#define MSGBEAN_H

#include <QString>
#include <QPixmap>
#include <QDateTime>

struct MsgBean
{
    qint64 senderId = 0;
    QString nickname;
    QString message;
    qint64 messageId = 0;
    QString subType;
    QString remark; // 自己对TA的备注

    qint64 targetId = 0; // 准备发给谁，给对方还是给自己

    qint64 groupId = 0;
    QString groupName;
    QString groupCard; // 群昵称

    QString fileId;
    QString fileName;
    qint64 fileSize;

    QString display; // 显示的纯文本

    QString imageId; // 显示唯一图片（不一定有）
    QColor bgColor; // 显示的背景（不一定有，除非开启动态背景）
    qint64 timestamp = 0; // 创建时间（毫秒级）

    MsgBean(qint64 senderId, QString nickname, QString message, qint64 messageId, QString subType)
        : senderId(senderId), nickname(nickname), message(message), messageId(messageId), subType(subType)
    {
        if (nickname.isEmpty())
            nickname = QString::number(senderId);
        timestamp = QDateTime::currentMSecsSinceEpoch();
    }

    MsgBean(qint64 senderId, QString nickname)
        : senderId(senderId), nickname(nickname)
    {
        if (nickname.isEmpty())
            nickname = QString::number(senderId);
        timestamp = QDateTime::currentMSecsSinceEpoch();
    }

    MsgBean& privt(qint64 targetId)
    {
        if (targetId == 0)
            this->targetId = senderId;
        else
            this->targetId = targetId;
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

    MsgBean& file(QString fileId, QString fileName, qint64 fileSize)
    {
        this->fileId = fileId;
        this->fileName = fileName;
        this->fileSize = fileSize;
        return *this;
    }

    MsgBean& frind(QString remark)
    {
        this->remark = remark;
        return *this;
    }

    QString displayNickname() const
    {
        if (!groupCard.isEmpty())
            return groupCard;
        if (!remark.isEmpty())
            return remark;
        return nickname;
    }

    QString displayString() const
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
};

#endif // MSGBEAN_H
