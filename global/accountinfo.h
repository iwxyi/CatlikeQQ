#ifndef ACCOUNTINFO_H
#define ACCOUNTINFO_H

#include <QString>
#include <QHash>
#include <QColor>
#include "msgbean.h"

class AccountInfo
{
public:
    struct CardColor
    {
        QColor bg;
        QColor fg;

        bool isValid() const
        {
            return bg.isValid() && fg.isValid();
        }
    };

    qint64 myId = 0; // 自己的QQ号
    QString myNickname; // 自己的昵称
    QPixmap myHeader; // 自己的头像

    QHash<qint64, FriendInfo> friendList; // 好友列表
    QHash<qint64, GroupInfo> groupList; // 群列表

    QHash<qint64, CardColor> userHeaderColor; // 用户头像，好友+群员+陌生人
    QHash<qint64, CardColor> groupHeaderColor; // 群头像
    QHash<qint64, QHash<qint64, QColor>> groupMemberColor; // 群组里每位用户的颜色

    QHash<qint64, QList<MsgBean>> userMsgHistory; // 私聊消息记录（不包括自己）
    QHash<qint64, QList<MsgBean>> groupMsgHistory; // 群聊消息记录（不包括自己）

    QHash<qint64, qint64> aiReplyPrivateTime; // 私聊最近一次自动回复
    QHash<qint64, qint64> aiReplyGroupTime; // 群组最近一次自动回复

    bool lastReceiveShowIsUser = false; // 最后接受到的一条消息是用户true还是群组false
    qint64 lastReceiveShowId = 0; // 最后接收到的用户ID或者群组ID

    QHash<qint64, QString> userLastInputUnsend; // 私聊未发送的内容
    QHash<qint64, QString> groupLastInputUnsend; // 群聊未发送的内容

    QHash<qint64, bool> gettingGroupMsgHistories; // 是否正在获取群消息历史记录

    QString friendName(qint64 userId) const
    {
        return friendList.contains(userId) ? friendList[userId].username() : "";
    }

    QString groupName(qint64 groupId) const
    {
        return groupList.value(groupId).name;
    }

    QString groupMemberName(qint64 groupId, qint64 userId)
    {
        if (!groupList.contains(groupId))
            return "";
        if (!groupList.value(groupId).members.contains(userId))
            return "";
        return groupList.value(groupId).members.value(userId).username();
    }

};

extern AccountInfo* ac;

#endif // ACCOUNTINFO_H
