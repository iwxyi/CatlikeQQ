#ifndef MSGBEAN_H
#define MSGBEAN_H

#include <QString>

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
        if (!fileId.isEmpty())
            return QString("[文件] %1").arg(fileName);
        return message;
    }
};

#endif // MSGBEAN_H
