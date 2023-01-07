#ifndef ACCOUNTINFO_H
#define ACCOUNTINFO_H

#include <QString>
#include <QHash>
#include <QColor>
#include "msgbean.h"

#define GROUP_MEMBER 0
#define GROUP_ADMIN 1
#define GROUP_OWNER 2

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

    QHash<qint64, QList<MsgBean>> userMsgHistory; // 私聊消息记录
    QHash<qint64, QList<MsgBean>> groupMsgHistory; // 群聊消息记录

    QHash<qint64, qint64> aiReplyPrivateTime; // 私聊最近一次自动回复
    QHash<qint64, qint64> aiReplyGroupTime; // 群组最近一次自动回复

    QHash<qint64, qint64> mySendPrivateTime; // 私聊对象自己发送的最后一次消息（动态重要性）
    QHash<qint64, qint64> mySendGroupTime; // 群组中自己发送的最后一次消息（动态重要性）
    QHash<qint64, int> receiveCountAfterMySendPrivate; // 动态重要性：私聊自己发送消息后收到多少条
    QHash<qint64, int> receivedCountAfterMySentGroup; // 动态重要性：群聊自己发送消息后收到多少条消息
    mutable QSet<qint64> askUser; // 智能聚焦：发送疑问消息的私聊
    mutable QSet<qint64> askGroup; // 智能聚焦：发送疑问消息的群聊

    bool lastReceiveShowIsUser = false; // 最后接受到的一条消息是用户true还是群组false
    qint64 lastReceiveShowId = 0; // 最后接收到的用户ID或者群组ID

    QHash<qint64, QString> userLastInputUnsend; // 私聊未发送的内容
    QHash<qint64, QString> groupLastInputUnsend; // 群聊未发送的内容

    QHash<qint64, bool> gettingGroupMembers; // 是否正在获取群成员
    QHash<qint64, bool> gettingGroupMsgHistories; // 是否正在获取群消息历史记录

    QHash<qint64, int> userUnreadCount;  // 私聊未读消息数量
    QHash<qint64, int> groupUnreadCount; // 群组未读消息数量
    qint64 lastUnreadId = 0; // 最后一个未读消息

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

    /// 获取管理员等级
    /// 成员0，管理员1，群主2
    int getGroupAdminLevel(qint64 groupId, qint64 userId)
    {
        if (!groupList.contains(groupId))
            return GROUP_MEMBER;
        if (!groupList.value(groupId).members.contains(userId))
            return GROUP_MEMBER;
        if (groupList.value(groupId).ownerId == userId)
            return GROUP_OWNER;
        if (groupList.value(groupId).adminIds.contains(userId))
            return GROUP_ADMIN;
        return GROUP_MEMBER;
    }

};

extern AccountInfo* ac;

#endif // ACCOUNTINFO_H
