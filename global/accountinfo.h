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
    };

    qint64 myId = 0; // 自己的QQ号
    QString myNickname; // 自己的昵称
    QHash<qint64, QString> friendNames; // 好友列表
    QHash<qint64, QString> groupNames; // 群列表
    QHash<qint64, QHash<qint64, QString>> groupMemberNames; // 群成员名字（不包含好友备注），@专用
    QHash<qint64, CardColor> userHeaderColor; // 用户头像，好友+群员+陌生人
    QHash<qint64, CardColor> groupHeaderColor; // 群头像
    QHash<qint64, QList<MsgBean>> userMsgHistory; // 私聊消息记录（不包括自己）
    QHash<qint64, QList<MsgBean>> groupMsgHistory; // 群聊消息记录（不包括自己）
    QHash<qint64, QHash<qint64, QColor>> groupMemberColor; // 群组里每位用户的颜色
    QHash<qint64, qint64> aiReplyUserTime; // 用户最近一次自动回复
    QHash<qint64, qint64> aiReplyGroupTime; // 群组最近一次自动回复
};

extern AccountInfo* ac;

#endif // ACCOUNTINFO_H
