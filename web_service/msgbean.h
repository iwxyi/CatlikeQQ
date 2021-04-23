#ifndef MSGBEAN_H
#define MSGBEAN_H

#include <QString>
#include <QPixmap>

struct MsgBean
{
    qint64 senderId = 0;
    QString nickname;
    QString message;
    qint64 messageId = 0;
    QString subType;

    qint64 groupId = 0;
    QString groupName;

    QString fileId;
    QString fileName;
    qint64 fileSize;

    QPixmap header;
    QString display;

    QString imageId; // 显示唯一图片（不一定有）
    QColor bgColor; // 显示的背景（不一定有，除非开启动态背景）

    MsgBean(qint64 senderId, QString nickname, QString message, qint64 messageId, QString subType)
        : senderId(senderId), nickname(nickname), message(message), messageId(messageId), subType(subType)
    {
        if (nickname.isEmpty())
            nickname = QString::number(senderId);
    }

    MsgBean(qint64 senderId, QString nickname)
        : senderId(senderId), nickname(nickname)
    {
        if (nickname.isEmpty())
            nickname = QString::number(senderId);
    }

    MsgBean& group(qint64 groupId, QString groupName, QString card = "")
    {
        this->groupId = groupId;
        this->groupName = groupName;
        if (!card.isEmpty())
            nickname = card;
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
