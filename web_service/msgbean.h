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
    }

    MsgBean(qint64 senderId)
        : senderId(senderId)
    {
    }

    MsgBean& group(qint64 groupId, QString card = "")
    {
        this->groupId = groupId;
        if (!card.isEmpty())
            nickname = card;
        return *this;
    }

    MsgBean& file(QString fileId, QString fileName, qint64 fileSize)
    {
        this->fileId = fileId;
        this->fileName = fileName;
        this->fileSize = fileSize;
        return *this;
    }
};

#endif // MSGBEAN_H
